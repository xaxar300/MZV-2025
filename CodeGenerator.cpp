// CodeGenerator.cpp - генератор кода 
#include "stdafx.h"
#include "CodeGenerator.h"
#include <sstream>
#include <algorithm>

namespace CodeGen
{
    // =========================================================
    // Конструктор и деструктор
    // =========================================================

    Generator::Generator(LT::LexTable& lt, IT::IdTable& it, const std::string& outPath)
        : lextable(lt), idtable(it), outputPath(outPath), labelCounter(0), tempStackOffset(0)
    {
    }

    Generator::~Generator()
    {
        if (out.is_open())
            out.close();
    }

    // =========================================================
    // Главная функция генерации
    // =========================================================

    bool Generator::Generate()
    {
        out.open(outputPath);
        if (!out.is_open())
        {
            cout << "    Ошибка: не удалось открыть файл " << outputPath << endl;
            return false;
        }

        cout << "    Сбор информации о функциях..." << endl;
        CollectFunctions();
        CollectStringLiterals();

        cout << "    Генерация ассемблерного кода..." << endl;
        GenerateHeader();
        GenerateDataSection();
        GenerateCodeSection();
        GenerateFooter();

        out.close();
        cout << "    Ассемблерный код сохранен в: " << outputPath << endl;
        return true;
    }

    // =========================================================
    // Сбор информации о функциях
    // =========================================================

    void Generator::CollectFunctions()
    {
        for (int i = 0; i < idtable.size; i++)
        {
            if (idtable.table[i].idtype == IT_F)
            {
                FunctionInfo func;
                func.name = idtable.table[i].id;
                func.paramCount = idtable.table[i].paramCount;
                func.localVarCount = 0;

                // Собираем переменные этой функции
                for (int j = 0; j < idtable.size; j++)
                {
                    if (strcmp(idtable.table[j].scope, func.name.c_str()) == 0)
                    {
                        if (idtable.table[j].idtype == IT_P || idtable.table[j].idtype == IT_V)
                        {
                            StackVar var;
                            var.name = idtable.table[j].id;
                            var.datatype = idtable.table[j].datatype;
                            var.idtype = idtable.table[j].idtype;
                            var.offset = 0; // Будет вычислено позже
                            func.vars.push_back(var);

                            if (idtable.table[j].idtype == IT_V)
                                func.localVarCount++;
                        }
                    }
                }

                CalculateStackLayout(func);
                functions[func.name] = func;
            }
        }
    }

    void Generator::CollectStringLiterals()
    {
        int strCount = 0;
        for (int i = 0; i < idtable.size; i++)
        {
            if (idtable.table[i].idtype == IT_L && idtable.table[i].datatype == IT_STR)
            {
                std::string label = "str_" + std::to_string(strCount++);
                std::string value = idtable.table[i].value.vstr.str;

                // Строка передаётся как есть (входной файл в CP1251)
                stringLiterals.push_back({label, value});

                // Сохраняем метку в id для последующего использования
                strcpy_s(idtable.table[i].id, label.c_str());
            }
        }
    }

    void Generator::CalculateStackLayout(FunctionInfo& func)
    {

        int offset = -8; // Начинаем после сохранённого RBP

        // Параметры (первые 4 передаются в регистрах, но копируются в shadow space)
        int paramIdx = 0;
        for (auto& var : func.vars)
        {
            if (var.idtype == IT_P)
            {
                var.offset = offset;
                offset -= 8;
                paramIdx++;
            }
        }

        // Локальные переменные
        for (auto& var : func.vars)
        {
            if (var.idtype == IT_V)
            {
                var.offset = offset;
                offset -= 8;
            }
        }

        // Размер стека должен быть выровнен по 16 байт
        // Минимум 32 байта для shadow space вызываемых функций
        int minStack = 32 + (-offset) + 8; // +8 для выравнивания
        func.stackSize = ((minStack + 15) / 16) * 16;
    }

    // =========================================================
    // Генерация секций
    // =========================================================

