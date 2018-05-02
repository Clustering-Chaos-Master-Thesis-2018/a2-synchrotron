
#include "contiki.h"
#include <string.h>
#include <stdio.h>

#include "chaos.h"
#include "chaos-random-generator.h"
#include "chaos-control.h"
#include "node.h"
#include "cluster.h"
#include "testbed.h"
#include "chaos-config.h"

#define ENABLE_COOJA_DEBUG COOJA
#include "dev/cooja-debug.h"

#include "sys/compower.h"

#include "lib.h"

static chaos_state_t process_cluster_head(uint16_t, uint16_t, chaos_state_t, int, size_t, cluster_t*, cluster_t*, uint8_t**);
static chaos_state_t process_cluster_node(uint16_t, uint16_t, chaos_state_t, int, size_t, cluster_t*, cluster_t*, uint8_t**);
static void round_begin(const uint16_t round_count, const uint8_t id);
static int is_pending(const uint16_t round_count);
static void round_begin_sniffer(chaos_header_t* header);
static void round_end_sniffer(const chaos_header_t* header);
static node_id_t pick_best_cluster(const cluster_head_information_t *cluster_head_list, uint8_t size);
static int index_of(const cluster_head_information_t *cluster_head_list, uint8_t size, node_id_t value);
static void log_cluster_heads(cluster_head_information_t *cluster_head_list, uint8_t cluster_head_count);
static uint8_t filter_valid_cluster_heads(const cluster_head_information_t* cluster_head_list, uint8_t cluster_head_count, cluster_head_information_t* const output, uint8_t threshold);
static float CH_probability(int8_t doubling_count);
static void heed_repeat(const cluster_head_information_t* cluster_head_list, uint8_t cluster_head_count, uint8_t consecutive_cluster_round_count);
static void update_hop_count(cluster_t* tx_payload);

static inline uint8_t set_best_available_hop_count(cluster_t* destination, const cluster_t* source);
static inline int merge_lists(cluster_t* cluster_tx, cluster_t* cluster_rx);

//The number of consecutive receive states we need to be in before forcing to send again.
//In order to combat early termination. This should probably be changed to something more robust.
#define CONSECUTIVE_RECEIVE_THRESHOLD 10
#define CLUSTER_SERVICE_PENDING_THRESHOLD 14

#define CLUSTER_SLOT_LEN          (7*(RTIMER_SECOND/1000)+0*(RTIMER_SECOND/1000)/2)
#define CLUSTER_SLOT_LEN_DCO      (CLUSTER_SLOT_LEN*CLOCK_PHI)

#define FLAGS_LEN(node_count)   ((node_count / 8) + ((node_count % 8) ? 1 : 0))
#define LAST_FLAGS(node_count)  ((1 << ((((node_count) - 1) % 8) + 1)) - 1)
#define FLAG_SUM(node_count)  ((((node_count) - 1) / 8 * 0xFF) + LAST_FLAGS(node_count))

#define CHAOS_RESTART_MAX 10
#define CHAOS_RESTART_MIN 4

CHAOS_SERVICE(cluster, CLUSTER_SLOT_LEN, CLUSTER_ROUND_MAX_SLOTS, 0, is_pending, round_begin, round_begin_sniffer, round_end_sniffer);

ALWAYS_INLINE static int get_flags_length(){
  return FLAGS_LEN(MAX_NODE_COUNT);
}

ALWAYS_INLINE uint32_t generate_restart_threshold() {
    return chaos_random_generator_fast_range(CHAOS_RESTART_MIN, CHAOS_RESTART_MAX);
}

cluster_t local_cluster_data = {
    .consecutive_cluster_round_count = -1
};

uint16_t neighbour_total_rx_count[MAX_NODE_COUNT] = {0};
uint16_t neighbour_list[MAX_NODE_COUNT] = {0};

unsigned long total_energy_used = 0;
static int8_t tentativeAnnouncementSlot = -1;

uint32_t restart_threshold = 0;
uint32_t invalid_rx_count = 0;
uint8_t got_valid_rx = 0;

uint8_t is_cluster_service_running = 0;

float base_CH_probability = -1.0f;
//Average energy used per round * some number of rounds
const uint64_t MAX_ENERGY = (uint64_t)30000 * (uint64_t)1000;
const float C_PROB = 0.005f;

