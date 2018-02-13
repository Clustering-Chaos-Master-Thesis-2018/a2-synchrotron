
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

uint32_t restart_threshold = 0;
uint32_t invalid_rx_count = 0;

node_id_t tentative_cluster_id = 0;

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
                restart_threshold = chaos_random_generator_fast() % (CHAOS_RESTART_MAX - CHAOS_RESTART_MIN) + CHAOS_RESTART_MIN;
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

static chaos_state_t process_cluster_head(uint16_t round_count, uint16_t slot,
    chaos_state_t current_state, int chaos_txrx_success, size_t payload_length,
    cluster_t* rx_payload, cluster_t* tx_payload, uint8_t** app_flags) {
        
    uint8_t delta = 0;

    if(current_state == CHAOS_RX) {
        delta = merge_lists(tx_payload, rx_payload);
        int node_in_list = index_of(tx_payload->cluster_head_list, tx_payload->cluster_head_count, node_id);
        if(node_in_list == -1) {
            // TODO: Check if list is full. Should we use NODE_LIST_LEN? Do we need to insert, not append?
            tx_payload->cluster_head_list[tx_payload->cluster_head_count++] = node_id;
        }
        return delta || !node_in_list ?  CHAOS_TX : CHAOS_RX;
        
    } else if (current_state == CHAOS_TX) {
        return CHAOS_RX;
    }
    return IS_MAJOR_CLUSTER_HEAD() ? CHAOS_TX : CHAOS_RX;
}

static chaos_state_t process_cluster_node(uint16_t round_count, uint16_t slot,
    chaos_state_t current_state, int chaos_txrx_success, size_t payload_length,
    cluster_t* rx_payload, cluster_t* tx_payload, uint8_t** app_flags) {
    
    if (current_state == CHAOS_RX) {
        tentative_cluster_id = pick_best_cluster(rx_payload->cluster_head_list, rx_payload->cluster_head_count);
        //COOJA_DEBUG_PRINTF("cluster: picked cluster: %u, rc: %u\n", cluster_id, round_count);
        memcpy(tx_payload, rx_payload, sizeof(cluster_t));
        return CHAOS_RX;
    } else if (current_state == CHAOS_TX) {
        return CHAOS_RX;
    } else {
        /* idunno */
        // COOJA_DEBUG_PRINTF("cluster round not in txrx state\n");
        return CHAOS_RX;
    }
}

static void round_begin(const uint16_t round_count, const uint8_t app_id) {
    cluster_t cluster_data;
    memset(&cluster_data, 0, sizeof(cluster_t));
    invalid_rx_count = 0;
    restart_threshold = chaos_random_generator_fast() % (CHAOS_RESTART_MAX - CHAOS_RESTART_MIN) + CHAOS_RESTART_MIN;
    // if( IS_DYNAMIC_INITIATOR() ){
    //   cluster_data.node_count = chaos_get_node_count();
    //   unsigned int array_index = chaos_get_node_index() / 8;
    //   unsigned int array_offset = chaos_get_node_index() % 8;
    //   cluster_data.flags[array_index] |= 1 << (array_offset);
    // } else if( !chaos_get_has_node_index() ){
    //   cluster_data.slot[0] = node_id;
    //   cluster_data.slot_count = 1;
    // } else {
    //   unsigned int array_index = chaos_get_node_index() / 8;
    //   unsigned int array_offset = chaos_get_node_index() % 8;
    //   cluster_data.flags[array_index] |= 1 << (array_offset);
    // }
    // COOJA_DEBUG_PRINTF("cluster round begin: restart_threshold:%u, rc: %u, flags_length: %u\n", restart_threshold, round_count, get_flags_length());
    chaos_round(round_count, app_id, (const uint8_t const*)&cluster_data, sizeof(cluster_data), (7*(RTIMER_SECOND/1000)+0*(RTIMER_SECOND/1000)/2) * CLOCK_PHI, JOIN_ROUND_MAX_SLOTS, get_flags_length(), process);
}

ALWAYS_INLINE static int is_pending(const uint16_t round_count) {
    // COOJA_DEBUG_PRINTF("cluster: is_pending {rc %u, pending: %u\n", round_count, round_count < 15);
    return round_count > 4 && round_count < 15;
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

static int index_of(const node_id_t *cluster_head_list, uint8_t size, node_id_t value) {
    int i;
    for(i = 0; i < size; ++i) {
        if (cluster_head_list[i] == value) {
            return i;
        }
    }
    return -1;
}

static void round_begin_sniffer(chaos_header_t* header){
  // header->join = !chaos_get_has_node_index() /*&& !is_join_round*/;
  // if( IS_DYNAMIC_INITIATOR() ){
  //   header->join |= pending /*&& !is_join_round*/;
  // }
}

static void round_end_sniffer(const chaos_header_t* header){
    cluster_t* const cluster_tx = (cluster_t*) header->payload;
    if(IS_CLUSTER_HEAD()) {
      cluster_id = node_id;
    } else {
      cluster_id = tentative_cluster_id;
    }
    COOJA_DEBUG_PRINTF("cluster: round_end_sniffer cluster_head_count: %u, list[0]: %u, list[1]: %u, picked cluster: %u\n",
                        cluster_tx->cluster_head_count, cluster_tx->cluster_head_list[0], cluster_tx->cluster_head_list[1], cluster_id);
    // int i;
    // char str[80];
    // strcpy(str, "cluster: round_end_sniffer ");
    // for(i = 0; i < cluster_tx->cluster_head_count; i++) {
    //     char tmp[10];
    //     sprintf(tmp, "%u,", cluster_tx->cluster_head_list[i]);
    //     strcat(str, tmp);
    // }
    // strcat(str, "\n");
    // PRINTF(str);
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
