/* Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */

/* 
 * File:   E70_868T14S.h
 * Author: [���� ��� / AI]
 * Comments: ���������� ��� �������� ���������� ������������ ������� E70-868T14S.
 *           ������������� ������� ��� ������������ �������, ��������� ����������,
 *           ��������/������ ������ � ��������� ������ � ����������.
 *           ���������� ��������������� MCC Melody �������� ��� UART � GPIO.
 * Revision history: 
 * v1.0 - �������� ������.
 * v1.1 - ��������� ��������� ������ � ��������.
 * v1.2 - ������� ������ (M0-RB0, M1-RB1, M2-RB2, AUX-RB3).
 * v1.3 - ���������� �� ������� MPLAB X, ������������� DELAY_milliseconds.
 * v1.4 - ���������� ���� ��������� MCC Melody ������ ������ ����������.
 * v1.5 - �������������� ��������� MCC Melody ������, ��������� ����������� system.h.
 * v1.6 - ���������� �� .h (����������) � .c (����������) �����.
 * v1.7 - ���������� ���� � eusart1.h � ������������� EUSART1_ �������.
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef E70_868T14S_H
#define	E70_868T14S_H

#include <xc.h> // include processor files - each processor file is guarded.  

// �������� ����������� MCC Melody ������������ �����.
// system.h �������� � ���� pins.h, eusart1.h (uart1.h), eusart2.h, tmr1.h, interrupt.h, clock.h.
// delay.h �������� ��������, ��� ��� �� �� ������ � system.h �� ��������������� ����������.
#include <stdbool.h>
#include <stdint.h>
#include "mcc_generated_files/system/system.h"    // ��� SYSTEM_GetTickCount()
#include "mcc_generated_files/uart/eusart1.h"     // ��� UART1 (����� � E70), ������������ eusart1.h
#include "mcc_generated_files/system/pins.h"      // ��� ���������� GPIO (IO_RBx_SetLow/High/GetValue)
#include "mcc_generated_files/timer/delay.h"     // ��� DELAY_milliseconds()

// --- ������������ ����� ������ E70 ---
// ��� ������� ������ ���� ������������ � ��������� ������ PIC18F46K22,
// ������������ � MCC Melody.
// �������, ��� � MCC Melody ��� ���� ����� ����������� ��������������� �����
// � ��� ��������� ��� �������� ������ (��� M0, M1, M2) � �������� ���� (��� AUX).

// ���� ������ ������ (M0, M1, M2)
// ���������: M0 -> RB0, M1 -> RB1, M2 -> RB2
#define E70_M0_SetLow()     IO_RB0_SetLow()      // ������������� RB0
#define E70_M0_SetHigh()    IO_RB0_SetHigh()     // ������������� RB0

#define E70_M1_SetLow()     IO_RB1_SetLow()      // ������������� RB1
#define E70_M1_SetHigh()    IO_RB1_SetHigh()     // ������������� RB1

#define E70_M2_SetLow()     IO_RB2_SetLow()      // ������������� RB2
#define E70_M2_SetHigh()    IO_RB2_SetHigh()     // ������������� RB2

// ��� AUX (��������� ��������� ������)
// ���������: AUX -> RB3
#define E70_AUX_GetValue()  IO_RB3_GetValue()   // ������������� RB3

// --- ��������� ��������� ---
#define E70_DEFAULT_AUX_TIMEOUT_MS          1000    /**< ������� �� ��������� ��� �������� AUX � ��. */
#define E70_DEFAULT_UART_RESPONSE_TIMEOUT_MS 2000    /**< ������� �� ��������� ��� ������ UART � ��. */
#define E70_MODE_SWITCH_DELAY_MS            2       /**< �������� ����� ����� ������ M0/M1/M2 � ��. */

// --- ������������ ��� ���������� ������ ---

/**
 * @brief ������������ ������� ������ ������ E70.
 * ������������ ����������� ��������� ����� M2, M1, M0.
 */
