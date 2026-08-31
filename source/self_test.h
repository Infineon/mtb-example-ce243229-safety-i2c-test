/******************************************************************************
 * File Name:   self_test.h
 *
 * Description: This file is the public interface of self_test.c source file.
 *
 * Related Document: See README.md
 *
 *
 *******************************************************************************
 * $ Copyright 2026 Infineon Technologies AG $
 *******************************************************************************/

#ifndef SELF_TEST_H_
#define SELF_TEST_H_

/*******************************************************************************
* Header Files
*******************************************************************************/
#include "cybsp.h"
#include "cy_pdl.h"
#include "SelfTest_I2C_SCB.h"

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
void    i2c_self_test_init(void);
uint8_t i2c_self_test(void);

#endif /* SELF_TEST_H_ */

/* [] END OF FILE */
