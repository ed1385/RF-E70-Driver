/* Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */

/* 
 * File:   E70_868T14S.c
 * Author: [Ваше имя / AI]
 * Comments: Реализация функций для библиотеки управления беспроводным модулем E70-868T14S.
 * Revision history: 
 * v1.0 - Исходная версия.
 * v1.1 - Добавлена обработка ошибок и таймауты.
 * v1.2 - Уточнен пинаут (M0-RB0, M1-RB1, M2-RB2, AUX-RB3).
 * v1.3 - Оформление по шаблону MPLAB X, использование DELAY_milliseconds.
 * v1.4 - Исправлены пути включения MCC Melody файлов внутри библиотеки.
 * v1.5 - Оптимизированы включения MCC Melody файлов, используя центральный system.h.
 * v1.6 - Разделение на .h (объявления) и .c (реализации) файлы.
 * v1.7 - Исправлены пути к eusart1.h и использование EUSART1_ функций.
 * v1.8 - Исправлена ошибка с 'config_cmd' в E70_ReadParameters.
 * v1.9 - Увеличен E70_DEFAULT_UART_RESPONSE_TIMEOUT_MS и добавлена очистка RX буфера.
 * v1.10 - Улучшена надежность E70_UART_WriteBytes с явной проверкой буфера и завершения TX.
 * v1.11 - Исправлена ошибка "EUSART1_TxBufferFull" на "EUSART1_IsTxReady".
 * v1.12 - Исправлена ошибка "EUSART1_TxComplete" на "EUSART1_IsTxDone".
 */

#include "E70_868T14S.h" // Включаем заголовочный файл нашей библиотеки

// --- Реализация внутренних функций ---

/**
 * @brief Внутренняя функция для отправки байтов через UART1.
 * @param data Указатель на буфер с данными.
 * @param length Количество байт для отправки.
 * @remarks Использует буфер передачи UART1, явно ожидая готовности буфера и завершения передачи.
 */
static void E70_UART_WriteBytes(uint8_t *data, uint16_t length) {
    for (uint16_t i = 0; i < length; i++) { // Используем uint16_t для счетчика, чтобы соответствовать length
        // Ждем, пока передатчик будет готов принять новый байт
        while(!EUSART1_IsTxReady()); 
        EUSART1_Write(data[i]);
    }
    // После отправки всех байтов, ждем, пока последний байт полностью выйдет из сдвигового регистра UART.
    // Это важно для командно-ориентированных протоколов, чтобы модуль получил всю команду целиком.
    while(!EUSART1_IsTxDone()); // Использована EUSART1_IsTxDone()
}

/**
 * @brief Внутренняя функция для чтения байтов из UART1 с таймаутом на каждый байт.
 * @param buffer Указатель на буфер для записи принятых данных.
 * @param length Количество байт для чтения.
 * @param timeout_ms Таймаут в миллисекундах на каждый байт.
 * @return true, если все байты успешно прочитаны; false в случае таймаута.
 * @remarks Ожидает поступления каждого байта в буфер приема UART1.
 */
static bool E70_UART_ReadBytes(uint8_t *buffer, uint8_t length, uint16_t timeout_ms) {
    uint32_t byte_start_time; // Время начала ожидания текущего байта

    for (uint8_t i = 0; i < length; i++) {
        byte_start_time = system_ticks_ms; // Сброс таймаута для каждого нового байта
        while (!EUSART1_IsRxReady()) { // Используем EUSART1_IsRxReady
            if ((system_ticks_ms - byte_start_time) >= timeout_ms) {
                return false; // Таймаут истек для текущего байта
            }
            // Здесь можно добавить вызов SYSTEM_Tasks() или других фоновых задач,
            // если используется RTOS или кооперативная многозадачность.
        }
        buffer[i] = EUSART1_Read(); // Используем EUSART1_Read
    }
    return true;
}

/**
 * @brief Внутренняя функция для очистки буфера приема UART1.
 * @remarks Считывает все доступные байты из буфера, пока он не станет пустым.
 */
static void E70_UART_RxBuffer_Clear(void) {
    while (EUSART1_IsRxReady()) { // Проверяем, есть ли данные
        EUSART1_Read(); // Считываем байт, чтобы очистить его из буфера
    }
}

// --- Реализация публичных функций ---

bool E70_Init(E70_Mode_t initial_mode) {
    // Инициализация UART1 и GPIO должна быть выполнена MCC Melody в main.c
    // Здесь просто устанавливаем начальный режим и ждем готовности.
    return E70_SetMode(initial_mode);
}

