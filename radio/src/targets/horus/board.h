/*
 * Copyright (C) OpenTX
 *
 * Based on code named
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _BOARD_H_
#define _BOARD_H_

#include "../definitions.h"
#include "../opentx_constants.h"
#include "board_common.h"
#include "hal.h"

PACK(typedef struct {
  uint8_t pcbrev : 2;
  uint8_t sticksPwmDisabled : 1;
  uint8_t pxx2Enabled : 1;
}) HardwareOptions;

extern HardwareOptions hardwareOptions;

#if !defined(LUA_EXPORT_GENERATION)
#include "STM32F4xx_DSP_StdPeriph_Lib_V1.4.0/Libraries/STM32F4xx_StdPeriph_Driver/inc/stm32f4xx_sdio.h"
#include "STM32F4xx_DSP_StdPeriph_Lib_V1.4.0/Libraries/STM32F4xx_StdPeriph_Driver/inc/stm32f4xx_dma2d.h"
#include "STM32F4xx_DSP_StdPeriph_Lib_V1.4.0/Libraries/STM32F4xx_StdPeriph_Driver/inc/stm32f4xx_ltdc.h"
#include "STM32F4xx_DSP_StdPeriph_Lib_V1.4.0/Libraries/STM32F4xx_StdPeriph_Driver/inc/stm32f4xx_fmc.h"
#endif

#define FLASHSIZE                      0x200000
#define BOOTLOADER_SIZE                0x20000
#define FIRMWARE_ADDRESS               0x08000000

#define MB                             *1024*1024
#define LUA_MEM_EXTRA_MAX              (2 MB)    // max allowed memory usage for Lua bitmaps (in bytes)
#define LUA_MEM_MAX                    (6 MB)    // max allowed memory usage for complete Lua  (in bytes), 0 means unlimited

// HSI is at 168Mhz (over-drive is not enabled!)
#define PERI1_FREQUENCY                42000000
#define PERI2_FREQUENCY                84000000
#define TIMER_MULT_APB1                2
#define TIMER_MULT_APB2                2

extern uint16_t sessionTimer;

#define SLAVE_MODE()                   (g_model.trainerData.mode == TRAINER_MODE_SLAVE)

#if defined(PCBX10) && !defined(RADIO_T16)
  #define TRAINER_CONNECTED()            (GPIO_ReadInputDataBit(TRAINER_DETECT_GPIO, TRAINER_DETECT_GPIO_PIN) == Bit_SET)
#else
  #define TRAINER_CONNECTED()            (GPIO_ReadInputDataBit(TRAINER_DETECT_GPIO, TRAINER_DETECT_GPIO_PIN) == Bit_RESET)
#endif

// Board driver
void boardInit();
void boardOff();

// Timers driver
void init2MhzTimer();
void init5msTimer();

// PCBREV driver
enum {
  // X12S
  PCBREV_X12S_LT13 = 0,
  PCBREV_X12S_GTE13 = 1,

  // X10
  PCBREV_X10_STD = 0,
  PCBREV_X10_EXPRESS = 3,
};

#if defined(SIMU)
  #define IS_FIRMWARE_COMPATIBLE_WITH_BOARD() true
#elif defined(PCBX10)
  #if defined(PCBREV_EXPRESS)
    #define IS_FIRMWARE_COMPATIBLE_WITH_BOARD() (hardwareOptions.pcbrev == PCBREV_X10_EXPRESS)
  #else
    #define IS_FIRMWARE_COMPATIBLE_WITH_BOARD() (hardwareOptions.pcbrev == PCBREV_X10_STD)
  #endif
#else
  #if PCBREV >= 13
    #define IS_FIRMWARE_COMPATIBLE_WITH_BOARD() (hardwareOptions.pcbrev == PCBREV_X12S_GTE13)
  #else
    #define IS_FIRMWARE_COMPATIBLE_WITH_BOARD() (hardwareOptions.pcbrev == PCBREV_X12S_LT13)
  #endif
#endif

// SD driver
#define BLOCK_SIZE                     512 /* Block Size in Bytes */
#if !defined(SIMU) || defined(SIMU_DISKIO)
uint32_t sdIsHC();
uint32_t sdGetSpeed();
#define SD_IS_HC()                     (sdIsHC())
#define SD_GET_SPEED()                 (sdGetSpeed())
#define SD_GET_FREE_BLOCKNR()          (sdGetFreeSectors())
#define SD_CARD_PRESENT()              (~SD_PRESENT_GPIO->IDR & SD_PRESENT_GPIO_PIN)
void sdInit();
void sdMount();
void sdDone();
#define sdPoll10ms()
uint32_t sdMounted();
#else
#define SD_IS_HC()                     (0)
#define SD_GET_SPEED()                 (0)
#define sdInit()
#define sdMount()
#define sdDone()
#define SD_CARD_PRESENT()              true
#endif

