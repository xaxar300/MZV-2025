// Log.h - модуль журналирования MZV-2025
// Добавлен variadic WriteLine (из laba18)
#pragma once

#include <cstdarg>

namespace Log
{
    struct LOGDATA
    {
        FILE* stream = nullptr;
        wchar_t filename[300] = {0};
    };

    // Создать/открыть файл журнала
    LOGDATA getlog(wchar_t filename[]);

    // Записать заголовок журнала
    void writelog(LOGDATA lg);

    // Записать информацию о параметрах
    void writeparm(LOGDATA lg, Parm::PARM p);

    // Записать информацию о входных данных
    void writein(LOGDATA lg, Input::INDATA data);

    // Записать сообщение об ошибке
    void writeerror(LOGDATA lg, Error::ERRORDATA e);

    // Записать строку в журнал
    void writeline(LOGDATA lg, const char* msg);

    // Закрыть файл журнала
    void close(LOGDATA lg);

    // =================================================================
    // Variadic функции записи (из laba18)
    // Позволяют использовать printf-подобный синтаксис
    // =================================================================

    // Записать форматированную строку (printf-стиль)
    // Пример: WriteLine(log, "Ошибка на строке %d: %s", line, msg);
    void WriteLine(LOGDATA lg, const char* format, ...);

    // Записать форматированную wide-строку
    void WriteLine(LOGDATA lg, const wchar_t* format, ...);
}
