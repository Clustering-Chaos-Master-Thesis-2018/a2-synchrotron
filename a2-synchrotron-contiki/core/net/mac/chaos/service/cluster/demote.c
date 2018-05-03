
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

static void round_begin(const uint16_t round_count, const uint8_t id);
static int is_pending(const uint16_t round_count);
static void round_begin_sniffer(chaos_header_t* header);
static void round_end_sniffer(const chaos_header_t* header);
static chaos_state_t handle_invalid_rx();

uint32_t restart_threshold = 0;
uint32_t invalid_rx_count = 0;
uint8_t got_valid_rx = 0;


#define DEMOTE_SERVICE_PENDING_THRESHOLD 21

#define CLUSTER_SLOT_LEN          (7*(RTIMER_SECOND/1000)+0*(RTIMER_SECOND/1000)/2)
#define CLUSTER_SLOT_LEN_DCO      (CLUSTER_SLOT_LEN*CLOCK_PHI)

#define FLAGS_LEN(node_count)   ((node_count / 8) + ((node_count % 8) ? 1 : 0))
#define LAST_FLAGS(node_count)  ((1 << ((((node_count) - 1) % 8) + 1)) - 1)
#define FLAG_SUM(node_count)  ((((node_count) - 1) / 8 * 0xFF) + LAST_FLAGS(node_count))

CHAOS_SERVICE(demote, CLUSTER_SLOT_LEN, CLUSTER_ROUND_MAX_SLOTS, 0, is_pending, round_begin, round_begin_sniffer, round_end_sniffer);

ALWAYS_INLINE static int get_flags_length(){
  return FLAGS_LEN(MAX_NODE_COUNT);
}

static int16_t index_of(const node_id_t const *array, uint8_t size, node_id_t value) {
    uint8_t i;
    for(i = 0; i < size; ++i) {
        if (array[i] == value) {
            return i;
        }
    }
    return -1;
}

static chaos_state_t process(uint16_t round_count, uint16_t slot,
    chaos_state_t current_state, int chaos_txrx_success, size_t payload_length,
    uint8_t* rx_payload, uint8_t* tx_payload, uint8_t** app_flags){
    chaos_state_t next_state = CHAOS_RX;

    demote_cluster_t* const cluster_rx = (demote_cluster_t*) rx_payload;
    demote_cluster_t* const cluster_tx = (demote_cluster_t*) tx_payload;

    uint8_t delta = 0;

    if(current_state == CHAOS_INIT && IS_INITIATOR()) {
        got_valid_rx = 1;
        return CHAOS_TX;
    }

    if(current_state == CHAOS_RX) {
        if(!chaos_txrx_success) {
            return handle_invalid_rx();
        }
        got_valid_rx = 1;
        invalid_rx_count = 0;
        uint8_t i, j = 0;
        uint8_t found = 0;
        for(i = 0; i < cluster_rx->node_count; ++i) {
            for(j = 0; j < cluster_tx->node_count; ++i) {
                if(cluster_tx->demoted_cluster_heads[j] == cluster_rx->demoted_cluster_heads[i]) {
                    found = 1;
                }
            }
            if(!found) {
                cluster_tx->demoted_cluster_heads[cluster_tx->node_count++] = cluster_rx->demoted_cluster_heads[i];
                delta = 1;
            }
        }

        if(demoted) {
            int16_t node_in_list = index_of(cluster_tx->demoted_cluster_heads, cluster_tx->node_count, node_id);
            if(node_in_list == -1) {
                cluster_tx->demoted_cluster_heads[cluster_tx->node_count++] = node_id;
                delta = 1;
            }
        }
    }

    if(delta && got_valid_rx) {
        next_state = CHAOS_TX;
    }

    return next_state;
}


static void round_begin(const uint16_t round_count, const uint8_t app_id) {
    got_valid_rx = 0;
    invalid_rx_count = 0;
    restart_threshold = generate_restart_threshold();

    demote_cluster_t initial_demote_data;
    memset(&initial_demote_data, 0, sizeof(cluster_t));

    chaos_round(round_count, app_id, (const uint8_t const*)&initial_demote_data, sizeof(initial_demote_data), CLUSTER_SLOT_LEN_DCO, CLUSTER_ROUND_MAX_SLOTS, get_flags_length(), process);
}

ALWAYS_INLINE static int is_pending(const uint16_t round_count) {
    return round_count <= DEMOTE_SERVICE_PENDING_THRESHOLD;
}

static void round_begin_sniffer(chaos_header_t* header){
}


static void round_end_sniffer(const chaos_header_t* header){

}

static chaos_state_t handle_invalid_rx() {
    invalid_rx_count++;
    if(invalid_rx_count > restart_threshold && got_valid_rx) {
        invalid_rx_count = 0;
        restart_threshold = generate_restart_threshold();
        return CHAOS_TX;
    }
    return CHAOS_RX;
}
