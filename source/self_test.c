/******************************************************************************
 * File Name:   self_test.c
 *
 * Description: Provides I2C (SCB4 host / SCB5 device) initialization and the
 *              Class-B I2C self-test wrapper.
 *
 *              Unlike the UART and SPI SCB self-tests, the I2C self-test
 *              cannot use a single-block hardware loopback. It requires two
 *              separate SCB instances - one configured as I2C host, one as
 *              I2C device - connected by external jumper wires (SDA-SDA,
 *              SCL-SCL). See README.md for the exact jumper wiring required
 *              on KIT_XMC52_EVK.
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
#include "cy_retarget_io.h"
#include "self_test.h"

/*******************************************************************************
* Macros
*******************************************************************************/
/* XMC52 = CAT1C: SCB interrupts must be routed through a NvicMux channel. */
#define INTR_SRC_HOST    ((NvicMux3_IRQn << 16) | CYBSP_DUT_I2C_MASTER_IRQ)
#define INTR_SRC_DEVICE  ((NvicMux2_IRQn << 16) | CYBSP_DUT_I2C_SLAVE_IRQ)

/*******************************************************************************
* Global Variables
*******************************************************************************/
static cy_stc_scb_i2c_context_t i2c_host_context;
static cy_stc_scb_i2c_context_t i2c_device_context;

/* I2C device data buffers (size defined in SelfTest_I2C_SCB.h) */
static uint8_t i2c_device_read_buf[PACKET_SIZE];
static uint8_t i2c_device_write_buf[PACKET_SIZE];

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
static void i2c_host_interrupt(void);
static void i2c_device_interrupt(void);

/*******************************************************************************
* Function Name: i2c_self_test_init
********************************************************************************
* Summary:
*  Initializes SCB4 (I2C host) and SCB5 (I2C device), including the NvicMux
*  IRQ routing required on XMC52 (CAT1C / M4CPUSS_V2). Configures the device
*  read/write buffers used by SelfTest_I2C_SCB().
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void i2c_self_test_init(void)
{
    cy_en_scb_i2c_status_t i2c_res;
    cy_en_sysint_status_t  int_res;

    /* ---- I2C Host (SCB4 / NvicMux3) ---- */
    const cy_stc_sysint_t CYBSP_DUT_I2C_MASTER_IRQ_config =
    {
        .intrSrc      = INTR_SRC_HOST,
        .intrPriority = 3u
    };

    i2c_res = Cy_SCB_I2C_Init(CYBSP_DUT_I2C_MASTER_HW, &CYBSP_DUT_I2C_MASTER_config,
                               &i2c_host_context);
    if (i2c_res != CY_SCB_I2C_SUCCESS)
    {
        CY_ASSERT(0);
    }

    int_res = Cy_SysInt_Init(&CYBSP_DUT_I2C_MASTER_IRQ_config, &i2c_host_interrupt);
    if (int_res != CY_SYSINT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    NVIC_EnableIRQ((IRQn_Type)NvicMux3_IRQn);
    Cy_SCB_I2C_Enable(CYBSP_DUT_I2C_MASTER_HW);

    /* ---- I2C Device (SCB5 / NvicMux2) ---- */
    const cy_stc_sysint_t CYBSP_DUT_I2C_SLAVE_IRQ_config =
    {
        .intrSrc      = INTR_SRC_DEVICE,
        .intrPriority = 3u
    };

    i2c_res = Cy_SCB_I2C_Init(CYBSP_DUT_I2C_SLAVE_HW, &CYBSP_DUT_I2C_SLAVE_config,
                               &i2c_device_context);
    if (i2c_res != CY_SCB_I2C_SUCCESS)
    {
        CY_ASSERT(0);
    }

    Cy_SCB_I2C_SlaveConfigReadBuf(CYBSP_DUT_I2C_SLAVE_HW, i2c_device_read_buf,
                                   PACKET_SIZE, &i2c_device_context);
    Cy_SCB_I2C_SlaveConfigWriteBuf(CYBSP_DUT_I2C_SLAVE_HW, i2c_device_write_buf,
                                    PACKET_SIZE, &i2c_device_context);

    int_res = Cy_SysInt_Init(&CYBSP_DUT_I2C_SLAVE_IRQ_config, &i2c_device_interrupt);
    if (int_res != CY_SYSINT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    NVIC_EnableIRQ((IRQn_Type)NvicMux2_IRQn);
    Cy_SCB_I2C_Enable(CYBSP_DUT_I2C_SLAVE_HW);

    printf("I2C host (SCB4) and device (SCB5) initialized.\r\n\n");
}

/*******************************************************************************
* Function Name: i2c_self_test
********************************************************************************
* Summary:
*  Runs one iteration of the I2C SCB self-test. The caller repeats this
*  function while PASS_STILL_TESTING_STATUS is returned; SelfTest_I2C_SCB()
*  advances the test byte-by-byte and returns PASS_COMPLETE_STATUS once the
*  full 0x00-0xFF range has been verified between the host and the device.
*
* Parameters:
*  void
*
* Return:
*  PASS_STILL_TESTING_STATUS - test in progress, call again
*  PASS_COMPLETE_STATUS      - full byte range verified, test passed
*  ERROR_STATUS or other     - test failure (see SelfTest_common.h)
*
*******************************************************************************/
uint8_t i2c_self_test(void)
{
    return SelfTest_I2C_SCB(CYBSP_DUT_I2C_MASTER_HW, &i2c_host_context,
                             CYBSP_DUT_I2C_SLAVE_HW,  &i2c_device_context,
                             i2c_device_read_buf, i2c_device_write_buf);
}

/*******************************************************************************
* Function Name: i2c_host_interrupt
*******************************************************************************/
static void i2c_host_interrupt(void)
{
    Cy_SCB_I2C_MasterInterrupt(CYBSP_DUT_I2C_MASTER_HW, &i2c_host_context);
}

/*******************************************************************************
* Function Name: i2c_device_interrupt
*******************************************************************************/
static void i2c_device_interrupt(void)
{
    Cy_SCB_I2C_SlaveInterrupt(CYBSP_DUT_I2C_SLAVE_HW, &i2c_device_context);
}

/* [] END OF FILE */
