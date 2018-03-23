
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

typedef struct __attribute__((packed)) {
    node_id_t id;
    uint8_t hop_count;
} cluster_head_information_t;

typedef struct __attribute__((packed)) {
    uint8_t cluster_head_count;
    node_id_t source_id;
    cluster_head_information_t cluster_head_list[NODE_LIST_LEN];
} cluster_t;

cluster_t local_cluster_data;
uint16_t neighbour_list[MAX_NODE_COUNT];

unsigned long total_energy_used = 0;

static chaos_state_t process_cluster_head(uint16_t, uint16_t, chaos_state_t, int, size_t, cluster_t*, cluster_t*, uint8_t**);
static chaos_state_t process_cluster_node(uint16_t, uint16_t, chaos_state_t, int, size_t, cluster_t*, cluster_t*, uint8_t**);
static CHState determine_cluster_head_state(node_id_t);
static void round_begin(const uint16_t round_count, const uint8_t id);
static int is_pending(const uint16_t round_count);
static void round_begin_sniffer(chaos_header_t* header);
static void round_end_sniffer(const chaos_header_t* header);
static cluster_head_information_t pick_best_cluster(const cluster_head_information_t *cluster_head_list, uint8_t size);
static int index_of(const cluster_head_information_t *cluster_head_list, uint8_t size, node_id_t value);
static void log_cluster_heads(cluster_head_information_t *cluster_head_list, uint8_t cluster_head_count);
static uint8_t filter_valid_cluster_heads(const cluster_head_information_t* cluster_head_list, uint8_t cluster_head_count, cluster_head_information_t* const output, uint8_t threshold);

static inline int32_t set_best_available_hop_count(cluster_t* destination, const cluster_t* source);
static inline int merge_lists(cluster_t* cluster_tx, cluster_t* cluster_rx);

//The number of consecutive receive states we need to be in before forcing to send again.
//In order to combat early termination. This should probably be changed to something more robust.
#define CONSECUTIVE_RECEIVE_THRESHOLD 10
#define CLUSTER_SERVICE_PENDING_THRESHOLD 20

//What is this
#define FLAGS_LEN(node_count)   ((node_count / 8) + ((node_count % 8) ? 1 : 0))
#define LAST_FLAGS(node_count)  ((1 << ((((node_count) - 1) % 8) + 1)) - 1)
#define FLAG_SUM(node_count)  ((((node_count) - 1) / 8 * 0xFF) + LAST_FLAGS(node_count))

#define CHAOS_RESTART_MAX 10
#define CHAOS_RESTART_MIN 4

//The percentage chance for a node to become a cluster head.
#define CLUSTER_HEAD_ELECTION_CHANCE 10

CHAOS_SERVICE(cluster, (7*(RTIMER_SECOND/1000)+0*(RTIMER_SECOND/1000)/2), 260 , 0, is_pending, round_begin, round_begin_sniffer, round_end_sniffer);

ALWAYS_INLINE static int get_flags_length(){
  return FLAGS_LEN(MAX_NODE_COUNT);
}

ALWAYS_INLINE static uint32_t generate_restart_threshold() {
    return chaos_random_generator_fast() % (CHAOS_RESTART_MAX - CHAOS_RESTART_MIN) + CHAOS_RESTART_MIN;
}

uint32_t restart_threshold = 0;
uint32_t invalid_rx_count = 0;

uint8_t is_cluster_service_running = 0;

//Hop count.
const uint8_t CLUSTER_HEAD_COMPETITION_RADIUS = 1;

float CH_probablity = -1.0f;
float previous_CH_probability = -1.0f;
//Average energy used per round * some number of rounds
uint64_t MAX_ENERGY = (uint64_t)30000 * (uint64_t)1000;
float C_PROB = 0.05f;

CHState cluster_head_state = NOT_INITIALIZED;

ALWAYS_INLINE int8_t is_cluster_head(void) {
    return cluster_head_state == TENTATIVE || cluster_head_state == FINAL;
}

