
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

typedef struct __attribute__((packed)) {
    node_id_t id;
    uint8_t hop_count;
} cluster_head_information_t;

typedef struct __attribute__((packed)) {
    uint8_t cluster_head_count;
    cluster_head_information_t cluster_head_list[NODE_LIST_LEN];
} cluster_t;

cluster_t cluster_data;
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

static inline int merge_lists(cluster_t* cluster_tx, cluster_t* cluster_rx);

//The number of consecutive receive states we need to be in before forcing to send again.
//In order to combat early termination. This should probably be changed to something more robust.
#define CONSECUTIVE_RECEIVE_THRESHOLD 10

#define NUMBER_OF_CLUSTER_ROUNDS 2

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

CHState cluster_head_state = NOT_INITIALIZED;

ALWAYS_INLINE int8_t is_cluster_head(void) {
    return cluster_head_state == TENTATIVE || cluster_head_state == FINAL;
}

ALWAYS_INLINE int8_t cluster_head_not_initialized(void) {
    return cluster_head_state == NOT_INITIALIZED;
}

static chaos_state_t process(uint16_t round_count, uint16_t slot,
    chaos_state_t current_state, int chaos_txrx_success, size_t payload_length,
    uint8_t* rx_payload, uint8_t* tx_payload, uint8_t** app_flags){
    
    if (current_state == CHAOS_INIT && IS_INITIATOR()) {
        return CHAOS_TX;
    } 

    if(current_state == CHAOS_RX) {
        if(chaos_txrx_success) {
            invalid_rx_count = 0;
        } else {
            invalid_rx_count++;
            if(invalid_rx_count > restart_threshold) {
                invalid_rx_count = 0;
                restart_threshold = generate_restart_threshold();
                return CHAOS_RX;
            }
        }
    }

    cluster_t* const cluster_tx = (cluster_t*) tx_payload;
    cluster_t* const cluster_rx = (cluster_t*) rx_payload;

    if(is_cluster_head()) {
        return process_cluster_head(round_count, slot, current_state, chaos_txrx_success, payload_length, cluster_rx, cluster_tx, app_flags);
    } else {
        return process_cluster_node(round_count, slot, current_state, chaos_txrx_success, payload_length, cluster_rx, cluster_tx, app_flags);
    }
}

static uint8_t consecutive_rx = 0;

