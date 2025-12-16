// In.h - чтение входного файла MZV-2025
#pragma once

#define IN_MAX_SIZE (1024 * 1024)   // Максимальный размер файла (1 МБ)
#define IN_CODE_ENDL '\n'           // Символ конца строки

namespace Input
{
    // Классификация символов для лексера
    // T - допустимый символ
    // F - запрещённый символ (ошибка)
    // I - игнорируемый символ
    // Числа 0-255 - замена символа на другой
    enum CharClass
    {
        T = 1024,   // Допустимый (True)
        F = 2048,   // Запрещённый (Forbidden)
        I = 4096    // Игнорируемый (Ignore)
    };

    // Таблица классификации символов для MZV-2025
    // Индекс = ASCII-код символа, значение = класс или код замены
    #define IN_CODE_TABLE { \
        /* 0x00-0x0F: управляющие символы */ \
        I, I, I, I, I, I, I, I, I, T, T, I, I, T, I, I, \
        /* 0x10-0x1F: управляющие символы */ \
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, \
        /* 0x20-0x2F: пробел ! " # $ % & ' ( ) * + , - . / */ \
        T, F, T, F, F, T, F, T, T, T, T, T, T, T, F, T, \
        /* 0x30-0x3F: 0-9 : ; < = > ? */ \
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, F, \
        /* 0x40-0x4F: @ A-O */ \
        F, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, \
        /* 0x50-0x5F: P-Z [ \ ] ^ _ */ \
        T, T, T, T, T, T, T, T, T, T, T, F, F, F, F, T, \
        /* 0x60-0x6F: ` a-o */ \
        F, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, \
        /* 0x70-0x7F: p-z { | } ~ DEL */ \
        T, T, T, T, T, T, T, T, T, T, T, T, F, T, T, I, \
        /* 0x80-0xFF: расширенные ASCII - кириллица CP1251 и UTF-8 */ \
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, \
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, \
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, \
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, \
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, \
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, \
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, \
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T  \
    }

    // Структура входных данных
    struct INDATA
    {
        int size = 0;               // Размер исходного кода
        int lines = 0;              // Количество строк
        int ignored = 0;            // Количество игнорированных символов
        char* text = nullptr;       // Исходный код
        int code[256] = IN_CODE_TABLE;  // Таблица классификации символов
    };

    // Прочитать и обработать входной файл
    INDATA getinput(wchar_t filename[]);

    // Освободить память
    void deleteinput(INDATA& data);

    // Проверка символа по таблице (inline для скорости)
    inline bool IsAllowed(const INDATA& data, unsigned char c)
    {
        return data.code[c] == T || (data.code[c] > 0 && data.code[c] < 256);
    }

    inline bool IsForbidden(const INDATA& data, unsigned char c)
    {
        return data.code[c] == F;
    }

    inline bool IsIgnored(const INDATA& data, unsigned char c)
    {
        return data.code[c] == I;
    }
};