CHState cluster_head_state = NOT_INITIALIZED;

ALWAYS_INLINE int8_t is_cluster_head(void) {
    return cluster_head_state == TENTATIVE || cluster_head_state == FINAL;
}

ALWAYS_INLINE int8_t cluster_head_not_initialized(void) {
    return cluster_head_state == NOT_INITIALIZED;
}

float calculate_initial_CH_prob(uint64_t total_energy_used) {
    uint64_t residualEnergy = MAX_ENERGY - total_energy_used;
    float probability = C_PROB * ((float)residualEnergy / (float)MAX_ENERGY);
    float min = 0.00001f;
    return probability > min ? probability : min;
}

ALWAYS_INLINE static void update_rx_statistics(node_id_t source_id, uint16_t total_rx) {
    if (source_id < MAX_NODE_COUNT) {
        neighbour_list[source_id]++;
        neighbour_total_rx_count[source_id] = total_rx;
    }
}

static void update_hop_count(cluster_t* tx_payload) {
    uint8_t i;
    for(i = 0; i < tx_payload->cluster_head_count; ++i) {
        tx_payload->cluster_head_list[i].hop_count++;
    }
}

ALWAYS_INLINE static void prepare_tx(cluster_t* const cluster_tx) {
    cluster_tx->source_id = node_id;
    update_hop_count(cluster_tx);
    cluster_tx->consecutive_cluster_round_count = local_cluster_data.consecutive_cluster_round_count;
    cluster_tx->total_rx_count = sum(neighbour_list, MAX_NODE_COUNT);
}

static chaos_state_t handle_invalid_rx(cluster_t* const cluster_tx) {
    invalid_rx_count++;
    if(invalid_rx_count > restart_threshold && got_valid_rx) {
        invalid_rx_count = 0;
        restart_threshold = generate_restart_threshold();
        prepare_tx(cluster_tx);
        return CHAOS_TX;
    }
    return CHAOS_RX;
}

uint8_t another_announced_CH_noticed(cluster_head_information_t* cluster_head_list, uint8_t cluster_head_count) {
    int i;
    for (i = 0; i < cluster_head_count; ++i) {
        if (cluster_head_list[i].status == TENTATIVE || cluster_head_list[i].status == FINAL) {
            return 1;
        }
    }
    return 0;
}

// Cannot be demoted to RX if TX has ever been decided
void set_next_state(chaos_state_t* next, chaos_state_t next_candidate) {
    if (*next == CHAOS_TX) {
        return;
    }
    *next = next_candidate;
}

static chaos_state_t process(uint16_t round_count, uint16_t slot,
    chaos_state_t current_state, int chaos_txrx_success, size_t payload_length,
    uint8_t* rx_payload, uint8_t* tx_payload, uint8_t** app_flags){
    chaos_state_t next_state = CHAOS_RX;

    cluster_t* const cluster_rx = (cluster_t*) rx_payload;
    cluster_t* const cluster_tx = (cluster_t*) tx_payload;

    if(current_state == CHAOS_INIT && IS_INITIATOR()) {
        got_valid_rx = 1;
        set_next_state(&next_state, CHAOS_TX);
    }


    if (tentativeAnnouncementSlot != -1) {
        uint8_t demote = another_announced_CH_noticed(cluster_rx->cluster_head_list, cluster_rx->cluster_head_count);
        if (demote) {
            tentativeAnnouncementSlot = -1; // Demote
        } else if (slot >= tentativeAnnouncementSlot) {
            tentativeAnnouncementSlot = -1;
            cluster_head_state = TENTATIVE;
            set_next_state(&next_state, CHAOS_TX);
        }
    }

    if(current_state == CHAOS_RX) {
        if(!chaos_txrx_success) {
            return handle_invalid_rx(cluster_tx);
        }
        got_valid_rx = 1;
        invalid_rx_count = 0;
        update_rx_statistics(cluster_rx->source_id, sum(neighbour_list, MAX_NODE_COUNT));

        if (local_cluster_data.consecutive_cluster_round_count == -1) {
            local_cluster_data.consecutive_cluster_round_count = cluster_rx->consecutive_cluster_round_count;
        }

        if(is_cluster_head()) {
            set_next_state(&next_state, process_cluster_head(round_count, slot, current_state, chaos_txrx_success, payload_length, cluster_rx, cluster_tx, app_flags));
        } else {
            set_next_state(&next_state, process_cluster_node(round_count, slot, current_state, chaos_txrx_success, payload_length, cluster_rx, cluster_tx, app_flags));
        }
    }

    set_best_available_hop_count(cluster_tx, &local_cluster_data);
    merge_lists(&local_cluster_data, cluster_tx);

    if(next_state == CHAOS_TX) {
        prepare_tx(cluster_tx);
    }
    return next_state;
}

