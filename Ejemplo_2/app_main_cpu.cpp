#include "xparameters.h"
#include "xil_exception.h"
#include <iomanip>
#include <iostream>
#include <xil_types.h>
#include <xstatus.h>
#include "xinterrupt_wrap.h"
#include "xscutimer.h"

#define VECTOR_SIZE     10          
#define BITSIZE_INPUT   12
#define BITSIZE_OUTPUT  32
#define N_TESTS         100

typedef int32_t   data_t;
typedef int32_t  o_data_t;

XScuTimer Timer;


void get_vector_from_host(data_t vec[VECTOR_SIZE])
{
    float temp;
    for (int i=0; i< VECTOR_SIZE; i++)
    {
        std::cin >> temp;
        vec[i] = (data_t) temp;
    }
}

void dot_product_cpu(data_t X[VECTOR_SIZE], data_t Y[VECTOR_SIZE], o_data_t* result)
{
    o_data_t sum = 0;
    for (int i = 0; i < VECTOR_SIZE; i++) 
    {
        sum += X[i] * Y[i];
    }
    *result = sum;
}

int main()
{
    int status;

    data_t vec_x [VECTOR_SIZE];
    data_t vec_y [VECTOR_SIZE];
    o_data_t cpu_result;

    XScuTimer_Config *ConfigPtr;

    ConfigPtr = XScuTimer_LookupConfig(XPAR_SCUTIMER_BASEADDR);

    status = XScuTimer_CfgInitialize(&Timer, ConfigPtr, ConfigPtr->BaseAddr);
    if (status != XST_SUCCESS) return XST_FAILURE;

    std::cout << "Running! ..." << std::endl;

    for(int i = 0; i < N_TESTS; i++)
    {

        get_vector_from_host(vec_x);
        get_vector_from_host(vec_y);

        XScuTimer_LoadTimer(&Timer, 0xFFFFFFFF);
        XScuTimer_Start(&Timer);
        dot_product_cpu(vec_x, vec_y, &cpu_result);
        XScuTimer_Stop(&Timer);
        u32 elapsed_time = 0xFFFFFFFF - XScuTimer_GetCounterValue(&Timer);

        std::cout << "Result: " << cpu_result << std::endl;
        std::cout << "Elapsed time: " << elapsed_time << " sw ticks" << std::endl;
        

    }

    return 0;
}
