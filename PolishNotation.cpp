// PolishNotation.cpp - реализация преобразования в ПОЛИЗ для MZV-2025
// Адаптировано из laba18
#include "stdafx.h"
#include <stack>
#include <vector>

namespace PolishNotation
{
    // Получить приоритет операции
    int GetPriority(char lexType, char op)
    {
        // Унарные операции имеют наивысший приоритет
        if (lexType == LEX_UNARY)
        {
            return 5;  // ~, ++, --
        }

        // Для остальных операций смотрим на символ
        switch (op)
        {
        case '(':
        case ')':
            return 0;
        case ',':
            return 1;
        case '<':       // Сравнения: <, >, <=, >=, ==, !=
        case '>':
        case '=':       // == (в op хранится '=')
        case '!':       // != (в op хранится '!')
            return 2;
        case '+':       // Сложение/вычитание (бинарные)
        case '-':
            return 3;
        case '*':       // Умножение/деление/остаток
        case '/':
        case '%':
            return 4;
        default:
            return -1;
        }
    }

    // Конвертировать выражение с явным указанием конца
    bool ConvertRange(int exprStart, int exprEnd, LT::LexTable& lextable, IT::IdTable& idtable)
    {
        if (exprEnd >= lextable.size || exprStart >= exprEnd)
        {
            return false;
        }

        // Создаём буферы для результата и стека
        std::vector<LT::Entry> output;      // Результирующая запись (ПОЛИЗ)
        std::stack<LT::Entry> opStack;      // Стек операций
        std::stack<int> funcParamCount;     // Счётчик параметров функций

        // Обрабатываем выражение по алгоритму сортировочной станции
        for (int i = exprStart; i < exprEnd; i++)
        {
            LT::Entry current = lextable.table[i];
            char lex = current.lexema;

            // Идентификатор или литерал
            if (lex == LEX_ID || lex == LEX_LITERAL)
            {
                // Проверяем, является ли следующий символ открывающей скобкой (вызов функции)
                if (i + 1 < exprEnd && lextable.table[i + 1].lexema == LEX_LEFTHESIS)
                {
                    // Это имя функции - помещаем идентификатор функции в стек
                    opStack.push(current);
                    funcParamCount.push(1);  // Минимум 1 параметр (или 0 если скобки пустые)
                }
                else
                {
                    // Обычный операнд - добавляем в выходную строку
                    output.push_back(current);
                }
            }
            // Открывающая скобка
            else if (lex == LEX_LEFTHESIS)
            {
                opStack.push(current);
            }
            // Закрывающая скобка
            else if (lex == LEX_RIGHTHESIS)
            {
                // Выталкиваем все операции до открывающей скобки
                while (!opStack.empty() && opStack.top().lexema != LEX_LEFTHESIS)
                {
                    LT::Entry topOp = opStack.top();
                    opStack.pop();

                    // Если это идентификатор (функция)
                    if (topOp.lexema == LEX_ID)
                    {
                        output.push_back(topOp);

                        // Создаём маркер вызова функции @n
                        LT::Entry funcMarker = LT::CreateEntry(LEX_CALL, topOp.line, topOp.col);
                        int paramCount = (!funcParamCount.empty()) ? funcParamCount.top() : 0;
                        funcMarker.idxTI = paramCount;
                        output.push_back(funcMarker);

                        if (!funcParamCount.empty()) funcParamCount.pop();
                    }
                    else
                    {
                        output.push_back(topOp);
                    }
                }

                // Удаляем открывающую скобку
                if (!opStack.empty() && opStack.top().lexema == LEX_LEFTHESIS)
                {
                    opStack.pop();
                }

                // Проверяем, есть ли функция в стеке
                if (!opStack.empty() && opStack.top().lexema == LEX_ID)
                {
                    LT::Entry funcEntry = opStack.top();
                    opStack.pop();

                    output.push_back(funcEntry);

                    // Создаём маркер вызова функции @n
                    LT::Entry funcMarker = LT::CreateEntry(LEX_CALL, funcEntry.line, funcEntry.col);
                    int paramCount = (!funcParamCount.empty()) ? funcParamCount.top() : 0;
                    funcMarker.idxTI = paramCount;
                    output.push_back(funcMarker);

                    if (!funcParamCount.empty()) funcParamCount.pop();
                }
            }
            // Запятая (разделитель параметров)
            else if (lex == LEX_COMMA)
            {
                // Увеличиваем счётчик параметров
                if (!funcParamCount.empty())
                {
                    int count = funcParamCount.top();
                    funcParamCount.pop();
                    funcParamCount.push(count + 1);
                }

                // Выталкиваем все операции до открывающей скобки или идентификатора функции
                while (!opStack.empty() &&
                       opStack.top().lexema != LEX_LEFTHESIS &&
                       opStack.top().lexema != LEX_ID)
                {
                    output.push_back(opStack.top());
                    opStack.pop();
                }
            }
            // Бинарная операция (+, -, *, /, %)
            else if (lex == LEX_BINOP)
            {
                int currentPriority = GetPriority(current.lexema, current.op);

                // Выталкиваем операции с большим или равным приоритетом
                while (!opStack.empty())
                {
                    LT::Entry topEntry = opStack.top();
                    char topLex = topEntry.lexema;

                    if (topLex == LEX_LEFTHESIS || topLex == LEX_ID)
                        break;

                    if (topLex == LEX_BINOP || topLex == LEX_UNARY)
                    {
                        int topPriority = GetPriority(topEntry.lexema, topEntry.op);
                        if (topPriority >= currentPriority)
                        {
                            output.push_back(opStack.top());
                            opStack.pop();
                        }
                        else
                        {
                            break;
                        }
                    }
                    else
                    {
                        break;
                    }
                }

                opStack.push(current);
            }
            // Унарная операция (~, ++, --)
            else if (lex == LEX_UNARY)
            {
                // Унарные операции имеют высший приоритет, сразу в стек
                opStack.push(current);
            }
            // Операции сравнения (<, >, <=, >=, ==, !=)
            else if (lex == LEX_COMPARE)
            {
                int currentPriority = GetPriority(current.lexema, current.op);

                // Выталкиваем операции с большим или равным приоритетом
                while (!opStack.empty())
                {
                    LT::Entry topEntry = opStack.top();
                    char topLex = topEntry.lexema;

                    if (topLex == LEX_LEFTHESIS || topLex == LEX_ID)
                        break;

                    if (topLex == LEX_BINOP || topLex == LEX_UNARY || topLex == LEX_COMPARE)
                    {
                        int topPriority = GetPriority(topEntry.lexema, topEntry.op);
                        if (topPriority >= currentPriority)
                        {
                            output.push_back(opStack.top());
                            opStack.pop();
                        }
                        else
                        {
                            break;
                        }
                    }
                    else
                    {
                        break;
                    }
                }

                opStack.push(current);
            }
        }

        // Выталкиваем оставшиеся операции из стека
        while (!opStack.empty())
        {
            LT::Entry topOp = opStack.top();
            opStack.pop();

            if (topOp.lexema != LEX_LEFTHESIS && topOp.lexema != LEX_RIGHTHESIS)
            {
                output.push_back(topOp);
            }
        }

        // Проверка: если выход пустой или равен входу, не меняем
        if (output.empty())
        {
            return false;
        }

        // Записываем ПОЛИЗ обратно в таблицу лексем
        int outputIdx = 0;
        for (int i = exprStart; i < exprEnd && outputIdx < (int)output.size(); i++)
        {
            lextable.table[i] = output[outputIdx++];
        }

        // Заполняем оставшиеся позиции псевдо-лексемами
        for (int i = exprStart + (int)output.size(); i < exprEnd; i++)
        {
            lextable.table[i].lexema = LEX_POLIZ_NULL;
            lextable.table[i].idxTI = LT_TI_NULLIDX;
            lextable.table[i].op = CHAR_NULL;
        }

        return true;
    }

