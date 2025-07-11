/*
 * MAIN Generated Driver File
 * 
 * @file main.c
 * 
 * @defgroup main MAIN
 * 
 * @brief This is the generated driver implementation file for the MAIN driver.
 *
 * @version MAIN Driver Version 1.0.2
 *
 * @version Package Version: 3.1.2
*/


#include <xc.h>
#include <stdio.h>      // Для printf
#include <stdlib.h>     // Для rand(), srand()
#include <string.h>     // Для strlen()
#include <time.h>       // Для time() (если доступно, иначе SYSTEM_GetTickCount())

// ВАЖНО: Убедитесь, что в MCC Melody настроены:
// 1. System Module: Internal RC Oscillator (FRC) на 8 MHz.
// 2. UART1: 9600 baud, 8N1 (для связи с модулем E70).
// 3. UART2: 115200 baud, 8N1 (для отладочного вывода).
// 4. GPIO пины для M0 (RB0), M1 (RB1), M2 (RB2) как выходы, и AUX (RB3) как вход.
// 5. Timer1: Настроен на генерацию прерывания по переполнению каждые 1 мс.

#include "mcc_generated_files/system/system.h" // Инициализация системы, включая UART и GPIO
#include "mcc_generated_files/uart/eusart1.h"   // Функции UART1 от MCC Melody (для E70)
#include "mcc_generated_files/uart/eusart2.h"   // Функции UART2 от MCC Melody (для отладки)
#include "mcc_generated_files/timer/tmr1.h"    // Функции Timer1 от MCC Melody
#include "mcc_generated_files/system/interrupt.h" // Для включения прерываний
#include "mcc_generated_files/timer/delay.h"   // Для функций задержки MCC Melody (DELAY_milliseconds)
#include "E70_868T14S.h"                       // Наша библиотека E70

// --- КОНФИГУРАЦИЯ ПЛАТЫ ---
// Раскомментируй одну из следующих строк, чтобы определить роль платы.
#define E70_IS_TRANSMITTER
//#define E70_IS_RECEIVER

// --- НАСТРОЙКИ МОДУЛЯ E70 ДЛЯ ПРИМЕРА ---
// Все эти настройки будут применены к модулю при его инициализации.
// Убедись, что они одинаковы для передатчика и приемника (кроме адреса, если используется фиксированная передача).
#define E70_CONFIG_ADDRESS              0x1234  // Адрес модуля (0x0000 - 0xFFFF)
#define E70_CONFIG_CHANNEL              0x0A    // Канал связи (0x00 - 0x1F)

#define E70_CONFIG_UART_PARITY          E70_PARITY_8N1      // Четность UART
#define E70_CONFIG_UART_BAUD_RATE       E70_UART_BAUD_9600  // Скорость UART
#define E70_CONFIG_AIR_DATA_RATE        E70_AIR_RATE_2_5K   // Скорость передачи по воздуху

#define E70_CONFIG_PACKET_LENGTH        E70_PACKET_LEN_64   // Длина пакета (только для режима подпакетов)
#define E70_CONFIG_FIXED_TRANSMISSION   E70_FIXED_TRANSMISSION_OFF // Режим фиксированной передачи (OFF для прозрачной)

#define E70_CONFIG_WAKEUP_TIME          E70_WAKEUP_1000MS   // Время пробуждения (для WOR режимов)
#define E70_CONFIG_FEC_ENABLED          E70_FEC_ON          // Включен ли FEC
#define E70_CONFIG_IO_DRIVE_MODE        E70_IO_PUSH_PULL_PULL_UP // Режим работы IO (TXD, AUX, RXD)
#define E70_CONFIG_TX_POWER             E70_POWER_4DBM     // Мощность передачи (для E70-868T14S)

// --- Глобальный счетчик миллисекунд ---
// Эта переменная будет инкрементироваться в прерывании Timer1
volatile uint32_t system_ticks_ms = 0;

// --- Коллбэк функция для прерывания Timer1 ---
// Эта функция будет вызываться MCC Melody драйвером Timer1 при каждом переполнении.
void TMR1_Millisecond_ISR_Callback(void) {
    system_ticks_ms++;
}

