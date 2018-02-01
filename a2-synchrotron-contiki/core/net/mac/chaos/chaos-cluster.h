
#ifndef  CHAOS_CLUSTER_H_
#define CHAOS_CLUSTER_H_

#include "contiki.h"

#define IS_CLUSTER_ROUND(ROUND)              (ROUND % 3 == 0)
#define IS_SAME_CLUSTER(CLUSTER_ID, NODE_ID) (CLUSTER_ID % 2 == NODE_ID % 2)

#if CHAOS_CLUSTER
    #define IS_MAJOR_CLUSTER_HEAD()          (node_id == 1)
    #define IS_CLUSTER_HEAD()                (node_id == 1 || node_id == 2)
    #define IS_DYNAMIC_INITIATOR(ROUND)      ((IS_CLUSTER_HEAD() && !IS_CLUSTER_ROUND(ROUND)) \
                                           || (IS_MAJOR_CLUSTER_HEAD() && IS_CLUSTER_ROUND(ROUND)))
#else
    #define IS_DYNAMIC_INITIATOR(ROUND)      IS_INITIATOR()

#endif /* CHAOS_CLUSTER */

void chaos_cluster_init(void);
ALWAYS_INLINE void chaos_cluster_round_init(uint8_t is_initiator, chaos_header_t* const tx_header);



#endif /* CHAOS_CLUSTER_H_ */