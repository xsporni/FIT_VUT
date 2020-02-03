/*
 * The Clear BSD License
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 * that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*  SDK Included Files */
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_i2c.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_gpio.h"
#include "fsl_port.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define ACCEL_I2C_CLK_SRC I2C1_CLK_SRC
#define ACCEL_I2C_CLK_FREQ CLOCK_GetFreq(I2C1_CLK_SRC)

#define I2C_RELEASE_SDA_PORT PORTD
#define I2C_RELEASE_SCL_PORT PORTD
#define I2C_RELEASE_SDA_GPIO GPIOD
#define I2C_RELEASE_SDA_PIN 6U
#define I2C_RELEASE_SCL_GPIO GPIOD
#define I2C_RELEASE_SCL_PIN 7U
#define I2C_RELEASE_BUS_COUNT 100U
#define I2C_BAUDRATE 100000U
#define ACCEL_STATUS 0x00U
#define ACCEL_XYZ_DATA_CFG 0x0EU
#define ACCEL_CTRL_REG1 0x2AU
#define ACCEL_READ_TIMES 10U

#define DEVICE_ADDR 0x1D

#define SAMPLES 50
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_I2C_ReleaseBus(void);

static bool I2C_WriteAccelReg(I2C_Type *base, uint8_t device_addr, uint8_t reg_addr, uint8_t value);
static bool I2C_ReadAccelRegs(I2C_Type *base, uint8_t device_addr, uint8_t reg_addr, uint8_t *rxBuff, uint32_t rxSize);

/*******************************************************************************
 * Variables
 ******************************************************************************/

i2c_master_handle_t g_m_handle;

volatile bool completionFlag = false;
volatile bool nakFlag = false;

/*******************************************************************************
 * Code
 ******************************************************************************/

static void i2c_release_bus_delay(void)
{
    uint32_t i = 0;
    for (i = 0; i < I2C_RELEASE_BUS_COUNT; i++)
    {
        __NOP();
    }
}

void BOARD_I2C_ReleaseBus(void)
{
    uint8_t i = 0;
    gpio_pin_config_t pin_config;
    port_pin_config_t i2c_pin_config = {0};

    /* Config pin mux as gpio */
    i2c_pin_config.pullSelect = kPORT_PullUp;
    i2c_pin_config.mux = kPORT_MuxAsGpio;

    pin_config.pinDirection = kGPIO_DigitalOutput;
    pin_config.outputLogic = 1U;
    CLOCK_EnableClock(kCLOCK_PortD);
    PORT_SetPinConfig(I2C_RELEASE_SCL_PORT, I2C_RELEASE_SCL_PIN, &i2c_pin_config);
    PORT_SetPinConfig(I2C_RELEASE_SCL_PORT, I2C_RELEASE_SDA_PIN, &i2c_pin_config);

    GPIO_PinInit(I2C_RELEASE_SCL_GPIO, I2C_RELEASE_SCL_PIN, &pin_config);
    GPIO_PinInit(I2C_RELEASE_SDA_GPIO, I2C_RELEASE_SDA_PIN, &pin_config);

    /* Drive SDA low first to simulate a start */
    GPIO_PinWrite(I2C_RELEASE_SDA_GPIO, I2C_RELEASE_SDA_PIN, 0U);
    i2c_release_bus_delay();

    /* Send 9 pulses on SCL and keep SDA high */
    for (i = 0; i < 9; i++)
    {
        GPIO_PinWrite(I2C_RELEASE_SCL_GPIO, I2C_RELEASE_SCL_PIN, 0U);
        i2c_release_bus_delay();

        GPIO_PinWrite(I2C_RELEASE_SDA_GPIO, I2C_RELEASE_SDA_PIN, 1U);
        i2c_release_bus_delay();

        GPIO_PinWrite(I2C_RELEASE_SCL_GPIO, I2C_RELEASE_SCL_PIN, 1U);
        i2c_release_bus_delay();
        i2c_release_bus_delay();
    }

    /* Send stop */
    GPIO_PinWrite(I2C_RELEASE_SCL_GPIO, I2C_RELEASE_SCL_PIN, 0U);
    i2c_release_bus_delay();

    GPIO_PinWrite(I2C_RELEASE_SDA_GPIO, I2C_RELEASE_SDA_PIN, 0U);
    i2c_release_bus_delay();

    GPIO_PinWrite(I2C_RELEASE_SCL_GPIO, I2C_RELEASE_SCL_PIN, 1U);
    i2c_release_bus_delay();

    GPIO_PinWrite(I2C_RELEASE_SDA_GPIO, I2C_RELEASE_SDA_PIN, 1U);
    i2c_release_bus_delay();
}

static void i2c_master_callback(I2C_Type *base, i2c_master_handle_t *handle, status_t status, void *userData)
{
    /* Signal transfer success when received success status. */
    if (status == kStatus_Success)
    {
        completionFlag = true;
    }
    /* Signal transfer success when received success status. */
    if ((status == kStatus_I2C_Nak) || (status == kStatus_I2C_Addr_Nak))
    {
        nakFlag = true;
    }
}