ALWAYS_INLINE int8_t cluster_head_not_initialized(void) {
    return cluster_head_state == NOT_INITIALIZED;
}

float calculate_initial_CH_prob(uint64_t total_energy_used) {
    uint64_t residualEnergy = MAX_ENERGY - total_energy_used;
    float probability = C_PROB * (float)(residualEnergy / MAX_ENERGY);
    float min = 0.0001f;
    return probability > min ? probability : min;
}

static chaos_state_t process(uint16_t round_count, uint16_t slot,
    chaos_state_t current_state, int chaos_txrx_success, size_t payload_length,
    uint8_t* rx_payload, uint8_t* tx_payload, uint8_t** app_flags){

    cluster_t* const cluster_rx = (cluster_t*) rx_payload;
    cluster_t* const cluster_tx = (cluster_t*) tx_payload;
    cluster_tx->source_id = node_id;

    if (current_state == CHAOS_INIT && IS_INITIATOR()) {
        return CHAOS_TX;
    }

    if(current_state == CHAOS_RX) {
        if(chaos_txrx_success) {
            invalid_rx_count = 0;
            neighbour_list[cluster_rx->source_id]++;
        } else {
            invalid_rx_count++;
            if(invalid_rx_count > restart_threshold) {
                invalid_rx_count = 0;
                restart_threshold = generate_restart_threshold();
                return CHAOS_RX;
            }
        }
    }

    if(is_cluster_head()) {
        return process_cluster_head(round_count, slot, current_state, chaos_txrx_success, payload_length, cluster_rx, cluster_tx, app_flags);
    } else {
        return process_cluster_node(round_count, slot, current_state, chaos_txrx_success, payload_length, cluster_rx, cluster_tx, app_flags);
    }
}

static uint8_t consecutive_rx = 0;

static void update_hop_count(cluster_t* tx_payload) {
    int i;
    for(i = 0; i < tx_payload->cluster_head_count; ++i) {
        tx_payload->cluster_head_list[i].hop_count++;
    }
}

static chaos_state_t process_cluster_head(uint16_t round_count, uint16_t slot,
    chaos_state_t current_state, int chaos_txrx_success, size_t payload_length,
    cluster_t* rx_payload, cluster_t* tx_payload, uint8_t** app_flags) {

    uint8_t delta = 0;
    chaos_state_t next_state = CHAOS_RX;

    if(current_state == CHAOS_RX) {
        delta |= merge_lists(tx_payload, rx_payload);
        delta |= set_best_available_hop_count(tx_payload, &local_cluster_data);
        consecutive_rx++;

        const int node_in_list = index_of(tx_payload->cluster_head_list, tx_payload->cluster_head_count, node_id);
        if(node_in_list == -1) {
            if(tx_payload->cluster_head_count < NODE_LIST_LEN) {
                cluster_head_information_t info;
                info.id = node_id;
                info.hop_count = 0;
                tx_payload->cluster_head_list[tx_payload->cluster_head_count++] = info;
            }
        }
        merge_lists(&local_cluster_data, tx_payload);
        if (delta || consecutive_rx == CONSECUTIVE_RECEIVE_THRESHOLD || node_in_list == -1) {
            next_state = CHAOS_TX;
            update_hop_count(tx_payload);
        }
    } else { /* CHAOS_TX */
        consecutive_rx = 0;
    }

    return next_state;
}

static chaos_state_t process_cluster_node(uint16_t round_count, uint16_t slot,
    chaos_state_t current_state, int chaos_txrx_success, size_t payload_length,
    cluster_t* rx_payload, cluster_t* tx_payload, uint8_t** app_flags) {
    uint8_t delta = 0;
    chaos_state_t next_state = CHAOS_RX;

    if (current_state == CHAOS_RX) {
        delta |= merge_lists(tx_payload, rx_payload);
        delta |= set_best_available_hop_count(tx_payload, &local_cluster_data);
        delta |= chaos_random_generator_fast() % 100 < 30;
        merge_lists(&local_cluster_data, tx_payload);
        if (delta) {
            next_state = CHAOS_TX;
            update_hop_count(tx_payload);
        }
    }
    return next_state;
}