    void Generator::GenerateHeader()
    {
        EmitLine("; =========================================");
        EmitLine("; MZV-2025 Compiler - Generated Assembly");
        EmitLine("; Target: Windows x64 (MASM)");
        EmitLine("; =========================================");
        EmitLine("");
        EmitLine(".686");
        EmitLine(".model flat, stdcall");
        EmitLine("option casemap:none");
        EmitLine("");
        EmitLine("; External functions from C runtime");
        EmitLine("includelib kernel32.lib");
        EmitLine("includelib msvcrt.lib");
        EmitLine("");
        EmitLine("ExitProcess PROTO :DWORD");
        EmitLine("printf PROTO C :VARARG");
        EmitLine("SetConsoleOutputCP PROTO :DWORD");
        EmitLine("");
    }

    void Generator::GenerateDataSection()
    {
        EmitLine(".data");
        EmitLine("");

        // Форматные строки для printf
        EmitLine("    fmt_int    db \"%d\", 10, 0        ; Format for integer");
        EmitLine("    fmt_char   db \"%c\", 10, 0        ; Format for char");
        EmitLine("    fmt_str    db \"%s\", 10, 0        ; Format for string");
        EmitLine("");

        // Строковые литералы
        if (!stringLiterals.empty())
        {
            EmitLine("    ; String literals");
            for (const auto& lit : stringLiterals)
            {
                // Записываем строку напрямую в кавычках
                EmitLine("    " + lit.first + " db \"" + lit.second + "\", 0");
            }
            EmitLine("");
        }

        // Глобальные переменные
        EmitLine("    ; Глобальные переменные");
        for (int i = 0; i < idtable.size; i++)
        {
            if (idtable.table[i].idtype == IT_V &&
                strlen(idtable.table[i].scope) == 0)
            {
                std::string varName = idtable.table[i].id;
                EmitLine("    _" + varName + " dd 0");
            }
        }
        EmitLine("");
    }

    void Generator::GenerateCodeSection()
    {
        EmitLine(".code");
        EmitLine("");

        // Генерируем все функции
        // Порядок в лексемах: t f i ( ... - тип, function, имя, скобка
        for (int i = 0; i < lextable.size; i++)
        {
            // Ищем объявление функции: t f i (
            // Тип уже прошел, function на позиции i
            if (lextable.table[i].lexema == LEX_FUNCTION)
            {
                // Следующая лексема - имя функции
                if (i + 1 < lextable.size && lextable.table[i + 1].lexema == LEX_ID)
                {
                    int funcIdxTI = lextable.table[i + 1].idxTI;
                    if (funcIdxTI != LT_TI_NULLIDX)
                    {
                        std::string funcName = idtable.table[funcIdxTI].id;
                        // Пропускаем встроенные функции (strcmp, strcpy)
                        if (funcName != "strcmp" && funcName != "strcpy")
                        {
                            GenerateFunction(i, funcName);
                        }
                    }
                }
            }
            // Ищем main
            else if (lextable.table[i].lexema == LEX_MAIN)
            {
                GenerateMainFunction(i);
            }
        }
    }

    void Generator::GenerateFooter()
    {
        EmitLine("");
        EmitLine("end main");
    }

    // =========================================================
    // Генерация функций
    // =========================================================

    void Generator::GenerateFunction(int startIdx, const std::string& funcName)
    {
        currentFunction = funcName;
        auto it = functions.find(funcName);
        if (it == functions.end())
        {
            EmitComment("Ошибка: функция " + funcName + " не найдена");
            return;
        }
        const FunctionInfo& func = it->second;

        EmitLine("");
        EmitComment("Функция: " + funcName);
        EmitLine(funcName + " PROC");

        GeneratePrologue(func);

        // x86 stdcall: параметры передаются через стек
        // Первый параметр на [ebp+8], второй на [ebp+12] и т.д.
        // Копируем их в локальные переменные для удобства
        int paramIdx = 0;
        for (const auto& var : func.vars)
        {
            if (var.idtype == IT_P)
            {
                std::stringstream ss;
                // Параметр на [ebp + 8 + paramIdx*4]
                ss << "    mov eax, DWORD PTR [ebp+" << (8 + paramIdx * 4) << "]";
                EmitLine(ss.str());
                ss.str("");
                ss << "    mov DWORD PTR [ebp" << var.offset << "], eax";
                EmitLine(ss.str());
                paramIdx++;
            }
        }

        // Находим тело функции (после {)
        int bodyStart = startIdx;
        while (bodyStart < lextable.size && lextable.table[bodyStart].lexema != LEX_LEFTBRACE)
            bodyStart++;
        bodyStart++; // Пропускаем {

        int bodyEnd = FindMatchingBrace(bodyStart - 1);

        // Генерируем тело функции
        int idx = bodyStart;
        while (idx < bodyEnd)
        {
            GenerateStatement(idx);
        }

        // Эпилог генерируется в GenerateReturn, но на случай если return нет
        // (такого не должно быть по семантике, но на всякий случай)
        EmitLine(funcName + " ENDP");
        currentFunction = "";
    }