static bool I2C_WriteAccelReg(I2C_Type *base, uint8_t device_addr, uint8_t reg_addr, uint8_t value)
{
    i2c_master_transfer_t masterXfer;
    memset(&masterXfer, 0, sizeof(masterXfer));

    masterXfer.slaveAddress = device_addr;
    masterXfer.direction = kI2C_Write;
    masterXfer.subaddress = reg_addr;
    masterXfer.subaddressSize = 1;
    masterXfer.data = &value;
    masterXfer.dataSize = 1;
    masterXfer.flags = kI2C_TransferDefaultFlag;

    /*  direction=write : start+device_write;cmdbuff;xBuff; */
    /*  direction=recive : start+device_write;cmdbuff;repeatStart+device_read;xBuff; */

    I2C_MasterTransferNonBlocking(base, &g_m_handle, &masterXfer);

    /*  wait for transfer completed. */
    while ((!nakFlag) && (!completionFlag))
    {
    }

    nakFlag = false;

    if (completionFlag == true)
    {
        completionFlag = false;
        return true;
    }
    else
    {
        return false;
    }
}

static bool I2C_ReadAccelRegs(I2C_Type *base, uint8_t device_addr, uint8_t reg_addr, uint8_t *rxBuff, uint32_t rxSize)
{
    /* Ukol prvni: Tuto funkci doplnte */
		i2c_master_transfer_t masterXfer;
		memset(&masterXfer, 0, sizeof(masterXfer));

		masterXfer.slaveAddress = device_addr;
		masterXfer.direction = kI2C_Read;
		masterXfer.subaddress = reg_addr;
		masterXfer.subaddressSize = 1;
		masterXfer.data = rxBuff;
		masterXfer.dataSize = rxSize;
		masterXfer.flags = kI2C_TransferDefaultFlag;
		I2C_MasterTransferBlocking(base, &masterXfer);


    return true;
}

