/* Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */

/* 
 * File:   E70_868T14S.h
 * Author: [Ваше имя / AI]
 * Comments: Библиотека для удобного управления беспроводным модулем E70-868T14S.
 *           Предоставляет функции для переключения режимов, настройки параметров,
 *           передачи/приема данных и обработки ошибок с таймаутами.
 *           Использует сгенерированные MCC Melody драйверы для UART и GPIO.
 * Revision history: 
 * v1.0 - Исходная версия.
 * v1.1 - Добавлена обработка ошибок и таймауты.
 * v1.2 - Уточнен пинаут (M0-RB0, M1-RB1, M2-RB2, AUX-RB3).
 * v1.3 - Оформление по шаблону MPLAB X, использование DELAY_milliseconds.
 * v1.4 - Исправлены пути включения MCC Melody файлов внутри библиотеки.
 * v1.5 - Оптимизированы включения MCC Melody файлов, используя центральный system.h.
 * v1.6 - Разделение на .h (объявления) и .c (реализации) файлы.
 * v1.7 - Исправлены пути к eusart1.h и использование EUSART1_ функций.
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef E70_868T14S_H
#define	E70_868T14S_H

#include <xc.h> // include processor files - each processor file is guarded.  

// Включаем необходимые MCC Melody заголовочные файлы.
// system.h включает в себя pins.h, eusart1.h (uart1.h), eusart2.h, tmr1.h, interrupt.h, clock.h.
// delay.h включаем отдельно, так как он не входит в system.h по предоставленной информации.
#include <stdbool.h>
#include <stdint.h>
#include "mcc_generated_files/system/system.h"    // Для SYSTEM_GetTickCount()
#include "mcc_generated_files/uart/eusart1.h"     // Для UART1 (связь с E70), используется eusart1.h
#include "mcc_generated_files/system/pins.h"      // Для управления GPIO (IO_RBx_SetLow/High/GetValue)
#include "mcc_generated_files/timer/delay.h"     // Для DELAY_milliseconds()

// --- Конфигурация пинов модуля E70 ---
// Эти дефайны должны быть сопоставлены с реальными пинами PIC18F46K22,
// настроенными в MCC Melody.
// Убедись, что в MCC Melody для этих пинов установлены соответствующие имена
// и они настроены как цифровые выходы (для M0, M1, M2) и цифровой вход (для AUX).

// Пины выбора режима (M0, M1, M2)
// УТОЧНЕНИЕ: M0 -> RB0, M1 -> RB1, M2 -> RB2
#define E70_M0_SetLow()     IO_RB0_SetLow()      // Соответствует RB0
#define E70_M0_SetHigh()    IO_RB0_SetHigh()     // Соответствует RB0

#define E70_M1_SetLow()     IO_RB1_SetLow()      // Соответствует RB1
#define E70_M1_SetHigh()    IO_RB1_SetHigh()     // Соответствует RB1

#define E70_M2_SetLow()     IO_RB2_SetLow()      // Соответствует RB2
#define E70_M2_SetHigh()    IO_RB2_SetHigh()     // Соответствует RB2

// Пин AUX (индикация состояния модуля)
// УТОЧНЕНИЕ: AUX -> RB3
#define E70_AUX_GetValue()  IO_RB3_GetValue()   // Соответствует RB3

// --- Константы таймаутов ---
#define E70_DEFAULT_AUX_TIMEOUT_MS          1000    /**< Таймаут по умолчанию для ожидания AUX в мс. */
#define E70_DEFAULT_UART_RESPONSE_TIMEOUT_MS 2000    /**< Таймаут по умолчанию для ответа UART в мс. */
#define E70_MODE_SWITCH_DELAY_MS            2       /**< Задержка после смены режима M0/M1/M2 в мс. */

// --- Перечисления для параметров модуля ---

/**
 * @brief Перечисление режимов работы модуля E70.
 * Определяется комбинацией состояний пинов M2, M1, M0.
 */