    void Generator::GenerateMainFunction(int startIdx)
    {
        currentFunction = "main";

        // Проверяем, есть ли main в functions, если нет - создаём
        if (functions.find("main") == functions.end())
        {
            FunctionInfo func;
            func.name = "main";
            func.paramCount = 0;
            func.localVarCount = 0;

            // Собираем локальные переменные main
            for (int j = 0; j < idtable.size; j++)
            {
                if (strcmp(idtable.table[j].scope, "main") == 0)
                {
                    if (idtable.table[j].idtype == IT_V)
                    {
                        StackVar var;
                        var.name = idtable.table[j].id;
                        var.datatype = idtable.table[j].datatype;
                        var.idtype = idtable.table[j].idtype;
                        var.offset = 0;
                        func.vars.push_back(var);
                        func.localVarCount++;
                    }
                }
            }
            CalculateStackLayout(func);
            functions["main"] = func;
        }

        const FunctionInfo& func = functions["main"];

        EmitLine("");
        EmitComment("Главная функция");
        EmitLine("main PROC");

        GeneratePrologue(func);

        // Устанавливаем кодовую страницу CP1251 для корректного вывода кириллицы
        EmitLine("");
        EmitComment("Установка кодовой страницы CP1251 для кириллицы");
        EmitLine("    push 1251");
        EmitLine("    call SetConsoleOutputCP");

        // Находим тело main (после {)
        int bodyStart = startIdx;
        while (bodyStart < lextable.size && lextable.table[bodyStart].lexema != LEX_LEFTBRACE)
            bodyStart++;
        bodyStart++; // Пропускаем {

        int bodyEnd = FindMatchingBrace(bodyStart - 1);

        // Генерируем тело main
        int idx = bodyStart;
        while (idx < bodyEnd)
        {
            GenerateStatement(idx);
        }

        // В main return уже вызывает ret, поэтому дополнительный код не нужен
        EmitLine("main ENDP");
        currentFunction = "";
    }

    void Generator::GeneratePrologue(const FunctionInfo& func)
    {
        EmitLine("    push ebp");
        EmitLine("    mov ebp, esp");

        if (func.stackSize > 0)
        {
            std::stringstream ss;
            ss << "    sub esp, " << func.stackSize;
            EmitLine(ss.str());
        }

        // Сохраняем callee-saved регистры
        EmitLine("    push ebx");
        EmitLine("    push esi");
        EmitLine("    push edi");
    }

    void Generator::GenerateEpilogue(const FunctionInfo& func)
    {
        EmitLine("");
        EmitLine("    ; Эпилог");
        EmitLine("    pop edi");
        EmitLine("    pop esi");
        EmitLine("    pop ebx");
        EmitLine("    mov esp, ebp");
        EmitLine("    pop ebp");
        EmitLine("    ret");
    }

    // =========================================================
    // Генерация операторов
    // =========================================================

