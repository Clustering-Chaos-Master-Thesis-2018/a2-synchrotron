#include "contiki.h"
#include "net/mac/chaos/chaos.h"
#include "net/mac/chaos/chaos-header.h"
#include "chaos-cluster.h"
#include "net/mac/chaos/chaos-random-generator.h"
#include "chaos-scheduler.h"

node_id_t cluster_id = 0;
uint8_t cluster_index = 0;

ALWAYS_INLINE void chaos_cluster_round_end(void) {
    scheduler_set_next_app(get_next_round_id());
}

ALWAYS_INLINE node_id_t chaos_get_cluster_id(void) {
  return cluster_id;
}

ALWAYS_INLINE uint8_t chaos_get_cluster_index(void) {
  return cluster_index;
}