static uint8_t insert(cluster_head_information_t* list, uint8_t size, cluster_head_information_t elem) {
    cluster_head_information_t new_list[NODE_LIST_LEN];
    memset(new_list, 0, sizeof(new_list));

    if (size == 0) {
        list[0] = elem;
        return 1;
    }

    uint8_t new_elem_added = 0;
    uint8_t i, j;
    for (i = 0, j = 0; i < size; ++i) {
        if (elem.id < list[i].id && !new_elem_added) {
            new_list[j++] = elem;
            new_elem_added = 1;
        }
        new_list[j++] = list[i];
    }

    if(!new_elem_added) {
        new_list[j++] = elem;
    }

    memcpy(list, new_list, sizeof(new_list));

    return j;
}

ALWAYS_INLINE static uint8_t update_cluster_head_status(cluster_head_information_t* const cluster_head_list, uint8_t size, node_id_t node_id) {
    const int index = index_of(cluster_head_list, size, node_id);
    if(index > 0 && index < size && cluster_head_list[index].status != cluster_head_state) {
        cluster_head_list[index].status = cluster_head_state;
        return 1;
    }
    return 0;
}

ALWAYS_INLINE static uint8_t cluster_head_exists(cluster_head_information_t* const cluster_head_list, uint8_t size, node_id_t cluster_id) {
    return index_of(cluster_head_list, size, cluster_id) != -1;
}

ALWAYS_INLINE static cluster_head_information_t create_cluster_head(node_id_t node_id, CHState cluster_head_state) {
    cluster_head_information_t cluster_head_information;
    cluster_head_information.id = node_id;
    cluster_head_information.hop_count = 0;
    cluster_head_information.status = cluster_head_state;

    return cluster_head_information;
}

static chaos_state_t process_cluster_head(uint16_t round_count, uint16_t slot,
    chaos_state_t current_state, int chaos_txrx_success, size_t payload_length,
    cluster_t* rx_payload, cluster_t* tx_payload, uint8_t** app_flags) {

    uint8_t delta = 0;
    chaos_state_t next_state = CHAOS_RX;

    delta |= merge_lists(tx_payload, rx_payload);

    if(!cluster_head_exists(tx_payload->cluster_head_list, tx_payload->cluster_head_count, node_id) && tx_payload->cluster_head_count < NODE_LIST_LEN) {
        tx_payload->cluster_head_count = insert(tx_payload->cluster_head_list, tx_payload->cluster_head_count, create_cluster_head(node_id, cluster_head_state));
        delta |= 1;
    }

    delta |= update_cluster_head_status(tx_payload->cluster_head_list, tx_payload->cluster_head_count, node_id);;

    if (delta && got_valid_rx) {
        next_state = CHAOS_TX;
    }

    return next_state;
}

static chaos_state_t process_cluster_node(uint16_t round_count, uint16_t slot,
    chaos_state_t current_state, int chaos_txrx_success, size_t payload_length,
    cluster_t* rx_payload, cluster_t* tx_payload, uint8_t** app_flags) {
    uint8_t delta = 0;
    chaos_state_t next_state = CHAOS_RX;

    delta |= merge_lists(tx_payload, rx_payload);
    if (delta && got_valid_rx) {
        next_state = CHAOS_TX;
    }
    return next_state;
}

