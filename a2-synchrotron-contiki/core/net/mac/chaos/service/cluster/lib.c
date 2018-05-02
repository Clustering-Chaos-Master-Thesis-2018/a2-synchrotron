#include<stdio.h>
#include<math.h>

#include "lib.h"

#include "contiki.h"

uint16_t sum(const uint16_t* const array, uint8_t size) {
    uint16_t total = 0;
    uint8_t i;
    for(i = 0; i < size; ++i) {
        total += array[i];
    }
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

ALWAYS_ACTUALLY_INLINE int16_t index_of(const cluster_head_information_t *array, uint8_t size, node_id_t value) {
    uint8_t i;
    for(i = 0; i < size; ++i) {
        if (array[i].id == value) {
            return i;
        }
    }
    return -1;
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