bool E70_SetMode(E70_Mode_t mode) {
    // Дожидаемся, пока модуль завершит текущие операции (AUX = HIGH)
    // перед сменой режима, как рекомендовано в документации.
    // Если AUX не переходит в HIGH, это уже проблема.
    if (!E70_WaitAuxHigh(E70_DEFAULT_AUX_TIMEOUT_MS)) {
        return false; // Модуль не готов к смене режима
    }

    // Устанавливаем пины M2, M1, M0 в соответствии с желаемым режимом
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

    // Небольшая задержка для стабилизации пинов и перехода модуля в новый режим
    DELAY_milliseconds(E70_MODE_SWITCH_DELAY_MS); // Использование DELAY_milliseconds из MCC Melody

    // Ждем, пока модуль завершит внутреннюю переконфигурацию в новом режиме
    return E70_WaitAuxHigh(E70_DEFAULT_AUX_TIMEOUT_MS);
}

bool E70_WaitAuxHigh(uint16_t timeout_ms) {
    uint32_t start_time = system_ticks_ms;

    // Ждем, пока пин AUX станет высоким
    // Это означает, что модуль завершил самопроверку, инициализацию
    // или передачу данных и готов к следующей операции.
    while (E70_AUX_GetValue() == 0) {
        if ((system_ticks_ms - start_time) >= timeout_ms) {
            return false; // Таймаут истек, AUX не стал высоким
        }
        // Активное ожидание. В реальном приложении можно добавить
        // вызов SYSTEM_Tasks() или других фоновых задач.
    }
    return true; // AUX стал высоким
}

bool E70_ResetModule(void) {
    // Переключаем модуль в режим конфигурации для отправки команды сброса
    if (!E70_SetMode(E70_MODE_CONFIGURATION)) {
        return false; // Не удалось перейти в режим конфигурации
    }

    E70_UART_RxBuffer_Clear(); // Очищаем буфер RX перед отправкой команды
    uint8_t reset_cmd[] = {0xC4, 0xC4, 0xC4};
    E70_UART_WriteBytes(reset_cmd, sizeof(reset_cmd));

    // После отправки команды сброса, модуль перезагрузится.
    // AUX перейдет в низкое состояние, затем в высокое.
    // Ждем завершения сброса и готовности модуля.
    return E70_WaitAuxHigh(E70_DEFAULT_AUX_TIMEOUT_MS);
}

bool E70_ReadParameters(E70_Parameters_t *params) {
    if (params == NULL) return false;

    // Убедимся, что модуль в режиме конфигурации
    if (!E70_SetMode(E70_MODE_CONFIGURATION)) {
        return false; // Не удалось перейти в режим конфигурации
    }

    E70_UART_RxBuffer_Clear(); // Очищаем буфер RX перед отправкой команды
    uint8_t read_cmd[] = {0xC1, 0xC1, 0xC1};
    E70_UART_WriteBytes(read_cmd, sizeof(read_cmd));

    uint8_t response[6]; // C0 + 5 байт параметров
    if (!E70_UART_ReadBytes(response, sizeof(response), E70_DEFAULT_UART_RESPONSE_TIMEOUT_MS)) {
        return false; // Таймаут при чтении ответа
    }

    if (response[0] != 0xC0) { // Проверяем заголовок ответа
        return false; // Некорректный заголовок ответа
    }

    // Разбираем полученные байты в структуру E70_Parameters_t
    params->address = ((uint16_t)response[1] << 8) | response[2];

    // SPED байт (response[3])
    params->uart_parity = (E70_Parity_t)((response[3] >> 6) & 0x03);
    params->uart_baud_rate = (E70_UART_BaudRate_t)((response[3] >> 3) & 0x07);
    params->air_data_rate = (E70_Air_DataRate_t)(response[3] & 0x07);

    // CHAN байт (response[4])
    params->packet_length = (E70_PacketLength_t)((response[4] >> 5) & 0x07);
    params->channel = response[4] & 0x1F;

    // OPTION байт (response[5])
    params->fixed_transmission = (E70_FixedTransmission_t)((response[5] >> 7) & 0x01);
    params->wakeup_time = (E70_WakeupTime_t)((response[5] >> 4) & 0x07);
    params->fec_enabled = (E70_FEC_t)((response[5] >> 3) & 0x01);
    params->io_drive_mode = (E70_IODriveMode_t)((response[5] >> 2) & 0x01);
    params->tx_power = (E70_Power_t)(response[5] & 0x03);

    return true;
}

