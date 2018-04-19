#ifndef _CLUSTER_H
#define _CLUSTER_H

#include "chaos-control.h"

extern const chaos_app_t cluster;
extern uint8_t is_cluster_service_running;

#ifndef NODE_LIST_LEN
#define NODE_LIST_LEN 50  //describes how many nodes can become clusterheads.
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

#endif /* CHAOS_CLUSTER_H */
