// StdLib.cpp - реализация стандартной библиотеки MZV-2025
// Встроенные функции: strcmp (сравнение строк), strcpy (копирование строк)
#include "StdLib.h"
#include <cstdio>
#include <cstring>

namespace StdLib
{
    // Глобальная таблица встроенных функций
    static StdLibTable stdlibTable;
    static bool initialized = false;

    // =====================================================
    // Инициализация таблицы стандартной библиотеки
    // =====================================================
    StdLibTable Create()
    {
        StdLibTable table;
        table.count = STDLIB_FUNC_COUNT;

        // -----------------------------------------------
        // strcmp(s1, s2) - лексикографическое сравнение
        // Возвращает: integer
        //   < 0 : s1 < s2
        //   = 0 : s1 == s2
        //   > 0 : s1 > s2
        // -----------------------------------------------
        strcpy_s(table.functions[STDLIB_IDX_STRCMP].name, STDLIB_STRCMP);
        table.functions[STDLIB_IDX_STRCMP].returnType = STDLIB_INT;
        table.functions[STDLIB_IDX_STRCMP].paramCount = 2;
        table.functions[STDLIB_IDX_STRCMP].paramTypes[0] = STDLIB_STR;  // s1
        table.functions[STDLIB_IDX_STRCMP].paramTypes[1] = STDLIB_STR;  // s2
        table.functions[STDLIB_IDX_STRCMP].isBuiltin = true;

        // -----------------------------------------------
        // strcpy(dest, src) - копирование строки
        // Возвращает: integer (длина скопированной строки)
        // Копирует src в dest
        // -----------------------------------------------
        strcpy_s(table.functions[STDLIB_IDX_STRCPY].name, STDLIB_STRCPY);
        table.functions[STDLIB_IDX_STRCPY].returnType = STDLIB_INT;
        table.functions[STDLIB_IDX_STRCPY].paramCount = 2;
        table.functions[STDLIB_IDX_STRCPY].paramTypes[0] = STDLIB_STR;  // dest
        table.functions[STDLIB_IDX_STRCPY].paramTypes[1] = STDLIB_STR;  // src
        table.functions[STDLIB_IDX_STRCPY].isBuiltin = true;

        // Заполняем хэш-индексы для быстрого поиска O(1)
        for (int i = 0; i < table.count; i++)
        {
            std::string name(table.functions[i].name);
            table.nameSet.insert(name);
            table.nameToIndex[name] = i;
        }

        stdlibTable = table;
        initialized = true;
        return table;
    }

    // =====================================================
    // Проверка: является ли идентификатор встроенной функцией
    // Оптимизировано: O(1) через хэш-таблицу
    // =====================================================
    bool IsBuiltinFunction(const char* name)
    {
        if (!initialized) Create();
        return stdlibTable.nameSet.find(std::string(name)) != stdlibTable.nameSet.end();
    }

    // =====================================================
    // Получить индекс встроенной функции
    // Оптимизировано: O(1) через хэш-таблицу
    // =====================================================
    int GetBuiltinIndex(const char* name)
    {
        if (!initialized) Create();
        auto it = stdlibTable.nameToIndex.find(std::string(name));
        if (it != stdlibTable.nameToIndex.end())
            return it->second;
        return -1;
    }

    // =====================================================
    // Получение информации о встроенной функции по индексу
    // =====================================================
    BuiltinFunc GetBuiltinInfo(int index)
    {
        if (!initialized) Create();

        if (index >= 0 && index < stdlibTable.count)
            return stdlibTable.functions[index];

        // Возвращаем пустую структуру при ошибке
        BuiltinFunc empty;
        memset(&empty, 0, sizeof(empty));
        return empty;
    }

    // =====================================================
    // Получение информации о встроенной функции по имени
    // =====================================================
    BuiltinFunc GetBuiltinInfo(const char* name)
    {
        int idx = GetBuiltinIndex(name);
        return GetBuiltinInfo(idx);
    }

    // =====================================================
    // Проверка корректности вызова встроенной функции
    // =====================================================
    bool ValidateCall(const char* name, int argCount, STDLIB_DATATYPE* argTypes, char* errorMsg)
    {
        if (!initialized) Create();

        int idx = GetBuiltinIndex(name);
        if (idx < 0)
        {
            sprintf_s(errorMsg, 256, "'%s' is not a standard library function", name);
            return false;
        }

        BuiltinFunc& func = stdlibTable.functions[idx];

        // Проверка количества аргументов
        if (argCount != func.paramCount)
        {
            sprintf_s(errorMsg, 256, "'%s' expects %d arguments, got %d",
                name, func.paramCount, argCount);
            return false;
        }

        // Проверка типов аргументов
        for (int i = 0; i < argCount; i++)
        {
            // Строгая проверка для строковых типов
            if (func.paramTypes[i] == STDLIB_STR && argTypes[i] != STDLIB_STR)
            {
                sprintf_s(errorMsg, 256, "'%s' argument %d must be string type", name, i + 1);
                return false;
            }
            // Для числовых типов: integer и char совместимы
            if ((func.paramTypes[i] == STDLIB_INT || func.paramTypes[i] == STDLIB_CHR) &&
                argTypes[i] == STDLIB_STR)
            {
                sprintf_s(errorMsg, 256, "'%s' argument %d cannot be string type", name, i + 1);
                return false;
            }
        }

        return true;
    }

