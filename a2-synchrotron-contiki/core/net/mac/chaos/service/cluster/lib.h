#ifndef _CHAOS_LIB_H_
#define _CHAOS_LIB_H_

uint16_t sum(const uint16_t* const array, uint8_t size);
uint16_t max(const uint16_t* const array, uint8_t size);
uint16_t min(const uint16_t* const array, uint8_t size);
float mean(const uint16_t* const array, uint8_t size);
float standard_deviation(const uint16_t* const array, uint8_t size);

uint8_t last_filled_index(const uint16_t* const array, uint8_t size);
uint8_t count_filled_slots(const uint16_t* const array, uint8_t size);

int ipow(int base, int exp);
void reverse(char *str, int len);
int intToStr(int x, char str[], int d);
void ftoa(float n, char *res, int afterpoint);

#endif /* _CHAOS_LIB_H_ */
