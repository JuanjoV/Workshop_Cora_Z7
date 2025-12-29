
#include "dot_product_config.h"

void dot_product_hls_main(data_t x[VECTOR_SIZE], data_t y[VECTOR_SIZE], o_data_t *result) {
    #pragma HLS INTERFACE port=x mode=s_axilite
    #pragma HLS INTERFACE port=y mode=s_axilite
    #pragma HLS INTERFACE port=result mode=s_axilite
    #pragma HLS INTERFACE port=return mode=s_axilite

    #pragma HLS ARRAY_PARTITION variable=x dim=1 type=complete
    #pragma HLS ARRAY_PARTITION variable=y dim=1 type=complete

    o_data_t temp = 0;

    ADD_LOOP : for (int i = 0; i < VECTOR_SIZE; i++) {
        #pragma HLS UNROLL
        //#pragma HLS PIPELINE II=6
        temp += x[i] * y[i];
    }
    *result = temp;
}