#if defined(DISK_CACHE)
#include "FatFs/diskio.h"
DRESULT __disk_read(BYTE drv, BYTE * buff, DWORD sector, UINT count);
DRESULT __disk_write(BYTE drv, const BYTE * buff, DWORD sector, UINT count);
#else
#define __disk_read                    disk_read
#define __disk_write                   disk_write
#endif

// Flash Write driver
#define FLASH_PAGESIZE                 256
void unlockFlash();
void lockFlash();
void flashWrite(uint32_t * address, uint32_t * buffer);
uint32_t isFirmwareStart(const uint8_t * buffer);
uint32_t isBootloaderStart(const uint8_t * buffer);

// SDRAM driver
void SDRAM_Init();

// Pulses driver
#define INTERNAL_MODULE_ON()           GPIO_SetBits(INTMODULE_PWR_GPIO, INTMODULE_PWR_GPIO_PIN)

#if defined(INTMODULE_USART)
  #define INTERNAL_MODULE_OFF()        intmoduleStop()
#else
  #define INTERNAL_MODULE_OFF()        GPIO_ResetBits(INTMODULE_PWR_GPIO, INTMODULE_PWR_GPIO_PIN)
#endif

#define EXTERNAL_MODULE_ON()           GPIO_SetBits(EXTMODULE_PWR_GPIO, EXTMODULE_PWR_GPIO_PIN)
#define EXTERNAL_MODULE_OFF()          GPIO_ResetBits(EXTMODULE_PWR_GPIO, EXTMODULE_PWR_GPIO_PIN)
#define IS_INTERNAL_MODULE_ON()        (GPIO_ReadInputDataBit(INTMODULE_PWR_GPIO, INTMODULE_PWR_GPIO_PIN) == Bit_SET)
#define IS_EXTERNAL_MODULE_ON()        (GPIO_ReadInputDataBit(EXTMODULE_PWR_GPIO, EXTMODULE_PWR_GPIO_PIN) == Bit_SET)

#if !defined(PXX2)
  #define IS_PXX2_INTERNAL_ENABLED()            (false)
  #define IS_PXX1_INTERNAL_ENABLED()            (true)
#elif !defined(PXX1)
  #define IS_PXX2_INTERNAL_ENABLED()            (true)
  #define IS_PXX1_INTERNAL_ENABLED()            (false)
#else
  // TODO #define PXX2_PROBE
  // TODO #define IS_PXX2_INTERNAL_ENABLED()            (hardwareOptions.pxx2Enabled)
  #define IS_PXX2_INTERNAL_ENABLED()            (true)
  #define IS_PXX1_INTERNAL_ENABLED()            (true)
#endif

void init_intmodule_heartbeat();
void check_intmodule_heartbeat();

void intmoduleSerialStart(uint32_t baudrate, uint8_t rxEnable, uint16_t parity, uint16_t stopBits, uint16_t wordLength);
#if defined(INTERNAL_MODULE_MULTI)
void intmoduleTimerStart(uint32_t periodMs);
#endif
void intmoduleSendByte(uint8_t byte);
void intmoduleSendBuffer(const uint8_t * data, uint8_t size);
void intmoduleSendNextFrame();

void extmoduleSerialStart(uint32_t baudrate, uint32_t period_half_us, bool inverted);
void extmoduleInvertedSerialStart(uint32_t baudrate);
void extmoduleSendBuffer(const uint8_t * data, uint8_t size);
void extmoduleSendNextFrame();
void extmoduleSendInvertedByte(uint8_t byte);

// Trainer driver
void init_trainer_ppm();
void stop_trainer_ppm();
void init_trainer_capture();
void stop_trainer_capture();

