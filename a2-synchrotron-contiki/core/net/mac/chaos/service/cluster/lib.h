#ifndef _CHAOS_LIB_H_
#define _CHAOS_LIB_H_
#include "cluster.h"

uint16_t sum(const uint16_t* const array, uint8_t size);
uint16_t max(const uint16_t* const array, uint8_t size);
uint16_t min(const uint16_t* const array, uint8_t size);
float mean(const uint16_t* const array, uint8_t size);
float standard_deviation(const uint16_t* const array, uint8_t size);

uint8_t last_filled_index(const uint16_t* const array, uint8_t size);
uint8_t count_filled_slots(const uint16_t* const array, uint8_t size);

uint8_t calculate_smallest_hop_count(const cluster_head_information_t const *cluster_head_list, uint8_t size);
uint8_t filter_valid_cluster_heads(const cluster_head_information_t* const cluster_head_list, uint8_t cluster_head_count, cluster_head_information_t* const output, uint8_t threshold);
node_id_t pick_best_cluster(const cluster_head_information_t *cluster_head_list, uint8_t size);

int ipow(int base, int exp);
void reverse(char *str, int len);
int intToStr(int x, char str[], int d);
void ftoa(float n, char *res, int afterpoint);

#endif /* _CHAOS_LIB_H_ */
