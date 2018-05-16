
#ifndef _CHAOS_CLUSTER_H_
#define _CHAOS_CLUSTER_H_

#include "contiki.h"

typedef uint16_t node_id_t;
typedef uint8_t node_index_t;

#if CHAOS_CLUSTER
    #include "cluster.h"

    #define JOIN_SERVICE_RUNNING()               (is_join_round)
    #define CLUSTER_SERVICE_RUNNING()            (is_cluster_service_running)
    #define DEMOTE_SERVICE_RUNNING()             (is_demote_service_running)
    #define IS_CLUSTER_HEAD_ROUND()              (chaos_get_round_number() % 2 == 0 && HAS_CLUSTER_ID() && !CLUSTER_SERVICE_RUNNING() && !DEMOTE_SERVICE_RUNNING() && !JOIN_SERVICE_RUNNING())
    #define IS_SAME_CLUSTER(RECEIVED_CLUSTER_ID) (RECEIVED_CLUSTER_ID == chaos_get_cluster_id() \
                                               || RECEIVED_CLUSTER_ID == 0 \
                                               || !HAS_CLUSTER_ID())
    #define IS_CLUSTER_HEAD()                    (node_id == chaos_get_cluster_id())
    #define HAS_CLUSTER_ID()                     (chaos_get_cluster_id() != 0)
    #define IS_FORWARDER()                       (IS_CLUSTER_HEAD_ROUND() && !IS_CLUSTER_HEAD())

    // During cluster service rounds, use no offset
    #define CLUSTER_HOP_CHANNEL_OFFSET() (CLUSTER_SERVICE_RUNNING() || IS_CLUSTER_HEAD_ROUND() ? 0 : chaos_get_cluster_index())

    extern node_id_t cluster_id;
    extern uint8_t cluster_index;
    extern uint8_t is_join_round;

#else
    #define IS_FORWARDER() 0

#endif /* CHAOS_CLUSTER */

ALWAYS_INLINE node_id_t chaos_get_cluster_id(void);
ALWAYS_INLINE uint8_t chaos_get_cluster_index(void);
ALWAYS_INLINE void chaos_cluster_round_end(void);

#endif /* _CHAOS_CLUSTER_H_ */
