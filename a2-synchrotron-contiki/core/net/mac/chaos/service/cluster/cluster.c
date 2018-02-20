
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

typedef struct __attribute__((packed)) {
    uint8_t cluster_head_count;

    node_id_t cluster_head_list[NODE_LIST_LEN];
} cluster_t;

static chaos_state_t process_cluster_head(uint16_t, uint16_t, chaos_state_t, int, size_t, cluster_t*, cluster_t*, uint8_t**);
static chaos_state_t process_cluster_node(uint16_t, uint16_t, chaos_state_t, int, size_t, cluster_t*, cluster_t*, uint8_t**);
static void round_begin(const uint16_t round_count, const uint8_t id);
static int is_pending(const uint16_t round_count);
static void round_begin_sniffer(chaos_header_t* header);
static void round_end_sniffer(const chaos_header_t* header);
static node_id_t pick_best_cluster(const node_id_t *cluster_head_list, uint8_t size);
static int index_of(const node_id_t *cluster_head_list, uint8_t size, node_id_t);
static inline int merge_lists(cluster_t* cluster_tx, cluster_t* cluster_rx);

//The number of consecutive receive states we need to be in before forcing to send again.
//In order to combat early termination. This should probably be changed to something more robust.
#define CONSECUTIVE_RECEIVE_THRESHOLD 10

#define NUMBER_OF_CLUSTER_ROUNDS 3

//What is this
#define FLAGS_LEN(node_count)   ((node_count / 8) + ((node_count % 8) ? 1 : 0))
#define LAST_FLAGS(node_count)  ((1 << ((((node_count) - 1) % 8) + 1)) - 1)
#define FLAG_SUM(node_count)  ((((node_count) - 1) / 8 * 0xFF) + LAST_FLAGS(node_count))

#define CHAOS_RESTART_MAX 10
#define CHAOS_RESTART_MIN 4

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


static chaos_state_t process(uint16_t round_count, uint16_t slot,
    chaos_state_t current_state, int chaos_txrx_success, size_t payload_length,
    uint8_t* rx_payload, uint8_t* tx_payload, uint8_t** app_flags){

    cluster_t* const cluster_tx = (cluster_t*) tx_payload;
    cluster_t* const cluster_rx = (cluster_t*) rx_payload;

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

    if(IS_CLUSTER_HEAD()) {
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

    if (current_state == CHAOS_INIT) {
        next_state = CHAOS_TX;
    } else if(current_state == CHAOS_RX) {
        delta = merge_lists(tx_payload, rx_payload);
        consecutive_rx++;
        if (delta || consecutive_rx == CONSECUTIVE_RECEIVE_THRESHOLD) {
            next_state = CHAOS_TX;
        }
        
        int node_in_list = index_of(tx_payload->cluster_head_list, tx_payload->cluster_head_count, node_id);
        if(node_in_list == -1) {
            // TODO: Check if list is full. Should we use NODE_LIST_LEN? Do we need to insert, not append?
            tx_payload->cluster_head_list[tx_payload->cluster_head_count++] = node_id;
            next_state = CHAOS_TX;
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
        delta = tx_payload->cluster_head_count != rx_payload->cluster_head_count;
        if (delta) {
            next_state = CHAOS_TX;
        }
        memcpy(tx_payload, rx_payload, sizeof(cluster_t));
    }

    return next_state;
}

static void round_begin(const uint16_t round_count, const uint8_t app_id) {
    is_cluster_service_running = 1;
    COOJA_DEBUG_PRINTF("cluster round begin\n");
    cluster_t cluster_data;
    memset(&cluster_data, 0, sizeof(cluster_t));

    if(IS_CLUSTER_HEAD()) {
        cluster_data.cluster_head_list[0] = node_id;
        cluster_data.cluster_head_count++;
    }

    invalid_rx_count = 0;
    restart_threshold = generate_restart_threshold();

    chaos_round(round_count, app_id, (const uint8_t const*)&cluster_data, sizeof(cluster_data), (7*(RTIMER_SECOND/1000)+0*(RTIMER_SECOND/1000)/2) * CLOCK_PHI, JOIN_ROUND_MAX_SLOTS, get_flags_length(), process);
}

ALWAYS_INLINE static int is_pending(const uint16_t round_count) {
    return round_count < NUMBER_OF_CLUSTER_ROUNDS;
}

static node_id_t pick_best_cluster(const node_id_t *cluster_head_list, uint8_t size) {
    // Should be based on some more complex calculation later.
    const node_id_t value = node_id % 2 == 0 ? 2 : 1;
    if(index_of(cluster_head_list, size, value) != -1) {
        return value;
    } else {
        return 0;
    }
}

static int index_of(const node_id_t *array, uint8_t size, node_id_t value) {
    int i;
    for(i = 0; i < size; ++i) {
        if (array[i] == value) {
            return i;
        }
    }
    return -1;
}

static void round_begin_sniffer(chaos_header_t* header){
  // header->join = !chaos_get_has_node_index() /*&& !is_join_round*/;
  // if( IS_INITIATOR() ){
  //   header->join |= pending /*&& !is_join_round*/;
  // }
}

static void round_end_sniffer(const chaos_header_t* header){
    if ( is_cluster_service_running ) {
        char str[200];

        is_cluster_service_running = 0;
        cluster_t* const cluster_tx = (cluster_t*) header->payload;
        if(IS_CLUSTER_HEAD()) {
            init_node_index();
            cluster_id = node_id;
        } else {
            cluster_id = pick_best_cluster(cluster_tx->cluster_head_list, cluster_tx->cluster_head_count);
        }
        char tmp[80];
        sprintf(tmp, "cluster: round_end_sniffer cluster_head_count: %u, picked_cluster: %u available_clusters: [ ", cluster_tx->cluster_head_count, cluster_id);
        strcpy(str, tmp);
        
        int i;
        for(i = 0; i < cluster_tx->cluster_head_count; i++) {
            char tmp[10];
            sprintf(tmp, (i == cluster_tx->cluster_head_count-1 ? "%u ":"%u, "), cluster_tx->cluster_head_list[i]);
            strcat(str, tmp);
        }
        
        strcat(str, "]\n");
        PRINTF(str);
    }
}

static inline int merge_lists(cluster_t* cluster_tx, cluster_t* cluster_rx) {
  uint8_t index_rx = 0;
  uint8_t index_tx = 0;
  uint8_t index_merge = 0;
  uint8_t delta = 0;

  node_id_t merge[NODE_LIST_LEN] = {0};

  while ((index_tx < cluster_tx->cluster_head_count || index_rx < cluster_rx->cluster_head_count ) && index_merge < NODE_LIST_LEN) {
    if (index_tx >= cluster_tx->cluster_head_count || (index_rx < cluster_rx->cluster_head_count && cluster_rx->cluster_head_list[index_rx] < cluster_tx->cluster_head_list[index_tx])) {
      merge[index_merge] = cluster_rx->cluster_head_list[index_rx];
      index_merge++;
      index_rx++;
      delta = 1; //arrays differs, so TX
    } else if (index_rx >= cluster_rx->cluster_head_count || (index_tx < cluster_tx->cluster_head_count && cluster_rx->cluster_head_list[index_rx] > cluster_tx->cluster_head_list[index_tx])) {
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
