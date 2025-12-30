
#include "xparameters.h"
#include "xil_exception.h"
#include <iomanip>
#include <iostream>
#include <xil_types.h>
#include <xstatus.h>
#include "xinterrupt_wrap.h"
#include "xscutimer.h"

#include "ap_int.h"

#include "xdot_product_hls_main.h"

#define HLS_IP_BASEADDRESS 		XPAR_DOT_PRODUCT_HLS_MAIN_0_BASEADDR
#define HLS_INTERRUPTS 			XPAR_DOT_PRODUCT_HLS_MAIN_0_INTERRUPTS
#define HLS_INTERRUPT_PARENT	XPAR_DOT_PRODUCT_HLS_MAIN_0_INTERRUPT_PARENT

#define VECTOR_SIZE     10          
#define BITSIZE_INPUT   12
#define BITSIZE_OUTPUT  32
#define N_TESTS         100

typedef ap_int<BITSIZE_INPUT>   data_t;
typedef ap_int<BITSIZE_OUTPUT>  o_data_t;

volatile int ip_status;
XDot_product_hls_main ip;
XScuTimer Timer;

enum IP_ready
{
    IP_READY,
    IP_BUSY
};

void IPHandler(void *InstPtr)
{
    XScuTimer_Stop(&Timer);
    u32 elapsed_time = 0xFFFFFFFF - XScuTimer_GetCounterValue(&Timer);
    u32 result_raw;
    o_data_t dut_result;
    XDot_product_hls_main_InterruptDisable(&ip, 1);

    result_raw = XDot_product_hls_main_Get_result(&ip);

    dut_result = *((o_data_t *) &result_raw);

    std::cout << "Result: " << std::fixed<< std::setprecision(10) << dut_result << std::endl;
    std::cout << "Elapsed time: " << elapsed_time << " sw ticks" << std::endl;

    ip_status = IP_READY;
    XDot_product_hls_main_InterruptClear(&ip,1);
    XDot_product_hls_main_InterruptEnable(&ip,1);
}

void get_vector_from_host(u32 vec[VECTOR_SIZE])
{
    float temp;
    for (int i=0; i< VECTOR_SIZE; i++)
    {
        std::cin >> temp;
        vec[i] = (data_t) temp;
    }
}

int main()
{
    int status;

    u32 vec_x [VECTOR_SIZE];
    u32 vec_y [VECTOR_SIZE];

    std::cout << "Running! ..." << std::endl;

    status = XDot_product_hls_main_Initialize(&ip, HLS_IP_BASEADDRESS);
    if (status != XST_SUCCESS){
        std::cout << "IP Initialization failed!" << std::endl;
        return XST_FAILURE;
    }

    XScuTimer_Config *ConfigPtr;

    ConfigPtr = XScuTimer_LookupConfig(XPAR_SCUTIMER_BASEADDR);

    status = XScuTimer_CfgInitialize(&Timer, ConfigPtr, ConfigPtr->BaseAddr);
    if (status != XST_SUCCESS) return XST_FAILURE;

    XSetupInterruptSystem(&ip, (void *) &IPHandler, HLS_INTERRUPTS, HLS_INTERRUPT_PARENT, XINTERRUPT_DEFAULT_PRIORITY);
    XDot_product_hls_main_InterruptGlobalEnable(&ip);
    XDot_product_hls_main_InterruptEnable(&ip, 1);

    ip_status = IP_READY;

    for(int i = 0; i < N_TESTS; i++)
    {
        while (ip_status == IP_BUSY) {};

        get_vector_from_host(vec_x);
        get_vector_from_host(vec_y);

        for (int j = 0; j < VECTOR_SIZE; j++)
        {
            vec_x[j] = vec_x[j] & ((1 << BITSIZE_INPUT) - 1);
            vec_y[j] = vec_y[j] & ((1 << BITSIZE_INPUT) - 1);
            XDot_product_hls_main_WriteReg(ip.Control_BaseAddress, XDOT_PRODUCT_HLS_MAIN_CONTROL_ADDR_X_0_DATA +j*8, vec_x[j]);
            XDot_product_hls_main_WriteReg(ip.Control_BaseAddress, XDOT_PRODUCT_HLS_MAIN_CONTROL_ADDR_Y_0_DATA +j*8, vec_y[j]);
        }
        XScuTimer_LoadTimer(&Timer, 0xFFFFFFFF);
        ip_status = IP_BUSY;
        XScuTimer_Start(&Timer);
        XDot_product_hls_main_Start(&ip);
    }

    return 0;
}