    void Generator::GenerateStatement(int& idx)
    {
        if (idx >= lextable.size)
            return;

        char lex = lextable.table[idx].lexema;

        // Пропускаем пустые лексемы (ПОЛИЗ)
        if (lex == LEX_POLIZ_NULL)
        {
            idx++;
            return;
        }

        switch (lex)
        {
        case LEX_TYPE: // Объявление переменной: t i = expr ;
        {
            idx++; // Пропускаем тип
            if (idx < lextable.size && lextable.table[idx].lexema == LEX_ID)
            {
                int varIdxTI = lextable.table[idx].idxTI;
                idx++; // Пропускаем имя

                if (idx < lextable.size && lextable.table[idx].lexema == LEX_ASSIGN)
                {
                    idx++; // Пропускаем =
                    GenerateAssignment(idx);
                    // Сохраняем результат в переменную
                    StoreFromRAX(varIdxTI);
                }

                // Пропускаем до ;
                while (idx < lextable.size && lextable.table[idx].lexema != LEX_SEMICOLON)
                    idx++;
                idx++; // Пропускаем ;
            }
            break;
        }

        case LEX_ID: // Присваивание: i = expr ; или унарная операция: i u ;
        {
            int varIdxTI = lextable.table[idx].idxTI;
            idx++; // Пропускаем имя

            if (idx < lextable.size && lextable.table[idx].lexema == LEX_ASSIGN)
            {
                idx++; // Пропускаем =
                GenerateAssignment(idx);
                StoreFromRAX(varIdxTI);
            }
            else if (idx < lextable.size && lextable.table[idx].lexema == LEX_UNARY)
            {
                // Унарная операция: i++, i--, ~i
                char op = lextable.table[idx].op;
                EmitLine("");
                EmitComment("Унарная операция");

                // Загружаем переменную
                LoadToRAX(varIdxTI);

                // Применяем операцию
                switch (op)
                {
                case '+':  // ++
                    EmitLine("    inc eax");
                    break;
                case '-':  // --
                    EmitLine("    dec eax");
                    break;
                case '~':  // Битовое отрицание
                    EmitLine("    not eax");
                    break;
                }

                // Сохраняем обратно
                StoreFromRAX(varIdxTI);
                idx++; // Пропускаем u
            }

            while (idx < lextable.size && lextable.table[idx].lexema != LEX_SEMICOLON)
                idx++;
            idx++; // ;
            break;
        }

        case LEX_RETURN:
            idx++; // Пропускаем return
            GenerateReturn(idx);
            break;

        case LEX_OUTPUT:
            idx++; // Пропускаем output
            GenerateOutput(idx);
            break;

        case LEX_IF:
            GenerateIf(idx);
            break;

        case LEX_LEFTBRACE:
        case LEX_RIGHTBRACE:
        case LEX_SEMICOLON:
            idx++;
            break;

        default:
            idx++;
            break;
        }
    }

    void Generator::GenerateAssignment(int& idx)
    {
        // Находим конец выражения (;)
        int exprEnd = idx;
        while (exprEnd < lextable.size && lextable.table[exprEnd].lexema != LEX_SEMICOLON)
            exprEnd++;

        // Генерируем код для выражения в ПОЛИЗ
        GeneratePolizExpression(idx, exprEnd);

        idx = exprEnd;
    }

    void Generator::GenerateReturn(int& idx)
    {
        EmitLine("");
        EmitComment("return");

        // Находим конец выражения
        int exprEnd = idx;
        while (exprEnd < lextable.size && lextable.table[exprEnd].lexema != LEX_SEMICOLON)
            exprEnd++;

        // Вычисляем возвращаемое значение
        GeneratePolizExpression(idx, exprEnd);

        // Результат уже в EAX
        // Переходим к эпилогу
        const FunctionInfo& func = GetCurrentFunction();
        EmitLine("    pop edi");
        EmitLine("    pop esi");
        EmitLine("    pop ebx");
        EmitLine("    mov esp, ebp");
        EmitLine("    pop ebp");
        EmitLine("    ret");

        idx = exprEnd + 1; // Пропускаем ;
    }

    void Generator::GenerateOutput(int& idx)
    {
        EmitLine("");
        EmitComment("output");

        // Находим конец выражения
        int exprEnd = idx;
        while (exprEnd < lextable.size && lextable.table[exprEnd].lexema != LEX_SEMICOLON)
            exprEnd++;

        // Определяем тип выражения по первому операнду
        IDDATATYPE outputType = IT_INT;
        for (int i = idx; i < exprEnd; i++)
        {
            if (lextable.table[i].lexema == LEX_ID || lextable.table[i].lexema == LEX_LITERAL)
            {
                int idxTI = lextable.table[i].idxTI;
                if (idxTI != LT_TI_NULLIDX)
                {
                    outputType = idtable.table[idxTI].datatype;
                    break;
                }
            }
        }

        // Вычисляем выражение
        GeneratePolizExpression(idx, exprEnd);

        // Вызываем printf
        if (outputType == IT_STR)
        {
            EmitLine("    push eax");
            EmitLine("    push OFFSET fmt_str");
        }
        else if (outputType == IT_CHR)
        {
            EmitLine("    push eax");
            EmitLine("    push OFFSET fmt_char");
        }
        else
        {
            EmitLine("    push eax");
            EmitLine("    push OFFSET fmt_int");
        }

        EmitLine("    call printf");
        EmitLine("    add esp, 8");

        idx = exprEnd + 1;
    }

