#include<stdio.h>
#include<math.h>

#include "lib.h"

#include "contiki.h"

uint16_t sum(const uint16_t* const array) {
    uint16_t total = 0;
    uint8_t i;
    for(i = 0; i < 255; ++i)  {
        total += array[i];
    }
    return total;
}


uint16_t max(const uint16_t* const array, uint8_t size) {
    uint16_t max = 0;
    uint8_t i;
    for(i = 0; i < size; ++i) {
        if(array[i] > max) {
            max = array[i];
        }
    }
    return  max;
}


uint16_t min(const uint16_t* const array, uint8_t size) {
    uint16_t min = 65535;
    uint8_t i;
    for(i = 0; i < size; ++i) {
         if(array[i] > 0 && array[i] < min) {
            min = array[i];
        }
    }
    return  min;
}

float mean(const uint16_t* const array, uint8_t size) {
    const uint8_t filled_slots = count_filled_slots(array, size);
    return filled_slots > 0 ? ((float)sum(array) / (float)filled_slots) : 0;
}

float standard_deviation(const uint16_t* const array, uint8_t size) {
    const float m = mean(array, size);
    uint8_t i;
    float squared_differences_sum = 0;
    uint8_t elems_count = 0;
    for (i = 0; i < size; ++i) {
        if (array[i] > 0) {
            elems_count++;
            float a = array[i];
            squared_differences_sum += (a - m)*(a - m);
        }
    }
    return (float)sqrt(squared_differences_sum/(elems_count-1));
}

uint8_t last_filled_index(const uint16_t* const array, uint8_t size) {
    uint8_t i;
    uint8_t largest_id_index = 0;
    for(i = 0; i < size; ++i) {
        if(array[i] > 0) {
            largest_id_index = i;
        }
    }
    return largest_id_index;
}

uint8_t count_filled_slots(const uint16_t* const array, uint8_t size) {
    uint8_t total = 0;
    uint8_t i;
    for(i = 0; i < size; ++i) {
        if(array[i] > 0) {
            total += 1;
        }
    }
    return total;
}

uint8_t calculate_smallest_hop_count(const cluster_head_information_t const *cluster_head_list, uint8_t size) {
    uint8_t i;
    uint8_t smallest_hop_count = 255;
    for(i = 0; i < size; ++i) {
        if(cluster_head_list[i].hop_count < smallest_hop_count) {
            smallest_hop_count = cluster_head_list[i].hop_count;
        }
    }
    return smallest_hop_count;
}

ALWAYS_INLINE uint8_t is_valid_cluster_head(const cluster_head_information_t* const cluster_head, uint8_t hop_count_threshold) {
    return cluster_head->hop_count <= hop_count_threshold;
}

uint8_t filter_valid_cluster_heads(const cluster_head_information_t* const cluster_head_list, uint8_t cluster_head_count, cluster_head_information_t* const output, uint8_t threshold) {
    uint8_t i, output_size = 0;
    for(i = 0; i < cluster_head_count; ++i) {
        if(is_valid_cluster_head(&cluster_head_list[i], threshold)) {
            output[output_size++] = cluster_head_list[i];
        }
    }
    return output_size;
}

uint8_t count_valid_cluster_heads(const cluster_head_information_t* const cluster_head_list, uint8_t cluster_head_count, uint8_t threshold) {
    uint8_t i, count = 0;
    for(i = 0; i < cluster_head_count; ++i) {
        count += is_valid_cluster_head(&cluster_head_list[i], threshold);
    }
    return count;
}

node_id_t pick_best_cluster(const cluster_head_information_t *cluster_head_list, uint8_t size) {
    cluster_head_information_t valid_cluster_heads[NODE_LIST_LEN];
    uint8_t smallest_hop_count = calculate_smallest_hop_count(cluster_head_list, size);
    const uint8_t valid_cluster_head_count = filter_valid_cluster_heads(cluster_head_list, size, valid_cluster_heads, smallest_hop_count);

    uint8_t i;
    uint16_t biggest_rx_count = 0;
    node_id_t biggest_rx_id = 0;
    for(i = 0; i < valid_cluster_head_count; ++i) {
        const node_id_t node_id = valid_cluster_heads[i].id;
        const uint16_t curent_rx_count = neighbour_list[node_id];
        if(curent_rx_count > biggest_rx_count) {
            biggest_rx_count = curent_rx_count;
            biggest_rx_id = node_id;
        }
    }
    return biggest_rx_id;
}

// C program for implementation of ftoa()
int ipow(int base, int exp) {
    int result = 1;
    while (exp)
    {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        base *= base;
    }

    return result;
}

// reverses a string 'str' of length 'len'
void reverse(char *str, int len) {
    int i=0, j=len-1, temp;
    while (i<j)
    {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++; j--;
    }
}

 // Converts a given integer x to string str[].  d is the number
 // of digits required in output. If d is more than the number
 // of digits in x, then 0s are added at the beginning.
int intToStr(int x, char str[], int d) {
    int i = 0;
    while (x)
    {
        str[i++] = (x%10) + '0';
        x = x/10;
    }

    // If number of digits required is more, then
    // add 0s at the beginning
    while (i < d)
        str[i++] = '0';

    reverse(str, i);
    str[i] = '\0';
    return i;
}

// Converts a floating point number to string.
void ftoa(float n, char *res, int afterpoint) {
    // Extract integer part
    int ipart = (int)n;

    // Extract floating part
    float fpart = n - (float)ipart;

    // convert integer part to string
    int i = intToStr(ipart, res, 0);

    // check for display option after point
    if (afterpoint != 0)
    {
        res[i] = '.';  // add dot

        // Get the value of fraction part upto given no.
        // of points after dot. The third parameter is needed
        // to handle cases like 233.007
        fpart = fpart * ipow(10, afterpoint);

        intToStr((int)fpart, res + i + 1, afterpoint);
    }
}