// Keys driver
enum EnumKeys
{
  KEY_PGUP,
  KEY_PGDN,
  KEY_ENTER,
  KEY_MODEL,
  KEY_UP = KEY_MODEL,
  KEY_EXIT,
  KEY_DOWN = KEY_EXIT,
  KEY_TELEM,
  KEY_RIGHT = KEY_TELEM,
  KEY_RADIO,
  KEY_LEFT = KEY_RADIO,

  TRM_BASE,
  TRM_LH_DWN = TRM_BASE,
  TRM_LH_UP,
  TRM_LV_DWN,
  TRM_LV_UP,
  TRM_RV_DWN,
  TRM_RV_UP,
  TRM_RH_DWN,
  TRM_RH_UP,
  TRM_LS_DWN,
  TRM_LS_UP,
  TRM_RS_DWN,
  TRM_RS_UP,
  TRM_LAST = TRM_RS_UP,

  NUM_KEYS
};

#define IS_SHIFT_KEY(index)             (false)
#define IS_SHIFT_PRESSED()              (false)

enum EnumSwitches
{
  SW_SA,
  SW_SB,
  SW_SC,
  SW_SD,
  SW_SE,
  SW_SF,
  SW_SG,
  SW_SH,
  SW_SI,
  SW_SJ,
  NUM_SWITCHES
};

#define STORAGE_NUM_SWITCHES           NUM_SWITCHES
#define IS_3POS(x)                     ((x) != SW_SF && (x) != SW_SH)

enum EnumSwitchesPositions
{
  SW_SA0,
  SW_SA1,
  SW_SA2,
  SW_SB0,
  SW_SB1,
  SW_SB2,
  SW_SC0,
  SW_SC1,
  SW_SC2,
  SW_SD0,
  SW_SD1,
  SW_SD2,
  SW_SE0,
  SW_SE1,
  SW_SE2,
  SW_SF0,
  SW_SF1,
  SW_SF2,
  SW_SG0,
  SW_SG1,
  SW_SG2,
  SW_SH0,
  SW_SH1,
  SW_SH2,
  SW_SI0,
  SW_SI1,
  SW_SI2,
  SW_SJ0,
  SW_SJ1,
  SW_SJ2,
  NUM_SWITCHES_POSITIONS
};

#define STORAGE_NUM_SWITCHES_POSITIONS  (STORAGE_NUM_SWITCHES * 3)

void keysInit();
uint32_t switchState(uint8_t index);
uint32_t readKeys();
#define KEYS_PRESSED()                          (readKeys())
#define DBLKEYS_PRESSED_RGT_LFT(in)             ((in & ((1<<KEY_RIGHT) + (1<<KEY_LEFT))) == ((1<<KEY_RIGHT) + (1<<KEY_LEFT)))
#define DBLKEYS_PRESSED_UP_DWN(in)              ((in & ((1<<KEY_UP) + (1<<KEY_DOWN))) == ((1<<KEY_UP) + (1<<KEY_DOWN)))
#define DBLKEYS_PRESSED_RGT_UP(in)              ((in & ((1<<KEY_RIGHT) + (1<<KEY_UP))) == ((1<<KEY_RIGHT) + (1<<KEY_UP)))
#define DBLKEYS_PRESSED_LFT_DWN(in)             ((in & ((1<<KEY_LEFT) + (1<<KEY_DOWN))) == ((1<<KEY_LEFT) + (1<<KEY_DOWN)))

// Trims driver
#define NUM_TRIMS                               6
#define NUM_TRIMS_KEYS                          (NUM_TRIMS * 2)
uint32_t readTrims();
#define TRIMS_PRESSED()                         (readTrims())

// Rotary encoder driver
#define ROTARY_ENCODER_NAVIGATION
void rotaryEncoderInit();
void rotaryEncoderCheck();

// WDT driver
#define WDTO_500MS                              500
extern uint32_t powerupReason;

#define SHUTDOWN_REQUEST                        0xDEADBEEF
#define NO_SHUTDOWN_REQUEST                     ~SHUTDOWN_REQUEST
#define DIRTY_SHUTDOWN                          0xCAFEDEAD
#define NORMAL_POWER_OFF                        ~DIRTY_SHUTDOWN

void watchdogInit(unsigned int duration);
#if defined(SIMU)
  #define WAS_RESET_BY_WATCHDOG()               (false)
  #define WAS_RESET_BY_SOFTWARE()               (false)
  #define WAS_RESET_BY_WATCHDOG_OR_SOFTWARE()   (false)
  #define wdt_enable(x)
  #define wdt_reset()
