#pragma once
#ifndef CONFIG_H
#define CONFIG_H

#define _SOFTWARE_REV_ 1.0

#define USART1_TX PB6
#define USART1_RX PB7
#define USART2_TX PA2
#define USART2_RX PA3
#define USART3_TX PC10
#define USART3_RX PC11

#define DebugUart Serial1
#define ModemUart Serial2
#define DisplayUart Serial3

#define LOG_OUTPUT DebugUart
#define LOG_LEVEL LOG_LEVEL_DEBUG

typedef enum {
  SYSTEM_M = 0,
  DISPLAY_M = 1,
  EC200_M = 2,
  COMPASS_M = 3,
  SDMEM_M = 4,
  NORMEM_M = 5,
  RTC_M = 6,
  BLUETOOTH_M = 7
} _SYS_MODULES_t;

const char *_SYSTEM_MODULE_LIST[] = {"SYSTEM", "DISPLAY", "EC200", "COMPASS", "SDMEM", "NORMEM", "RTC", "BLUETOOTH"};

#define SD_SPI_BUS SPI
#define SD_CS_PIN PD6
#define SPI_MOSI PB5
#define SPI_MISO PB4
#define SPI_SCLK PB3

#define I2C_SDA PC9
#define I2C_SCL PA8

#endif