    void Generator::GenerateIf(int& idx)
    {
        EmitLine("");
        EmitComment("if");

        std::string labelElse = NewLabel("else_");
        std::string labelEnd = NewLabel("endif_");

        idx++; // Пропускаем if '?'

        // Пропускаем (
        if (idx < lextable.size && lextable.table[idx].lexema == LEX_LEFTHESIS)
            idx++;

        // Находим условие (до закрывающей скобки)
        int condEnd = idx;
        int parenDepth = 1;
        while (condEnd < lextable.size && parenDepth > 0)
        {
            if (lextable.table[condEnd].lexema == LEX_LEFTHESIS)
                parenDepth++;
            else if (lextable.table[condEnd].lexema == LEX_RIGHTHESIS)
                parenDepth--;
            if (parenDepth > 0)
                condEnd++;
        }

        // Вычисляем условие
        GeneratePolizExpression(idx, condEnd);
        idx = condEnd + 1; // Пропускаем )

        // Проверка условия
        EmitLine("    cmp eax, 0");
        EmitLine("    je " + labelElse);

        // Пропускаем {
        if (idx < lextable.size && lextable.table[idx].lexema == LEX_LEFTBRACE)
            idx++;

        // Генерируем then-ветку
        int thenEnd = FindMatchingBrace(idx - 1);
        while (idx < thenEnd)
        {
            GenerateStatement(idx);
        }
        idx++; // Пропускаем }

        EmitLine("    jmp " + labelEnd);

        // else-ветка
        EmitLine(labelElse + ":");

        // Проверяем наличие else
        if (idx < lextable.size && lextable.table[idx].lexema == LEX_ELSE)
        {
            idx++; // Пропускаем else ':'

            // Пропускаем {
            if (idx < lextable.size && lextable.table[idx].lexema == LEX_LEFTBRACE)
                idx++;

            int elseEnd = FindMatchingBrace(idx - 1);
            while (idx < elseEnd)
            {
                GenerateStatement(idx);
            }
            idx++; // Пропускаем }
        }

        EmitLine(labelEnd + ":");
    }

    // =========================================================
    // Генерация выражений (ПОЛИЗ)
    // =========================================================

