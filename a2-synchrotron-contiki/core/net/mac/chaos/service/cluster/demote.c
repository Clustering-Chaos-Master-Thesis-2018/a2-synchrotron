
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
static int16_t index_of(const node_id_t const *array, uint8_t size, node_id_t value);
static void round_begin_sniffer(chaos_header_t* header);
static void round_end_sniffer(const chaos_header_t* header);
static chaos_state_t handle_invalid_rx();

static uint8_t demoted = 0;
static uint32_t restart_threshold = 0;
static uint32_t invalid_rx_count = 0;
static uint8_t got_valid_rx = 0;
uint8_t is_demote_service_running = 0;

static demote_cluster_t local_demote_data;

#define CLUSTER_SLOT_LEN          (7*(RTIMER_SECOND/1000)+0*(RTIMER_SECOND/1000)/2)
#define CLUSTER_SLOT_LEN_DCO      (CLUSTER_SLOT_LEN*CLOCK_PHI)

#define FLAGS_LEN(node_count)   ((node_count / 8) + ((node_count % 8) ? 1 : 0))
#define LAST_FLAGS(node_count)  ((1 << ((((node_count) - 1) % 8) + 1)) - 1)
#define FLAG_SUM(node_count)  ((((node_count) - 1) / 8 * 0xFF) + LAST_FLAGS(node_count))

CHAOS_SERVICE(demote, CLUSTER_SLOT_LEN, CLUSTER_ROUND_MAX_SLOTS, 0, is_pending, round_begin, round_begin_sniffer, round_end_sniffer);

ALWAYS_INLINE static int get_flags_length(){
  return FLAGS_LEN(MAX_NODE_COUNT);
}

__attribute__((always_inline)) static int16_t index_of(const node_id_t const *array, uint8_t size, node_id_t value) {
    uint8_t i;
    for(i = 0; i < size; ++i) {
        if (array[i] == value) {
            return i;
        }
    }
    return -1;
}

__attribute__((always_inline)) static uint8_t merge(node_id_t* src, uint8_t src_size, node_id_t* dst, uint8_t dst_size, uint8_t* delta) {
    uint8_t i, j = 0;
    uint8_t found = 0;
    *delta |= src_size != dst_size;
    for(i = 0; i < src_size; ++i) {
        found = 0;
        for(j = 0; j < dst_size; ++j) {
            if(src[i] == dst[j]) {
                found = 1;
                break;
            }
        }
        if(!found && dst_size < NODE_LIST_LEN) {
            dst[dst_size++] = src[i];
            *delta |= 1;
        }
    }
    return dst_size;
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
        cluster_tx->node_count = merge(cluster_rx->demoted_cluster_heads, cluster_rx->node_count, cluster_tx->demoted_cluster_heads, cluster_tx->node_count, &delta);

        if(demoted) {
            const int16_t node_in_list = index_of(cluster_tx->demoted_cluster_heads, cluster_tx->node_count, node_id);
            if(node_in_list == -1) {
                cluster_tx->demoted_cluster_heads[cluster_tx->node_count++] = node_id;
                delta |= 1;
            }
        }
    }

    if(delta && got_valid_rx) {
        next_state = CHAOS_TX;
    }

    return next_state;
}


ALWAYS_INLINE uint8_t should_demote(uint8_t neighbour_count, uint8_t joined_nodes_count) {
    return joined_nodes_count < 4 /*&& neighbour_count > 8*/;
}

static void round_begin(const uint16_t round_count, const uint8_t app_id) {
    got_valid_rx = 0;
    invalid_rx_count = 0;
    is_demote_service_running = 1;
    restart_threshold = generate_restart_threshold();


    const uint8_t count = count_filled_slots(neighbour_list, MAX_NODE_COUNT);
    if(IS_CLUSTER_HEAD() && should_demote(count, chaos_node_count)) {
        COOJA_DEBUG_PRINTF("cluster demoting myself, chaos_node_count: %u < 4", chaos_node_count);
        if(index_of(local_demote_data.demoted_cluster_heads, local_demote_data.node_count, node_id) == -1) {
            local_demote_data.demoted_cluster_heads[local_demote_data.node_count++] = node_id;
        }
        demoted = 1;
        cluster_head_state = NOT_INITIALIZED;
    } else {
        demoted = 0;
    }

    chaos_round(round_count, app_id, (const uint8_t const*)&local_demote_data, sizeof(local_demote_data), CLUSTER_SLOT_LEN_DCO, CLUSTER_ROUND_MAX_SLOTS, get_flags_length(), process);
}

ALWAYS_INLINE static int is_pending(const uint16_t round_count) {
    return (round_count >= 20 && round_count <= 25) || (round_count >= 220 && round_count <= 225) || (round_count >= 420 && round_count <= 425);
}

static void round_begin_sniffer(chaos_header_t* header){}


uint8_t filter_demoted_cluster_heads(const cluster_head_information_t* const cluster_head_list, uint8_t cluster_head_count, cluster_head_information_t* const output, const demote_cluster_t* const demoted_packet) {
    uint8_t i, j, output_size = 0, found = 0;
    for(i = 0; i < cluster_head_count; ++i) {
        found = 0;
        for(j = 0; j < demoted_packet->node_count; ++j) {
            if(cluster_head_list[i].id == demoted_packet->demoted_cluster_heads[j]) {
                found = 1;
                break;
            }
        }
        if(!found) {
            output[output_size++] = cluster_head_list[i];
        }
    }
    return output_size;
}

static void round_end_sniffer(const chaos_header_t* header){
    if(!is_demote_service_running) {
        return;
    }

    demote_cluster_t* payload = (demote_cluster_t*)header->payload;
    uint8_t delta = 0;
    local_demote_data.node_count = merge(payload->demoted_cluster_heads, payload->node_count, local_demote_data.demoted_cluster_heads, local_demote_data.node_count, &delta);

    is_demote_service_running = is_pending(header->round_number + 1);
    if(!is_demote_service_running) {

        cluster_head_information_t filtered_cluster_heads[NODE_LIST_LEN];
        cluster_head_information_t valid_cluster_heads[NODE_LIST_LEN];

        const uint8_t output_size = filter_demoted_cluster_heads(local_cluster_data.cluster_head_list, local_cluster_data.cluster_head_count, filtered_cluster_heads, &local_demote_data);
        if(output_size == local_cluster_data.cluster_head_count) {
            return;
        }
        const uint8_t valid_cluster_head_count = filter_valid_cluster_heads(filtered_cluster_heads, output_size, valid_cluster_heads, CLUSTER_COMPETITION_RADIUS);
        if(valid_cluster_head_count == 0) {
            COOJA_DEBUG_PRINTF("No valid cluster heads left after demotion, doing nothing");
        } else {
            set_global_cluster_variables(filtered_cluster_heads, output_size);
            chaos_cluster_node_count = output_size;
            init_node_index();
            COOJA_DEBUG_PRINTF("cluster head demoted, changing to cluster: %u, index: %u, new_cluster_count: %u", cluster_id, cluster_index, chaos_cluster_node_count);
        }
    }
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
