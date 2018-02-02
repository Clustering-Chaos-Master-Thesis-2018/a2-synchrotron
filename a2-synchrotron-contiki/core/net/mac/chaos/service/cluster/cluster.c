
#include "contiki.h"
#include <string.h>
#include <stdio.h>

#include "chaos.h"
// #include "chaos-random-generator.h"
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
} cluster_t

static void round_begin(const uint16_t round_count, const uint8_t id);
static int is_pending(const uint16_t round_count);
static void round_begin_sniffer(chaos_header_t* header);
static void round_end_sniffer(const chaos_header_t* header);
static node_id_t pick_best_cluster(const node_id_t *cluster_head_list, uint8_t size);
static int index_of(const node_id_t *cluster_head_list, uint8_t size);
static inline int merge_lists(cluster_t* cluster_tx, cluster_t* cluster_rx);

static chaos_state_t process(uint16_t round_count, uint16_t slot,
    chaos_state_t current_state, int chaos_txrx_success, size_t payload_length,
    uint8_t* rx_payload, uint8_t* tx_payload, uint8_t** app_flags){

    cluster_t* cluster_tx = (cluster_t*) tx_payload;
    cluster_t* cluster_rx = (cluster_t*) rx_payload;

    uint8_t delta = 0;



    /* if(index_of(cluster_tx->cluster_head_list, cluster_tx->cluster_head_count) != -1
       || index_of(cluster_rx->cluster_head_list, cluster_rx->cluster_head_count) != -1) { */
    if(IS_CLUSTER_HEAD()) {
        if(current_state == CHAOS_RX) {
            delta = merge_lists(cluster_tx, cluster_rx);
            int node_in_list = index_of(cluster_tx->cluster_head_list, cluster_tx->cluster_head_count, node_id);
            if(node_in_list == -1) {
                // TODO: Check if list is full. Should we use NODE_LIST_LEN?
                cluster_tx->cluster_head_list[cluster_head_count] = node_id;
            }
            return CHAOS_TX;
        } else if (current_state == CHAOS_TX) {

        }


        return CHAOS_TX;
    }

    if (current_state == CHAOS_RX) {
        cluster_id = pick_best_cluster(cluster_rx->cluster_head_list, cluster_rx->cluster_head_count));
        memcpy(cluster_tx, cluster_rx, sizeof(cluster_t));
        return CHAOS_TX;
    } else if (current_state == CHAOS_TX) {
        return CHAOS_RX;
    } else {
        /* idunno */
        COOJA_DEBUG_PRINTF("cluster round not in txrx state\n");
    }
}

static int is_pending(const uint16_t round_count) {
    return round_count < 5;
}

static node_id_t pick_best_cluster(const node_id_t *cluster_head_list, uint8_t size) {
    // Should be based on some more complex calculation later.
    return node_id % 2 == 0 ? 2 : 1;
}

static int index_of(const node_id_t *cluster_head_list, uint8_t size, node_id_t) {
    int i;
    for(i = 0; i < size; ++i) {
        if (cluster_head_list[i] == node_id) {
            return i;
        }
    }
    return -1;
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
    cluster_tx->overflow = 1;
    delta = 1; //arrays differs, so TX
  }
  cluster_tx->cluster_head_count = index_merge;

  memcpy(cluster_tx->cluster_head_list, merge, sizeof(merge));
  return delta;
}