    void Generator::GeneratePolizExpression(int startIdx, int endIdx)
    {
        // Обрабатываем ПОЛИЗ: операнды -> push, операторы -> pop, compute, push

        for (int i = startIdx; i < endIdx; i++)
        {
            char lex = lextable.table[i].lexema;

            if (lex == LEX_POLIZ_NULL)
                continue;

            if (lex == LEX_ID || lex == LEX_LITERAL)
            {
                // Проверяем, не является ли это именем вызываемой функции
                // (если следующий не-NULL токен - LEX_CALL)
                bool isFunctionName = false;
                for (int j = i + 1; j < endIdx; j++)
                {
                    if (lextable.table[j].lexema == LEX_POLIZ_NULL)
                        continue;
                    if (lextable.table[j].lexema == LEX_CALL)
                        isFunctionName = true;
                    break;
                }

                if (!isFunctionName)
                {
                    // Обычный операнд - загружаем в EAX и кладём на стек
                    int idxTI = lextable.table[i].idxTI;
                    LoadToRAX(idxTI);
                    EmitLine("    push eax");
                }
                // Если это имя функции - пропускаем, обработаем при LEX_CALL
            }
            else if (lex == LEX_BINOP)
            {
                // Бинарная операция
                char op = lextable.table[i].op;
                GenerateBinaryOp(op);
            }
            else if (lex == LEX_UNARY)
            {
                // Унарная операция
                char op = lextable.table[i].op;
                GenerateUnaryOp(op);
            }
            else if (lex == LEX_COMPARE)
            {
                // Операция сравнения
                char op = lextable.table[i].op;
                GenerateCompareOp(op);
            }
            else if (lex == LEX_CALL)
            {
                // Вызов функции
                // idxTI содержит количество параметров
                int paramCount = lextable.table[i].idxTI;

                // Имя функции - предыдущая не-NULL лексема
                int funcNameIdx = i - 1;
                while (funcNameIdx >= startIdx && lextable.table[funcNameIdx].lexema == LEX_POLIZ_NULL)
                    funcNameIdx--;

                if (funcNameIdx >= startIdx && lextable.table[funcNameIdx].lexema == LEX_ID)
                {
                    int funcIdxTI = lextable.table[funcNameIdx].idxTI;
                    if (funcIdxTI != LT_TI_NULLIDX)
                    {
                        // Получаем имя функции
                        // Ищем функцию с таким именем в глобальной области
                        std::string varName = idtable.table[funcIdxTI].id;
                        std::string funcName = varName;

                        // Проверяем, есть ли функция с таким именем
                        for (int fi = 0; fi < idtable.size; fi++)
                        {
                            if (idtable.table[fi].idtype == IT_F &&
                                strcmp(idtable.table[fi].id, varName.c_str()) == 0)
                            {
                                funcName = idtable.table[fi].id;
                                break;
                            }
                        }

                        // cdecl: параметры передаются справа налево
                        // Сейчас на стеке параметры в порядке: param1, param2, ... paramN (слева направо)
                        // Нужно: paramN, ..., param2, param1 (справа налево)
                        // Переворачиваем порядок параметров
                        if (paramCount > 1)
                        {
                            // Перекладываем параметры в правильном порядке
                            // Снимаем все параметры во временные регистры/стек
                            // и кладём обратно в обратном порядке

                            // Сохраняем параметры в регистры/память
                            for (int p = 0; p < paramCount; p++)
                            {
                                std::stringstream ss;
                                ss << "    pop DWORD PTR [ebp-" << (200 + p * 4) << "]";
                                EmitLine(ss.str());
                            }
                            // Кладём обратно в обратном порядке
                            for (int p = 0; p < paramCount; p++)
                            {
                                std::stringstream ss;
                                ss << "    push DWORD PTR [ebp-" << (200 + p * 4) << "]";
                                EmitLine(ss.str());
                            }
                        }

                        EmitLine("    ; Вызов " + funcName);
                        EmitLine("    call " + funcName);

                        // Очищаем параметры со стека
                        if (paramCount > 0)
                        {
                            std::stringstream ss;
                            ss << "    add esp, " << (paramCount * 4);
                            EmitLine(ss.str());
                        }

                        // Результат в EAX, кладём обратно на стек
                        EmitLine("    push eax");
                    }
                }
            }
        }

        // Результат на вершине стека, извлекаем в EAX
        EmitLine("    pop eax");
    }

    void Generator::GenerateBinaryOp(char op)
    {
        // Снимаем операнды со стека
        EmitLine("    pop ebx");       // Второй операнд
        EmitLine("    pop eax");       // Первый операнд

        switch (op)
        {
        case '+':
            EmitLine("    add eax, ebx");
            break;
        case '-':
            EmitLine("    sub eax, ebx");
            break;
        case '*':
            EmitLine("    imul eax, ebx");
            break;
        case '/':
            EmitLine("    cdq");           // Расширение знака
            EmitLine("    idiv ebx");
            break;
        case '%':
            EmitLine("    cdq");
            EmitLine("    idiv ebx");
            EmitLine("    mov eax, edx");  // Остаток в EDX
            break;
        }

        // Кладём результат обратно
        EmitLine("    push eax");
    }

    void Generator::GenerateUnaryOp(char op)
    {
        EmitLine("    pop eax");

        switch (op)
        {
        case '+':  // ++
            EmitLine("    inc eax");
            break;
        case '-':  // --
            EmitLine("    dec eax");
            break;
        case '~':  // Битовое отрицание
            EmitLine("    not eax");
            break;
        }

        EmitLine("    push eax");
    }