static CHState determine_cluster_head_state(node_id_t node_id) {
    return (chaos_random_generator_fast() % 100) < CLUSTER_HEAD_ELECTION_CHANCE || node_id == 1 ?
            FINAL : NOT_CH;
}

static void round_begin(const uint16_t round_count, const uint8_t app_id) {
    is_cluster_service_running = 1;
    cluster_t initial_local_cluster_data;
    memset(&initial_local_cluster_data, 0, sizeof(cluster_t));
    // memset(&local_cluster_data, 0, sizeof(cluster_t));

    // if(cluster_head_not_initialized()) {
        // cluster_head_state = determine_cluster_head_state(node_id);
    // }

    // if(is_cluster_head()) {
    //     initial_local_cluster_data.cluster_head_list[0].id= node_id;
    //     initial_local_cluster_data.cluster_head_list[0].hop_count = 1;
    //     initial_local_cluster_data.cluster_head_count++;
    // }

    invalid_rx_count = 0;
    restart_threshold = generate_restart_threshold();
    if(CH_probablity < 0){
      CH_probablity = calculate_initial_CH_prob(total_energy_used);
    }

    chaos_round(round_count, app_id, (const uint8_t const*)&initial_local_cluster_data, sizeof(initial_local_cluster_data), (7*(RTIMER_SECOND/1000)+0*(RTIMER_SECOND/1000)/2) * CLOCK_PHI, JOIN_ROUND_MAX_SLOTS, get_flags_length(), process);
}

ALWAYS_INLINE static int is_pending(const uint16_t round_count) {
    return round_count <= CLUSTER_SERVICE_PENDING_THRESHOLD;
}

ALWAYS_INLINE static uint8_t calculate_smalles_hop_count(const cluster_head_information_t *cluster_head_list, uint8_t size) {
    uint8_t i;
    uint8_t smallest_hop_count = 255;
    for(i = 0; i < size; ++i) {
        if(cluster_head_list[i].hop_count < smallest_hop_count) {
            smallest_hop_count = cluster_head_list[i].hop_count;
        }
    }
    return smallest_hop_count;
}

static cluster_head_information_t pick_best_cluster(const cluster_head_information_t *cluster_head_list, uint8_t size) {
    cluster_head_information_t valid_cluster_heads[NODE_LIST_LEN];
    const uint8_t smallest_hop_count = calculate_smalles_hop_count(cluster_head_list, size);
    const uint8_t valid_cluster_head_count = filter_valid_cluster_heads(cluster_head_list, size, valid_cluster_heads, smallest_hop_count);;

    cluster_head_information_t best_cluster_head = valid_cluster_heads[chaos_random_generator_fast() % valid_cluster_head_count];
    COOJA_DEBUG_PRINTF("cluster, valid cluster count %u, chosen id: %d \n", valid_cluster_head_count, best_cluster_head.id);
    return best_cluster_head;
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
    COOJA_DEBUG_PRINTF("cluster total energy used: %lu", total_energy_used);
}

