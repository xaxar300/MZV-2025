// Error.cpp - реализация обработки ошибок MZV-2025
#include "stdafx.h"

namespace Error
{
    /*
    Серии ошибок:
        0 - 99     системные ошибки
        100 - 109  ошибки параметров
        110 - 119  ошибки открытия и чтения файлов
        600 - 699  синтаксические ошибки
        700 - 799  семантические ошибки
    */
    ERRORDATA errors[ERROR_MAX_ENTRY] =
    {
        // 0-9: Системные ошибки
        ERROR_ENTRY(0, "Недопустимый код ошибки"),
        ERROR_ENTRY(1, "Системный сбой"),
        ERROR_ENTRY(2, "Не указан параметр -in"),
        ERROR_ENTRY(3, "Неверный параметр -out"),
        ERROR_ENTRY(4, "Неверный параметр -log"),
        ERROR_ENTRY_NODEF(5), ERROR_ENTRY_NODEF(6), ERROR_ENTRY_NODEF(7),
        ERROR_ENTRY_NODEF(8), ERROR_ENTRY_NODEF(9),

        // 10-99: Зарезервировано
        ERROR_ENTRY_NODEF10(10), ERROR_ENTRY_NODEF10(20), ERROR_ENTRY_NODEF10(30),
        ERROR_ENTRY_NODEF10(40), ERROR_ENTRY_NODEF10(50), ERROR_ENTRY_NODEF10(60),
        ERROR_ENTRY_NODEF10(70), ERROR_ENTRY_NODEF10(80), ERROR_ENTRY_NODEF10(90),

        // 100-109: Ошибки параметров и файлов
        ERROR_ENTRY(100, "Ошибка при открытии файла с исходным кодом (-in)"),
        ERROR_ENTRY_NODEF(101), ERROR_ENTRY_NODEF(102), ERROR_ENTRY_NODEF(103),
        ERROR_ENTRY(104, "Превышена длина входного параметра"),
        ERROR_ENTRY_NODEF(105), ERROR_ENTRY_NODEF(106), ERROR_ENTRY_NODEF(107),
        ERROR_ENTRY_NODEF(108), ERROR_ENTRY_NODEF(109),

        // 110-119: Ошибки файлов и таблиц
        ERROR_ENTRY(110, "Ошибка при создании файла протокола (-log)"),
        ERROR_ENTRY(111, "Недопустимый символ в исходном коде (-in)"),
        ERROR_ENTRY(112, "Ошибка при создании выходного файла (-out)"),
        ERROR_ENTRY(113, "Недопустимый размер таблицы лексем (> 4096)"),
        ERROR_ENTRY(114, "Переполнение таблицы лексем"),
        ERROR_ENTRY(115, "Выход за границы таблицы лексем"),
        ERROR_ENTRY(116, "Переполнение таблицы идентификаторов"),
        ERROR_ENTRY(117, "Ошибка лексического анализа"),
        ERROR_ENTRY(118, "Недопустимые символы в строке (только ASCII)"),
        ERROR_ENTRY_NODEF(119),

        // 120-599: Зарезервировано
        ERROR_ENTRY_NODEF10(120), ERROR_ENTRY_NODEF10(130), ERROR_ENTRY_NODEF10(140),
        ERROR_ENTRY_NODEF10(150), ERROR_ENTRY_NODEF10(160), ERROR_ENTRY_NODEF10(170),
        ERROR_ENTRY_NODEF10(180), ERROR_ENTRY_NODEF10(190),
        ERROR_ENTRY_NODEF100(200), ERROR_ENTRY_NODEF100(300),
        ERROR_ENTRY_NODEF100(400), ERROR_ENTRY_NODEF100(500),

        // 600-609: Синтаксические ошибки
        ERROR_ENTRY(600, "Ошибка в структуре программы"),
        ERROR_ENTRY(601, "Ошибка в последовательности функций"),
        ERROR_ENTRY(602, "Ошибка в параметрах функции"),
        ERROR_ENTRY(603, "Ошибка в теле функции"),
        ERROR_ENTRY(604, "Ошибка в выражении"),
        ERROR_ENTRY(605, "Ошибка в хвосте выражения"),
        ERROR_ENTRY(606, "Ошибка в аргументах вызываемой функции"),
        ERROR_ENTRY(607, "Ошибка в условии"),
        ERROR_ENTRY_NODEF(608), ERROR_ENTRY_NODEF(609),

        // 610-699: Зарезервировано для синтаксических ошибок
        ERROR_ENTRY_NODEF10(610), ERROR_ENTRY_NODEF10(620), ERROR_ENTRY_NODEF10(630),
        ERROR_ENTRY_NODEF10(640), ERROR_ENTRY_NODEF10(650), ERROR_ENTRY_NODEF10(660),
        ERROR_ENTRY_NODEF10(670), ERROR_ENTRY_NODEF10(680), ERROR_ENTRY_NODEF10(690),

        // 700-717: Семантические ошибки
        ERROR_ENTRY(700, "Семантическая ошибка"),
        ERROR_ENTRY(701, "Необъявленный идентификатор"),
        ERROR_ENTRY(702, "Повторное объявление идентификатора"),
        ERROR_ENTRY(703, "Несоответствие типов"),
        ERROR_ENTRY(704, "Отсутствует функция main"),
        ERROR_ENTRY(705, "Неверное количество параметров функции"),
        ERROR_ENTRY(706, "Неверный тип параметра"),
        ERROR_ENTRY(707, "Неверный тип возвращаемого значения"),
        ERROR_ENTRY(708, "Недопустимая операция для данного типа"),
        ERROR_ENTRY(709, "Несоответствие типов при присваивании"),
        ERROR_ENTRY(710, "Неверный тип в условии"),
        ERROR_ENTRY(711, "Недопустимый тип для оператора output"),
        ERROR_ENTRY(712, "Недопустимый тип для унарной операции"),
        ERROR_ENTRY(713, "Использование переменной до объявления"),
        ERROR_ENTRY(714, "Вызов необъявленной функции"),
        ERROR_ENTRY(715, "Ошибка области видимости"),
        ERROR_ENTRY(716, "Недопустимый тип для арифметической операции"),
        ERROR_ENTRY(717, "Недопустимый тип для операции сравнения"),
        ERROR_ENTRY_NODEF(718), ERROR_ENTRY_NODEF(719),

        // 720-799: Зарезервировано для семантических ошибок
        ERROR_ENTRY_NODEF10(720), ERROR_ENTRY_NODEF10(730), ERROR_ENTRY_NODEF10(740),
        ERROR_ENTRY_NODEF10(750), ERROR_ENTRY_NODEF10(760), ERROR_ENTRY_NODEF10(770),
        ERROR_ENTRY_NODEF10(780), ERROR_ENTRY_NODEF10(790),

        // 800-999: Зарезервировано
        ERROR_ENTRY_NODEF100(800), ERROR_ENTRY_NODEF100(900)
    };

    ERRORDATA geterror(int id)
    {
        if (id > 0 && id < ERROR_MAX_ENTRY)
            return errors[id];
        return errors[0];
    }

    ERRORDATA geterrorin(int id, int line, int col)
    {
        if (id > 0 && id < ERROR_MAX_ENTRY)
        {
            errors[id].inext.line = line;
            errors[id].inext.col = col;
            return errors[id];
        }
        return errors[0];
    }
}