static void round_begin(const uint16_t round_count, const uint8_t app_id) {
    restart_threshold = generate_restart_threshold();
    invalid_rx_count = 0;
    got_valid_rx = 0;
    is_cluster_service_running = 1;

    cluster_t initial_local_cluster_data;
    memset(&initial_local_cluster_data, 0, sizeof(cluster_t));

    if (IS_INITIATOR()) {
        local_cluster_data.consecutive_cluster_round_count++;
    } else {
        if (local_cluster_data.consecutive_cluster_round_count != -1) {
            local_cluster_data.consecutive_cluster_round_count++;
        }
    }

    if(base_CH_probability < 0) {
    #if CLUSTER_RANDOMIZE_STARTING_ENERGY
        uint8_t energy_coef = chaos_random_generator_fast_range(0, CLUSTER_RANDOMIZE_STARTING_ENERGY);
        total_energy_used = MAX_ENERGY * ((float)energy_coef / 100.0f);
    #endif /* CLUSTER_RANDOMIZE_STARTING_ENERGY */
        base_CH_probability = calculate_initial_CH_prob(total_energy_used);
    }

    chaos_round(round_count, app_id, (const uint8_t const*)&initial_local_cluster_data, sizeof(initial_local_cluster_data), CLUSTER_SLOT_LEN_DCO, CLUSTER_ROUND_MAX_SLOTS, get_flags_length(), process);
}

ALWAYS_INLINE static int is_pending(const uint16_t round_count) {
    return round_count <= CLUSTER_SERVICE_PENDING_THRESHOLD;
}

ALWAYS_INLINE static uint8_t calculate_smallest_hop_count(const cluster_head_information_t *cluster_head_list, uint8_t size) {
    uint8_t i;
    uint8_t smallest_hop_count = 255;
    for(i = 0; i < size; ++i) {
        if(cluster_head_list[i].hop_count < smallest_hop_count) {
            smallest_hop_count = cluster_head_list[i].hop_count;
        }
    }
    return smallest_hop_count;
}

static node_id_t pick_best_cluster(const cluster_head_information_t *cluster_head_list, uint8_t size) {
    cluster_head_information_t valid_cluster_heads[NODE_LIST_LEN];
    uint8_t smallest_hop_count = calculate_smallest_hop_count(cluster_head_list, size);
    const uint8_t valid_cluster_head_count = filter_valid_cluster_heads(cluster_head_list, size, valid_cluster_heads, smallest_hop_count);;

    uint8_t i;
    uint16_t biggest_rx_count = 0;
    node_id_t biggest_rx_id = 0;
    for(i = 0; i < valid_cluster_head_count; ++i) {
        const node_id_t node_id = valid_cluster_heads[i].id;
        const uint16_t curent_rx_count = neighbour_list[node_id];
        if(curent_rx_count > biggest_rx_count) {
            biggest_rx_count = curent_rx_count;
            biggest_rx_id = node_id;
        }
    }
    return biggest_rx_id;
}

ALWAYS_ACTUALLY_INLINE static int index_of(const cluster_head_information_t *array, uint8_t size, node_id_t value) {
    uint8_t i;
    for(i = 0; i < size; ++i) {
        if (array[i].id == value) {
            return i;
        }
    }
    return -1;
}

static void round_begin_sniffer(chaos_header_t* header){
    unsigned long all_cpu = energest_type_time(ENERGEST_TYPE_CPU);
    unsigned long all_lpm = energest_type_time(ENERGEST_TYPE_LPM);
    unsigned long all_transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT);
    unsigned long all_listen = energest_type_time(ENERGEST_TYPE_LISTEN);
    unsigned long all_idle_transmit = compower_idle_activity.transmit;
    unsigned long all_idle_listen = compower_idle_activity.listen;
    total_energy_used += all_cpu + all_lpm + all_transmit + all_listen + all_idle_transmit + all_idle_listen;
}

ALWAYS_ACTUALLY_INLINE static void log_cluster_heads(cluster_head_information_t *cluster_head_list, uint8_t cluster_head_count) {
    char str[600];
    cluster_head_information_t valid_cluster_heads[NODE_LIST_LEN];
    const uint8_t valid_cluster_head_count = filter_valid_cluster_heads(cluster_head_list, cluster_head_count, valid_cluster_heads, CLUSTER_COMPETITION_RADIUS);
    char res[20];
    ftoa(CH_probability(local_cluster_data.consecutive_cluster_round_count), res, 4);
    sprintf(str, "cluster: rd: %u, CH election probability: %s, cluster_head_count: %u, valid_cluster_head_count: %u, picked_cluster: %u, cluster_index: %u.\n available_clusters: [ ",
     chaos_get_round_number(),
     res,
     cluster_head_count,
     valid_cluster_head_count,
     cluster_id,
     cluster_index);

    uint8_t i;
    for(i = 0; i < cluster_head_count; i++) {
        char tmp[35];
        sprintf(tmp, (i == cluster_head_count-1 ? "%u -> (d: %u, rx_count: %u) ":"%u -> (d: %u, rx_count: %u), "),
            cluster_head_list[i].id,
            cluster_head_list[i].hop_count,
            neighbour_list[cluster_head_list[i].id]);

        strcat(str, tmp);
    }

    strcat(str, "]\n");
    PRINTF(str);
}