typedef enum {
    E70_MODE_RSSI           = 0b000, /**< ����� RSSI (M2=0, M1=0, M0=0). UART ������, ������������ ����� �������. ������ ������� �������� RSSI ������ 100��. */
    E70_MODE_CONTINUOUS     = 0b001, /**< ����������� ����� (M2=0, M1=0, M0=1). UART ������, ����������� ���������� ��������. �������� �� ������� �������������� ��� UART. */
    E70_MODE_SUBPACKAGE     = 0b010, /**< ����� ���������� (M2=0, M1=1, M0=0). UART ������, ���������� �������� �����������. �������� �� ������� � UART ����������. */
    E70_MODE_CONFIGURATION  = 0b011, /**< ����� ������������ (M2=0, M1=1, M0=1). UART ������, ������������ ����� �������. ��������� ������������� ����� UART 9600 8N1. */
    E70_MODE_WOR_TRANSMIT   = 0b100, /**< ����� ����������� (WOR) �������� (M2=1, M1=0, M0=0). UART ������, �������� �����������. ������������ ��� ����������� ��������� � ������ 6. */
    E70_MODE_CONFIGURATION_ALT = 0b101, /**< �������������� ����� ������������ (M2=1, M1=0, M0=1). ���������� E70_MODE_CONFIGURATION. */
    E70_MODE_POWER_SAVING   = 0b110, /**< ����� ���������������� (M2=1, M1=1, M0=0). UART ������, ������������ ����� � ������ WOR. �������� ������������ �����������. */
    E70_MODE_SLEEP          = 0b111  /**< ����� ��� (M2=1, M1=1, M2=1). UART ������, ������������ ����� �������. ����� �� ��� �� ��������� M0, M1, M2. */
} E70_Mode_t;

/**
 * @brief ������������ ��������� UART.
 * ������������ ��� ��������� ��������� SPED.
 */
typedef enum {
    E70_UART_BAUD_1200  = 0b000,
    E70_UART_BAUD_2400  = 0b001,
    E70_UART_BAUD_4800  = 0b010,
    E70_UART_BAUD_9600  = 0b011, /**< �� ��������� */
    E70_UART_BAUD_19200 = 0b100,
    E70_UART_BAUD_38400 = 0b101,
    E70_UART_BAUD_57600 = 0b110,
    E70_UART_BAUD_115200 = 0b111
} E70_UART_BaudRate_t;

/**
 * @brief ������������ ��������� �������� ������ �� ������� (Air Data Rate).
 * ������������ ��� ��������� ��������� SPED.
 */
typedef enum {
    E70_AIR_RATE_2_5K   = 0b000, /**< �� ��������� */
    E70_AIR_RATE_5K     = 0b001,
    E70_AIR_RATE_12K    = 0b010,
    E70_AIR_RATE_28K    = 0b011,
    E70_AIR_RATE_64K    = 0b100,
    E70_AIR_RATE_168K   = 0b101,
    // 0b110 � 0b111 ����� 168kbps �������� ������������
} E70_Air_DataRate_t;

/**
 * @brief ������������ ������� �������� UART.
 * ������������ ��� ��������� ��������� SPED.
 */
typedef enum {
    E70_PARITY_8N1 = 0b00, /**< 8N1 (�� ���������) */
    E70_PARITY_8O1 = 0b01, /**< 8O1 */
    E70_PARITY_8E1 = 0b10, /**< 8E1 */
    // 0b11 ����� 8N1
} E70_Parity_t;

/**
 * @brief ������������ ���� ������� ��� ������ ����������.
 * ������������ ��� ��������� ��������� CHAN.
 */
typedef enum {
    E70_PACKET_LEN_16   = 0b000,
    E70_PACKET_LEN_32   = 0b001,
    E70_PACKET_LEN_64   = 0b010, /**< �� ��������� */
    E70_PACKET_LEN_128  = 0b011,
    E70_PACKET_LEN_256  = 0b100,
    E70_PACKET_LEN_512  = 0b101,
    E70_PACKET_LEN_1024 = 0b110,
    E70_PACKET_LEN_2048 = 0b111
} E70_PacketLength_t;

/**
 * @brief ������������ ������� ����������� ��� ������� WOR.
 * ������������ ��� ��������� ��������� OPTION.
 */
typedef enum {
    E70_WAKEUP_500MS    = 0b000,
    E70_WAKEUP_1000MS   = 0b001, /**< �� ��������� */
    E70_WAKEUP_1500MS   = 0b010,
    E70_WAKEUP_2000MS   = 0b011,
    E70_WAKEUP_2500MS   = 0b100,
    E70_WAKEUP_3000MS   = 0b101,
    E70_WAKEUP_3500MS   = 0b110,
    E70_WAKEUP_4000MS   = 0b111
} E70_WakeupTime_t;