    // Конвертировать выражение в ПОЛИЗ (автоматически находит конец)
    bool Convert(int lextable_pos, LT::LexTable& lextable, IT::IdTable& idtable)
    {
        // Находим конец выражения (точка с запятой или закрывающая фигурная скобка)
        int exprStart = lextable_pos;
        int exprEnd = lextable_pos;

        int parenDepth = 0;
        while (exprEnd < lextable.size)
        {
            char lex = lextable.table[exprEnd].lexema;

            if (lex == LEX_LEFTHESIS) parenDepth++;
            else if (lex == LEX_RIGHTHESIS) parenDepth--;
            else if (lex == LEX_SEMICOLON && parenDepth == 0) break;
            else if (lex == LEX_RIGHTBRACE && parenDepth == 0) break;

            exprEnd++;
        }

        return ConvertRange(exprStart, exprEnd, lextable, idtable);
    }

    // Вывод ПОЛИЗ для отладки
    void PrintPoliz(LT::LexTable& lextable, IT::IdTable& idtable, int start, int end)
    {
        cout << "ПОЛИЗ: ";
        for (int i = start; i < end; i++)
        {
            char lex = lextable.table[i].lexema;

            if (lex == LEX_POLIZ_NULL) continue;

            if (lex == LEX_ID || lex == LEX_LITERAL)
            {
                int idx = lextable.table[i].idxTI;
                if (idx != LT_TI_NULLIDX && idx < idtable.size)
                {
                    cout << idtable.table[idx].id << " ";
                }
                else
                {
                    cout << lex << " ";
                }
            }
            else if (lex == LEX_CALL)
            {
                cout << "@" << lextable.table[i].idxTI << " ";
            }
            else if (lex == LEX_BINOP || lex == LEX_UNARY || lex == LEX_COMPARE)
            {
                cout << lextable.table[i].op << " ";
            }
            else
            {
                cout << lex << " ";
            }
        }
        cout << endl;
    }