ALWAYS_ACTUALLY_INLINE static void log_cluster_heads(cluster_head_information_t *cluster_head_list, uint8_t cluster_head_count) {
    char str[200];
    sprintf(str, "cluster: rd: %u, cluster_head_count: %u, picked_cluster: %u available_clusters: [ ", chaos_get_round_number(), cluster_head_count, cluster_id);

    uint8_t i;
    for(i = 0; i < cluster_head_count; i++) {
        char tmp[20];
        sprintf(tmp, (i == cluster_head_count-1 ? "%u -> %u ":"%u -> %u, "), cluster_head_list[i].id, cluster_head_list[i].hop_count);
        strcat(str, tmp);
    }

    strcat(str, "]\n");
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

static void heed_repeat(const cluster_head_information_t* cluster_head_list, uint8_t cluster_head_count) {
    if(previous_CH_probability >= 1.0f) {
        return;
    }
    cluster_head_information_t valid_cluster_heads[NODE_LIST_LEN];
    uint8_t valid_cluster_head_count = filter_valid_cluster_heads(cluster_head_list, cluster_head_count, valid_cluster_heads, CLUSTER_HEAD_COMPETITION_RADIUS);
    if(valid_cluster_head_count > 0) {
        cluster_id = pick_best_cluster(valid_cluster_heads, valid_cluster_head_count).id;
        cluster_index = index_of(cluster_head_list, cluster_head_count, node_id);

        if(cluster_id == node_id) {
            if(CH_probablity >= 1.0f) {
                cluster_head_state = FINAL;
                COOJA_DEBUG_PRINTF("cluster, I AM FINAL\n");
            } else {
                cluster_head_state = TENTATIVE;
            }
        }
    } else if(CH_probablity >= 1.0f) {
        cluster_head_state = FINAL;
        COOJA_DEBUG_PRINTF("cluster, I AM FINAL\n");
    } else {
        uint32_t precision = 1000;
        uint32_t probability = CH_probablity * precision;
        if(chaos_random_generator_fast() % precision <= probability) {
            cluster_head_state = TENTATIVE;
            if(index_of(local_cluster_data.cluster_head_list, local_cluster_data.cluster_head_count, node_id) == -1) {
                cluster_head_information_t cluster_info;
                cluster_info.id = node_id;
                cluster_info.hop_count = 0;
                local_cluster_data.cluster_head_list[local_cluster_data.cluster_head_count++] = cluster_info;
            }
            COOJA_DEBUG_PRINTF("cluster, I AM TENTATIVE\n");
        }
    }
    previous_CH_probability = CH_probablity;
    CH_probablity = 2.0f * CH_probablity;
    if(CH_probablity >= 1.0f) {
        CH_probablity = 1.0f;
    }
    char res[20];
    ftoa(CH_probablity, res, 4);
    COOJA_DEBUG_PRINTF("cluster, CH probability: %s\n", res);
}

static void round_end_sniffer(const chaos_header_t* header){
    if (is_cluster_service_running) {
        is_cluster_service_running = 0;

        heed_repeat(local_cluster_data.cluster_head_list, local_cluster_data.cluster_head_count);

        // if(is_cluster_head()) {
        //     if(chaos_cluster_node_count < local_cluster_data.cluster_head_count) {
        //         chaos_cluster_node_count = local_cluster_data.cluster_head_count;
        //         chaos_cluster_node_index = node_id - 1;
        //     }
        //     cluster_id = node_id;
        //     cluster_index = index_of(local_cluster_data.cluster_head_list, local_cluster_data.cluster_head_count, node_id);
        //     init_node_index();
        // } else {
        // }
        log_cluster_heads(local_cluster_data.cluster_head_list, local_cluster_data.cluster_head_count);
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
      delta = 1; //arrays differs, so TX
    } else if (index_rx >= cluster_rx->cluster_head_count || (index_tx < cluster_tx->cluster_head_count && cluster_rx->cluster_head_list[index_rx].id > cluster_tx->cluster_head_list[index_tx].id)) {
      merge[index_merge] = cluster_tx->cluster_head_list[index_tx];
      index_merge++;
      index_tx++;
      delta = 1; //arrays differs, so TX
    } else { //(remote.cluster_head_list[remote_index] == local.cluster_head_list[local_index]){
      merge[index_merge] = cluster_rx->cluster_head_list[index_rx];
      index_merge++;
      index_rx++;
      index_tx++;
    }
  }

  /* New overflow? */
  if (index_merge >= NODE_LIST_LEN && (index_rx < cluster_rx->cluster_head_count || index_tx < cluster_tx->cluster_head_count)) {
    // cluster_tx->overflow = 1;
    delta = 1; //arrays differs, so TX
  }
  cluster_tx->cluster_head_count = index_merge;

  memcpy(cluster_tx->cluster_head_list, merge, sizeof(merge));
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