/**
 * @brief ������������ �������� ��������.
 * ������������ ��� ��������� ��������� OPTION.
 * �������� ��� E70-868T14S (14dBm).
 */
typedef enum {
    E70_POWER_14DBM = 0b00, /**< 14dBm (�� ��������� ��� T14S) */
    E70_POWER_10DBM = 0b01, /**< 10dBm */
    E70_POWER_7DBM  = 0b10, /**< 7dBm */
    E70_POWER_4DBM  = 0b11  /**< 4dBm */
} E70_Power_t;

/**
 * @brief ������������ ������� ������ IO (TXD, AUX, RXD).
 * ������������ ��� ��������� ��������� OPTION.
 */
typedef enum {
    E70_IO_PUSH_PULL_PULL_UP = 0b0, /**< TXD � AUX push-pull, RXD pull-up (�� ���������) */
    E70_IO_OPEN_DRAIN_OPEN_DRAIN = 0b1 /**< TXD � AUX open-collector, RXD open-collector */
} E70_IODriveMode_t;

/**
 * @brief ������������ ��������� FEC (Forward Error Correction).
 * ������������ ��� ��������� ��������� OPTION.
 */
typedef enum {
    E70_FEC_OFF = 0b0, /**< FEC �������� */
    E70_FEC_ON  = 0b1  /**< FEC ������� (�� ���������) */
} E70_FEC_t;

/**
 * @brief ������������ ������ ������������� ��������.
 * ������������ ��� ��������� ��������� OPTION.
 */
typedef enum {
    E70_FIXED_TRANSMISSION_OFF = 0b0, /**< ���������� ����� �������� (�� ���������) */
    E70_FIXED_TRANSMISSION_ON  = 0b1  /**< ������������� ����� �������� */
} E70_FixedTransmission_t;

/**
 * @brief ��������� ��� �������� ���� ������������� ���������� ������ E70.
 */
typedef struct {
    uint16_t address;               /**< ����� ������ (0x0000 - 0xFFFF) */
    uint8_t channel;                /**< ����� ����� (0x00 - 0x1F) */
    E70_Parity_t uart_parity;       /**< �������� UART */
    E70_UART_BaudRate_t uart_baud_rate; /**< �������� UART */
    E70_Air_DataRate_t air_data_rate; /**< �������� �������� �� ������� */
    E70_PacketLength_t packet_length; /**< ����� ������ (������ ��� ������ ����������) */
    E70_FixedTransmission_t fixed_transmission; /**< ����� ������������� �������� */
    E70_WakeupTime_t wakeup_time;   /**< ����� ����������� (��� WOR �������) */
    E70_FEC_t fec_enabled;          /**< ������� �� FEC */
    E70_IODriveMode_t io_drive_mode; /**< ����� ������ IO */
    E70_Power_t tx_power;           /**< �������� �������� */
} E70_Parameters_t;

/**
 * @brief ��������� ��� �������� ���������� � ������ ������.
 */
typedef struct {
    uint8_t model;      /**< ������ ������ (��������, 0x70 ��� E70) */
    uint8_t version;    /**< ����� ������ */
    uint8_t feature1;   /**< �������������� ����������� 1 */
    uint8_t feature2;   /**< �������������� ����������� 2 */
    uint8_t feature3;   /**< �������������� ����������� 3 */
    uint8_t feature4;   /**< �������������� ����������� 4 */
} E70_Version_t;


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

// --- ��������� ������� ---

// ���������� ����������� �������� ����������� �� main.c
extern volatile uint32_t system_ticks_ms;

/**
 * @brief ���������� ������� ��� �������� ������ ����� UART1.
 * @param data ��������� �� ����� � �������.
 * @param length ���������� ���� ��� ��������.
 * @remarks ���������� ����� �������� UART1, ���� ������ ���������� ������ � ���������� ��������.
 */
static void E70_UART_WriteBytes(uint8_t *data, uint16_t length);

/**
 * @brief ���������� ������� ��� ������ ������ �� UART1 � ���������.
 * @param buffer ��������� �� ����� ��� ������ �������� ������.
 * @param length ���������� ���� ��� ������.
 * @param timeout_ms ������� � ������������� �� ��� �������� ������.
 * @return true, ���� ��� ����� ������� ���������; false � ������ ��������.
 * @remarks ������� ����������� ������� ����� � ����� ������ UART1.
 */