#else
  #if defined(WATCHDOG)
    #define wdt_enable(x)                       watchdogInit(x)
    #define wdt_reset()                         IWDG->KR = 0xAAAA
  #else
    #define wdt_enable(x)
    #define wdt_reset()
  #endif
  #define WAS_RESET_BY_WATCHDOG()               (RCC->CSR & (RCC_CSR_WDGRSTF | RCC_CSR_WWDGRSTF))
  #define WAS_RESET_BY_SOFTWARE()               (RCC->CSR & RCC_CSR_SFTRSTF)
  #define WAS_RESET_BY_WATCHDOG_OR_SOFTWARE()   (RCC->CSR & (RCC_CSR_WDGRSTF | RCC_CSR_WWDGRSTF | RCC_CSR_SFTRSTF))
#endif

// ADC driver

#if defined(PCBX10)
#define NUM_POTS                       5
#else
#define NUM_POTS                       3
#endif

#define NUM_XPOTS                      NUM_POTS
#define STORAGE_NUM_POTS               5

#if defined(PCBX10)
  #define NUM_SLIDERS                  2
  #define NUM_PWMSTICKS                4
#else
  #define NUM_SLIDERS                  4
  #define NUM_PWMSTICKS                0
#endif

#define STORAGE_NUM_SLIDERS            4

enum Analogs {
  STICK1,
  STICK2,
  STICK3,
  STICK4,
  POT_FIRST,
  POT1 = POT_FIRST,
  POT2,
  POT3,
#if defined(PCBX10)
  EXT1,
  EXT2,
#endif
  POT_LAST = POT_FIRST + NUM_POTS - 1,
  SLIDER_FIRST,
  SLIDER_FRONT_LEFT = SLIDER_FIRST,
  SLIDER_FRONT_RIGHT,
#if defined(PCBX12S)
  SLIDER_REAR_LEFT,
  SLIDER_REAR_RIGHT,
#endif
  SLIDER_LAST = SLIDER_FIRST + NUM_SLIDERS - 1,
  TX_VOLTAGE,
#if defined(PCBX12S)
  MOUSE1, // TODO why after voltage?
  MOUSE2,
#endif
  NUM_ANALOGS
};

#define SLIDER1 SLIDER_FRONT_LEFT
#define SLIDER2 SLIDER_FRONT_RIGHT

#define DEFAULT_SWITCH_CONFIG  (SWITCH_TOGGLE << 14) + (SWITCH_3POS << 12) + (SWITCH_2POS << 10) + (SWITCH_3POS << 8) + (SWITCH_3POS << 6) + (SWITCH_3POS << 4) + (SWITCH_3POS << 2) + (SWITCH_3POS << 0)
#define DEFAULT_POTS_CONFIG    (POT_WITH_DETENT << 4) + (POT_MULTIPOS_SWITCH << 2) + (POT_WITHOUT_DETENT << 0)

#if defined(PCBX12S)
#define DEFAULT_SLIDERS_CONFIG (SLIDER_WITH_DETENT << 3) + (SLIDER_WITH_DETENT << 2) + (SLIDER_WITH_DETENT << 1) + (SLIDER_WITH_DETENT << 0)
#else
#define DEFAULT_SLIDERS_CONFIG (SLIDER_WITH_DETENT << 1) + (SLIDER_WITH_DETENT << 0)
#endif

enum CalibratedAnalogs {
  CALIBRATED_STICK1,
  CALIBRATED_STICK2,
  CALIBRATED_STICK3,
  CALIBRATED_STICK4,
  CALIBRATED_POT1,
  CALIBRATED_POT2,
  CALIBRATED_POT3,
#if defined(PCBX12S)
  CALIBRATED_SLIDER_FRONT_LEFT,
  CALIBRATED_SLIDER_FRONT_RIGHT,
  CALIBRATED_SLIDER_REAR_LEFT,
  CALIBRATED_SLIDER_REAR_RIGHT,
#else
  CALIBRATED_POT_EXT1,
  CALIBRATED_POT_EXT2,
  CALIBRATED_SLIDER_REAR_LEFT,
  CALIBRATED_SLIDER_REAR_RIGHT,
#endif
  CALIBRATED_MOUSE1,
  CALIBRATED_MOUSE2,
  NUM_CALIBRATED_ANALOGS
};