ALWAYS_ACTUALLY_INLINE static void log_rx_count() {
    char str[900];
    sprintf(str, "rx_count [ ");

    uint8_t i;
    uint8_t largest_id_in_network = 0;
    for(i = 0; i < MAX_NODE_COUNT; ++i) { 
        if(neighbour_list[i] > 0) {
            largest_id_in_network = i;
        }
    }

    const uint16_t rx_sum = sum(neighbour_total_rx_count, MAX_NODE_COUNT);
    const uint8_t number_of_nodes = count_filled_slots(neighbour_total_rx_count, MAX_NODE_COUNT);
    uint16_t rx_average = 0;
    if(number_of_nodes > 0) {
        rx_average = rx_sum / count_filled_slots(neighbour_total_rx_count, MAX_NODE_COUNT);
    }

    const uint16_t rx_min = min(neighbour_total_rx_count, MAX_NODE_COUNT);
    const uint16_t rx_max = max(neighbour_total_rx_count, MAX_NODE_COUNT);

    char tmp[35];
    for(i = 0; i < largest_id_in_network; i++) {
        sprintf(tmp, (i == largest_id_in_network-1 ? "%u ":"%u, "), neighbour_list[i]);
        strcat(str, tmp);
    }

    char end_line[80];
    sprintf(end_line, "total: %u, average: %u, min: %u, max: %u]\n", sum(neighbour_list, MAX_NODE_COUNT), rx_average, rx_min, rx_max);
    strcat(str, end_line);

    PRINTF(str);
}

static uint8_t filter_valid_cluster_heads(const cluster_head_information_t* cluster_head_list, uint8_t cluster_head_count, cluster_head_information_t* const output, uint8_t threshold) {
    uint8_t i, j = 0;
    for(i = 0; i < cluster_head_count; ++i) {
        if(cluster_head_list[i].hop_count <= threshold) {
            output[j++] = cluster_head_list[i];
        }
    }
    return j;
}

static float CH_probability(int8_t doubling_count) {
    if (doubling_count < 0) {
        doubling_count = 0;
    }
    float prob = 1.0f*(1 << doubling_count) * base_CH_probability;
    return prob < 1.0f ? prob : 1.0f;
}

static void set_global_cluster_variables(const cluster_head_information_t* cluster_head_list, uint8_t cluster_head_count) {
    cluster_head_information_t valid_cluster_heads[NODE_LIST_LEN];
    const uint8_t valid_cluster_head_count = filter_valid_cluster_heads(cluster_head_list, cluster_head_count, valid_cluster_heads, CLUSTER_COMPETITION_RADIUS);
    cluster_id = is_cluster_head() ? node_id : pick_best_cluster(valid_cluster_heads, valid_cluster_head_count);
    cluster_index = index_of(cluster_head_list, cluster_head_count, cluster_id);
}