static bool E70_UART_ReadBytes(uint8_t *buffer, uint8_t length, uint16_t timeout_ms);

/**
    <p><b>Function prototype:</b></p>
    <code>bool E70_Init(E70_Mode_t initial_mode)</code>
  
    <p><b>Summary:</b></p>
    �������������� ������ E70, ������������ ��������� ����� � ������ ����������.

    <p><b>Description:</b></p>
    ��� ������� ��������� ��������� ��������� ������ E70, �������� ��� � ���������
    ����� ������. ��� ������� ���������� ������ (��� AUX ��������� � ������� ���������)
    ����� ����� ������, ����������� ���������� �����.

    <p><b>Precondition:</b></p>
    MCC Melody UART1 � GPIO ������ ���� ����������������.

    <p><b>Parameters:</b></p>
    @param initial_mode �����, � ������� ������ ������ ������� ����� �������������.

    <p><b>Returns:</b></p>
    true, ���� ������������� � ������� � ����� �������; false � ��������� ������.

    <p><b>Example:</b></p>
    <code>
    // � main.c ��� ������ ����� �������������:
    // SYSTEM_Initialize(); // ������������� �������, UART1 � GPIO ����� MCC Melody
    // if (E70_Init(E70_MODE_CONFIGURATION)) {
    //     // ������ ������� ��������������� � ����� � ������
    // } else {
    //     // ������ ������������� ������
    // }
    </code>

    <p><b>Remarks:</b></p>
    ��� ������� �������� ����������� �� ��� ���, ���� ������ �� �������� � ��������� �����
    � �� ������������� � ���������� ����� ��� AUX.
 */
bool E70_Init(E70_Mode_t initial_mode);

/**
    <p><b>Function prototype:</b></p>
    <code>bool E70_SetMode(E70_Mode_t mode)</code>
  
    <p><b>Summary:</b></p>
    ������������� ����� ������ ������ E70 ����� ���������� ������ M0, M1, M2.

    <p><b>Description:</b></p>
    ������� ������� �������, ���� ������ �������� ������� �������� (��� AUX �������� � HIGH),
    ����� ������������� ���� M0, M1, M2 � ������������ � �������� �������. ����� ���������
    ��������, ��� ����� ������� ���������� ������.

    <p><b>Precondition:</b></p>
    MCC Melody GPIO ��� M0, M1, M2 (������) � AUX (����) ������ ���� ����������������.

    <p><b>Parameters:</b></p>
    @param mode �������� ����� ������ �� ������������ E70_Mode_t.

    <p><b>Returns:</b></p>
    true, ���� ����� ������ � �������� ���������� AUX �������; false � ��������� ������.

    <p><b>Example:</b></p>
    <code>
    if (E70_SetMode(E70_MODE_SUBPACKAGE)) { // ������������ ������ � ����� ����������
        // ������ ����� � ������ ����������
    } else {
        // ������ ����� ������ ��� ������� AUX
    }
    </code>

    <p><b>Remarks:</b></p>
    ��� ������� �������� ����������� �� ��� ���, ���� ������ �� �������� � ��������� �����
    � �� ������������� � ���������� ����� ��� AUX.
 */
bool E70_SetMode(E70_Mode_t mode);

/**
    <p><b>Function prototype:</b></p>
    <code>bool E70_WaitAuxHigh(uint16_t timeout_ms)</code>
  
    <p><b>Summary:</b></p>
    �������, ���� ��� AUX ������ E70 �������� � ������� ���������.

    <p><b>Description:</b></p>
    ��� ��������� �� ���������� ���������� �������� ������ (������������, �������������,
    ���������� �������� ������) � ��� ���������� � ������ ��� ������ ����� ������.
    ������� ���������� ��������� ������ ��� ������� ��������, ������������ ����������� ���������.

    <p><b>Precondition:</b></p>
    MCC Melody GPIO ��� AUX (����) � ��������� ������ (SYSTEM_GetTickCount()) ������ ���� ����������������.

    <p><b>Parameters:</b></p>
    @param timeout_ms ������������ ����� �������� � �������������.

    <p><b>Returns:</b></p>
    true, ���� AUX ������� � ������� ���������; false � ������ ��������.

    <p><b>Example:</b></p>
    <code>
    if (E70_WaitAuxHigh(2000)) { // ���� �� 2 ������
        // AUX ���� �������
    } else {
        // �������, AUX �� ���� �������
    }
    </code>

    <p><b>Remarks:</b></p>
    ��� ������� �������� �����������. ��� ������������ ���������,
    �������� E70_AUX_GetValue() � �������� ����� ���������.
 */
