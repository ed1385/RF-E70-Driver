/* Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */

/* 
 * File:   E70_868T14S.c
 * Author: [���� ��� / AI]
 * Comments: ���������� ������� ��� ���������� ���������� ������������ ������� E70-868T14S.
 * Revision history: 
 * v1.0 - �������� ������.
 * v1.1 - ��������� ��������� ������ � ��������.
 * v1.2 - ������� ������ (M0-RB0, M1-RB1, M2-RB2, AUX-RB3).
 * v1.3 - ���������� �� ������� MPLAB X, ������������� DELAY_milliseconds.
 * v1.4 - ���������� ���� ��������� MCC Melody ������ ������ ����������.
 * v1.5 - �������������� ��������� MCC Melody ������, ��������� ����������� system.h.
 * v1.6 - ���������� �� .h (����������) � .c (����������) �����.
 * v1.7 - ���������� ���� � eusart1.h � ������������� EUSART1_ �������.
 * v1.8 - ���������� ������ � 'config_cmd' � E70_ReadParameters.
 * v1.9 - �������� E70_DEFAULT_UART_RESPONSE_TIMEOUT_MS � ��������� ������� RX ������.
 * v1.10 - �������� ���������� E70_UART_WriteBytes � ����� ��������� ������ � ���������� TX.
 * v1.11 - ���������� ������ "EUSART1_TxBufferFull" �� "EUSART1_IsTxReady".
 * v1.12 - ���������� ������ "EUSART1_TxComplete" �� "EUSART1_IsTxDone".
 */

#include "E70_868T14S.h" // �������� ������������ ���� ����� ����������

// --- ���������� ���������� ������� ---

/**
 * @brief ���������� ������� ��� �������� ������ ����� UART1.
 * @param data ��������� �� ����� � �������.
 * @param length ���������� ���� ��� ��������.
 * @remarks ���������� ����� �������� UART1, ���� ������ ���������� ������ � ���������� ��������.
 */
static void E70_UART_WriteBytes(uint8_t *data, uint16_t length) {
    for (uint16_t i = 0; i < length; i++) { // ���������� uint16_t ��� ��������, ����� ��������������� length
        // ����, ���� ���������� ����� ����� ������� ����� ����
        while(!EUSART1_IsTxReady()); 
        EUSART1_Write(data[i]);
    }
    // ����� �������� ���� ������, ����, ���� ��������� ���� ��������� ������ �� ���������� �������� UART.
    // ��� ����� ��� ��������-��������������� ����������, ����� ������ ������� ��� ������� �������.
    while(!EUSART1_IsTxDone()); // ������������ EUSART1_IsTxDone()
}

/**
 * @brief ���������� ������� ��� ������ ������ �� UART1 � ��������� �� ������ ����.
 * @param buffer ��������� �� ����� ��� ������ �������� ������.
 * @param length ���������� ���� ��� ������.
 * @param timeout_ms ������� � ������������� �� ������ ����.
 * @return true, ���� ��� ����� ������� ���������; false � ������ ��������.
 * @remarks ������� ����������� ������� ����� � ����� ������ UART1.
 */
static bool E70_UART_ReadBytes(uint8_t *buffer, uint8_t length, uint16_t timeout_ms) {
    uint32_t byte_start_time; // ����� ������ �������� �������� �����

    for (uint8_t i = 0; i < length; i++) {
        byte_start_time = system_ticks_ms; // ����� �������� ��� ������� ������ �����
        while (!EUSART1_IsRxReady()) { // ���������� EUSART1_IsRxReady
            if ((system_ticks_ms - byte_start_time) >= timeout_ms) {
                return false; // ������� ����� ��� �������� �����
            }
            // ����� ����� �������� ����� SYSTEM_Tasks() ��� ������ ������� �����,
            // ���� ������������ RTOS ��� ������������� ���������������.
        }
        buffer[i] = EUSART1_Read(); // ���������� EUSART1_Read
    }
    return true;
}

/**
 * @brief ���������� ������� ��� ������� ������ ������ UART1.
 * @remarks ��������� ��� ��������� ����� �� ������, ���� �� �� ������ ������.
 */
static void E70_UART_RxBuffer_Clear(void) {
    while (EUSART1_IsRxReady()) { // ���������, ���� �� ������
        EUSART1_Read(); // ��������� ����, ����� �������� ��� �� ������
    }
}

// --- ���������� ��������� ������� ---

bool E70_Init(E70_Mode_t initial_mode) {
    // ������������� UART1 � GPIO ������ ���� ��������� MCC Melody � main.c
    // ����� ������ ������������� ��������� ����� � ���� ����������.
    return E70_SetMode(initial_mode);
}

