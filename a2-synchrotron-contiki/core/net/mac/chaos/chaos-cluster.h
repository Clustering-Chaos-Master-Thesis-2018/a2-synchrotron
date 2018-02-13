
#ifndef _CHAOS_CLUSTER_H_
#define _CHAOS_CLUSTER_H_

#include "contiki.h"
#include "chaos-control.h"

typedef uint16_t node_id_t;
typedef uint8_t node_index_t;

#if CHAOS_CLUSTER
    #define IS_CLUSTER_ROUND()                   (chaos_get_round_number() % 3 == 0)
    #define IS_SAME_CLUSTER(CLUSTER_ID)          (CLUSTER_ID == chaos_get_cluster_id() || CLUSTER_ID == 0)
    #define IS_MAJOR_CLUSTER_HEAD()              (node_id == 1)
    #define IS_CLUSTER_HEAD()                    (node_id == 1 || node_id == 2)
    #define IS_DYNAMIC_INITIATOR()               ((IS_CLUSTER_HEAD() && !IS_CLUSTER_ROUND()) \
                                                 || (IS_MAJOR_CLUSTER_HEAD() && IS_CLUSTER_ROUND()))
    #define HAS_CLUSTER_ID()                     (chaos_get_cluster_id() != 0)
    typedef uint16_t node_id_t; //Temporary fix, shold probably not redefine this here.
    extern node_id_t cluster_id;
#else
    #define IS_DYNAMIC_INITIATOR()               IS_INITIATOR()

#endif /* CHAOS_CLUSTER */

void chaos_cluster_init(void);
ALWAYS_INLINE void chaos_cluster_round_init(uint8_t is_initiator, chaos_header_t* const tx_header);
ALWAYS_INLINE node_id_t chaos_get_cluster_id(void);

#endif /* _CHAOS_CLUSTER_H_ */