#define IS_POT(x)                      ((x)>=POT_FIRST && (x)<=POT_LAST)
#define IS_SLIDER(x)                   ((x)>=SLIDER_FIRST && (x)<=SLIDER_LAST)
extern uint16_t adcValues[NUM_ANALOGS];

#if defined(PCBX12S)
  #define NUM_MOUSE_ANALOGS            2
#else
  #define NUM_MOUSE_ANALOGS            0
#endif
#define STORAGE_NUM_MOUSE_ANALOGS      2

#if NUM_PWMSTICKS > 0
#define STICKS_PWM_ENABLED()          (!hardwareOptions.sticksPwmDisabled)
void sticksPwmInit();
void sticksPwmRead(uint16_t * values);
extern volatile uint32_t pwm_interrupt_count;
#else
#define STICKS_PWM_ENABLED()          (false)
#endif

// Battery driver
#if defined(PCBX10)
  // Lipo 2S
  #define BATTERY_WARN      66 // 6.6V
  #define BATTERY_MIN       67 // 6.7V
  #define BATTERY_MAX       83 // 8.3V
#else
  // NI-MH 9.6V
  #define BATTERY_WARN      87 // 8.7V
  #define BATTERY_MIN       85 // 8.5V
  #define BATTERY_MAX       115 // 11.5V
#endif