static void heed_repeat(const cluster_head_information_t* cluster_head_list, uint8_t cluster_head_count, uint8_t consecutive_cluster_round_count) {
    float current_CH_prob = CH_probability(consecutive_cluster_round_count);

    char res[20];
    ftoa(current_CH_prob, res, 4);
    COOJA_DEBUG_PRINTF("cluster heed_repeat CH_prob: %s", res);

    cluster_head_information_t valid_cluster_heads[NODE_LIST_LEN];
    uint8_t valid_cluster_head_count = filter_valid_cluster_heads(cluster_head_list, cluster_head_count, valid_cluster_heads, CLUSTER_COMPETITION_RADIUS);

    float previous_CH_probability = CH_probability(consecutive_cluster_round_count - 1);
    if(previous_CH_probability >= 1.0f) {
        if(valid_cluster_head_count > 0) {
            set_global_cluster_variables(cluster_head_list, cluster_head_count);
        }
        return;
    }
    if(valid_cluster_head_count > 0) {
        set_global_cluster_variables(cluster_head_list, cluster_head_count);

        if(cluster_id == node_id) {
            if(current_CH_prob >= 1.0f) {
                cluster_head_state = FINAL;
                COOJA_DEBUG_PRINTF("cluster, I AM FINAL\n");
            } else {
                cluster_head_state = TENTATIVE;
            }
        }
    } else if(current_CH_prob >= 1.0f) {
        cluster_head_state = FINAL;
        COOJA_DEBUG_PRINTF("cluster, I AM FINAL\n");
    } else {
        uint32_t precision = 1000;
        uint32_t probability = current_CH_prob * precision;
        if(chaos_random_generator_fast_range(0, precision) <= probability) {
            tentativeAnnouncementSlot = chaos_random_generator_fast_range(0, CLUSTER_ROUND_MAX_SLOTS/2);
            COOJA_DEBUG_PRINTF("cluster, I AM TENTATIVE, decided to become CH in slot %d\n", tentativeAnnouncementSlot);
        }
    }
}

static void round_end_sniffer(const chaos_header_t* header){
    if (is_cluster_service_running) {
        is_cluster_service_running = is_pending(header->round_number + 1);
        heed_repeat(local_cluster_data.cluster_head_list, local_cluster_data.cluster_head_count, local_cluster_data.consecutive_cluster_round_count);

        if(cluster_head_state == FINAL) {
            chaos_cluster_node_count = local_cluster_data.cluster_head_count;
            chaos_cluster_node_index = cluster_index;
            COOJA_DEBUG_PRINTF("setting chaos cluster node index %d", chaos_cluster_node_index);
        } else {
            chaos_cluster_node_count = local_cluster_data.cluster_head_count;
        }
        init_node_index();
        log_cluster_heads(local_cluster_data.cluster_head_list, local_cluster_data.cluster_head_count);
        log_rx_count();
    } else {
        local_cluster_data.consecutive_cluster_round_count = -1;
    }
}

static inline int merge_lists(cluster_t* cluster_tx, cluster_t* cluster_rx) {
  uint8_t index_rx = 0;
  uint8_t index_tx = 0;
  uint8_t index_merge = 0;
  uint8_t delta = 0;

  cluster_head_information_t merge[NODE_LIST_LEN];
  memset(merge, 0, sizeof(merge));

  while ((index_tx < cluster_tx->cluster_head_count || index_rx < cluster_rx->cluster_head_count ) && index_merge < NODE_LIST_LEN) {
    if (index_tx >= cluster_tx->cluster_head_count || (index_rx < cluster_rx->cluster_head_count && cluster_rx->cluster_head_list[index_rx].id < cluster_tx->cluster_head_list[index_tx].id)) {
      merge[index_merge] = cluster_rx->cluster_head_list[index_rx];
      index_merge++;
      index_rx++;
      delta = 1;
    } else if (index_rx >= cluster_rx->cluster_head_count || (index_tx < cluster_tx->cluster_head_count && cluster_rx->cluster_head_list[index_rx].id > cluster_tx->cluster_head_list[index_tx].id)) {
      merge[index_merge] = cluster_tx->cluster_head_list[index_tx];
      index_merge++;
      index_tx++;
      delta = 1;
    } else { //(remote.cluster_head_list[remote_index] == local.cluster_head_list[local_index]){
      merge[index_merge] = cluster_rx->cluster_head_list[index_rx];
      index_merge++;
      index_rx++;
      index_tx++;
    }
  }

  memcpy(cluster_tx->cluster_head_list, merge, sizeof(merge));
  cluster_tx->cluster_head_count = index_merge;

  return delta;
}
static inline uint8_t set_best_available_hop_count(cluster_t* destination, const cluster_t* source) {
    uint8_t delta = 0;
    uint8_t i, j;
    for(i = 0; i < source->cluster_head_count; ++i) {
        for(j = 0; j < destination->cluster_head_count; ++j) {
            if(destination->cluster_head_list[j].id == source->cluster_head_list[i].id &&
                destination->cluster_head_list[j].hop_count > source->cluster_head_list[i].hop_count) {
                destination->cluster_head_list[j].hop_count = source->cluster_head_list[i].hop_count;
                delta = 1;
            }
        }
    }
    return delta;
}
