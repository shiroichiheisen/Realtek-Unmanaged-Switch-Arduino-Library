/*
 * Copyright c                  Realtek Semiconductor Corporation, 2006
 * All rights reserved.
 *
 * Program : Control smi connected RTL8366
 * Abstract :
 * Author : Yu-Mei Pan (ympan@realtek.com.cn)
 *  $Id: smi.c,v 1.2 2008-04-10 03:04:19 shiehyy Exp $
 */
#include "rtk_types.h"
#include "smi.h"
#include "rtk_error.h"
#include "Arduino.h"

/*******************************************************************************/
/*  I2C porting                                                                */
/*******************************************************************************/
/* Define the GPIO ID for SCK & SDA */
rtk_uint8 RTK_I2C_SCK_PIN = 1; /* GPIO used for SMI Clock Generation */
rtk_uint8 RTK_I2C_SDA_PIN = 2; /* GPIO used for SMI Data signal */
rtk_uint8 RTK_I2C_DELAY = 1;   /* Delay time for I2C */

#define CLK_DURATION delayMicroseconds(RTK_I2C_DELAY)

static void _smi_start(void)
{
    /* change GPIO pin to Output only */
    pinMode(RTK_I2C_SCK_PIN, OUTPUT);
    pinMode(RTK_I2C_SDA_PIN, OUTPUT);

    /* Initial state: SCK: 0, SDA: 1 */
    digitalWrite(RTK_I2C_SCK_PIN, 0);
    digitalWrite(RTK_I2C_SDA_PIN, 1);
    CLK_DURATION;

    /* CLK 1: 0 -> 1, 1 -> 0 */
    digitalWrite(RTK_I2C_SCK_PIN, 1);
    CLK_DURATION;
    digitalWrite(RTK_I2C_SCK_PIN, 0);
    CLK_DURATION;

    /* CLK 2: */
    digitalWrite(RTK_I2C_SCK_PIN, 1);
    CLK_DURATION;
    digitalWrite(RTK_I2C_SDA_PIN, 0);
    CLK_DURATION;
    digitalWrite(RTK_I2C_SCK_PIN, 0);
    CLK_DURATION;
    digitalWrite(RTK_I2C_SDA_PIN, 1);
}

static void _smi_writeBit(rtk_uint16 signal, rtk_uint32 bitLen)
{
    /* change GPIO pin to Output only */
    pinMode(RTK_I2C_SDA_PIN, OUTPUT);

    for (; bitLen > 0; bitLen--)
    {
        CLK_DURATION;

        /* prepare data */
        if (signal & (1 << (bitLen - 1)))
            digitalWrite(RTK_I2C_SDA_PIN, 1);
        else
            digitalWrite(RTK_I2C_SDA_PIN, 0);

        CLK_DURATION;

        /* clocking */
        digitalWrite(RTK_I2C_SCK_PIN, 1);
        CLK_DURATION;
        digitalWrite(RTK_I2C_SCK_PIN, 0);
    }
}

static void _smi_readBit(rtk_uint32 bitLen, rtk_uint32 *rData)
{
    rtk_uint32 u = 0;

    /* change GPIO pin to Input only */
    pinMode(RTK_I2C_SDA_PIN, INPUT);

    for (*rData = 0; bitLen > 0; bitLen--)
    {
        CLK_DURATION;

        /* clocking */
        digitalWrite(RTK_I2C_SCK_PIN, 1);
        CLK_DURATION;
        u = digitalRead(RTK_I2C_SDA_PIN);
        digitalWrite(RTK_I2C_SCK_PIN, 0);

        *rData |= (u << (bitLen - 1));
    }
}

static void _smi_stop(void)
{
    /* change GPIO pin to Output only */
    pinMode(RTK_I2C_SDA_PIN, OUTPUT);
    CLK_DURATION;
    digitalWrite(RTK_I2C_SDA_PIN, 0);
    digitalWrite(RTK_I2C_SCK_PIN, 1);
    CLK_DURATION;
    digitalWrite(RTK_I2C_SDA_PIN, 1);
    CLK_DURATION;
    digitalWrite(RTK_I2C_SCK_PIN, 1);
    CLK_DURATION;
    digitalWrite(RTK_I2C_SCK_PIN, 0);
    CLK_DURATION;
    digitalWrite(RTK_I2C_SCK_PIN, 1);

    /* add a click */
    CLK_DURATION;
    digitalWrite(RTK_I2C_SCK_PIN, 0);
    CLK_DURATION;
    digitalWrite(RTK_I2C_SCK_PIN, 1);

    /* change GPIO pin to Input only */
    pinMode(RTK_I2C_SDA_PIN, INPUT);
    pinMode(RTK_I2C_SCK_PIN, INPUT);
}