    // Конвертировать все выражения в программе
    void ConvertAllExpressions(LT::LexTable& lextable, IT::IdTable& idtable)
    {
        cout << endl;
        cout << "========================================" << endl;
        cout << "Преобразование выражений в ПОЛИЗ" << endl;
        cout << "========================================" << endl;

        int converted = 0;
        int i = 0;

        while (i < lextable.size)
        {
            char lex = lextable.table[i].lexema;

            // Ищем начало выражений:
            // 1. После знака = (присваивание)
            // 2. После return 'r'
            // 3. После output 'o'
            // 4. После if '?' (условие)

            if (lex == LEX_ASSIGN)
            {
                if (i + 1 < lextable.size)
                {
                    int exprStart = i + 1;
                    if (Convert(exprStart, lextable, idtable))
                    {
                        // Находим конец выражения для вывода
                        int exprEnd = exprStart;
                        while (exprEnd < lextable.size && lextable.table[exprEnd].lexema != LEX_SEMICOLON)
                            exprEnd++;

                        cout << "  Присваивание (строка " << lextable.table[i].line << "): ";
                        PrintPoliz(lextable, idtable, exprStart, exprEnd);
                        converted++;
                    }
                }
                // Пропускаем до точки с запятой
                while (i < lextable.size && lextable.table[i].lexema != LEX_SEMICOLON)
                    i++;
            }
            else if (lex == LEX_RETURN || lex == LEX_OUTPUT)
            {
                if (i + 1 < lextable.size)
                {
                    int exprStart = i + 1;
                    if (Convert(exprStart, lextable, idtable))
                    {
                        int exprEnd = exprStart;
                        while (exprEnd < lextable.size && lextable.table[exprEnd].lexema != LEX_SEMICOLON)
                            exprEnd++;

                        const char* type = (lex == LEX_RETURN) ? "Return" : "Output";
                        cout << "  " << type << " (строка " << lextable.table[i].line << "): ";
                        PrintPoliz(lextable, idtable, exprStart, exprEnd);
                        converted++;
                    }
                }
                while (i < lextable.size && lextable.table[i].lexema != LEX_SEMICOLON)
                    i++;
            }
            else if (lex == LEX_IF)
            {
                // Обрабатываем условие if (выражение между '?' и '(')
                if (i + 1 < lextable.size && lextable.table[i + 1].lexema == LEX_LEFTHESIS)
                {
                    // Находим выражение внутри скобок if (expr)
                    int exprStart = i + 2;  // После '?('
                    int exprEnd = exprStart;
                    int parenDepth = 1;

                    // Ищем закрывающую скобку
                    while (exprEnd < lextable.size && parenDepth > 0)
                    {
                        if (lextable.table[exprEnd].lexema == LEX_LEFTHESIS)
                            parenDepth++;
                        else if (lextable.table[exprEnd].lexema == LEX_RIGHTHESIS)
                            parenDepth--;
                        if (parenDepth > 0)
                            exprEnd++;
                    }

                    if (exprStart < exprEnd && ConvertRange(exprStart, exprEnd, lextable, idtable))
                    {
                        cout << "  Условие if (строка " << lextable.table[i].line << "): ";
                        PrintPoliz(lextable, idtable, exprStart, exprEnd);
                        converted++;
                    }

                    // Пропускаем до конца условия
                    i = exprEnd;
                }
            }

            i++;
        }

        cout << "========================================" << endl;
        cout << "Преобразовано выражений: " << converted << endl;
        cout << "========================================" << endl;
    }
}