// --- putch для printf ---
// Эту функцию НЕ НУЖНО определять здесь, если MCC Melody уже сгенерировал ее.
// MCC Melody обычно генерирует putch (или _putch) в файле eusart2.c (или eusart1.c)
// для перенаправления printf. Удаление этой функции решает ошибку "redefined".
// void putch(char data) {
//     EUSART2_Write(data); // Используем EUSART2_Write для UART2
// }

/**
 * @brief Выводит текущие параметры модуля E70 в отладочный порт.
 * @param params Указатель на структуру с параметрами.
 */
void print_e70_parameters(const E70_Parameters_t *params) {
    printf("--- Current E70 Module Parameters ---\r\n");
    printf("  Address: 0x%04X\r\n", params->address);
    printf("  Channel: 0x%02X\r\n", params->channel);

    printf("  UART: ");
    switch (params->uart_baud_rate) {
        case E70_UART_BAUD_1200: printf("1200"); break;
        case E70_UART_BAUD_2400: printf("2400"); break;
        case E70_UART_BAUD_4800: printf("4800"); break;
        case E70_UART_BAUD_9600: printf("9600"); break;
        case E70_UART_BAUD_19200: printf("19200"); break;
        case E70_UART_BAUD_38400: printf("38400"); break;
        case E70_UART_BAUD_57600: printf("57600"); break;
        case E70_UART_BAUD_115200: printf("115200"); break;
        default: printf("Unknown"); break;
    }
    printf(" baud, ");
    switch (params->uart_parity) {
        case E70_PARITY_8N1: printf("8N1"); break;
        case E70_PARITY_8O1: printf("8O1"); break;
        case E70_PARITY_8E1: printf("8E1"); break;
        default: printf("Unknown"); break;
    }
    printf("\r\n");

    printf("  Air Data Rate: ");
    switch (params->air_data_rate) {
        case E70_AIR_RATE_2_5K: printf("2.5kbps"); break;
        case E70_AIR_RATE_5K: printf("5kbps"); break;
        case E70_AIR_RATE_12K: printf("12kbps"); break;
        case E70_AIR_RATE_28K: printf("28kbps"); break;
        case E70_AIR_RATE_64K: printf("64kbps"); break;
        case E70_AIR_RATE_168K: printf("168kbps"); break;
        default: printf("Unknown"); break;
    }
    printf("\r\n");

    printf("  Packet Length (Sub-package): ");
    switch (params->packet_length) {
        case E70_PACKET_LEN_16: printf("16 bytes"); break;
        case E70_PACKET_LEN_32: printf("32 bytes"); break;
        case E70_PACKET_LEN_64: printf("64 bytes"); break;
        case E70_PACKET_LEN_128: printf("128 bytes"); break;
        case E70_PACKET_LEN_256: printf("256 bytes"); break;
        case E70_PACKET_LEN_512: printf("512 bytes"); break;
        case E70_PACKET_LEN_1024: printf("1024 bytes"); break;
        case E70_PACKET_LEN_2048: printf("2048 bytes"); break;
        default: printf("Unknown"); break;
    }
    printf("\r\n");

    printf("  Fixed Transmission: %s\r\n", params->fixed_transmission == E70_FIXED_TRANSMISSION_ON ? "Enabled" : "Disabled");
    printf("  Wakeup Time (WOR): ");
    switch (params->wakeup_time) {
        case E70_WAKEUP_500MS: printf("500ms"); break;
        case E70_WAKEUP_1000MS: printf("1000ms"); break;
        case E70_WAKEUP_1500MS: printf("1500ms"); break;
        case E70_WAKEUP_2000MS: printf("2000ms"); break;
        case E70_WAKEUP_2500MS: printf("2500ms"); break;
        case E70_WAKEUP_3000MS: printf("3000ms"); break;
        case E70_WAKEUP_3500MS: printf("3500ms"); break;
        case E70_WAKEUP_4000MS: printf("4000ms"); break;
        default: printf("Unknown"); break;
    }
    printf("\r\n");

    printf("  FEC: %s\r\n", params->fec_enabled == E70_FEC_ON ? "Enabled" : "Disabled");
    printf("  IO Mode: %s\r\n", params->io_drive_mode == E70_IO_PUSH_PULL_PULL_UP ? "Push-pull/Pull-up" : "Open-drain");

    printf("  TX Power: ");
    switch (params->tx_power) {
        case E70_POWER_14DBM: printf("14dBm"); break;
        case E70_POWER_10DBM: printf("10dBm"); break;
        case E70_POWER_7DBM: printf("7dBm"); break;
        case E70_POWER_4DBM: printf("4dBm"); break;
        default: printf("Unknown"); break;
    }
    printf("\r\n");
    printf("-----------------------------------\r\n\r\n");
}