    void Generator::GenerateCompareOp(char op)
    {
        std::string labelTrue = NewLabel("cmp_true_");
        std::string labelEnd = NewLabel("cmp_end_");

        EmitLine("    pop ebx");       // Второй операнд
        EmitLine("    pop eax");       // Первый операнд
        EmitLine("    cmp eax, ebx");

        switch (op)
        {
        case '<':
            EmitLine("    jl " + labelTrue);
            break;
        case '>':
            EmitLine("    jg " + labelTrue);
            break;
        case '=':  // ==
            EmitLine("    je " + labelTrue);
            break;
        case '!':  // !=
            EmitLine("    jne " + labelTrue);
            break;
        }

        // False: 0
        EmitLine("    mov eax, 0");
        EmitLine("    jmp " + labelEnd);

        // True: 1
        EmitLine(labelTrue + ":");
        EmitLine("    mov eax, 1");

        EmitLine(labelEnd + ":");
        EmitLine("    push eax");
    }

    // =========================================================
    // Работа с переменными
    // =========================================================

    std::string Generator::GetVarAddress(int idxTI)
    {
        if (idxTI == LT_TI_NULLIDX || idxTI >= idtable.size)
            return "0";

        IT::Entry& entry = idtable.table[idxTI];

        // Литерал
        if (entry.idtype == IT_L)
        {
            if (entry.datatype == IT_STR)
            {
                return "OFFSET " + std::string(entry.id);
            }
            else if (entry.datatype == IT_CHR)
            {
                return std::to_string((int)entry.value.vchr);
            }
            else
            {
                return std::to_string(entry.value.vint);
            }
        }

        // Глобальная переменная
        if (strlen(entry.scope) == 0)
        {
            return "_" + std::string(entry.id);
        }

        // Локальная переменная или параметр
        const FunctionInfo& func = GetCurrentFunction();
        for (const auto& var : func.vars)
        {
            if (var.name == entry.id)
            {
                std::stringstream ss;
                ss << "DWORD PTR [ebp" << var.offset << "]";
                return ss.str();
            }
        }

        return "_" + std::string(entry.id);
    }

    void Generator::LoadToRAX(int idxTI)
    {
        if (idxTI == LT_TI_NULLIDX || idxTI >= idtable.size)
        {
            EmitLine("    mov eax, 0");
            return;
        }

        IT::Entry& entry = idtable.table[idxTI];

        // Литерал
        if (entry.idtype == IT_L)
        {
            if (entry.datatype == IT_STR)
            {
                EmitLine("    mov eax, OFFSET " + std::string(entry.id));
            }
            else if (entry.datatype == IT_CHR)
            {
                std::stringstream ss;
                ss << "    mov eax, " << (int)entry.value.vchr;
                EmitLine(ss.str());
            }
            else
            {
                std::stringstream ss;
                ss << "    mov eax, " << entry.value.vint;
                EmitLine(ss.str());
            }
            return;
        }

        // Глобальная переменная
        if (strlen(entry.scope) == 0)
        {
            EmitLine("    mov eax, _" + std::string(entry.id));
            return;
        }

        // Локальная переменная или параметр
        const FunctionInfo& func = GetCurrentFunction();
        for (const auto& var : func.vars)
        {
            if (var.name == entry.id)
            {
                std::stringstream ss;
                ss << "    mov eax, DWORD PTR [ebp" << var.offset << "]";
                EmitLine(ss.str());
                return;
            }
        }

        EmitLine("    mov eax, _" + std::string(entry.id));
    }

    void Generator::StoreFromRAX(int idxTI)
    {
        if (idxTI == LT_TI_NULLIDX || idxTI >= idtable.size)
            return;

        IT::Entry& entry = idtable.table[idxTI];

        // Глобальная переменная
        if (strlen(entry.scope) == 0)
        {
            EmitLine("    mov _" + std::string(entry.id) + ", eax");
            return;
        }

        // Локальная переменная
        const FunctionInfo& func = GetCurrentFunction();
        for (const auto& var : func.vars)
        {
            if (var.name == entry.id)
            {
                std::stringstream ss;
                ss << "    mov DWORD PTR [ebp" << var.offset << "], eax";
                EmitLine(ss.str());
                return;
            }
        }

        EmitLine("    mov _" + std::string(entry.id) + ", eax");
    }

