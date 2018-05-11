#ifndef _CLUSTER_H
#define _CLUSTER_H

#include "chaos-control.h"
#include "node.h"

#ifndef MAX_NODE_COUNT
#define MAX_NODE_COUNT 254
#endif

extern const chaos_app_t cluster;
extern const chaos_app_t demote;


#ifndef NODE_LIST_LEN
#define NODE_LIST_LEN 30  //describes how many nodes can become clusterheads.
#endif

typedef enum {
    NOT_INITIALIZED,
    NOT_CH,
    TENTATIVE,
    FINAL
} CHState;

#undef CLUSTER_COMPETITION_RADIUS
#define CLUSTER_COMPETITION_RADIUS _param_cluster_competition_radius

#undef CLUSTER_RANDOMIZE_STARTING_ENERGY
#define CLUSTER_RANDOMIZE_STARTING_ENERGY _param_randomize_starting_energy

#undef CLUSTER_NODES_PER_CLUSTER
#define CLUSTER_NODES_PER_CLUSTER _param_nodes_per_cluster

#if CLUSTER_NODES_PER_CLUSTER == 0
#define CLUSTER_NODES_PER_CLUSTER MAX_NODE_COUNT
#endif

typedef struct __attribute__((packed)) {
    node_id_t id;
    struct __attribute__((packed)) {
      uint8_t
        hop_count :6,    /* The distance in hops to the cluster head */
        status :2;     /* CH status, can either be TENTATIVE or FINAL */
    };
} cluster_head_information_t;

typedef struct __attribute__((packed)) {
    uint8_t cluster_head_count;
    uint8_t source_id;
    uint16_t total_rx_count;
    int8_t consecutive_cluster_round_count;
    cluster_head_information_t cluster_head_list[NODE_LIST_LEN];
} cluster_t;

typedef struct __attribute__((packed)) {
    uint8_t node_count;
    node_id_t demoted_cluster_heads[NODE_LIST_LEN];
} demote_cluster_t;

ALWAYS_INLINE uint32_t generate_restart_threshold();
void set_global_cluster_variables(const cluster_head_information_t* cluster_head_list, uint8_t cluster_head_count);

extern uint8_t is_cluster_service_running;
extern uint16_t neighbour_list[MAX_NODE_COUNT];
extern cluster_t local_cluster_data;
extern CHState cluster_head_state;


#endif /* CHAOS_CLUSTER_H */
