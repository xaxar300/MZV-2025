// main.cpp - точка входа компилятора MZV-2025
#include "stdafx.h"

// Вывод таблицы лексем
void PrintLexTable(LT::LexTable& lt)
{
    printf("\n=== Таблица лексем ===\n");
    printf("%-6s %-10s %-8s %-6s %-10s %-6s\n", "N", "Lexema", "Line", "Col", "idxTI", "Op");
    printf("------------------------------------------------------\n");

    for (int i = 0; i < lt.size; i++)
    {
        printf("%-6d %-10c %-8d %-6d ", i, lt.table[i].lexema, lt.table[i].line, lt.table[i].col);

        if (lt.table[i].idxTI != LT_TI_NULLIDX)
            printf("%-10d ", lt.table[i].idxTI);
        else
            printf("%-10s ", "-");

        if (lt.table[i].op != CHAR_NULL)
            printf("%-6c", lt.table[i].op);
        printf("\n");
    }
}

// Вывод таблицы идентификаторов
void PrintIdTable(IT::IdTable& it)
{
    printf("\n=== Таблица идентификаторов ===\n");
    printf("%-5s %-12s %-12s %-8s %-8s %s\n", "N", "Name", "Scope", "Type", "Kind", "Value");
    printf("--------------------------------------------------------------\n");

    for (int i = 0; i < it.size; i++)
    {
        const char* scope = it.table[i].scope[0] ? it.table[i].scope : "global";
        const char* type = "";
        const char* kind = "";

        switch (it.table[i].datatype)
        {
        case IT_INT: type = "int"; break;
        case IT_CHR: type = "char"; break;
        case IT_STR: type = "str"; break;
        }

        switch (it.table[i].idtype)
        {
        case IT_V: kind = "var"; break;
        case IT_F: kind = "func"; break;
        case IT_P: kind = "param"; break;
        case IT_L: kind = "lit"; break;
        }

        printf("%-5d %-12s %-12s %-8s %-8s ", i, it.table[i].id, scope, type, kind);

        if (it.table[i].idtype == IT_L)
        {
            if (it.table[i].datatype == IT_STR)
                printf("\"%s\"", it.table[i].value.vstr.str);
            else
                printf("%d", it.table[i].value.vint);
        }
        printf("\n");
    }
}

int _tmain(int argc, _TCHAR* argv[])
{
    // UTF-8 кодировка для консоли Windows 10+
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);
    setlocale(LC_ALL, "ru_RU.UTF-8");

    Log::LOGDATA lg;
    lg.stream = nullptr;

    cout << "=== Компилятор MZV-2025 ===\n\n";

    try
    {
        cout << "[1] Разбор параметров...\n";
        Parm::PARM parm = Parm::getparm(argc, argv);

        // Выводим уровень оптимизации
        const char* optNames[] = { "-O0 (без оптимизаций)", "-O1 (peephole)", "-O2 (peephole + const folding)" };
        cout << "    Уровень оптимизации: " << optNames[parm.optLevel] << endl;

        cout << "[2] Инициализация журнала...\n";
        lg = Log::getlog(parm.log);
        Log::writelog(lg);
        Log::writeparm(lg, parm);

        cout << "[3] Чтение исходного файла...\n";
        Input::INDATA indata = Input::getinput(parm.in);
        Log::writein(lg, indata);

        cout << "[4] Лексический анализ...\n";
        Lexer::LEXRESULT lexResult = Lexer::Analyze(indata);

        cout << "    Лексем: " << lexResult.lextable.size << endl;
        cout << "    Идентификаторов: " << lexResult.idtable.size << endl;

        if (lexResult.errors > 0)
            cout << "    Ошибок: " << lexResult.errors << endl;

        // Вывод таблиц
        PrintLexTable(lexResult.lextable);
        PrintIdTable(lexResult.idtable);

        cout << "\n[5] Синтаксический анализ...\n";

        if (MFST::Analyze(lexResult.lextable, lg))
        {
            cout << "\n    Синтаксический анализ завершен успешно\n";
        }

        cout << "[6] Семантический анализ...\n";
        Semantic::SEMRESULT semResult = Semantic::Analyze(lexResult.lextable,
            lexResult.idtable, lg);

        if (semResult.errors > 0)
        {
            cout << "    Семантических ошибок: " << semResult.errors << endl;
        }
        else
        {
            cout << "    Семантический анализ завершен успешно\n";
        }

        // Свёртка констант (если включена O2)
        if (parm.optLevel >= 2)
        {
            cout << "\n[6.5] Свёртка констант (Constant Folding)...\n";
            int folded = Optimizer::ConstFold::FoldConstants(lexResult.lextable, lexResult.idtable);
            cout << "    Свёрнуто выражений: " << folded << endl;
        }

        cout << "\n[7] Преобразование в польскую нотацию (ПОЛИЗ)...\n";
        PolishNotation::ConvertAllExpressions(lexResult.lextable, lexResult.idtable);

        cout << "\n[8] Генерация кода...\n";
        if (CodeGen::GenerateCode(lexResult.lextable, lexResult.idtable, parm.out))
        {
            cout << "    Генерация кода завершена успешно\n";

            // Оптимизация ассемблерного кода (если включена O1 или O2)
            if (parm.optLevel >= 1)
            {
                cout << "\n[9] Оптимизация ассемблерного кода...\n";

                // Получаем путь к ASM файлу
                char outPath[PARM_MAX_SIZE];
                size_t converted;
                wcstombs_s(&converted, outPath, parm.out, PARM_MAX_SIZE);

                std::string fullPath = outPath;
                size_t lastSlash = fullPath.rfind('\\');
                if (lastSlash == std::string::npos)
                    lastSlash = fullPath.rfind('/');

                std::string dir = "";
                if (lastSlash != std::string::npos)
                    dir = fullPath.substr(0, lastSlash + 1);

                std::string asmPath = dir + "..\\ASM\\output.asm";

                Optimizer::AsmOptimizer optimizer(static_cast<Optimizer::OptLevel>(parm.optLevel));
                if (optimizer.LoadFile(asmPath))
                {
                    int beforeCount = optimizer.GetInstructionCount();
                    int optimized = optimizer.Optimize();
                    int afterCount = optimizer.GetInstructionCount();

                    cout << "    Инструкций до: " << beforeCount << endl;
                    cout << "    Инструкций после: " << afterCount << endl;
                    cout << "    Удалено инструкций: " << optimized << endl;

                    optimizer.SaveFile(asmPath);
                    cout << "    Оптимизированный код сохранен\n";
                }
                else
                {
                    cout << "    Не удалось загрузить ASM файл для оптимизации\n";
                }
            }
        }
        else
        {
            cout << "    Ошибка генерации кода\n";
        }

        // Освобождение памяти
        LT::Delete(lexResult.lextable);
        IT::Delete(lexResult.idtable);
        Input::deleteinput(indata);

        cout << "\n=== Компиляция завершена ===\n";
    }
    catch (Error::ERRORDATA e)
    {
        cout << "\n=== ОШИБКА ===\n";
        cout << "Код: " << e.id << " - " << e.message << "\n";
        if (e.inext.line >= 0)
            cout << "Строка: " << e.inext.line << ", позиция: " << e.inext.col << "\n";
        Log::writeerror(lg, e);
    }

    Log::close(lg);
    system("pause");
    return 0;
}