#if defined(__cplusplus) && !defined(SIMU)
extern "C" {
#endif

// Power driver
#define SOFT_PWR_CTRL
extern uint32_t shutdownRequest; // Stores intentional shutdown to avoid reboot loop
extern uint32_t shutdownReason; // Used for detecting unexpected reboots regardless of reason
void pwrInit();
uint32_t pwrCheck();
void pwrOn();
void pwrOff();
void pwrResetHandler();
bool pwrPressed();
uint32_t pwrPressedDuration();
#if defined(SIMU) || defined(NO_UNEXPECTED_SHUTDOWN)
  #define UNEXPECTED_SHUTDOWN()                 (false)
#else
  #define UNEXPECTED_SHUTDOWN()                 ((powerupReason == DIRTY_SHUTDOWN) || WAS_RESET_BY_WATCHDOG_OR_SOFTWARE())
#endif

// Led driver
void ledInit();
void ledOff();
void ledRed();
void ledBlue();
#if defined(PCBX10)
  void ledGreen();
#endif

// LCD driver
#define LCD_W                          480
#define LCD_H                          272
#define LCD_DEPTH                      16
void lcdInit();
void lcdRefresh();
void lcdCopy(void * dest, void * src);
void DMAFillRect(uint16_t * dest, uint16_t destw, uint16_t desth, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void DMACopyBitmap(uint16_t * dest, uint16_t destw, uint16_t desth, uint16_t x, uint16_t y, const uint16_t * src, uint16_t srcw, uint16_t srch, uint16_t srcx, uint16_t srcy, uint16_t w, uint16_t h);
void DMACopyAlphaBitmap(uint16_t * dest, uint16_t destw, uint16_t desth, uint16_t x, uint16_t y, const uint16_t * src, uint16_t srcw, uint16_t srch, uint16_t srcx, uint16_t srcy, uint16_t w, uint16_t h);
void DMABitmapConvert(uint16_t * dest, const uint8_t * src, uint16_t w, uint16_t h, uint32_t format);
void lcdStoreBackupBuffer();
int lcdRestoreBackupBuffer();
void lcdSetContrast();
#define lcdOff()              backlightEnable(0) /* just disable the backlight */
#define lcdSetRefVolt(...)
#define lcdRefreshWait(...)

// Backlight driver
void backlightInit();
#if defined(SIMU) || !defined(__cplusplus)
#define backlightEnable(...)
#else
void backlightEnable(uint8_t dutyCycle = 0);
#endif
#define BACKLIGHT_LEVEL_MAX   100
#if defined(PCBX12S)
#define BACKLIGHT_LEVEL_MIN   5
#elif defined(RADIO_T16)
#define BACKLIGHT_LEVEL_MIN   1
#else
#define BACKLIGHT_LEVEL_MIN   46
#endif
#define BACKLIGHT_ENABLE()    backlightEnable(globalData.unexpectedShutdown ? BACKLIGHT_LEVEL_MAX : BACKLIGHT_LEVEL_MAX - g_eeGeneral.backlightBright)
#define BACKLIGHT_DISABLE()   backlightEnable(globalData.unexpectedShutdown ? BACKLIGHT_LEVEL_MAX : ((g_eeGeneral.blOffBright == BACKLIGHT_LEVEL_MIN) && (g_eeGeneral.backlightMode != e_backlight_mode_off)) ? 0 : g_eeGeneral.blOffBright)
#define isBacklightEnabled()  true

#if !defined(SIMU)
void usbJoystickUpdate();
#endif
#if defined(PCBX12S)
  #define USB_NAME                     "FrSky Horus"
  #define USB_MANUFACTURER             'F', 'r', 'S', 'k', 'y', ' ', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'H', 'o', 'r', 'u', 's', ' ', ' ', ' '  /* 8 Bytes */
#elif defined(RADIO_T16)
  #define USB_NAME                     "Jumper T16"
  #define USB_MANUFACTURER             'J', 'u', 'm', 'p', 'e', 'r', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'T', '1', '6', ' ', ' ', ' ', ' ', ' '  /* 8 Bytes */  
#elif defined(PCBX10)
  #define USB_NAME                     "FrSky X10"
  #define USB_MANUFACTURER             'F', 'r', 'S', 'k', 'y', ' ', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'X', '1', '0', ' ', ' ', ' ', ' ', ' '  /* 8 Bytes */
#endif

#if defined(__cplusplus) && !defined(SIMU)
}
#endif

// Audio driver
void audioInit();
void audioConsumeCurrentBuffer();
#define audioDisableIrq()             // interrupts must stay enabled on Horus
#define audioEnableIrq()              // interrupts must stay enabled on Horus
#if defined(PCBX12S)
#define setSampleRate(freq)
#else
void setSampleRate(uint32_t frequency);
#endif
void setScaledVolume(uint8_t volume);
void setVolume(uint8_t volume);
int32_t getVolume();
#define VOLUME_LEVEL_MAX               23
#define VOLUME_LEVEL_DEF               12

// Telemetry driver
#define TELEMETRY_FIFO_SIZE            512
void telemetryPortInit(uint32_t baudrate, uint8_t mode);
void telemetryPortSetDirectionInput();
void telemetryPortSetDirectionOutput();
void sportSendByte(uint8_t byte);
void sportSendBuffer(const uint8_t * buffer, uint32_t count);
bool telemetryGetByte(uint8_t * byte);
void telemetryClearFifo();
extern uint32_t telemetryErrors;

// soft-serial
void telemetryPortInvertedInit(uint32_t baudrate);

// Sport update driver
#if defined(PCBX10) && !defined(RADIO_T16)
void sportUpdatePowerOn();
void sportUpdatePowerOff();
#define SPORT_UPDATE_POWER_ON()        sportUpdatePowerOn()
#define SPORT_UPDATE_POWER_OFF()       sportUpdatePowerOff()
#else
#define SPORT_UPDATE_POWER_ON()
#define SPORT_UPDATE_POWER_OFF()
#endif

// Haptic driver
void hapticInit();
void hapticDone();
void hapticOff();
void hapticOn(uint32_t pwmPercent);

// GPS driver
void gpsInit(uint32_t baudrate);
uint8_t gpsGetByte(uint8_t * byte);
#if defined(DEBUG)
extern uint8_t gpsTraceEnabled;
#endif
void gpsSendByte(uint8_t byte);
#if defined(INTERNAL_GPS)
#define PILOTPOS_MIN_HDOP             500
#endif

#define USART_FLAG_ERRORS              (USART_FLAG_ORE | USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE)

// BT driver
#define BT_TX_FIFO_SIZE    64
#define BT_RX_FIFO_SIZE    128
#define BLUETOOTH_BOOTLOADER_BAUDRATE  230400
#define BLUETOOTH_FACTORY_BAUDRATE     57600
#define BLUETOOTH_DEFAULT_BAUDRATE     115200
void bluetoothInit(uint32_t baudrate, bool enable);
void bluetoothWriteWakeup();
uint8_t bluetoothIsWriting();
void bluetoothDisable();

#if defined(__cplusplus)
#include "fifo.h"
#include "dmafifo.h"
extern DMAFifo<512> telemetryFifo;
typedef DMAFifo<32> AuxSerialRxFifo;
extern AuxSerialRxFifo auxSerialRxFifo;
extern volatile uint32_t externalModulePort;
#endif

#endif // _BOARD_H_
