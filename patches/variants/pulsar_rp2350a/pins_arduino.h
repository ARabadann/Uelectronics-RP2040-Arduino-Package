/*
    Pin definitions for the UNIT Electronics Pulsar RP2350A

    Copyright (c) 2026 UNIT Electronics
    Author: Adrián Rabadán Ortiz

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.
*/

#pragma once

// ============================================================
// UNIT Electronics - Pulsar RP2350A
// RP2350A / 16MB Flash / 8MB PSRAM
// ============================================================

#define PICO_RP2350A 1


// ============================================================
// Arduino Digital Pin Mapping
// ============================================================

#define __PIN_D0   (19u)   // RX
#define __PIN_D1   (18u)   // TX
#define __PIN_D2   (17u)
#define __PIN_D3   (16u)
#define __PIN_D4   (15u)
#define __PIN_D5   (14u)
#define __PIN_D6   (13u)
#define __PIN_D7   (12u)

// D8 and D9:
// ============================================================
// Arduino D8 / D9 - RP2350 GPIO Bank 1
//
// The Pulsar RP2350A header exposes:
//   D8 -> USB_DP
//   D9 -> USB_DM
//
// USB_DP and USB_DM are GPIO Bank 1 signals. They are NOT
// GPIO8/GPIO9 from GPIO Bank 0 and therefore must not be
// mapped through the standard Arduino GPIO numbering.
//
// When the native USB peripheral is disabled, these pins may
// be controlled through the RP2350 GPIO Bank 1 / SIO HI
// registers or configured for their alternate functions.
//
// QFN-60 physical pins:
//   USB_DP -> pin 52
//   USB_DM -> pin 51
//
// SIO GPIO_HI mapping:
//   USB_DP -> bit 28
//   USB_DM -> bit 27
// ============================================================

#define __PIN_D10  (21u)   // SPI SS
#define __PIN_D11  (23u)   // SPI MOSI
#define __PIN_D12  (20u)   // SPI MISO
#define __PIN_D13  (22u)   // SPI SCK

#define __PIN_D14  (28u)   // A0
#define __PIN_D15  (29u)   // A1
#define __PIN_D16  (27u)   // A2
#define __PIN_D17  (26u)   // A3

#define __PIN_D18  (8u)    // SDA
#define __PIN_D19  (9u)    // SCL

#define __PIN_D22  (1u)    // NeoPixel


// ============================================================
// Analog Pins
// ============================================================

#define __PIN_A0   (28u)
#define __PIN_A1   (29u)
#define __PIN_A2   (27u)
#define __PIN_A3   (26u)


// ============================================================
// UART
// ============================================================

// Serial1 - UART0
// Arduino D1 / D0
#define __SERIAL1_DEVICE uart0
#define PIN_SERIAL1_TX   (18u)
#define PIN_SERIAL1_RX   (19u)

// Serial2 - UART1
// GPIO26 / GPIO27
// These pins are also available as analog inputs A3 / A2.
#define __SERIAL2_DEVICE uart1
#define PIN_SERIAL2_TX   (26u)
#define PIN_SERIAL2_RX   (27u)

#define SERIAL_HOWMANY   (2u)


// ============================================================
// SPI
// ============================================================

#define __SPI0_DEVICE    spi0

#define PIN_SPI0_MISO    (20u)
#define PIN_SPI0_MOSI    (23u)
#define PIN_SPI0_SCK     (22u)
#define PIN_SPI0_SS      (21u)

#define SPI_HOWMANY      (1u)


// ============================================================
// I2C / Wire
// ============================================================

// Main Arduino I2C bus
#define __WIRE0_DEVICE   i2c0

#define PIN_WIRE0_SDA    (8u)
#define PIN_WIRE0_SCL    (9u)


// QWIIC I2C bus
#define __WIRE1_DEVICE   i2c1

#define PIN_WIRE1_SDA    (24u)
#define PIN_WIRE1_SCL    (25u)

