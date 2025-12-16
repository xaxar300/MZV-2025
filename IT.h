// IT.h - таблица идентификаторов MZV-2025
// Добавлен union для экономии памяти (из laba18)
#pragma once

#include <unordered_map>
#include <string>
#include <vector>

#define IT_MAXSIZE      4096
#define IT_STR_MAXSIZE  255
#define IT_NULL_IDX     0xffffffff

// Значения по умолчанию
#define IT_INT_DEFAULT  0x00000000
#define IT_CHR_DEFAULT  0x00
#define IT_STR_DEFAULT  ""

// Типы данных
enum IDDATATYPE
{
    IT_INT,         // integer
    IT_CHR,         // char
    IT_STR          // string (для литералов)
};

// Типы идентификаторов
enum IDTYPE
{
    IT_V,           // переменная
    IT_F,           // функция
    IT_P,           // параметр функции
    IT_L            // литерал
};

namespace IT
{
    // =================================================================
    // Значение идентификатора (union для экономии памяти - из laba18)
    // В один момент времени используется только одно из полей
    // =================================================================
    union IDVALUE
    {
        int vint;                       // для integer (4 байта)
        char vchr;                      // для char (1 байт)
        struct
        {
            unsigned char len;          // длина строки
            char str[IT_STR_MAXSIZE];   // содержимое строки
        } vstr;                         // для string

        // Конструктор по умолчанию
        IDVALUE() : vint(0) {}

        // Конструкторы для разных типов
        explicit IDVALUE(int i) : vint(i) {}
        explicit IDVALUE(char c) : vchr(c) {}
    };

    // Запись в таблице идентификаторов
    struct Entry
    {
        char id[IT_STR_MAXSIZE] = {0};      // имя идентификатора
        char scope[IT_STR_MAXSIZE] = {0};   // область видимости
        IDDATATYPE datatype = IT_INT;       // тип данных
        IDTYPE idtype = IT_V;               // тип идентификатора
        IDVALUE value;                      // значение (union)
        int firstLine = 0;                  // строка появления в коде
        int paramCount = 0;                 // количество параметров (для функций)

        Entry() : value() {}
    };

    // Таблица идентификаторов
    struct IdTable
    {
        int maxsize = 0;
        int size = 0;
        Entry* table = nullptr;
        // Хэш-индекс для быстрого поиска: имя -> список индексов
        std::unordered_map<std::string, std::vector<int>> nameIndex;
    };

    IdTable Create(int size);
    void Add(IdTable& it, Entry e);
    Entry GetEntry(IdTable& it, int idx);
    int IsId(IdTable& it, const char* id, const char* scope);
    void Delete(IdTable& it);
    Entry CreateEntry(const char* id, const char* scope, IDDATATYPE dtype, IDTYPE itype, int line);

    // =================================================================
    // Вспомогательные функции для работы со значениями (новое)
    // =================================================================

    // Установить целочисленное значение
    inline void SetIntValue(Entry& e, int val)
    {
        e.datatype = IT_INT;
        e.value.vint = val;
    }

    // Установить символьное значение
    inline void SetCharValue(Entry& e, char val)
    {
        e.datatype = IT_CHR;
        e.value.vchr = val;
    }

    // Установить строковое значение
    inline void SetStringValue(Entry& e, const char* str)
    {
        e.datatype = IT_STR;
        size_t len = strlen(str);
        if (len >= IT_STR_MAXSIZE) len = IT_STR_MAXSIZE - 1;
        e.value.vstr.len = static_cast<unsigned char>(len);
        strncpy_s(e.value.vstr.str, str, len);
        e.value.vstr.str[len] = '\0';
    }

    // Получить целочисленное значение
    inline int GetIntValue(const Entry& e)
    {
        return (e.datatype == IT_INT) ? e.value.vint :
               (e.datatype == IT_CHR) ? static_cast<int>(e.value.vchr) : 0;
    }

    // Получить символьное значение
    inline char GetCharValue(const Entry& e)
    {
        return (e.datatype == IT_CHR) ? e.value.vchr :
               (e.datatype == IT_INT) ? static_cast<char>(e.value.vint) : '\0';
    }

    // Получить строковое значение
    inline const char* GetStringValue(const Entry& e)
    {
        return (e.datatype == IT_STR) ? e.value.vstr.str : "";
    }
};
