// Log.cpp - реализация модуля журналирования MZV-2025
// Добавлен variadic WriteLine (из laba18)
#include "stdafx.h"

namespace Log
{
    LOGDATA getlog(wchar_t filename[])
    {
        LOGDATA lg;
        wcscpy_s(lg.filename, filename);
        lg.stream = nullptr;
        _wfopen_s(&lg.stream, filename, L"w, ccs=UTF-8");
        if (lg.stream == nullptr)
            throw ERROR_THROW(110);
        return lg;
    }

    void writelog(LOGDATA lg)
    {
        if (lg.stream == nullptr) return;
        time_t now = time(nullptr);
        tm t;
        localtime_s(&t, &now);
        fwprintf(lg.stream, L"=== MZV-2025 ===\n");
        fwprintf(lg.stream, L"Date: %02d.%02d.%d %02d:%02d:%02d\n\n",
            t.tm_mday, t.tm_mon + 1, t.tm_year + 1900,
            t.tm_hour, t.tm_min, t.tm_sec);
    }

    void writeparm(LOGDATA lg, Parm::PARM p)
    {
        if (lg.stream == nullptr) return;
        fwprintf(lg.stream, L"--- Parameters ---\n");
        fwprintf(lg.stream, L"in: %s\n", p.in);
        fwprintf(lg.stream, L"out: %s\n", p.out);
        fwprintf(lg.stream, L"log: %s\n\n", p.log);
    }

    void writein(LOGDATA lg, Input::INDATA data)
    {
        if (lg.stream == nullptr) return;
        fwprintf(lg.stream, L"--- Source code (%d bytes, %d lines) ---\n", data.size, data.lines);
        fwprintf(lg.stream, L"%hs\n\n", data.text);
    }

    void writeerror(LOGDATA lg, Error::ERRORDATA e)
    {
        if (lg.stream == nullptr) return;
        fwprintf(lg.stream, L"--- Error %d: %hs ---\n", e.id, e.message);
        if (e.inext.line >= 0)
            fwprintf(lg.stream, L"Line %d, position %d\n", e.inext.line, e.inext.col);
    }

    void writeline(LOGDATA lg, const char* msg)
    {
        if (lg.stream == nullptr) return;
        fwprintf(lg.stream, L"%hs\n", msg);
    }

    void close(LOGDATA lg)
    {
        if (lg.stream != nullptr)
            fclose(lg.stream);
    }

    // =================================================================
    // Variadic функции записи (из laba18)
    // =================================================================

    void WriteLine(LOGDATA lg, const char* format, ...)
    {
        if (lg.stream == nullptr) return;

        // Конвертируем format в wide string и вызываем vfwprintf
        // Для простоты используем буфер
        char buf[4096];
        va_list args;
        va_start(args, format);
        vsnprintf(buf, sizeof(buf), format, args);
        va_end(args);

        fwprintf(lg.stream, L"%hs\n", buf);
        fflush(lg.stream);  // Немедленная запись
    }

    void WriteLine(LOGDATA lg, const wchar_t* format, ...)
    {
        if (lg.stream == nullptr) return;

        va_list args;
        va_start(args, format);
        vfwprintf(lg.stream, format, args);
        va_end(args);

        fwprintf(lg.stream, L"\n");
        fflush(lg.stream);  // Немедленная запись
    }
}
