/******************************************************************************
 * File Name:   main.c
 *
 * Description: This is the source code for the MTB STL SCB I2C Safety Test
 *              for XMC5000 MCUs.
 *
 *              Configures SCB4 as I2C host and SCB5 as I2C device, runs the
 *              I2C self-test once at startup using the SelfTest_I2C_SCB()
 *              API, prints the result, and halts.
 *
 *              Unlike UART/SPI, the I2C self-test requires external jumper
 *              wires between the host and device SCB pins - see README.md
 *              for the exact hardware setup on KIT_XMC52_EVK.
 *
 * Related Document: See README.md
 *
 *
 *******************************************************************************
 * $ Copyright 2026 Infineon Technologies AG $
 *******************************************************************************/

/*******************************************************************************
* Header Files
*******************************************************************************/
#include "cybsp.h"
#include "cy_pdl.h"
#include "mtb_hal.h"
#include "cy_retarget_io.h"
#include "self_test.h"

/*******************************************************************************
* Global Variables
*******************************************************************************/
/* For the Retarget-IO (Debug UART) usage */
static cy_stc_scb_uart_context_t  DEBUG_UART_context;   /* Debug UART context */
static mtb_hal_uart_t DEBUG_UART_hal_obj;               /* Debug UART HAL object */

/*******************************************************************************
* Function Definitions
*******************************************************************************/

/*******************************************************************************
* Function Name: main
*********************************************************************************
* Summary:
* This is the main function. It does...
*    1. Initialize the device and board peripherals and retarget-io for prints
*    2. Initialize the DUT I2C host (SCB4) and device (SCB5)
*    3. Run the I2C self-test to completion
*    4. Print PASS/FAIL result and halt
*
* Parameters:
*  void
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;
    uint8_t ret;

    /* Initialize the device and board peripherals */
    result = cybsp_init();
    /* Board init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize the debug UART */
    result = Cy_SCB_UART_Init(CYBSP_DEBUG_UART_HW, &CYBSP_DEBUG_UART_config, &DEBUG_UART_context);
    /* UART init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }
    Cy_SCB_UART_Enable(CYBSP_DEBUG_UART_HW);

    /* Setup the HAL UART */
    result = mtb_hal_uart_setup(&DEBUG_UART_hal_obj, &CYBSP_DEBUG_UART_hal_config, &DEBUG_UART_context, NULL);
    /* HAL UART init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize retarget-io to use the debug UART port */
    result = cy_retarget_io_init(&DEBUG_UART_hal_obj);
    /* retarget-io init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Enable global interrupts */
    __enable_irq();

    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");

    printf("****************** "
           "MTB STL: SCB I2C Safety Test"
           " ******************\r\n\n");

    /* Initialize DUT I2C host (SCB4) and device (SCB5) */
    i2c_self_test_init();

    /* Run the I2C self-test - SelfTest_I2C_SCB() advances the test
     * byte-by-byte on each call and returns PASS_STILL_TESTING_STATUS until
     * the full 0x00-0xFF range has been verified. */
    do
    {
        ret = i2c_self_test();
    } while (ret == PASS_STILL_TESTING_STATUS);

    if (ret == PASS_COMPLETE_STATUS)
    {
        printf("SUCCESS: I2C SCB test passed.\r\n");
    }
    else
    {
        printf("ERROR: I2C SCB test failed (status=%d).\r\n", (int)ret);
    }

    printf("\r\nTest complete.\r\n");

    /* Halt - test is done */
    for (;;)
    {
    }
}

/* [] END OF FILE */

