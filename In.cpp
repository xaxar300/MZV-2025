// In.cpp - реализация чтения входного файла MZV-2025
#include "stdafx.h"

namespace Input
{
    INDATA getinput(wchar_t filename[])
    {
        INDATA data;  // Используем конструктор по умолчанию
        data.lines = 1;

        FILE* f = nullptr;
        _wfopen_s(&f, filename, L"rb");
        if (!f) throw ERROR_THROW(100);

        fseek(f, 0, SEEK_END);
        long fileSize = ftell(f);
        fseek(f, 0, SEEK_SET);

        // Проверка на отрицательный размер (ошибка ftell)
        if (fileSize < 0)
        {
            fclose(f);
            throw ERROR_THROW(100);
        }

        data.size = fileSize;

        if (data.size > IN_MAX_SIZE)
        {
            fclose(f);
            throw ERROR_THROW(100);
        }

        // Безопасное выделение памяти
        size_t allocSize = static_cast<size_t>(data.size) + 1;
        data.text = new char[allocSize];

        size_t bytesRead = fread(data.text, 1, static_cast<size_t>(data.size), f);
        data.text[bytesRead] = CHAR_NULL;  // Используем реально прочитанный размер
        data.size = static_cast<int>(bytesRead);
        fclose(f);

        // Подсчёт строк
        for (int i = 0; i < data.size; i++)
            if (data.text[i] == CHAR_NEWLINE) data.lines++;

        return data;
    }

    void deleteinput(INDATA& data)
    {
        delete[] data.text;
        data.text = nullptr;
        data.size = data.lines = 0;
    }
};
