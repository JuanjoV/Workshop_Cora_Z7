#ifndef DOT_PRODUCT_CONFIG_H_
#define DOT_PRODUCT_CONFIG_H_

#define VECTOR_SIZE      10

#include "ap_int.h"
//#include "ap_fixed.h"

typedef ap_int<12> data_t;
typedef ap_int<32> o_data_t;

void dot_product_hls_main(data_t x[VECTOR_SIZE], data_t y[VECTOR_SIZE], o_data_t *result);

#endif