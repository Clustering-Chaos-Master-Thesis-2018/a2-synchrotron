/*******************************************************************************
 * BSD 3-Clause License
 *
 * Copyright (c) 2017 Beshr Al Nahas and Olaf Landsiedel.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

/*
 * \file
 *         Node manager
 * \author
 *         Beshr Al Nahas <beshr@chalmers.se>
 *         Olaf Landsiedel <olafl@chalmers.se>
 */

#include "contiki.h"
#include "join.h"
#include "node.h"
#include "net/mac/chaos/node/testbed.h"

volatile uint8_t chaos_node_index = 0;
volatile uint8_t chaos_node_count = 0;
volatile uint8_t chaos_has_node_index = 0;

volatile uint8_t chaos_cluster_node_index = 0;
volatile uint8_t chaos_cluster_node_count = 0;

const uint16_t mapping[] = (uint16_t[])TESTBED_MAPPING;

void init_node_index(){
  join_init(IS_INITIATOR());
}

#if !CHAOS_CLUSTER
  #define IS_CLUSTER_HEAD_ROUND() 0
#endif

ALWAYS_INLINE uint8_t chaos_get_has_node_index(void) {
  if(IS_CLUSTER_HEAD_ROUND()) {
    return chaos_cluster_node_count != 0;
  } else {
    return chaos_has_node_index;
  }
}

ALWAYS_INLINE uint8_t chaos_get_node_count(void) {
  if(IS_CLUSTER_HEAD_ROUND()) {
    return chaos_cluster_node_count;
  } else {
    return chaos_node_count;
  }
}

ALWAYS_INLINE uint8_t chaos_get_node_index(void) {
  if(IS_CLUSTER_HEAD_ROUND()) {
    return chaos_cluster_node_index;
  } else {
    return chaos_node_index;
  }
}

ALWAYS_INLINE void chaos_set_node_index(uint8_t node_index) {
  if(IS_CLUSTER_HEAD_ROUND()) {
    chaos_cluster_node_index = node_index;
  } else {
    chaos_node_index = node_index;
  }
}

ALWAYS_INLINE void chaos_set_node_count(uint8_t node_count) {
 if(IS_CLUSTER_HEAD_ROUND()) {
    chaos_cluster_node_count = node_count;
 } else {
    chaos_node_count = node_count;
  }
}
