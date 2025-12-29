
#include "xparameters.h"
#include "xgpio.h"
#include "xil_exception.h"
#include "xil_printf.h"
#include <xil_types.h>
#include <xstatus.h>

#include "xinterrupt_wrap.h"


// Parameter definitions

#define LEDS_BASEADDRESS	XPAR_XGPIO_0_BASEADDR
#define BTNS_BASEADDRESS	XPAR_XGPIO_0_BASEADDR
#define BTNS_INT_ID			XPAR_AXI_GPIO_0_INTERRUPTS
#define BTNS_INT_PARENT		XPAR_AXI_GPIO_0_INTERRUPT_PARENT

#define BTN_CHANNEL			1
#define LED_CHANNEL			2

#define BTN_MASK			XGPIO_IR_CH1_MASK

XGpio LEDInst, BTNInst;

static int led_data;
static int btn_value;

//----------------------------------------------------
// PROTOTYPE FUNCTIONS
//----------------------------------------------------
static void BTN_Intr_Handler(void *baseaddr_p);


static void BTN_Intr_Handler(void *baseaddr_p)
{
	// Disable GPIO interrupts
	XGpio_InterruptDisable(&BTNInst, BTN_MASK);
	// Ignore additional button presses
	if ((XGpio_InterruptGetStatus(&BTNInst) & BTN_MASK) !=
			BTN_MASK) {
		/* Clean GPIO interrupts */
		XGpio_InterruptClear(&BTNInst, BTN_MASK);
		// Enable GPIO interrupts
		XGpio_InterruptEnable(&BTNInst, BTN_MASK);
		return;
	}
	if (!XGpio_DiscreteRead(&BTNInst, BTN_CHANNEL))
	{
		/* Clean GPIO interrupts */
		XGpio_InterruptClear(&BTNInst, BTN_MASK);
		// Enable GPIO interrupts
		XGpio_InterruptEnable(&BTNInst, BTN_MASK);
		return;
	}
	/* Read button value*/
	btn_value = 2* XGpio_DiscreteRead(&BTNInst, BTN_CHANNEL) - 3;

	// Increment counter based on button value
	led_data = led_data + btn_value;
	xil_printf("led_data 0x%x \t - btn_value %x \n", led_data, btn_value);

	/* Write led_data into LEDs */
    XGpio_DiscreteWrite(&LEDInst, LED_CHANNEL, led_data);

	/* Clean GPIO interrupts */
    XGpio_InterruptClear(&BTNInst, BTN_MASK);
    // Enable GPIO interrupts
    XGpio_InterruptEnable(&BTNInst, BTN_MASK);
}

//----------------------------------------------------
// MAIN FUNCTION
//----------------------------------------------------
int main (void)
{
	int status;
	/* Initialize value used to represent leds*/
	led_data = 0;
	//----------------------------------------------------
	// INITIALIZE THE PERIPHERALS & SET DIRECTIONS OF GPIO
	//----------------------------------------------------
	/* Initialize leds */
	status = XGpio_Initialize(&LEDInst, LEDS_BASEADDRESS);
	if(status != XST_SUCCESS) return XST_FAILURE;
	/* Initialize buttons */
	status = XGpio_Initialize(&BTNInst, BTNS_BASEADDRESS);
	if(status != XST_SUCCESS) return XST_FAILURE;
	
	// Set LEDs direction to outputs
	XGpio_SetDataDirection(&LEDInst, LED_CHANNEL, 0x00);
	// Set all buttons direction to inputs
	XGpio_SetDataDirection(&BTNInst, BTN_CHANNEL, 0xFF);

	/* Write ones to LEDs to make them blink*/
	XGpio_DiscreteWrite(&LEDInst, LED_CHANNEL, 0xFF);

	/* Initialize interrupt from buttons */
	XGpio_Config *ConfigPtr;
	ConfigPtr = XGpio_LookupConfig(XPAR_AXI_GPIO_0_BASEADDR);
	ConfigPtr->IntrId = (ConfigPtr->IntrId & (~0x0000F000)) | (2 << 12);
	XSetupInterruptSystem(&BTNInst, &BTN_Intr_Handler,ConfigPtr->IntrId, ConfigPtr->IntrParent, XINTERRUPT_DEFAULT_PRIORITY);

	/* Enable GPIO interrupt*/
	XGpio_InterruptEnable(&BTNInst, BTN_MASK);
	XGpio_InterruptGlobalEnable(&BTNInst);
	
	/* Turn off LEDs */
	XGpio_DiscreteWrite(&LEDInst, LED_CHANNEL, 0x00);

	xil_printf("\nRunning...\n");

	while(1);
	/* Unreachable return*/
	return 0;
}