bool E70_WaitAuxHigh(uint16_t timeout_ms);

/**
    <p><b>Function prototype:</b></p>
    <code>bool E70_ResetModule(void)</code>
  
    <p><b>Summary:</b></p>
    ���������� ������� ������ ������ E70.

    <p><b>Description:</b></p>
    ������ �������������� ����� ��������� ������� ������. ��� AUX �������� � ������
    ��������� �� ����� ������, ����� � ������� ����� ����������. ������� �������
    ����������� ������ � ������� ���������.

    <p><b>Precondition:</b></p>
    MCC Melody UART1 � GPIO ��� M0, M1, M2, AUX ������ ���� ����������������.

    <p><b>Parameters:</b></p>
    None.

    <p><b>Returns:</b></p>
    true, ���� ������� ������ ���������� ������� � ������ �������� � ����������;
    false � ������ ������ �������� ��� �������� �������� AUX.

    <p><b>Example:</b></p>
    <code>
    if (E70_ResetModule()) {
        // ������ ������� ������� � �����
    } else {
        // ������ ������ ������
    }
    </code>

    <p><b>Remarks:</b></p>
    ��� ������� �������� ����������� �� ���������� �������� ������ ������.
 */
bool E70_ResetModule(void);

/**
    <p><b>Function prototype:</b></p>
    <code>bool E70_ReadParameters(E70_Parameters_t *params)</code>
  
    <p><b>Summary:</b></p>
    ������ ������� ����������� ��������� ������ E70.

    <p><b>Description:</b></p>
    ������ ������ ���������� � ������ ������������ (E70_MODE_CONFIGURATION).
    ������� ���������� ������� ������ ���������� � ������ ���������� �����
    � ��������� E70_Parameters_t. �������� ������� ��� �������� ������.

    <p><b>Precondition:</b></p>
    MCC Melody UART1 ������ ���� ���������������. ������ E70 ������ ���� � ������ ������������.

    <p><b>Parameters:</b></p>
    @param params ��������� �� ��������� E70_Parameters_t, ���� ����� �������� ����������� ���������.

    <p><b>Returns:</b></p>
    true, ���� ��������� ������� ���������; false � ��������� ������ (��������, ������� ��� ������������ �����).

    <p><b>Example:</b></p>
    <code>
    E70_Parameters_t currentParams;
    if (E70_SetMode(E70_MODE_CONFIGURATION) && E70_ReadParameters(&currentParams)) {
        // ��������� ������� ���������, ����� �� ������������
        // printf("�����: 0x%04X, �����: 0x%02X\r\n", currentParams.address, currentParams.channel);
    } else {
        // ������ ������ ���������� ��� �������
    }
    </code>

    <p><b>Remarks:</b></p>
    ��� ������� �������� ����������� �� ��������� ������ �� ������ ��� ��������� ��������.
 */
bool E70_ReadParameters(E70_Parameters_t *params);

/**
    <p><b>Function prototype:</b></p>
    <code>bool E70_SetParameters(E70_Parameters_t *params, bool save_to_flash)</code>
  
    <p><b>Summary:</b></p>
    ������������� ��������� ������ E70.

    <p><b>Description:</b></p>
    ������ ������ ���������� � ������ ������������ (E70_MODE_CONFIGURATION).
    ������� ��������� ������� ��������� ���������� �� ��������� E70_Parameters_t
    � ���������� �� ������. ����� ������� ���������� ���������� �� ����-������
    ������ (���������) ��� ��������� ����������. ����� �������� ������� ���������� ������.

    <p><b>Precondition:</b></p>
    MCC Melody UART1 ������ ���� ���������������. ������ E70 ������ ���� � ������ ������������.

    <p><b>Parameters:</b></p>
    @param params ��������� �� ��������� E70_Parameters_t � ��������� �����������.
    @param save_to_flash ���� true, ��������� ����� ��������� �� ����-������ ������ (������� C0).
                         ���� false, ��������� ����� ��������� �������� (������� C2) � ��������� ��� ����������.

    <p><b>Returns:</b></p>
    true, ���� ��������� ������� ����������� � ������ �������� � ����������; false � ��������� ������.

    <p><b>Example:</b></p>
    <code>
    E70_Parameters_t newParams;
    // ��������� newParams ��������� ����������
    newParams.address = 0x1234;
    newParams.channel = 0x05;
    // ... ������ ���������
    
    if (E70_SetMode(E70_MODE_CONFIGURATION) && E70_SetParameters(&newParams, true)) { // ���������� � ��������� �� ����
        // ��������� ������� �����������
    } else {
        // ������ ��������� ����������
    }
    </code>

    <p><b>Remarks:</b></p>
    ��� ������� �������� ����������� �� ���������� �������� ��������� ���������� � ����������� ������ � ������� ���������.
 */
