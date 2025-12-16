// StdLib.h - стандартная библиотека MZV-2025
// Встроенные функции для работы со строками
#pragma once

#include <unordered_set>
#include <unordered_map>
#include <string>
#include <cstring>

#define STDLIB_FUNC_COUNT   2   // Количество встроенных функций

// Имена встроенных функций
#define STDLIB_STRCMP       "strcmp"    // Лексикографическое сравнение строк
#define STDLIB_STRCPY       "strcpy"    // Копирование строки

// Индексы встроенных функций
#define STDLIB_IDX_STRCMP   0
#define STDLIB_IDX_STRCPY   1

// Типы данных (дублируем для независимости библиотеки)
enum STDLIB_DATATYPE
{
    STDLIB_INT,         // integer
    STDLIB_CHR,         // char
    STDLIB_STR          // string
};

// Типы идентификаторов
enum STDLIB_IDTYPE
{
    STDLIB_V,           // переменная
    STDLIB_F,           // функция
    STDLIB_P,           // параметр функции
    STDLIB_L            // литерал
};

namespace StdLib
{
    // Информация о встроенной функции
    struct BuiltinFunc
    {
        char name[32] = {0};                // Имя функции
        STDLIB_DATATYPE returnType = STDLIB_INT; // Тип возвращаемого значения
        int paramCount = 0;                 // Количество параметров
        STDLIB_DATATYPE paramTypes[4] = {STDLIB_INT, STDLIB_INT, STDLIB_INT, STDLIB_INT}; // Типы параметров
        bool isBuiltin = false;             // Флаг встроенной функции
    };

    // Таблица встроенных функций
    struct StdLibTable
    {
        BuiltinFunc functions[STDLIB_FUNC_COUNT] = {};
        int count = 0;
        // Хэш-индексы для быстрого поиска O(1)
        std::unordered_set<std::string> nameSet;
        std::unordered_map<std::string, int> nameToIndex;
    };

    // Создание таблицы стандартной библиотеки
    StdLibTable Create();

    // Проверка: является ли идентификатор встроенной функцией
    bool IsBuiltinFunction(const char* name);

    // Получить индекс встроенной функции (-1 если не найдена)
    int GetBuiltinIndex(const char* name);

    // Получить информацию о встроенной функции
    BuiltinFunc GetBuiltinInfo(int index);
    BuiltinFunc GetBuiltinInfo(const char* name);

    // Проверка корректности вызова встроенной функции
    bool ValidateCall(const char* name, int argCount, STDLIB_DATATYPE* argTypes, char* errorMsg);

    // Получить ассемблерный код для встроенной функции
    const char* GetAsmWrapper(const char* name);

    // Генерация вызова встроенной функции
    const char* GenerateCall(const char* name, const char* arg1, const char* arg2);
}