#define WIRE_HOWMANY     (2u)


// ============================================================
// QWIIC
// ============================================================

#define PIN_QWIIC_SDA    (24u)
#define PIN_QWIIC_SCL    (25u)


// ============================================================
// On-board LED
// Arduino D13 / GPIO22
// ============================================================

#define PIN_LED          (22u)
#define LED_BUILTIN      PIN_LED

// ============================================================
// On-board NeoPixels
// GPIO1
// ============================================================

#define PIN_NEOPIXEL     (1u)
#define NUM_NEOPIXEL     (3u)


// ============================================================
// PSRAM
// APS6404L-3SQR-ZR
// 8MB external PSRAM
// ============================================================

#define RP2350_PSRAM_CS  (0u)


// ============================================================
// microSD - SDIO 4-bit
// ============================================================

#define PIN_SD_CLK       (2u)
#define PIN_SD_CMD       (3u)

#define PIN_SD_DAT0      (4u)
#define PIN_SD_DAT1      (5u)
#define PIN_SD_DAT2      (6u)
#define PIN_SD_DAT3      (7u)

// SPI-compatible aliases
#define PIN_SD_MOSI      PIN_SD_CMD
#define PIN_SD_MISO      PIN_SD_DAT0
#define PIN_SD_CS        PIN_SD_DAT3


// ============================================================
// PDM Microphone - ICS-41350
// ============================================================

#define PIN_MIC_CLK      (10u)
#define PIN_MIC_DATA     (11u)

#define PIN_PDM_CLK      PIN_MIC_CLK
#define PIN_PDM_DATA     PIN_MIC_DATA
#define PIN_PDM_DIN      PIN_MIC_DATA   // name used by the PDM library


// ============================================================
// IMU - BMI270
// Shares primary I2C bus
// ============================================================

#define PIN_IMU_SDA      (8u)
#define PIN_IMU_SCL      (9u)

// INT1 / INT2 intentionally not defined yet.


// ============================================================
// RP2350 HSTX / FPC-22
// GPIO12 through GPIO19 are routed to the FPC connector.
// ============================================================

#define PIN_HSTX_GPIO12  (12u)
#define PIN_HSTX_GPIO13  (13u)
#define PIN_HSTX_GPIO14  (14u)
#define PIN_HSTX_GPIO15  (15u)
#define PIN_HSTX_GPIO16  (16u)
#define PIN_HSTX_GPIO17  (17u)
#define PIN_HSTX_GPIO18  (18u)
#define PIN_HSTX_GPIO19  (19u)


// ============================================================
// FPC-22 / HSTX Connector
//
// The FPC-22 connector exposes:
//   - GPIO24 / GPIO25 : SDA / SCL
//   - GPIO26 / GPIO27 : A0 / A1
//   - GPIO12..GPIO19  : D7..D0
//
// GPIO12 through GPIO19 form the RP2350 HSTX-capable group
// routed to the FPC connector.
//
// Arduino mapping:
//   D7 -> GPIO12
//   D6 -> GPIO13
//   D5 -> GPIO14
//   D4 -> GPIO15
//   D3 -> GPIO16
//   D2 -> GPIO17
//   D1 -> GPIO18
//   D0 -> GPIO19
// ============================================================

// HSTX-capable GPIO group
#define PIN_HSTX_D7      (12u)
#define PIN_HSTX_D6      (13u)
#define PIN_HSTX_D5      (14u)
#define PIN_HSTX_D4      (15u)
#define PIN_HSTX_D3      (16u)
#define PIN_HSTX_D2      (17u)
#define PIN_HSTX_D1      (18u)
#define PIN_HSTX_D0      (19u)

// Additional signals available on FPC-22
#define PIN_FPC_A0       (28u)
#define PIN_FPC_A1       (29u)

#define PIN_FPC_SDA      (24u)
#define PIN_FPC_SCL      (25u)

// ============================================================
// Arduino-Pico common definitions
// ============================================================

#include "../generic/common.h"