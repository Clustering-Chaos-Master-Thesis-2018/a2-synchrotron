
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
ALWAYS_ACTUALLY_INLINE static int16_t index_of(const cluster_head_information_t *array, uint8_t size, node_id_t value);
static void log_cluster_heads(cluster_head_information_t *cluster_head_list, uint8_t cluster_head_count);
static float CH_probability(int8_t doubling_count);
static void heed_repeat(const cluster_head_information_t* cluster_head_list, uint8_t cluster_head_count, int8_t consecutive_cluster_round_count);
static void update_hop_count(cluster_t* tx_payload);

static inline uint8_t set_best_available_hop_count(cluster_t* destination, const cluster_t* source);
static inline int merge_lists(cluster_t* cluster_tx, cluster_t* cluster_rx);

//The number of consecutive receive states we need to be in before forcing to send again.
//In order to combat early termination. This should probably be changed to something more robust.
#define CONSECUTIVE_RECEIVE_THRESHOLD 10

#define SHOULD_RUN_CLUSTERING_SERVICE(ROUND) (ROUND <= 14 || (ROUND >= 200 && ROUND <= 214) || (ROUND >= 400 && ROUND <= 414))

#define CLUSTER_SLOT_LEN          (7*(RTIMER_SECOND/1000)+0*(RTIMER_SECOND/1000)/2)
#define CLUSTER_SLOT_LEN_DCO      (CLUSTER_SLOT_LEN*CLOCK_PHI)

#define FLAGS_LEN(node_count)   ((node_count / 8) + ((node_count % 8) ? 1 : 0))
#define LAST_FLAGS(node_count)  ((1 << ((((node_count) - 1) % 8) + 1)) - 1)
#define FLAG_SUM(node_count)  ((((node_count) - 1) / 8 * 0xFF) + LAST_FLAGS(node_count))

#define CLUSTER_RESTART_MAX 7
#define CLUSTER_RESTART_MIN 3

CHAOS_SERVICE(cluster, CLUSTER_SLOT_LEN, CLUSTER_ROUND_MAX_SLOTS, 0, is_pending, round_begin, round_begin_sniffer, round_end_sniffer);

ALWAYS_INLINE static int get_flags_length(){
  return FLAGS_LEN(MAX_NODE_COUNT);
}

ALWAYS_INLINE uint32_t generate_restart_threshold() {
    return chaos_random_generator_fast_range(CLUSTER_RESTART_MIN, CLUSTER_RESTART_MAX);
}

cluster_t local_cluster_data = {
    .consecutive_cluster_round_count = -1
};

uint16_t neighbour_list[MAX_NODE_COUNT] = {0};

unsigned long total_energy_used = 0;
static int8_t tentativeAnnouncementSlot = -1;

static uint32_t restart_threshold = 0;
static uint32_t invalid_rx_count = 0;
static uint8_t got_valid_rx = 0;

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