typedef enum {
    E70_MODE_RSSI           = 0b000, /**< Режим RSSI (M2=0, M1=0, M0=0). UART открыт, беспроводная связь закрыта. Модуль выводит значение RSSI каждые 100мс. */
    E70_MODE_CONTINUOUS     = 0b001, /**< Непрерывный режим (M2=0, M1=0, M0=1). UART открыт, непрерывная прозрачная передача. Скорость по воздуху подстраивается под UART. */
    E70_MODE_SUBPACKAGE     = 0b010, /**< Режим подпакетов (M2=0, M1=1, M0=0). UART открыт, прозрачная передача подпакетами. Скорость по воздуху и UART независимы. */
    E70_MODE_CONFIGURATION  = 0b011, /**< Режим конфигурации (M2=0, M1=1, M0=1). UART открыт, беспроводная связь закрыта. Параметры настраиваются через UART 9600 8N1. */
    E70_MODE_WOR_TRANSMIT   = 0b100, /**< Режим пробуждения (WOR) передачи (M2=1, M1=0, M0=0). UART открыт, передача подпакетами. Используется для пробуждения приемника в режиме 6. */
    E70_MODE_CONFIGURATION_ALT = 0b101, /**< Альтернативный режим конфигурации (M2=1, M1=0, M0=1). Аналогичен E70_MODE_CONFIGURATION. */
    E70_MODE_POWER_SAVING   = 0b110, /**< Режим энергосбережения (M2=1, M1=1, M0=0). UART закрыт, беспроводная связь в режиме WOR. Приемник периодически просыпается. */
    E70_MODE_SLEEP          = 0b111  /**< Режим сна (M2=1, M1=1, M2=1). UART закрыт, беспроводная связь закрыта. Выход из сна по изменению M0, M1, M2. */
} E70_Mode_t;

/**
 * @brief Перечисление скоростей UART.
 * Используется для настройки параметра SPED.
 */
typedef enum {
    E70_UART_BAUD_1200  = 0b000,
    E70_UART_BAUD_2400  = 0b001,
    E70_UART_BAUD_4800  = 0b010,
    E70_UART_BAUD_9600  = 0b011, /**< По умолчанию */
    E70_UART_BAUD_19200 = 0b100,
    E70_UART_BAUD_38400 = 0b101,
    E70_UART_BAUD_57600 = 0b110,
    E70_UART_BAUD_115200 = 0b111
} E70_UART_BaudRate_t;

/**
 * @brief Перечисление скоростей передачи данных по воздуху (Air Data Rate).
 * Используется для настройки параметра SPED.
 */
typedef enum {
    E70_AIR_RATE_2_5K   = 0b000, /**< По умолчанию */
    E70_AIR_RATE_5K     = 0b001,
    E70_AIR_RATE_12K    = 0b010,
    E70_AIR_RATE_28K    = 0b011,
    E70_AIR_RATE_64K    = 0b100,
    E70_AIR_RATE_168K   = 0b101,
    // 0b110 и 0b111 также 168kbps согласно документации
} E70_Air_DataRate_t;

/**
 * @brief Перечисление режимов четности UART.
 * Используется для настройки параметра SPED.
 */
typedef enum {
    E70_PARITY_8N1 = 0b00, /**< 8N1 (по умолчанию) */
    E70_PARITY_8O1 = 0b01, /**< 8O1 */
    E70_PARITY_8E1 = 0b10, /**< 8E1 */
    // 0b11 также 8N1
} E70_Parity_t;

/**
 * @brief Перечисление длин пакетов для режима подпакетов.
 * Используется для настройки параметра CHAN.
 */
typedef enum {
    E70_PACKET_LEN_16   = 0b000,
    E70_PACKET_LEN_32   = 0b001,
    E70_PACKET_LEN_64   = 0b010, /**< По умолчанию */
    E70_PACKET_LEN_128  = 0b011,
    E70_PACKET_LEN_256  = 0b100,
    E70_PACKET_LEN_512  = 0b101,
    E70_PACKET_LEN_1024 = 0b110,
    E70_PACKET_LEN_2048 = 0b111
} E70_PacketLength_t;

/**
 * @brief Перечисление времени пробуждения для режимов WOR.
 * Используется для настройки параметра OPTION.
 */
typedef enum {
    E70_WAKEUP_500MS    = 0b000,
    E70_WAKEUP_1000MS   = 0b001, /**< По умолчанию */
    E70_WAKEUP_1500MS   = 0b010,
    E70_WAKEUP_2000MS   = 0b011,
    E70_WAKEUP_2500MS   = 0b100,
    E70_WAKEUP_3000MS   = 0b101,
    E70_WAKEUP_3500MS   = 0b110,
    E70_WAKEUP_4000MS   = 0b111
} E70_WakeupTime_t;