bool E70_SetParameters(E70_Parameters_t *params, bool save_to_flash);

/**
    <p><b>Function prototype:</b></p>
    <code>bool E70_ReadVersion(E70_Version_t *version)</code>
  
    <p><b>Summary:</b></p>
    ������ ���������� � ������ ������ E70.

    <p><b>Description:</b></p>
    ������ ������ ���������� � ������ ������������ (E70_MODE_CONFIGURATION).
    ������� ���������� ������� ������ ������ � ������ ���������� �����
    � ��������� E70_Version_t. �������� ������� ��� �������� ������.

    <p><b>Precondition:</b></p>
    MCC Melody UART1 ������ ���� ���������������. ������ E70 ������ ���� � ������ ������������.

    <p><b>Parameters:</b></p>
    @param version ��������� �� ��������� E70_Version_t, ���� ����� �������� ���������� � ������.

    <p><b>Returns:</b></p>
    true, ���� ���������� � ������ ������� ���������; false � ��������� ������.

    <p><b>Example:</b></p>
    <code>
    E70_Version_t moduleVersion;
    if (E70_SetMode(E70_MODE_CONFIGURATION) && E70_ReadVersion(&moduleVersion)) {
        // ���������� � ������ ������� ���������
        // printf("������: 0x%02X, ������: 0x%02X\r\n", moduleVersion.model, moduleVersion.version);
    } else {
        // ������ ������ ������ ��� �������
    }
    </code>

    <p><b>Remarks:</b></p>
    ��� ������� �������� ����������� �� ��������� ������ �� ������ ��� ��������� ��������.
 */
bool E70_ReadVersion(E70_Version_t *version);

/**
    <p><b>Function prototype:</b></p>
    <code>bool E70_SendData(uint8_t *data, uint16_t length)</code>
  
    <p><b>Summary:</b></p>
    ���������� ������ ����� ������ E70 � ���������� ������.

    <p><b>Description:</b></p>
    ������ ������ ���������� � ������ ����������� �������� (E70_MODE_CONTINUOUS)
    ��� ������ ���������� (E70_MODE_SUBPACKAGE). ������� ������� ���������� ������,
    ����� �������� ������ � ����� �������� UART1. ����� ����� ������� ����������
    �������� ������ �� ����������� (��� AUX ����� ������ HIGH).

    <p><b>Precondition:</b></p>
    MCC Melody UART1 ������ ���� ���������������. ������ E70 ������ ���� � ������
    E70_MODE_CONTINUOUS ��� E70_MODE_SUBPACKAGE.

    <p><b>Parameters:</b></p>
    @param data ��������� �� ����� � ������� ��� ��������.
    @param length ����� ������ � ������.

    <p><b>Returns:</b></p>
    true, ���� ������ ������� ���������� � ������ �������� � ����������; false � ��������� ������.

    <p><b>Example:</b></p>
    <code>
    char message[] = "Hello, world!";
    if (E70_SetMode(E70_MODE_SUBPACKAGE) && E70_SendData((uint8_t*)message, sizeof(message) - 1)) {
        // ������ ������� ����������
    } else {
        // ������ �������� ������
    }
    </code>

    <p><b>Remarks:</b></p>
    � ������ ���������� ����� ������ ���������� ����������� ������ ������.
    � ����������� ������ ����� ������ �� ����������.
    ������� �������� ����������� �� ���������� �������� ������ �� �����������.
 */
bool E70_SendData(uint8_t *data, uint16_t length);