uint8_t allowed_to_announce_myself(const cluster_head_information_t* const cluster_head_list, uint8_t cluster_head_count) {
    const uint8_t valid_cluster_head_count = count_valid_cluster_heads(cluster_head_list, cluster_head_count, CLUSTER_COMPETITION_RADIUS);
    const uint8_t neighbour_count = count_filled_slots(neighbour_list, NODE_LIST_LEN);
    return (valid_cluster_head_count == 0 || neighbour_count / valid_cluster_head_count > CLUSTER_NODES_PER_CLUSTER);
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
        if (!allowed_to_announce_myself(local_cluster_data.cluster_head_list, local_cluster_data.cluster_head_count)) {
            tentativeAnnouncementSlot = -1; // Demote
        } else if (slot >= tentativeAnnouncementSlot) {
            tentativeAnnouncementSlot = -1;
            cluster_head_state = TENTATIVE;
            COOJA_DEBUG_PRINTF("cluster announced myself as tentative in slot: %u", slot);
            set_next_state(&next_state, CHAOS_TX);
        }
    }

    if(current_state == CHAOS_RX) {
        if(!chaos_txrx_success) {
            return handle_invalid_rx(cluster_tx);
        }
        got_valid_rx = 1;
        invalid_rx_count = 0;
        update_rx_statistics(cluster_rx->source_id, sum(neighbour_list));

        if (local_cluster_data.consecutive_cluster_round_count == -1 || local_cluster_data.consecutive_cluster_round_count < cluster_rx->consecutive_cluster_round_count) {
            local_cluster_data.consecutive_cluster_round_count = cluster_rx->consecutive_cluster_round_count;
            set_next_state(&next_state, CHAOS_TX);
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
        if(!got_valid_rx) {
            next_state = CHAOS_RX;
        } else {
            prepare_tx(cluster_tx);
        }
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

ALWAYS_ACTUALLY_INLINE static int16_t index_of(const cluster_head_information_t *array, uint8_t size, node_id_t value) {
    uint8_t i;
    for(i = 0; i < size; ++i) {
        if (array[i].id == value) {
            return i;
        }
    }
    return -1;
}

ALWAYS_INLINE static uint8_t update_cluster_head_status(cluster_head_information_t* const cluster_head_list, uint8_t size, node_id_t node_id) {
    const int16_t index = index_of(cluster_head_list, size, node_id);
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

    if (delta) {
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
    if (delta) {
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
    return SHOULD_RUN_CLUSTERING_SERVICE(round_count);
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

static void log_cluster_heads(cluster_head_information_t *cluster_head_list, uint8_t cluster_head_count) {
    PRINTF("available_clusters: (hop_count, rx_count) [ ");

    uint8_t i;
    for(i = 0; i < cluster_head_count; i++) {
        PRINTF(i == cluster_head_count-1 ? "%u (%u, %u) ":"%u (%u, %u), ",
            cluster_head_list[i].id,
            cluster_head_list[i].hop_count,
            neighbour_list[cluster_head_list[i].id]);
    }
    PRINTF("]\n");

    char ch_prob_str[20];
    ftoa(CH_probability(local_cluster_data.consecutive_cluster_round_count), ch_prob_str, 4);

    cluster_head_information_t valid_cluster_heads[NODE_LIST_LEN];
    const uint8_t valid_cluster_head_count = filter_valid_cluster_heads(cluster_head_list, cluster_head_count, valid_cluster_heads, CLUSTER_COMPETITION_RADIUS);

    PRINTF("cluster: rd: %u, prob: %s, CH_count: %u, valid_CH_count: %u, picked: %u, cluster_index: %u\n",
        chaos_get_round_number(),
        ch_prob_str,
        cluster_head_count,
        valid_cluster_head_count,
        cluster_id,
        cluster_index);
}

static void log_rx_count() {
    PRINTF("rx_count [");

    const uint16_t rx_sum = sum(neighbour_list);
    const uint16_t rx_min = min(neighbour_list, MAX_NODE_COUNT);
    const uint16_t rx_max = max(neighbour_list, MAX_NODE_COUNT);

    char rx_average_string[20];
    ftoa(mean(neighbour_list, MAX_NODE_COUNT), rx_average_string, 4);
    char rx_sd_string[20];
    ftoa(standard_deviation(neighbour_list, MAX_NODE_COUNT), rx_sd_string, 4);

    const uint8_t largest_id_in_network = last_filled_index(neighbour_list, MAX_NODE_COUNT);
    uint8_t i;
    for(i = 0; i <= largest_id_in_network; i++) {
        PRINTF(i == largest_id_in_network ? "%u ":"%u, ", neighbour_list[i]);
    }

    PRINTF("]\ntotal: %u, mean: %s, min: %u, max: %u, sd: %s]\n", rx_sum, rx_average_string, rx_min, rx_max, rx_sd_string);
}

static float CH_probability(int8_t doubling_count) {
    if (doubling_count < 0) {
        doubling_count = 0;
    }
    float prob = 1.0f*(1 << doubling_count) * base_CH_probability;
    return prob < 1.0f ? prob : 1.0f;
}

void set_global_cluster_variables(const cluster_head_information_t* cluster_head_list, uint8_t cluster_head_count) {
    cluster_head_information_t valid_cluster_heads[NODE_LIST_LEN];
    const uint8_t valid_cluster_head_count = filter_valid_cluster_heads(cluster_head_list, cluster_head_count, valid_cluster_heads, CLUSTER_COMPETITION_RADIUS);
    cluster_id = is_cluster_head() ? node_id : pick_best_cluster(valid_cluster_heads, valid_cluster_head_count);
    cluster_index = index_of(cluster_head_list, cluster_head_count, cluster_id);
}

static void heed_repeat(const cluster_head_information_t* cluster_head_list, uint8_t cluster_head_count, int8_t consecutive_cluster_round_count) {
    float current_CH_prob = CH_probability(consecutive_cluster_round_count);

    char res[20];
    ftoa(current_CH_prob, res, 4);
    COOJA_DEBUG_PRINTF("cluster heed_repeat CH_prob: %s\n", res);


    float previous_CH_probability = CH_probability(consecutive_cluster_round_count - 1);
    if(previous_CH_probability >= 1.0f) {
        if(count_valid_cluster_heads(cluster_head_list, cluster_head_count, CLUSTER_COMPETITION_RADIUS) > 0) {
            set_global_cluster_variables(cluster_head_list, cluster_head_count);
        }
        return;
    }

    if(!allowed_to_announce_myself(cluster_head_list, cluster_head_count) || is_cluster_head()) {
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
            join_init(1);
        } else {
            chaos_cluster_node_count = local_cluster_data.cluster_head_count;
            join_init(0);
        }
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