    // =========================================================
    // Вспомогательные функции
    // =========================================================

    std::string Generator::NewLabel(const std::string& prefix)
    {
        return prefix + std::to_string(labelCounter++);
    }

    void Generator::Emit(const std::string& code)
    {
        out << code;
    }

    void Generator::EmitLine(const std::string& code)
    {
        // Оставляем файл в UTF-8 для корректного отображения в VS
        out << code << "\n";
    }

    // Запись строки без конвертации
    void Generator::EmitLineRaw(const std::string& code)
    {
        out << code << "\n";
    }

    void Generator::EmitComment(const std::string& comment)
    {
        // Комментарии в UTF-8 для читаемости в редакторе
        out << "    ; " << comment << "\n";
    }

    int Generator::FindFunctionEnd(int startIdx)
    {
        int braceCount = 0;
        bool foundBrace = false;

        for (int i = startIdx; i < lextable.size; i++)
        {
            if (lextable.table[i].lexema == LEX_LEFTBRACE)
            {
                braceCount++;
                foundBrace = true;
            }
            else if (lextable.table[i].lexema == LEX_RIGHTBRACE)
            {
                braceCount--;
                if (foundBrace && braceCount == 0)
                    return i;
            }
        }
        return lextable.size;
    }

    int Generator::FindSemicolon(int startIdx)
    {
        for (int i = startIdx; i < lextable.size; i++)
        {
            if (lextable.table[i].lexema == LEX_SEMICOLON)
                return i;
        }
        return lextable.size;
    }

    int Generator::FindMatchingBrace(int startIdx)
    {
        int braceCount = 1;
        for (int i = startIdx + 1; i < lextable.size; i++)
        {
            if (lextable.table[i].lexema == LEX_LEFTBRACE)
                braceCount++;
            else if (lextable.table[i].lexema == LEX_RIGHTBRACE)
            {
                braceCount--;
                if (braceCount == 0)
                    return i;
            }
        }
        return lextable.size;
    }

    int Generator::FindElse(int startIdx)
    {
        for (int i = startIdx; i < lextable.size; i++)
        {
            if (lextable.table[i].lexema == LEX_ELSE)
                return i;
            if (lextable.table[i].lexema == LEX_RIGHTBRACE)
                return -1; // Не найден
        }
        return -1;
    }

    const FunctionInfo& Generator::GetCurrentFunction()
    {
        static FunctionInfo empty;
        auto it = functions.find(currentFunction);
        if (it != functions.end())
            return it->second;
        return empty;
    }

    bool Generator::IsParameter(int idxTI, const FunctionInfo& func)
    {
        if (idxTI >= idtable.size)
            return false;

        std::string name = idtable.table[idxTI].id;
        for (const auto& var : func.vars)
        {
            if (var.name == name && var.idtype == IT_P)
                return true;
        }
        return false;
    }

    int Generator::GetParamIndex(int idxTI, const FunctionInfo& func)
    {
        if (idxTI >= idtable.size)
            return -1;

        std::string name = idtable.table[idxTI].id;
        int idx = 0;
        for (const auto& var : func.vars)
        {
            if (var.idtype == IT_P)
            {
                if (var.name == name)
                    return idx;
                idx++;
            }
        }
        return -1;
    }

    // =========================================================
    // Публичная функция
    // =========================================================

    bool GenerateCode(LT::LexTable& lt, IT::IdTable& it, const wchar_t* outputPath)
    {
        // Конвертируем путь
        char path[PARM_MAX_SIZE];
        size_t converted;
        wcstombs_s(&converted, path, outputPath, PARM_MAX_SIZE);

        // Получаем директорию из пути
        std::string fullPath = path;
        size_t lastSlash = fullPath.rfind('\\');
        if (lastSlash == std::string::npos)
            lastSlash = fullPath.rfind('/');

        std::string dir = "";
        if (lastSlash != std::string::npos)
            dir = fullPath.substr(0, lastSlash + 1);

        // Генерируем в папку ASM относительно директории проекта
        // Поднимаемся на уровень выше из Debug и идём в ASM
        std::string asmPath = dir + "..\\ASM\\output.asm";

        Generator gen(lt, it, asmPath);
        return gen.Generate();
    }
}