/**
 * @brief Перечисление мощности передачи.
 * Используется для настройки параметра OPTION.
 * Значения для E70-868T14S (14dBm).
 */
typedef enum {
    E70_POWER_14DBM = 0b00, /**< 14dBm (по умолчанию для T14S) */
    E70_POWER_10DBM = 0b01, /**< 10dBm */
    E70_POWER_7DBM  = 0b10, /**< 7dBm */
    E70_POWER_4DBM  = 0b11  /**< 4dBm */
} E70_Power_t;

/**
 * @brief Перечисление режимов работы IO (TXD, AUX, RXD).
 * Используется для настройки параметра OPTION.
 */
typedef enum {
    E70_IO_PUSH_PULL_PULL_UP = 0b0, /**< TXD и AUX push-pull, RXD pull-up (по умолчанию) */
    E70_IO_OPEN_DRAIN_OPEN_DRAIN = 0b1 /**< TXD и AUX open-collector, RXD open-collector */
} E70_IODriveMode_t;

/**
 * @brief Перечисление состояния FEC (Forward Error Correction).
 * Используется для настройки параметра OPTION.
 */
typedef enum {
    E70_FEC_OFF = 0b0, /**< FEC выключен */
    E70_FEC_ON  = 0b1  /**< FEC включен (по умолчанию) */
} E70_FEC_t;

/**
 * @brief Перечисление режима фиксированной передачи.
 * Используется для настройки параметра OPTION.
 */
typedef enum {
    E70_FIXED_TRANSMISSION_OFF = 0b0, /**< Прозрачный режим передачи (по умолчанию) */
    E70_FIXED_TRANSMISSION_ON  = 0b1  /**< Фиксированный режим передачи */
} E70_FixedTransmission_t;

/**
 * @brief Структура для хранения всех настраиваемых параметров модуля E70.
 */
typedef struct {
    uint16_t address;               /**< Адрес модуля (0x0000 - 0xFFFF) */
    uint8_t channel;                /**< Канал связи (0x00 - 0x1F) */
    E70_Parity_t uart_parity;       /**< Четность UART */
    E70_UART_BaudRate_t uart_baud_rate; /**< Скорость UART */
    E70_Air_DataRate_t air_data_rate; /**< Скорость передачи по воздуху */
    E70_PacketLength_t packet_length; /**< Длина пакета (только для режима подпакетов) */
    E70_FixedTransmission_t fixed_transmission; /**< Режим фиксированной передачи */
    E70_WakeupTime_t wakeup_time;   /**< Время пробуждения (для WOR режимов) */
    E70_FEC_t fec_enabled;          /**< Включен ли FEC */
    E70_IODriveMode_t io_drive_mode; /**< Режим работы IO */
    E70_Power_t tx_power;           /**< Мощность передачи */
} E70_Parameters_t;

/**
 * @brief Структура для хранения информации о версии модуля.
 */
typedef struct {
    uint8_t model;      /**< Модель модуля (например, 0x70 для E70) */
    uint8_t version;    /**< Номер версии */
    uint8_t feature1;   /**< Дополнительные особенности 1 */
    uint8_t feature2;   /**< Дополнительные особенности 2 */
    uint8_t feature3;   /**< Дополнительные особенности 3 */
    uint8_t feature4;   /**< Дополнительные особенности 4 */
} E70_Version_t;


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

// --- Прототипы функций ---

// Объявление глобального счетчика миллисекунд из main.c
extern volatile uint32_t system_ticks_ms;

/**
 * @brief Внутренняя функция для отправки байтов через UART1.
 * @param data Указатель на буфер с данными.
 * @param length Количество байт для отправки.
 * @remarks Использует буфер передачи UART1, явно ожидая готовности буфера и завершения передачи.
 */
static void E70_UART_WriteBytes(uint8_t *data, uint16_t length);

/**
 * @brief Внутренняя функция для чтения байтов из UART1 с таймаутом.
 * @param buffer Указатель на буфер для записи принятых данных.
 * @param length Количество байт для чтения.
 * @param timeout_ms Таймаут в миллисекундах на всю операцию чтения.
 * @return true, если все байты успешно прочитаны; false в случае таймаута.
 * @remarks Ожидает поступления каждого байта в буфер приема UART1.
 */