/**
 * @brief Выводит информацию о версии модуля E70 в отладочный порт.
 * @param version Указатель на структуру с информацией о версии.
 */
void print_e70_version(const E70_Version_t *version) {
    printf("--- E70 Module Version Information ---\r\n");
    printf("  Model: 0x%02X (E%02X Series)\r\n", version->model, version->model);
    printf("  Version: 0x%02X\r\n", version->version);
    printf("  Features: 0x%02X 0x%02X 0x%02X 0x%02X\r\n",
           version->feature1, version->feature2, version->feature3, version->feature4);
    printf("--------------------------------------\r\n\r\n");
}

/*
    Main application
*/
int main(void)
{
    // Инициализация системы, UART1 и GPIO через MCC Melody
    SYSTEM_Initialize();

    // Включение глобальных и периферийных прерываний.
    // Это необходимо для работы Timer1 и UART в режиме прерываний.
    INTERRUPT_GlobalInterruptEnable();
    INTERRUPT_PeripheralInterruptEnable();

    // Регистрация коллбэк-функции для прерывания Timer1.
    // Убедитесь, что Timer1 настроен в MCC Melody на генерацию прерывания каждые 1 мс.
    TMR1_OverflowCallbackRegister(TMR1_Millisecond_ISR_Callback);

    // Инициализация генератора случайных чисел
    // Для более случайных чисел можно использовать показания АЦП с неподключенного пина
    // или другой источник энтропии.
    // Исправлено: явное приведение к unsigned int для srand
    srand((unsigned int)system_ticks_ms); 

    printf("\r\n--- Starting E70 Board ---\r\n");

    // --- Однократный опрос модуля E70 при включении ---
    E70_Parameters_t current_params;
    E70_Version_t module_version;

    printf("Attempting to initialize E70 module in configuration mode...\r\n");
    if (!E70_Init(E70_MODE_CONFIGURATION)) {
        printf("!!! ERROR: Failed to initialize E70 module or enter configuration mode.\r\n");
        printf("!!! Check M0, M1, M2, AUX pin connections and module power supply.\r\n");
        while (1); // Зависаем при критической ошибке
    }
    printf("E70 module successfully initialized in configuration mode.\r\n");

    printf("Reading current module parameters...\r\n");
    if (E70_ReadParameters(&current_params)) {
        print_e70_parameters(&current_params);
    } else {
        printf("!!! ERROR: Failed to read E70 module parameters.\r\n");
    }

    printf("Reading module version information...\r\n");
    if (E70_ReadVersion(&module_version)) {
        print_e70_version(&module_version);
    } else {
        printf("!!! ERROR: Failed to read E70 module version.\r\n");
    }

    // --- Применение настроек, определенных через DEFINE ---
    printf("Applying defined settings...\r\n");
    E70_Parameters_t defined_params;
    defined_params.address = E70_CONFIG_ADDRESS;
    defined_params.channel = E70_CONFIG_CHANNEL;
    defined_params.uart_parity = E70_CONFIG_UART_PARITY;
    defined_params.uart_baud_rate = E70_CONFIG_UART_BAUD_RATE;
    defined_params.air_data_rate = E70_CONFIG_AIR_DATA_RATE;
    defined_params.packet_length = E70_CONFIG_PACKET_LENGTH;
    defined_params.fixed_transmission = E70_CONFIG_FIXED_TRANSMISSION;
    defined_params.wakeup_time = E70_CONFIG_WAKEUP_TIME;
    defined_params.fec_enabled = E70_CONFIG_FEC_ENABLED;
    defined_params.io_drive_mode = E70_CONFIG_IO_DRIVE_MODE;
    defined_params.tx_power = E70_CONFIG_TX_POWER;

    if (E70_SetParameters(&defined_params, true)) { // Сохраняем во флэш
        printf("Defined settings successfully applied and saved.\r\n");
        // После применения настроек, можно снова прочитать их, чтобы убедиться, что они применились
        if (E70_ReadParameters(&current_params)) {
            print_e70_parameters(&current_params);
        } else {
            printf("!!! ERROR: Failed to re-read parameters after applying defined settings.\r\n");
        }
    } else {
        printf("!!! ERROR: Failed to apply defined settings.\r\n");
        while (1); // Зависаем при критической ошибке
    }

    // --- Основной цикл работы ---
    while(1)
    {
#ifdef E70_IS_TRANSMITTER

        printf("--- Mode: TRANSMITTER ---\r\n");
        if (!E70_SetMode(E70_MODE_SUBPACKAGE)) { // Переходим в режим передачи
            printf("!!! ERROR: Failed to switch to transmission mode.\r\n");
            while (1);
        }
        printf("Module in transmission mode (sub-package)...\r\n");

        uint32_t tx_counter = 0;
        char tx_buffer[E70_CONFIG_PACKET_LENGTH + 1]; // Максимальный размер пакета + null-терминатор

        while (1) {
            tx_counter++;
            // Исправлено: явное приведение к uint16_t для rand()
            uint16_t random_code = (uint16_t)(rand() % 65536); // Случайное число от 0 до 65535
            sprintf(tx_buffer, "TX_MSG:%lu_CODE:%05u", tx_counter, random_code);
            uint16_t msg_len = strlen(tx_buffer);

            printf("Sending message (%u bytes): \"%s\"\r\n", msg_len, tx_buffer);
            if (E70_SendData((uint8_t*)tx_buffer, msg_len)) {
                printf("Message sent successfully.\r\n");
            } else {
                printf("!!! ERROR: Failed to send message.\r\n");
            }
          // Инвертируем состояние пина RE2 при каждом прохождении цикла
            IO_RE2_Toggle(); // Используем макрос MCC Melody для RE2
            DELAY_milliseconds(2000); // Задержка 2 секунды между отправками
        }

#else // E70_IS_RECEIVER или не определен
        printf("--- Mode: RECEIVER ---\r\n");
        if (!E70_SetMode(E70_MODE_SUBPACKAGE)) { // Переходим в режим приема
            printf("!!! ERROR: Failed to switch to reception mode.\r\r\n");
            while (1);
        }
        printf("Module in reception mode (sub-package). Waiting for data...\r\n");

        char rx_buffer[E70_CONFIG_PACKET_LENGTH + 1]; // Максимальный размер пакета + null-терминатор
        uint16_t rx_index = 0;
        uint32_t last_rx_time = system_ticks_ms; // Для обнаружения таймаута пакета
        const uint16_t PACKET_RECEIVE_TIMEOUT_MS = 500; // Таймаут для завершения приема пакета

        while (1) {
            if (E70_DataAvailable()) { // Теперь возвращает bool
                uint8_t received_byte = E70_ReadByte();
                last_rx_time = system_ticks_ms; // Сброс таймаута при получении нового байта

                if (rx_index < sizeof(rx_buffer) - 1) {
                    rx_buffer[rx_index++] = received_byte;
                }

                // Простая логика завершения пакета: по символу новой строки или по достижении максимальной длины
                // В реальном приложении рекомендуется использовать более надежный протокол (длина пакета, CRC).
                if (received_byte == '\n' || received_byte == '\r' || rx_index >= E70_CONFIG_PACKET_LENGTH) {
                    rx_buffer[rx_index] = '\0'; // Добавляем null-терминатор
                    printf("Received message (%u bytes): \"%s\"\r\n", rx_index, rx_buffer);
                    rx_index = 0; // Сброс для следующего сообщения
                }
            } else {
                // Проверка таймаута пакета, если давно не было новых данных
                if (rx_index > 0 && (system_ticks_ms - last_rx_time) >= PACKET_RECEIVE_TIMEOUT_MS) {
                    rx_buffer[rx_index] = '\0'; // Добавляем null-терминатор к незавершенному пакету
                    printf("!!! WARNING: Packet receive timeout. Incomplete message (%u bytes): \"%s\"\r\n", rx_index, rx_buffer);
                    rx_index = 0; // Сброс для следующего сообщения
                }
            }
               // Инвертируем состояние пина RE2 при каждом прохождении цикла
              IO_RE2_Toggle(); // Используем макрос MCC Melody для RE2
            // Можно добавить небольшую задержку или вызов SYSTEM_Tasks() для фоновых операций
             DELAY_milliseconds(50); // Если нужно активное ожидание с задержкой
        }
#endif
    }    
}