#include "contiki.h"
#include "net/mac/chaos/chaos.h"
#include "net/mac/chaos/chaos-header.h"
#include "chaos-cluster.h"
#include "net/mac/chaos/chaos-random-generator.h"

node_id_t cluster_id = 0;

void chaos_cluster_init(void) {
    PRINTF("where should this run?");
}


ALWAYS_INLINE void chaos_cluster_round_init(uint8_t is_initiator, chaos_header_t* const tx_header) {
    PRINTF("where should this run?");
}