static bool E70_UART_ReadBytes(uint8_t *buffer, uint8_t length, uint16_t timeout_ms);

/**
    <p><b>Function prototype:</b></p>
    <code>bool E70_Init(E70_Mode_t initial_mode)</code>
  
    <p><b>Summary:</b></p>
    Инициализирует модуль E70, устанавливая начальный режим и ожидая готовности.

    <p><b>Description:</b></p>
    Эта функция выполняет начальную настройку модуля E70, переводя его в указанный
    режим работы. Она ожидает готовности модуля (пин AUX переходит в высокое состояние)
    после смены режима, обеспечивая стабильный старт.

    <p><b>Precondition:</b></p>
    MCC Melody UART1 и GPIO должны быть инициализированы.

    <p><b>Parameters:</b></p>
    @param initial_mode Режим, в который модуль должен перейти после инициализации.

    <p><b>Returns:</b></p>
    true, если инициализация и переход в режим успешны; false в противном случае.

    <p><b>Example:</b></p>
    <code>
    // В main.c или другом файле инициализации:
    // SYSTEM_Initialize(); // Инициализация системы, UART1 и GPIO через MCC Melody
    // if (E70_Init(E70_MODE_CONFIGURATION)) {
    //     // Модуль успешно инициализирован и готов к работе
    // } else {
    //     // Ошибка инициализации модуля
    // }
    </code>

    <p><b>Remarks:</b></p>
    Эта функция является блокирующей до тех пор, пока модуль не перейдет в указанный режим
    и не сигнализирует о готовности через пин AUX.
 */
bool E70_Init(E70_Mode_t initial_mode);

/**
    <p><b>Function prototype:</b></p>
    <code>bool E70_SetMode(E70_Mode_t mode)</code>
  
    <p><b>Summary:</b></p>
    Устанавливает режим работы модуля E70 путем управления пинами M0, M1, M2.

    <p><b>Description:</b></p>
    Функция сначала ожидает, пока модуль завершит текущие операции (пин AUX перейдет в HIGH),
    затем устанавливает пины M0, M1, M2 в соответствии с желаемым режимом. После небольшой
    задержки, она снова ожидает готовности модуля.

    <p><b>Precondition:</b></p>
    MCC Melody GPIO для M0, M1, M2 (выходы) и AUX (вход) должны быть инициализированы.

    <p><b>Parameters:</b></p>
    @param mode Желаемый режим работы из перечисления E70_Mode_t.

    <p><b>Returns:</b></p>
    true, если смена режима и ожидание готовности AUX успешны; false в противном случае.

    <p><b>Example:</b></p>
    <code>
    if (E70_SetMode(E70_MODE_SUBPACKAGE)) { // Переключение модуля в режим подпакетов
        // Модуль готов в режиме подпакетов
    } else {
        // Ошибка смены режима или таймаут AUX
    }
    </code>

    <p><b>Remarks:</b></p>
    Эта функция является блокирующей до тех пор, пока модуль не перейдет в указанный режим
    и не сигнализирует о готовности через пин AUX.
 */
bool E70_SetMode(E70_Mode_t mode);

/**
    <p><b>Function prototype:</b></p>
    <code>bool E70_WaitAuxHigh(uint16_t timeout_ms)</code>
  
    <p><b>Summary:</b></p>
    Ожидает, пока пин AUX модуля E70 перейдет в высокое состояние.

    <p><b>Description:</b></p>
    Это указывает на завершение внутренних операций модуля (самопроверка, инициализация,
    завершение передачи данных) и его готовность к работе или приему новых команд.
    Функция использует системный таймер для отсчета таймаута, предотвращая бесконечное зависание.

    <p><b>Precondition:</b></p>
    MCC Melody GPIO для AUX (вход) и системный таймер (SYSTEM_GetTickCount()) должны быть инициализированы.

    <p><b>Parameters:</b></p>
    @param timeout_ms Максимальное время ожидания в миллисекундах.

    <p><b>Returns:</b></p>
    true, если AUX перешел в высокое состояние; false в случае таймаута.

    <p><b>Example:</b></p>
    <code>
    if (E70_WaitAuxHigh(2000)) { // Ждем до 2 секунд
        // AUX стал высоким
    } else {
        // Таймаут, AUX не стал высоким
    }
    </code>

    <p><b>Remarks:</b></p>
    Эта функция является блокирующей. Для асинхронного поведения,
    проверяй E70_AUX_GetValue() в основном цикле программы.
 */