bool E70_SetMode(E70_Mode_t mode) {
    // ����������, ���� ������ �������� ������� �������� (AUX = HIGH)
    // ����� ������ ������, ��� ������������� � ������������.
    // ���� AUX �� ��������� � HIGH, ��� ��� ��������.
    if (!E70_WaitAuxHigh(E70_DEFAULT_AUX_TIMEOUT_MS)) {
        return false; // ������ �� ����� � ����� ������
    }

    // ������������� ���� M2, M1, M0 � ������������ � �������� �������
    if ((mode >> 2) & 0x01) { // M2
        E70_M2_SetHigh();
    } else {
        E70_M2_SetLow();
    }

    if ((mode >> 1) & 0x01) { // M1
        E70_M1_SetHigh();
    } else {
        E70_M1_SetLow();
    }

    if (mode & 0x01) { // M0
        E70_M0_SetHigh();
    } else {
        E70_M0_SetLow();
    }

    // ��������� �������� ��� ������������ ����� � �������� ������ � ����� �����
    DELAY_milliseconds(E70_MODE_SWITCH_DELAY_MS); // ������������� DELAY_milliseconds �� MCC Melody

    // ����, ���� ������ �������� ���������� ���������������� � ����� ������
    return E70_WaitAuxHigh(E70_DEFAULT_AUX_TIMEOUT_MS);
}

bool E70_WaitAuxHigh(uint16_t timeout_ms) {
    uint32_t start_time = system_ticks_ms;

    // ����, ���� ��� AUX ������ �������
    // ��� ��������, ��� ������ �������� ������������, �������������
    // ��� �������� ������ � ����� � ��������� ��������.
    while (E70_AUX_GetValue() == 0) {
        if ((system_ticks_ms - start_time) >= timeout_ms) {
            return false; // ������� �����, AUX �� ���� �������
        }
        // �������� ��������. � �������� ���������� ����� ��������
        // ����� SYSTEM_Tasks() ��� ������ ������� �����.
    }
    return true; // AUX ���� �������
}

bool E70_ResetModule(void) {
    // ����������� ������ � ����� ������������ ��� �������� ������� ������
    if (!E70_SetMode(E70_MODE_CONFIGURATION)) {
        return false; // �� ������� ������� � ����� ������������
    }

    E70_UART_RxBuffer_Clear(); // ������� ����� RX ����� ��������� �������
    uint8_t reset_cmd[] = {0xC4, 0xC4, 0xC4};
    E70_UART_WriteBytes(reset_cmd, sizeof(reset_cmd));

    // ����� �������� ������� ������, ������ ��������������.
    // AUX �������� � ������ ���������, ����� � �������.
    // ���� ���������� ������ � ���������� ������.
    return E70_WaitAuxHigh(E70_DEFAULT_AUX_TIMEOUT_MS);
}

bool E70_ReadParameters(E70_Parameters_t *params) {
    if (params == NULL) return false;

    // ��������, ��� ������ � ������ ������������
    if (!E70_SetMode(E70_MODE_CONFIGURATION)) {
        return false; // �� ������� ������� � ����� ������������
    }

    E70_UART_RxBuffer_Clear(); // ������� ����� RX ����� ��������� �������
    uint8_t read_cmd[] = {0xC1, 0xC1, 0xC1};
    E70_UART_WriteBytes(read_cmd, sizeof(read_cmd));

    uint8_t response[6]; // C0 + 5 ���� ����������
    if (!E70_UART_ReadBytes(response, sizeof(response), E70_DEFAULT_UART_RESPONSE_TIMEOUT_MS)) {
        return false; // ������� ��� ������ ������
    }

    if (response[0] != 0xC0) { // ��������� ��������� ������
        return false; // ������������ ��������� ������
    }

    // ��������� ���������� ����� � ��������� E70_Parameters_t
    params->address = ((uint16_t)response[1] << 8) | response[2];

    // SPED ���� (response[3])
    params->uart_parity = (E70_Parity_t)((response[3] >> 6) & 0x03);
    params->uart_baud_rate = (E70_UART_BaudRate_t)((response[3] >> 3) & 0x07);
    params->air_data_rate = (E70_Air_DataRate_t)(response[3] & 0x07);

    // CHAN ���� (response[4])
    params->packet_length = (E70_PacketLength_t)((response[4] >> 5) & 0x07);
    params->channel = response[4] & 0x1F;

    // OPTION ���� (response[5])
    params->fixed_transmission = (E70_FixedTransmission_t)((response[5] >> 7) & 0x01);
    params->wakeup_time = (E70_WakeupTime_t)((response[5] >> 4) & 0x07);
    params->fec_enabled = (E70_FEC_t)((response[5] >> 3) & 0x01);
    params->io_drive_mode = (E70_IODriveMode_t)((response[5] >> 2) & 0x01);
    params->tx_power = (E70_Power_t)(response[5] & 0x03);

    return true;
}