bool E70_SetParameters(E70_Parameters_t *params, bool save_to_flash) {
    if (params == NULL) return false;

    // Убедимся, что модуль в режиме конфигурации
    if (!E70_SetMode(E70_MODE_CONFIGURATION)) {
        return false; // Не удалось перейти в режим конфигурации
    }

    E70_UART_RxBuffer_Clear(); // Очищаем буфер RX перед отправкой команды
    uint8_t cmd_header = save_to_flash ? 0xC0 : 0xC2;
    uint8_t config_cmd[6];
    config_cmd[0] = cmd_header;
    config_cmd[1] = (uint8_t)(params->address >> 8); // ADDH
    config_cmd[2] = (uint8_t)(params->address & 0xFF); // ADDL

    // SPED байт
    config_cmd[3] = (uint8_t)(((params->uart_parity & 0x03) << 6) |
                              ((params->uart_baud_rate & 0x07) << 3) |
                              (params->air_data_rate & 0x07));

    // CHAN байт
    config_cmd[4] = (uint8_t)(((params->packet_length & 0x07) << 5) |
                              (params->channel & 0x1F));

    // OPTION байт
    config_cmd[5] = (uint8_t)(((params->fixed_transmission & 0x01) << 7) |
                              ((params->wakeup_time & 0x07) << 4) |
                              ((params->fec_enabled & 0x01) << 3) |
                              ((params->io_drive_mode & 0x01) << 2) |
                              (params->tx_power & 0x03));

    E70_UART_WriteBytes(config_cmd, sizeof(config_cmd));

    // Модуль не отправляет подтверждение после установки параметров.
    // Просто ждем, пока AUX снова станет высоким после внутренней переконфигурации.
    return E70_WaitAuxHigh(E70_DEFAULT_AUX_TIMEOUT_MS);
}

bool E70_ReadVersion(E70_Version_t *version) {
    if (version == NULL) return false;

    // Убедимся, что модуль в режиме конфигурации
    if (!E70_SetMode(E70_MODE_CONFIGURATION)) {
        return false; // Не удалось перейти в режим конфигурации
    }

    E70_UART_RxBuffer_Clear(); // Очищаем буфер RX перед отправкой команды
    uint8_t read_cmd[] = {0xC3, 0xC3, 0xC3};
    E70_UART_WriteBytes(read_cmd, sizeof(read_cmd));

    uint8_t response[7]; // C3 + 6 байт версии
    if (!E70_UART_ReadBytes(response, sizeof(response), E70_DEFAULT_UART_RESPONSE_TIMEOUT_MS)) {
        return false; // Таймаут при чтении ответа
    }

    if (response[0] != 0xC3) { // Проверяем заголовок ответа
        return false; // Некорректный заголовок ответа
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
    // Убедимся, что модуль в режиме передачи (Continuous или Sub-package)
    // E70_SetMode() должен быть вызван ранее для установки нужного режима.
    if (!E70_WaitAuxHigh(E70_DEFAULT_AUX_TIMEOUT_MS)) {
        return false; // Модуль не готов к приему данных по UART
    }

    E70_UART_RxBuffer_Clear(); // Очищаем буфер RX перед отправкой данных
    E70_UART_WriteBytes(data, length);

    // После отправки данных, AUX перейдет в низкое состояние, пока данные
    // передаются по воздуху, затем снова в высокое.
    return E70_WaitAuxHigh(E70_DEFAULT_AUX_TIMEOUT_MS);
}

bool E70_SendFixedData(uint16_t address, uint8_t channel, uint8_t *data, uint16_t length) {
    // Убедимся, что модуль в режиме передачи (Continuous или Sub-package)
    // и настроен на фиксированную передачу.
    // E70_SetMode() должен быть вызван ранее для установки нужного режима.
    if (!E70_WaitAuxHigh(E70_DEFAULT_AUX_TIMEOUT_MS)) {
        return false; // Модуль не готов к приему данных по UART
    }

    E70_UART_RxBuffer_Clear(); // Очищаем буфер RX перед отправкой данных
    uint8_t header[3];
    header[0] = (uint8_t)(address >> 8); // ADDH
    header[1] = (uint8_t)(address & 0xFF); // ADDL
    header[2] = channel;

    E70_UART_WriteBytes(header, sizeof(header));
    E70_UART_WriteBytes(data, length);

    // После отправки данных, AUX перейдет в низкое состояние, пока данные
    // передаются по воздуху, затем снова в высокое.
    return E70_WaitAuxHigh(E70_DEFAULT_AUX_TIMEOUT_MS);
}

bool E70_DataAvailable(void) { // Изменен тип возвращаемого значения на bool
    return EUSART1_IsRxReady(); // Используем EUSART1_IsRxReady
}

uint8_t E70_ReadByte(void) {
    return EUSART1_Read(); // Используем EUSART1_Read
}