bool E70_WaitAuxHigh(uint16_t timeout_ms);

/**
    <p><b>Function prototype:</b></p>
    <code>bool E70_ResetModule(void)</code>
  
    <p><b>Summary:</b></p>
    Отправляет команду сброса модуля E70.

    <p><b>Description:</b></p>
    Модуль перезагрузится после получения команды сброса. Пин AUX перейдет в низкое
    состояние во время сброса, затем в высокое после завершения. Функция ожидает
    возвращения модуля в готовое состояние.

    <p><b>Precondition:</b></p>
    MCC Melody UART1 и GPIO для M0, M1, M2, AUX должны быть инициализированы.

    <p><b>Parameters:</b></p>
    None.

    <p><b>Returns:</b></p>
    true, если команда сброса отправлена успешно и модуль вернулся в готовность;
    false в случае ошибки отправки или таймаута ожидания AUX.

    <p><b>Example:</b></p>
    <code>
    if (E70_ResetModule()) {
        // Модуль успешно сброшен и готов
    } else {
        // Ошибка сброса модуля
    }
    </code>

    <p><b>Remarks:</b></p>
    Эта функция является блокирующей до завершения операции сброса модуля.
 */
bool E70_ResetModule(void);

/**
    <p><b>Function prototype:</b></p>
    <code>bool E70_ReadParameters(E70_Parameters_t *params)</code>
  
    <p><b>Summary:</b></p>
    Читает текущие сохраненные параметры модуля E70.

    <p><b>Description:</b></p>
    Модуль должен находиться в режиме конфигурации (E70_MODE_CONFIGURATION).
    Функция отправляет команду чтения параметров и парсит полученный ответ
    в структуру E70_Parameters_t. Включает таймаут для ожидания ответа.

    <p><b>Precondition:</b></p>
    MCC Melody UART1 должен быть инициализирован. Модуль E70 должен быть в режиме конфигурации.

    <p><b>Parameters:</b></p>
    @param params Указатель на структуру E70_Parameters_t, куда будут записаны прочитанные параметры.

    <p><b>Returns:</b></p>
    true, если параметры успешно прочитаны; false в противном случае (например, таймаут или некорректный ответ).

    <p><b>Example:</b></p>
    <code>
    E70_Parameters_t currentParams;
    if (E70_SetMode(E70_MODE_CONFIGURATION) && E70_ReadParameters(&currentParams)) {
        // Параметры успешно прочитаны, можно их использовать
        // printf("Адрес: 0x%04X, Канал: 0x%02X\r\n", currentParams.address, currentParams.channel);
    } else {
        // Ошибка чтения параметров или таймаут
    }
    </code>

    <p><b>Remarks:</b></p>
    Эта функция является блокирующей до получения ответа от модуля или истечения таймаута.
 */
bool E70_ReadParameters(E70_Parameters_t *params);

/**
    <p><b>Function prototype:</b></p>
    <code>bool E70_SetParameters(E70_Parameters_t *params, bool save_to_flash)</code>
  
    <p><b>Summary:</b></p>
    Устанавливает параметры модуля E70.

    <p><b>Description:</b></p>
    Модуль должен находиться в режиме конфигурации (E70_MODE_CONFIGURATION).
    Функция формирует команду установки параметров из структуры E70_Parameters_t
    и отправляет ее модулю. Можно выбрать сохранение параметров во флэш-памяти
    модуля (постоянно) или временное применение. После отправки ожидает готовности модуля.

    <p><b>Precondition:</b></p>
    MCC Melody UART1 должен быть инициализирован. Модуль E70 должен быть в режиме конфигурации.

    <p><b>Parameters:</b></p>
    @param params Указатель на структуру E70_Parameters_t с желаемыми параметрами.
    @param save_to_flash Если true, параметры будут сохранены во флэш-памяти модуля (команда C0).
                         Если false, параметры будут применены временно (команда C2) и сбросятся при выключении.

    <p><b>Returns:</b></p>
    true, если параметры успешно установлены и модуль вернулся в готовность; false в противном случае.

    <p><b>Example:</b></p>
    <code>
    E70_Parameters_t newParams;
    // Заполняем newParams желаемыми значениями
    newParams.address = 0x1234;
    newParams.channel = 0x05;
    // ... другие параметры
    
    if (E70_SetMode(E70_MODE_CONFIGURATION) && E70_SetParameters(&newParams, true)) { // Установить и сохранить во флэш
        // Параметры успешно установлены
    } else {
        // Ошибка установки параметров
    }
    </code>

    <p><b>Remarks:</b></p>
    Эта функция является блокирующей до завершения операции установки параметров и возвращения модуля в готовое состояние.
 */
