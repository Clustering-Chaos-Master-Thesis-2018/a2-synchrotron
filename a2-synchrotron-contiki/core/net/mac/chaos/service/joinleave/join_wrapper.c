#include "chaos.h"
#include "chaos-random-generator.h"
#include "chaos-control.h"
#include "node.h"
#include "join.h"
#include "testbed.h"
#include "chaos-config.h"
#include "chaos-cluster.h"

#define ENABLE_COOJA_DEBUG COOJA
#include "dev/cooja-debug.h"

static void wrapper_round_begin(const uint16_t round_count, const uint8_t id);
static int wrapper_is_pending(const uint16_t round_count);
static void wrapper_round_begin_sniffer(chaos_header_t* header);
static void wrapper_round_end_sniffer(const chaos_header_t* header);

#define JOIN_SLOT_LEN          (7*(RTIMER_SECOND/1000)+0*(RTIMER_SECOND/1000)/2)    //TODO needs calibration
#define JOIN_SLOT_LEN_DCO      (JOIN_SLOT_LEN*CLOCK_PHI)    //TODO needs calibration

#define FLAGS_LEN(node_count)   ((node_count / 8) + ((node_count % 8) ? 1 : 0))
#define LAST_FLAGS(node_count)  ((1 << ((((node_count) - 1) % 8) + 1)) - 1)
#define FLAG_SUM(node_count)  ((((node_count) - 1) / 8 * 0xFF) + LAST_FLAGS(node_count))

typedef struct __attribute__((packed)) {
  uint8_t node_count;
  union {
    uint8_t commit_field;
    struct{
      uint8_t                   //control flags
        slot_count :5,       //number of slots into which nodes wrote their ids
        overflow :1,              /* available join slots too short */
        commit :2;                /* commit join */
    };
  };
  node_id_t slot[NODE_LIST_LEN];              //slots to write node ids into
  node_index_t index[NODE_LIST_LEN]; /* assigned indices */
  uint8_t flags[FLAGS_LEN(MAX_NODE_COUNT)];  //flags used to confirm the commit
} join_t;

CHAOS_SERVICE(cluster_join_wrapper, JOIN_SLOT_LEN, JOIN_ROUND_MAX_SLOTS, 0,
              wrapper_is_pending, wrapper_round_begin, wrapper_round_begin_sniffer, wrapper_round_end_sniffer);

static uint8_t pending = 0;
uint8_t is_cluster_join_round = 0;
uint8_t cluster_head_round_initiated = 0;

#define NUMBER_OF_CLUSTER_JOIN_ROUNDS 10
#define NUMBER_OF_JOIN_ROUNDS 15

ALWAYS_INLINE static uint8_t should_switch_to_cluster_heads(const uint16_t round_number) {
  return round_number > NUMBER_OF_JOIN_ROUNDS;
}


static void wrapper_round_begin( const uint16_t round_number, const uint8_t app_id ) {
  COOJA_DEBUG_PRINTF("cluster join wrapper round begin\n");
  is_cluster_join_round = 1;
  if((should_switch_to_cluster_heads(round_number) || round_number - NUMBER_OF_JOIN_ROUNDS == 0) && !cluster_head_round_initiated) {
    COOJA_DEBUG_PRINTF("cluster switching to cluster head join\n");
    cluster_head_round_initiated = 1;
    join_init();
  }

  round_begin(round_number, app_id);
}

static void wrapper_round_begin_sniffer(chaos_header_t* header) {
  round_begin_sniffer(header);
  // COOJA_DEBUG_PRINTF("cluster, halli halla\n");
  // header->join = !chaos_get_has_node_index() /*&& !is_join_round*/;
  // if( IS_INITIATOR() ){
  //   header->join |= pending /*&& !is_join_round*/;
  // }
}

static void wrapper_round_end_sniffer(const chaos_header_t* header) {
  round_end_sniffer(header);
  // COOJA_DEBUG_PRINTF("cluster join wrapper end\n");
  // is_cluster_join_round = 0;
  // pending |= IS_INITIATOR() && ( header->join || chaos_node_count < 2);
}

static int wrapper_is_pending( const uint16_t round_count ) {
  //TODO: optimiziation, enable this after testing and bug fixing
  if( round_count < (NUMBER_OF_JOIN_ROUNDS + NUMBER_OF_CLUSTER_JOIN_ROUNDS) )
  {
    // COOJA_DEBUG_PRINTF("cluster Set pending\n")
    pending = 1;
  }
  return pending;
  //return 1;
}