bool E70_SetParameters(E70_Parameters_t *params, bool save_to_flash) {
    if (params == NULL) return false;

    // ��������, ��� ������ � ������ ������������
    if (!E70_SetMode(E70_MODE_CONFIGURATION)) {
        return false; // �� ������� ������� � ����� ������������
    }

    E70_UART_RxBuffer_Clear(); // ������� ����� RX ����� ��������� �������
    uint8_t cmd_header = save_to_flash ? 0xC0 : 0xC2;
    uint8_t config_cmd[6];
    config_cmd[0] = cmd_header;
    config_cmd[1] = (uint8_t)(params->address >> 8); // ADDH
    config_cmd[2] = (uint8_t)(params->address & 0xFF); // ADDL

    // SPED ����
    config_cmd[3] = (uint8_t)(((params->uart_parity & 0x03) << 6) |
                              ((params->uart_baud_rate & 0x07) << 3) |
                              (params->air_data_rate & 0x07));

    // CHAN ����
    config_cmd[4] = (uint8_t)(((params->packet_length & 0x07) << 5) |
                              (params->channel & 0x1F));

    // OPTION ����
    config_cmd[5] = (uint8_t)(((params->fixed_transmission & 0x01) << 7) |
                              ((params->wakeup_time & 0x07) << 4) |
                              ((params->fec_enabled & 0x01) << 3) |
                              ((params->io_drive_mode & 0x01) << 2) |
                              (params->tx_power & 0x03));

    E70_UART_WriteBytes(config_cmd, sizeof(config_cmd));

    // ������ �� ���������� ������������� ����� ��������� ����������.
    // ������ ����, ���� AUX ����� ������ ������� ����� ���������� ����������������.
    return E70_WaitAuxHigh(E70_DEFAULT_AUX_TIMEOUT_MS);
}

bool E70_ReadVersion(E70_Version_t *version) {
    if (version == NULL) return false;

    // ��������, ��� ������ � ������ ������������
    if (!E70_SetMode(E70_MODE_CONFIGURATION)) {
        return false; // �� ������� ������� � ����� ������������
    }

    E70_UART_RxBuffer_Clear(); // ������� ����� RX ����� ��������� �������
    uint8_t read_cmd[] = {0xC3, 0xC3, 0xC3};
    E70_UART_WriteBytes(read_cmd, sizeof(read_cmd));

    uint8_t response[7]; // C3 + 6 ���� ������
    if (!E70_UART_ReadBytes(response, sizeof(response), E70_DEFAULT_UART_RESPONSE_TIMEOUT_MS)) {
        return false; // ������� ��� ������ ������
    }

    if (response[0] != 0xC3) { // ��������� ��������� ������
        return false; // ������������ ��������� ������
    }

    version->model = response[1];
    version->version = response[2];
    version->feature1 = response[3];
    version->feature2 = response[4];
    version->feature3 = response[5];
    version->feature4 = response[6];

    return true;
}

bool E70_SendData(uint8_t *data, uint16_t length) {
    // ��������, ��� ������ � ������ �������� (Continuous ��� Sub-package)
    // E70_SetMode() ������ ���� ������ ����� ��� ��������� ������� ������.
    if (!E70_WaitAuxHigh(E70_DEFAULT_AUX_TIMEOUT_MS)) {
        return false; // ������ �� ����� � ������ ������ �� UART
    }

    E70_UART_RxBuffer_Clear(); // ������� ����� RX ����� ��������� ������
    E70_UART_WriteBytes(data, length);

    // ����� �������� ������, AUX �������� � ������ ���������, ���� ������
    // ���������� �� �������, ����� ����� � �������.
    return E70_WaitAuxHigh(E70_DEFAULT_AUX_TIMEOUT_MS);
}

bool E70_SendFixedData(uint16_t address, uint8_t channel, uint8_t *data, uint16_t length) {
    // ��������, ��� ������ � ������ �������� (Continuous ��� Sub-package)
    // � �������� �� ������������� ��������.
    // E70_SetMode() ������ ���� ������ ����� ��� ��������� ������� ������.
    if (!E70_WaitAuxHigh(E70_DEFAULT_AUX_TIMEOUT_MS)) {
        return false; // ������ �� ����� � ������ ������ �� UART
    }

    E70_UART_RxBuffer_Clear(); // ������� ����� RX ����� ��������� ������
    uint8_t header[3];
    header[0] = (uint8_t)(address >> 8); // ADDH
    header[1] = (uint8_t)(address & 0xFF); // ADDL
    header[2] = channel;

    E70_UART_WriteBytes(header, sizeof(header));
    E70_UART_WriteBytes(data, length);

    // ����� �������� ������, AUX �������� � ������ ���������, ���� ������
    // ���������� �� �������, ����� ����� � �������.
    return E70_WaitAuxHigh(E70_DEFAULT_AUX_TIMEOUT_MS);
}

bool E70_DataAvailable(void) { // ������� ��� ������������� �������� �� bool
    return EUSART1_IsRxReady(); // ���������� EUSART1_IsRxReady
}

uint8_t E70_ReadByte(void) {
    return EUSART1_Read(); // ���������� EUSART1_Read
}