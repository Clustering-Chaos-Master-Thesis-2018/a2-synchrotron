#include<stdio.h>
#include<math.h>

#include "lib.h"

#include "contiki.h"

uint16_t sum(const uint16_t* const array) {
    uint16_t total = 0;
    total = array[0] +
            array[1] +
            array[2] +
            array[3] +
            array[4] +
            array[5] +
            array[6] +
            array[7] +
            array[8] +
            array[9] +
            array[10] +
            array[11] +
            array[12] +
            array[13] +
            array[14] +
            array[15] +
            array[16] +
            array[17] +
            array[18] +
            array[19] +
            array[20] +
            array[21] +
            array[22] +
            array[23] +
            array[24] +
            array[25] +
            array[26] +
            array[27] +
            array[28] +
            array[29] +
            array[30] +
            array[31] +
            array[32] +
            array[33] +
            array[34] +
            array[35] +
            array[36] +
            array[37] +
            array[38] +
            array[39] +
            array[40] +
            array[41] +
            array[42] +
            array[43] +
            array[44] +
            array[45] +
            array[46] +
            array[47] +
            array[48] +
            array[49] +
            array[50] +
            array[51] +
            array[52] +
            array[53] +
            array[54] +
            array[55] +
            array[56] +
            array[57] +
            array[58] +
            array[59] +
            array[60] +
            array[61] +
            array[62] +
            array[63] +
            array[64] +
            array[65] +
            array[66] +
            array[67] +
            array[68] +
            array[69] +
            array[70] +
            array[71] +
            array[72] +
            array[73] +
            array[74] +
            array[75] +
            array[76] +
            array[77] +
            array[78] +
            array[79] +
            array[80] +
            array[81] +
            array[82] +
            array[83] +
            array[84] +
            array[85] +
            array[86] +
            array[87] +
            array[88] +
            array[89] +
            array[90] +
            array[91] +
            array[92] +
            array[93] +
            array[94] +
            array[95] +
            array[96] +
            array[97] +
            array[98] +
            array[99] +
            array[100] +
            array[101] +
            array[102] +
            array[103] +
            array[104] +
            array[105] +
            array[106] +
            array[107] +
            array[108] +
            array[109] +
            array[110] +
            array[111] +
            array[112] +
            array[113] +
            array[114] +
            array[115] +
            array[116] +
            array[117] +
            array[118] +
            array[119] +
            array[120] +
            array[121] +
            array[122] +
            array[123] +
            array[124] +
            array[125] +
            array[126] +
            array[127] +
            array[128] +
            array[129] +
            array[130] +
            array[131] +
            array[132] +
            array[133] +
            array[134] +
            array[135] +
            array[136] +
            array[137] +
            array[138] +
            array[139] +
            array[140] +
            array[141] +
            array[142] +
            array[143] +
            array[144] +
            array[145] +
            array[146] +
            array[147] +
            array[148] +
            array[149] +
            array[150] +
            array[151] +
            array[152] +
            array[153] +
            array[154] +
            array[155] +
            array[156] +
            array[157] +
            array[158] +
            array[159] +
            array[160] +
            array[161] +
            array[162] +
            array[163] +
            array[164] +
            array[165] +
            array[166] +
            array[167] +
            array[168] +
            array[169] +
            array[170] +
            array[171] +
            array[172] +
            array[173] +
            array[174] +
            array[175] +
            array[176] +
            array[177] +
            array[178] +
            array[179] +
            array[180] +
            array[181] +
            array[182] +
            array[183] +
            array[184] +
            array[185] +
            array[186] +
            array[187] +
            array[188] +
            array[189] +
            array[190] +
            array[191] +
            array[192] +
            array[193] +
            array[194] +
            array[195] +
            array[196] +
            array[197] +
            array[198] +
            array[199] +
            array[200] +
            array[201] +
            array[202] +
            array[203] +
            array[204] +
            array[205] +
            array[206] +
            array[207] +
            array[208] +
            array[209] +
            array[210] +
            array[211] +
            array[212] +
            array[213] +
            array[214] +
            array[215] +
            array[216] +
            array[217] +
            array[218] +
            array[219] +
            array[220] +
            array[221] +
            array[222] +
            array[223] +
            array[224] +
            array[225] +
            array[226] +
            array[227] +
            array[228] +
            array[229] +
            array[230] +
            array[231] +
            array[232] +
            array[233] +
            array[234] +
            array[235] +
            array[236] +
            array[237] +
            array[238] +
            array[239] +
            array[240] +
            array[241] +
            array[242] +
            array[243] +
            array[244] +
            array[245] +
            array[246] +
            array[247] +
            array[248] +
            array[249] +
            array[250] +
            array[251] +
            array[252] +
            array[253] +
            array[254];

    return  total;
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

uint8_t filter_valid_cluster_heads(const cluster_head_information_t* const cluster_head_list, uint8_t cluster_head_count, cluster_head_information_t* const output, uint8_t threshold) {
    uint8_t i, output_size = 0;
    for(i = 0; i < cluster_head_count; ++i) {
        if(cluster_head_list[i].hop_count <= threshold) {
            output[output_size++] = cluster_head_list[i];
        }
    }
    return output_size;
}

node_id_t pick_best_cluster(const cluster_head_information_t *cluster_head_list, uint8_t size) {
    cluster_head_information_t valid_cluster_heads[NODE_LIST_LEN];
    uint8_t smallest_hop_count = calculate_smallest_hop_count(cluster_head_list, size);
    const uint8_t valid_cluster_head_count = filter_valid_cluster_heads(cluster_head_list, size, valid_cluster_heads, smallest_hop_count);;

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