static chaos_state_t process_cluster_head(uint16_t round_count, uint16_t slot,
    chaos_state_t current_state, int chaos_txrx_success, size_t payload_length,
    cluster_t* rx_payload, cluster_t* tx_payload, uint8_t** app_flags) {
        
    uint8_t delta = 0;
    chaos_state_t next_state = CHAOS_RX;

    if(current_state == CHAOS_RX) {
        delta = merge_lists(tx_payload, rx_payload);
        consecutive_rx++;
        if (delta || consecutive_rx == CONSECUTIVE_RECEIVE_THRESHOLD) {
            next_state = CHAOS_TX;
        }
        
        const int node_in_list = index_of(tx_payload->cluster_head_list, tx_payload->cluster_head_count, node_id);
        if(node_in_list == -1) {
            if(tx_payload->cluster_head_count < NODE_LIST_LEN) {
                cluster_head_information_t info;
                info.id = node_id;
                info.hop_count = 1;
                tx_payload->cluster_head_list[tx_payload->cluster_head_count++] = info;
                next_state = CHAOS_TX;
            }
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
        delta = merge_lists(tx_payload, rx_payload);
        memcpy(&cluster_data, tx_payload, sizeof(cluster_t));
        if (delta) {
            next_state = CHAOS_TX;
            int i;
            for(i = 0; i < tx_payload->cluster_head_count; ++i) {
                tx_payload->cluster_head_list[i].hop_count++;
            }
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
    cluster_t initial_cluster_data;
    memset(&initial_cluster_data, 0, sizeof(cluster_t));
    memset(&cluster_data, 0, sizeof(cluster_t));

    if(cluster_head_not_initialized()) {
        cluster_head_state = determine_cluster_head_state(node_id);
    }

    if(is_cluster_head()) {
        initial_cluster_data.cluster_head_list[0].id= node_id;
        initial_cluster_data.cluster_head_list[0].hop_count = 1;
        initial_cluster_data.cluster_head_count++;
    }

    invalid_rx_count = 0;
    restart_threshold = generate_restart_threshold();

    chaos_round(round_count, app_id, (const uint8_t const*)&initial_cluster_data, sizeof(initial_cluster_data), (7*(RTIMER_SECOND/1000)+0*(RTIMER_SECOND/1000)/2) * CLOCK_PHI, JOIN_ROUND_MAX_SLOTS, get_flags_length(), process);
}

ALWAYS_INLINE static int is_pending(const uint16_t round_count) {
    return round_count <= NUMBER_OF_CLUSTER_ROUNDS;
}

static cluster_head_information_t pick_best_cluster(const cluster_head_information_t *cluster_head_list, uint8_t size) {
    uint8_t smallest_hop_count = 255;
    uint8_t valid_cluster_head_count = 0;
    cluster_head_information_t valid_cluster_heads[NODE_LIST_LEN];
    int i;
    for(i = 0; i < size; ++i) {
        if(cluster_head_list[i].hop_count < smallest_hop_count) {
            smallest_hop_count = cluster_head_list[i].hop_count;
            // COOJA_DEBUG_PRINTF("cluster new smallest hop_count: %u", )
        }
    }
    for(i = 0; i < size; ++i) {
        if(cluster_head_list[i].hop_count == smallest_hop_count) {
            valid_cluster_heads[valid_cluster_head_count++] = cluster_head_list[i];
        }
    }
    return valid_cluster_heads[chaos_random_generator_fast() % valid_cluster_head_count];
}

ALWAYS_ACTUALLY_INLINE static int index_of(const cluster_head_information_t *array, uint8_t size, node_id_t value) {
    int i;
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
    char str[200];
    sprintf(str, "cluster: rd: %u, cluster_head_count: %u, picked_cluster: %u available_clusters: [ ", chaos_get_round_number(), cluster_head_count, cluster_id);

    int i;
    for(i = 0; i < cluster_head_count; i++) {
        char tmp[20];
        sprintf(tmp, (i == cluster_head_count-1 ? "%u -> %u ":"%u -> %u, "), cluster_head_list[i].id, cluster_head_list[i].hop_count);
        strcat(str, tmp);
    }

    strcat(str, "]\n");
    PRINTF(str);
}

static void round_end_sniffer(const chaos_header_t* header){
    if (is_cluster_service_running) {
        is_cluster_service_running = 0;

        if(is_cluster_head()) {
            if(chaos_cluster_node_count < cluster_data.cluster_head_count) {
                chaos_cluster_node_count = cluster_data.cluster_head_count;
                chaos_cluster_node_index = node_id - 1;
            }
            cluster_id = node_id;
            cluster_index = index_of(cluster_data.cluster_head_list, cluster_data.cluster_head_count, node_id);
            init_node_index();
        } else {
            cluster_id = pick_best_cluster(cluster_data.cluster_head_list, cluster_data.cluster_head_count).id;
            cluster_index = index_of(cluster_data.cluster_head_list, cluster_data.cluster_head_count, node_id);
        }
        log_cluster_heads(cluster_data.cluster_head_list, cluster_data.cluster_head_count);
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
    int i, j;
    for(i = 0; i < cluster_data.cluster_head_count; ++i) {
        for(j = 0; j < index_merge; ++j) {
            if(merge[j].id == cluster_data.cluster_head_list[i].id &&
               merge[j].hop_count > cluster_data.cluster_head_list[i].hop_count) {
                // COOJA_DEBUG_PRINTF("cluster updated existing hop_count with better local one, cluster_id: %d recv hop_count :%d, our hop_count %d\n",
                    // merge[j].id, merge[j].hop_count, cluster_data.cluster_head_list[i].hop_count);
                merge[j].hop_count = cluster_data.cluster_head_list[i].hop_count;
                delta = 1;
            }
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