bool E70_SetParameters(E70_Parameters_t *params, bool save_to_flash);

/**
    <p><b>Function prototype:</b></p>
    <code>bool E70_ReadVersion(E70_Version_t *version)</code>
  
    <p><b>Summary:</b></p>
    Читает информацию о версии модуля E70.

    <p><b>Description:</b></p>
    Модуль должен находиться в режиме конфигурации (E70_MODE_CONFIGURATION).
    Функция отправляет команду чтения версии и парсит полученный ответ
    в структуру E70_Version_t. Включает таймаут для ожидания ответа.

    <p><b>Precondition:</b></p>
    MCC Melody UART1 должен быть инициализирован. Модуль E70 должен быть в режиме конфигурации.

    <p><b>Parameters:</b></p>
    @param version Указатель на структуру E70_Version_t, куда будет записана информация о версии.

    <p><b>Returns:</b></p>
    true, если информация о версии успешно прочитана; false в противном случае.

    <p><b>Example:</b></p>
    <code>
    E70_Version_t moduleVersion;
    if (E70_SetMode(E70_MODE_CONFIGURATION) && E70_ReadVersion(&moduleVersion)) {
        // Информация о версии успешно прочитана
        // printf("Модель: 0x%02X, Версия: 0x%02X\r\n", moduleVersion.model, moduleVersion.version);
    } else {
        // Ошибка чтения версии или таймаут
    }
    </code>

    <p><b>Remarks:</b></p>
    Эта функция является блокирующей до получения ответа от модуля или истечения таймаута.
 */
bool E70_ReadVersion(E70_Version_t *version);

/**
    <p><b>Function prototype:</b></p>
    <code>bool E70_SendData(uint8_t *data, uint16_t length)</code>
  
    <p><b>Summary:</b></p>
    Отправляет данные через модуль E70 в прозрачном режиме.

    <p><b>Description:</b></p>
    Модуль должен находиться в режиме непрерывной передачи (E70_MODE_CONTINUOUS)
    или режиме подпакетов (E70_MODE_SUBPACKAGE). Функция ожидает готовности модуля,
    затем помещает данные в буфер передачи UART1. После этого ожидает завершения
    передачи данных по радиоканалу (пин AUX снова станет HIGH).

    <p><b>Precondition:</b></p>
    MCC Melody UART1 должен быть инициализирован. Модуль E70 должен быть в режиме
    E70_MODE_CONTINUOUS или E70_MODE_SUBPACKAGE.

    <p><b>Parameters:</b></p>
    @param data Указатель на буфер с данными для отправки.
    @param length Длина данных в байтах.

    <p><b>Returns:</b></p>
    true, если данные успешно отправлены и модуль вернулся в готовность; false в противном случае.

    <p><b>Example:</b></p>
    <code>
    char message[] = "Hello, world!";
    if (E70_SetMode(E70_MODE_SUBPACKAGE) && E70_SendData((uint8_t*)message, sizeof(message) - 1)) {
        // Данные успешно отправлены
    } else {
        // Ошибка отправки данных
    }
    </code>

    <p><b>Remarks:</b></p>
    В режиме подпакетов длина данных ограничена настроенной длиной пакета.
    В непрерывном режиме длина пакета не ограничена.
    Функция является блокирующей до завершения передачи данных по радиоканалу.
 */
bool E70_SendData(uint8_t *data, uint16_t length);