static void LEDs_Off()
{
    GPIO_PinWrite(LED1_GPIO, LED1_PIN, 0U);
    GPIO_PinWrite(LED2_GPIO, LED2_PIN, 0U);
    GPIO_PinWrite(LED3_GPIO, LED3_PIN, 0U);
    GPIO_PinWrite(LED4_GPIO, LED4_PIN, 0U);
    GPIO_PinWrite(LED5_GPIO, LED5_PIN, 0U);
    GPIO_PinWrite(LED6_GPIO, LED6_PIN, 0U);
    GPIO_PinWrite(LED7_GPIO, LED7_PIN, 0U);
    GPIO_PinWrite(LED8_GPIO, LED8_PIN, 0U);
    GPIO_PinWrite(LED9_GPIO, LED9_PIN, 0U);
    GPIO_PinWrite(LED10_GPIO, LED10_PIN, 0U);
    GPIO_PinWrite(LED11_GPIO, LED11_PIN, 0U);
    GPIO_PinWrite(LED12_GPIO, LED12_PIN, 0U);
    GPIO_PinWrite(LED13_GPIO, LED13_PIN, 0U);
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Init variables */
    uint8_t databyte = 0;
    uint8_t write_reg = 0;
    uint8_t readBuff[7];
    int16_t x, y, z;
    int32_t x_filt, y_filt, z_filt;
    int16_t x_buff[SAMPLES];
    int16_t y_buff[SAMPLES];
    int16_t z_buff[SAMPLES];

    uint8_t status0_value = 0;
    uint32_t i, next_avg = 0;

    i2c_master_config_t masterConfig;

    /* Board init */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_I2C_ReleaseBus();
    BOARD_I2C_ConfigurePins();
    BOARD_InitDebugConsole();

    PRINTF("\r\nI2C example -- Read Accelerometer Value\r\n");

    /* Create I2C Handler */
    I2C_MasterTransferCreateHandle(BOARD_ACCEL_I2C_BASEADDR, &g_m_handle, i2c_master_callback, NULL);

    /* masterConfig.baudRate_Bps = 100000U;
     * masterConfig.enableStopHold = false;
     * masterConfig.glitchFilterWidth = 0U;
     * masterConfig.enableMaster = true;
     */
    I2C_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate_Bps = I2C_BAUDRATE;
    /* Init I2C master side */
    I2C_MasterInit(BOARD_ACCEL_I2C_BASEADDR, &masterConfig, ACCEL_I2C_CLK_FREQ);

    /*  please refer to the "example FXOS8700CQ Driver Code" in FXOS8700 datasheet. */
    /*  write 0000 0000 = 0x00 to accelerometer control register 1 */
    /*  standby */
    /*  [7-1] = 0000 000 */
    /*  [0]: active=0 */
    write_reg = ACCEL_CTRL_REG1;
    databyte = 0;
    I2C_WriteAccelReg(BOARD_ACCEL_I2C_BASEADDR, DEVICE_ADDR, write_reg, databyte);

    /*  write 0000 0001= 0x01 to XYZ_DATA_CFG register */
    /*  [7]: reserved */
    /*  [6]: reserved */
    /*  [5]: reserved */
    /*  [4]: hpf_out=0 */
    /*  [3]: reserved */
    /*  [2]: reserved */
    /*  [1-0]: fs=01 for accelerometer range of +/-4g range with 0.488mg/LSB */
    /*  databyte = 0x01; */
    write_reg = ACCEL_XYZ_DATA_CFG;
    databyte = 0x01;
    I2C_WriteAccelReg(BOARD_ACCEL_I2C_BASEADDR, DEVICE_ADDR, write_reg, databyte);

    /*  write 0000 1101 = 0x0D to accelerometer control register 1 */
    /*  [7-6]: aslp_rate=00 */
    /*  [5-3]: dr=001 for 200Hz data rate (when in hybrid mode) */
    /*  [2]: lnoise=1 for low noise mode */
    /*  [1]: f_read=0 for normal 16 bit reads */
    /*  [0]: active=1 to take the part out of standby and enable sampling */
    /*   databyte = 0x0D; */
    write_reg = ACCEL_CTRL_REG1;
    databyte = 0x0d;
    I2C_WriteAccelReg(BOARD_ACCEL_I2C_BASEADDR, DEVICE_ADDR, write_reg, databyte);
    PRINTF("The accel values:\r\n");

    while (1)
    {
        status0_value = 0;
        /*  wait for new data are ready. */
        while (status0_value != 0xff)
        {
            I2C_ReadAccelRegs(BOARD_ACCEL_I2C_BASEADDR, DEVICE_ADDR, ACCEL_STATUS, &status0_value, 1);
        }

        /*  Multiple-byte Read from STATUS (0x00) register */
        I2C_ReadAccelRegs(BOARD_ACCEL_I2C_BASEADDR, DEVICE_ADDR, ACCEL_STATUS, readBuff, 7);

        status0_value = readBuff[0];

        /* 14-bit resolution  "MMMM MMMM LLLL LL00" */
        x = ((int16_t)(((readBuff[1] * 256U) | readBuff[2])) >> 2);
        y = ((int16_t)(((readBuff[3] * 256U) | readBuff[4])) >> 2);
        z = ((int16_t)(((readBuff[5] * 256U) | readBuff[6])) >> 2);

        x_buff[next_avg] = x / 23; // Convert -8192 - 8192 interval to -360 - +360
        y_buff[next_avg] = y / 23; // Convert -8192 - 8192 interval to -360 - +360
        z_buff[next_avg] = z / 23; // Convert -8192 - 8192 interval to -360 - +360

        x_filt = y_filt = z_filt = 0;

        for(i = 0; i < SAMPLES; i++){
            x_filt += x_buff[i];
            y_filt += y_buff[i];
            z_filt += z_buff[i];
        }
        next_avg++;
        if (next_avg >= SAMPLES){
            next_avg = 0;
        }

        x_filt /= SAMPLES;
        y_filt /= SAMPLES;
        z_filt /= SAMPLES;

        /* For bonus purposes */
        PRINTF("A %d %d %d\r\n", x_filt, y_filt, z_filt);

        x_filt /= 10; // Convert -360 - +360 interval to -36 - +36
        y_filt /= 10; // Convert -360 - +360 interval to -36 - +36
        z_filt /= 10; // Convert -360 - +360 interval to -36 - +36

        /* Ukol druhy: Doplnte kod tak, aby se rozsvecovaly vsechny LED */
        LEDs_Off();
        /* Led blinking */
        if (x_filt <= -1) GPIO_PinWrite(LED10_GPIO, LED10_PIN, 1U);
        if (x_filt <= -2) GPIO_PinWrite(LED9_GPIO, LED9_PIN, 1U);
        if (x_filt <= -3) GPIO_PinWrite(LED8_GPIO, LED8_PIN, 1U);

        if (x_filt >= 1)  GPIO_PinWrite(LED6_GPIO, LED6_PIN, 1U);
        if (x_filt >= 2)  GPIO_PinWrite(LED5_GPIO, LED5_PIN, 1U);
        if (x_filt >= 3)  GPIO_PinWrite(LED4_GPIO, LED4_PIN, 1U);

        if (y_filt <= -1) GPIO_PinWrite(LED11_GPIO, LED11_PIN, 1U);
        if (y_filt <= -2) GPIO_PinWrite(LED12_GPIO, LED12_PIN, 1U);
        if (y_filt <= -3) GPIO_PinWrite(LED13_GPIO, LED13_PIN, 1U);


        if (y_filt >= 1)  GPIO_PinWrite(LED3_GPIO, LED3_PIN, 1U);
        if (y_filt >= 2)  GPIO_PinWrite(LED2_GPIO, LED2_PIN, 1U);
        if (y_filt >= 3)  GPIO_PinWrite(LED1_GPIO, LED1_PIN, 1U);

        if ((x_filt == 0) || (y_filt == 0)) GPIO_PinWrite(LED7_GPIO, LED7_PIN, 1U);

    }

    PRINTF("\r\nEnd of I2C example .\r\n");
    while (1)
    {
    }
}