/**
    <p><b>Function prototype:</b></p>
    <code>bool E70_SendFixedData(uint16_t address, uint8_t channel, uint8_t *data, uint16_t length)</code>
  
    <p><b>Summary:</b></p>
    ���������� ������ ����� ������ E70 � ������ ������������� ��������.

    <p><b>Description:</b></p>
    ������ ������ ���������� � ������ ����������� �������� (E70_MODE_CONTINUOUS)
    ��� ������ ���������� (E70_MODE_SUBPACKAGE), � �������� `fixed_transmission`
    ������ ���� ���������� � `E70_FIXED_TRANSMISSION_ON`. ������� ���������
    ����� � ����� � ������ ������ ������, ����� ���������� �� ����� UART1.
    ����� ����� ������� ���������� �������� �� �����������.

    <p><b>Precondition:</b></p>
    MCC Melody UART1 ������ ���� ���������������. ������ E70 ������ ���� � ������
    E70_MODE_CONTINUOUS ��� E70_MODE_SUBPACKAGE, � �������� �� ������������� ��������.

    <p><b>Parameters:</b></p>
    @param address ����� �������� ������ (2 �����).
    @param channel ����� �������� ������ (1 ����).
    @param data ��������� �� ����� � ������� ��� ��������.
    @param length ����� ������ � ������.

    <p><b>Returns:</b></p>
    true, ���� ������ ������� ���������� � ������ �������� � ����������; false � ��������� ������.

    <p><b>Example:</b></p>
    <code>
    uint16_t target_addr = 0x0001;
    uint8_t target_channel = 0x04;
    char command[] = "STATUS";
    
    E70_Parameters_t currentParams;
    // ������������, ��� currentParams ��� ��������� ��� �����������
    currentParams.fixed_transmission = E70_FIXED_TRANSMISSION_ON;
    if (E70_SetMode(E70_MODE_CONFIGURATION) && E70_SetParameters(&currentParams, true) &&
        E70_SetMode(E70_MODE_SUBPACKAGE) &&
        E70_SendFixedData(target_addr, target_channel, (uint8_t*)command, sizeof(command) - 1)) {
        // ������ ������� ����������
    } else {
        // ������ �������� ������
    }
    </code>

    <p><b>Remarks:</b></p>
    ������ ��� ����� ������ ����� ���������������� ������� ��� ����� � �����.
    ������ �������� ������� ���� �����/����� ��� ���� ��������.
    ������� �������� ����������� �� ���������� �������� ������ �� �����������.
 */
bool E70_SendFixedData(uint16_t address, uint8_t channel, uint8_t *data, uint16_t length);

/**
    <p><b>Function prototype:</b></p>
    <code>bool E70_DataAvailable(void)</code>
  
    <p><b>Summary:</b></p>
    ��������� ������� �������� ������ � ������ UART1.

    <p><b>Description:</b></p>
    ���������� true, ���� � ������ ������ UART1 ���� ���� �� ���� ���� ������;
    false � ��������� ������.

    <p><b>Precondition:</b></p>
    MCC Melody UART1 ������ ���� ���������������.

    <p><b>Parameters:</b></p>
    None.

    <p><b>Returns:</b></p>
    true, ���� ������ ��������; false, ���� ����� ����.

    <p><b>Example:</b></p>
    <code>
    if (E70_DataAvailable()) { // ������ ���������� bool
        uint8_t receivedByte = E70_ReadByte();
        // ��������� ��������� �����
    }
    </code>

    <p><b>Remarks:</b></p>
    ��� ������� �� ��������� ����������.
 */
bool E70_DataAvailable(void);

/**
    <p><b>Function prototype:</b></p>
    <code>uint8_t E70_ReadByte(void)</code>
  
    <p><b>Summary:</b></p>
    ������ ���� ���� �� ������ UART1.

    <p><b>Description:</b></p>
    ��������� ���� ���� �� ���������� ������ ������ UART1.

    <p><b>Precondition:</b></p>
    MCC Melody UART1 ������ ���� ���������������. ����� ������� ���� �������
    ������������� ��������� ������� ������ � ������� E70_DataAvailable().

    <p><b>Parameters:</b></p>
    None.

    <p><b>Returns:</b></p>
    ����������� ����.

    <p><b>Example:</b></p>
    <code>
    uint8_t receivedByte = E70_ReadByte();
    </code>

    <p><b>Remarks:</b></p>
    ��� ������� ����� �����������, ���� ����� ����, � ����������� �� ���������� UART1_Read().
    ������ ���������� E70_DataAvailable() ����� �������.
 */
uint8_t E70_ReadByte(void);


#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* E70_868T14S_H */