/**
    <p><b>Function prototype:</b></p>
    <code>bool E70_SendFixedData(uint16_t address, uint8_t channel, uint8_t *data, uint16_t length)</code>
  
    <p><b>Summary:</b></p>
    Отправляет данные через модуль E70 в режиме фиксированной передачи.

    <p><b>Description:</b></p>
    Модуль должен находиться в режиме непрерывной передачи (E70_MODE_CONTINUOUS)
    или режиме подпакетов (E70_MODE_SUBPACKAGE), и параметр `fixed_transmission`
    должен быть установлен в `E70_FIXED_TRANSMISSION_ON`. Функция добавляет
    адрес и канал в начало пакета данных, затем отправляет их через UART1.
    После этого ожидает завершения передачи по радиоканалу.

    <p><b>Precondition:</b></p>
    MCC Melody UART1 должен быть инициализирован. Модуль E70 должен быть в режиме
    E70_MODE_CONTINUOUS или E70_MODE_SUBPACKAGE, и настроен на фиксированную передачу.

    <p><b>Parameters:</b></p>
    @param address Адрес целевого модуля (2 байта).
    @param channel Канал целевого модуля (1 байт).
    @param data Указатель на буфер с данными для отправки.
    @param length Длина данных в байтах.

    <p><b>Returns:</b></p>
    true, если данные успешно отправлены и модуль вернулся в готовность; false в противном случае.

    <p><b>Example:</b></p>
    <code>
    uint16_t target_addr = 0x0001;
    uint8_t target_channel = 0x04;
    char command[] = "STATUS";
    
    E70_Parameters_t currentParams;
    // Предполагаем, что currentParams уже прочитаны или установлены
    currentParams.fixed_transmission = E70_FIXED_TRANSMISSION_ON;
    if (E70_SetMode(E70_MODE_CONFIGURATION) && E70_SetParameters(&currentParams, true) &&
        E70_SetMode(E70_MODE_SUBPACKAGE) &&
        E70_SendFixedData(target_addr, target_channel, (uint8_t*)command, sizeof(command) - 1)) {
        // Данные успешно отправлены
    } else {
        // Ошибка отправки данных
    }
    </code>

    <p><b>Remarks:</b></p>
    Первые три байта данных будут интерпретированы модулем как адрес и канал.
    Модуль временно изменит свой адрес/канал для этой передачи.
    Функция является блокирующей до завершения передачи данных по радиоканалу.
 */
bool E70_SendFixedData(uint16_t address, uint8_t channel, uint8_t *data, uint16_t length);

/**
    <p><b>Function prototype:</b></p>
    <code>bool E70_DataAvailable(void)</code>
  
    <p><b>Summary:</b></p>
    Проверяет наличие принятых данных в буфере UART1.

    <p><b>Description:</b></p>
    Возвращает true, если в буфере приема UART1 есть хотя бы один байт данных;
    false в противном случае.

    <p><b>Precondition:</b></p>
    MCC Melody UART1 должен быть инициализирован.

    <p><b>Parameters:</b></p>
    None.

    <p><b>Returns:</b></p>
    true, если данные доступны; false, если буфер пуст.

    <p><b>Example:</b></p>
    <code>
    if (E70_DataAvailable()) { // Теперь возвращает bool
        uint8_t receivedByte = E70_ReadByte();
        // Обработка принятого байта
    }
    </code>

    <p><b>Remarks:</b></p>
    Эта функция не блокирует выполнение.
 */
bool E70_DataAvailable(void);

/**
    <p><b>Function prototype:</b></p>
    <code>uint8_t E70_ReadByte(void)</code>
  
    <p><b>Summary:</b></p>
    Читает один байт из буфера UART1.

    <p><b>Description:</b></p>
    Считывает один байт из кольцевого буфера приема UART1.

    <p><b>Precondition:</b></p>
    MCC Melody UART1 должен быть инициализирован. Перед вызовом этой функции
    рекомендуется проверить наличие данных с помощью E70_DataAvailable().

    <p><b>Parameters:</b></p>
    None.

    <p><b>Returns:</b></p>
    Прочитанный байт.

    <p><b>Example:</b></p>
    <code>
    uint8_t receivedByte = E70_ReadByte();
    </code>

    <p><b>Remarks:</b></p>
    Эта функция может блокировать, если буфер пуст, в зависимости от реализации UART1_Read().
    Всегда проверяйте E70_DataAvailable() перед чтением.
 */
uint8_t E70_ReadByte(void);


#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* E70_868T14S_H */