    // =====================================================
    // Получение ассемблерного кода для встроенной функции
    // Генерируется функция-обертка для вызова стандартных функций
    // =====================================================
    const char* GetAsmWrapper(const char* name)
    {
        static char buffer[2048];

        int idx = GetBuiltinIndex(name);
        if (idx < 0) return "";

        switch (idx)
        {
        case STDLIB_IDX_STRCMP:
            strcpy_s(buffer,
                "; ========== strcmp - лексикографическое сравнение строк ==========\n"
                "; Входные параметры:\n"
                ";   arg1 (ecx) - указатель на первую строку\n"
                ";   arg2 (edx) - указатель на вторую строку\n"
                "; Возвращает:\n"
                ";   eax < 0  если s1 < s2\n"
                ";   eax = 0  если s1 == s2\n"
                ";   eax > 0  если s1 > s2\n"
                "strcmp_mzv PROC\n"
                "    push ebx\n"
                "    push esi\n"
                "    push edi\n"
                "    mov esi, [esp+16]     ; esi = s1\n"
                "    mov edi, [esp+20]     ; edi = s2\n"
                "strcmp_loop:\n"
                "    movzx eax, byte ptr [esi]\n"
                "    movzx ebx, byte ptr [edi]\n"
                "    cmp al, bl\n"
                "    jne strcmp_diff\n"
                "    test al, al           ; проверка конца строки\n"
                "    jz strcmp_equal\n"
                "    inc esi\n"
                "    inc edi\n"
                "    jmp strcmp_loop\n"
                "strcmp_diff:\n"
                "    sub eax, ebx          ; eax = s1[i] - s2[i]\n"
                "    jmp strcmp_done\n"
                "strcmp_equal:\n"
                "    xor eax, eax          ; eax = 0 (строки равны)\n"
                "strcmp_done:\n"
                "    pop edi\n"
                "    pop esi\n"
                "    pop ebx\n"
                "    ret\n"
                "strcmp_mzv ENDP\n\n"
            );
            break;

        case STDLIB_IDX_STRCPY:
            strcpy_s(buffer,
                "; ========== strcpy - копирование строки ==========\n"
                "; Входные параметры:\n"
                ";   arg1 (ecx) - указатель на dest (приемник)\n"
                ";   arg2 (edx) - указатель на src (источник)\n"
                "; Возвращает:\n"
                ";   eax = длина скопированной строки\n"
                "strcpy_mzv PROC\n"
                "    push ebx\n"
                "    push esi\n"
                "    push edi\n"
                "    mov edi, [esp+16]     ; edi = dest\n"
                "    mov esi, [esp+20]     ; esi = src\n"
                "    xor ecx, ecx          ; ecx = счетчик длины\n"
                "strcpy_loop:\n"
                "    movzx eax, byte ptr [esi]\n"
                "    mov byte ptr [edi], al\n"
                "    test al, al           ; проверка конца строки\n"
                "    jz strcpy_done\n"
                "    inc esi\n"
                "    inc edi\n"
                "    inc ecx\n"
                "    jmp strcpy_loop\n"
                "strcpy_done:\n"
                "    mov eax, ecx          ; возвращаем длину\n"
                "    pop edi\n"
                "    pop esi\n"
                "    pop ebx\n"
                "    ret\n"
                "strcpy_mzv ENDP\n\n"
            );
            break;

        default:
            buffer[0] = '\0';
        }

        return buffer;
    }

    // =====================================================
    // Генерация вызова встроенной функции
    // =====================================================
    const char* GenerateCall(const char* name, const char* arg1, const char* arg2)
    {
        static char buffer[512];

        int idx = GetBuiltinIndex(name);
        if (idx < 0) return "";

        switch (idx)
        {
        case STDLIB_IDX_STRCMP:
            sprintf_s(buffer,
                "    ; вызов strcmp(%s, %s)\n"
                "    push %s              ; второй аргумент\n"
                "    push %s              ; первый аргумент\n"
                "    call strcmp_mzv\n"
                "    add esp, 8           ; очистка стека\n"
                "    ; результат в eax\n",
                arg1, arg2, arg2, arg1
            );
            break;

        case STDLIB_IDX_STRCPY:
            sprintf_s(buffer,
                "    ; вызов strcpy(%s, %s)\n"
                "    push %s              ; src\n"
                "    push %s              ; dest\n"
                "    call strcpy_mzv\n"
                "    add esp, 8           ; очистка стека\n"
                "    ; длина скопированной строки в eax\n",
                arg1, arg2, arg2, arg1
            );
            break;

        default:
            buffer[0] = '\0';
        }

        return buffer;
    }
}