rtk_int32 smi_read(rtk_uint32 mAddrs, rtk_uint32 *rData)
{
    rtk_uint32 rawData = 0, ACK;
    rtk_uint8 con;
    rtk_uint32 ret = RT_ERR_OK;

    if (mAddrs > 0xFFFF)
        return RT_ERR_INPUT;

    if (rData == NULL)
        return RT_ERR_NULL_POINTER;

    _smi_start(); /* Start SMI */

    _smi_writeBit(0x0b, 4); /* CTRL code: 4'b1011 for RTL8370 */

    _smi_writeBit(0x4, 3); /* CTRL code: 3'b100 */

    _smi_writeBit(0x1, 1); /* 1: issue READ command */

    con = 0;
    do
    {
        con++;
        _smi_readBit(1, &ACK); /* ACK for issuing READ command*/
    } while ((ACK != 0) && (con < ack_timer));

    if (ACK != 0)
        ret = RT_ERR_FAILED;

    _smi_writeBit((mAddrs & 0xff), 8); /* Set reg_addr[7:0] */

    con = 0;
    do
    {
        con++;
        _smi_readBit(1, &ACK); /* ACK for setting reg_addr[7:0] */
    } while ((ACK != 0) && (con < ack_timer));

    if (ACK != 0)
        ret = RT_ERR_FAILED;

    _smi_writeBit((mAddrs >> 8), 8); /* Set reg_addr[15:8] */

    con = 0;
    do
    {
        con++;
        _smi_readBit(1, &ACK); /* ACK by RTL8369 */
    } while ((ACK != 0) && (con < ack_timer));
    if (ACK != 0)
        ret = RT_ERR_FAILED;

    _smi_readBit(8, &rawData); /* Read DATA [7:0] */
    *rData = rawData & 0xff;

    _smi_writeBit(0x00, 1); /* ACK by CPU */

    _smi_readBit(8, &rawData); /* Read DATA [15: 8] */

    _smi_writeBit(0x01, 1); /* ACK by CPU */
    *rData |= (rawData << 8);

    _smi_stop();

    return ret;
}

rtk_int32 smi_write(rtk_uint32 mAddrs, rtk_uint32 rData)
{
    rtk_int8 con;
    rtk_uint32 ACK;
    rtk_uint32 ret = RT_ERR_OK;

    if (mAddrs > 0xFFFF)
        return RT_ERR_INPUT;

    if (rData > 0xFFFF)
        return RT_ERR_INPUT;

    _smi_start(); /* Start SMI */

    _smi_writeBit(0x0b, 4); /* CTRL code: 4'b1011 for RTL8370*/

    _smi_writeBit(0x4, 3); /* CTRL code: 3'b100 */

    _smi_writeBit(0x0, 1); /* 0: issue WRITE command */

    con = 0;
    do
    {
        con++;
        _smi_readBit(1, &ACK); /* ACK for issuing WRITE command*/
    } while ((ACK != 0) && (con < ack_timer));
    if (ACK != 0)
        ret = RT_ERR_FAILED;

    _smi_writeBit((mAddrs & 0xff), 8); /* Set reg_addr[7:0] */

    con = 0;
    do
    {
        con++;
        _smi_readBit(1, &ACK); /* ACK for setting reg_addr[7:0] */
    } while ((ACK != 0) && (con < ack_timer));
    if (ACK != 0)
        ret = RT_ERR_FAILED;

    _smi_writeBit((mAddrs >> 8), 8); /* Set reg_addr[15:8] */

    con = 0;
    do
    {
        con++;
        _smi_readBit(1, &ACK); /* ACK for setting reg_addr[15:8] */
    } while ((ACK != 0) && (con < ack_timer));
    if (ACK != 0)
        ret = RT_ERR_FAILED;

    _smi_writeBit(rData & 0xff, 8); /* Write Data [7:0] out */

    con = 0;
    do
    {
        con++;
        _smi_readBit(1, &ACK); /* ACK for writting data [7:0] */
    } while ((ACK != 0) && (con < ack_timer));
    if (ACK != 0)
        ret = RT_ERR_FAILED;

    _smi_writeBit(rData >> 8, 8); /* Write Data [15:8] out */

    con = 0;
    do
    {
        con++;
        _smi_readBit(1, &ACK); /* ACK for writting data [15:8] */
    } while ((ACK != 0) && (con < ack_timer));
    if (ACK != 0)
        ret = RT_ERR_FAILED;

    _smi_stop();

    return ret;
}
