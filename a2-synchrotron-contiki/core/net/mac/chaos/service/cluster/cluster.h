#ifndef _CLUSTER_H
#define _CLUSTER_H

#include "chaos-cluster.h"

extern const chaos_app_t cluster;

#ifndef NODE_LIST_LEN
#define NODE_LIST_LEN 10  //describes how many nodes can become clusterheads.
#endif

#endif /* CHAOS_CLUSTER_H */
