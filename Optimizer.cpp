// Optimizer.cpp - реализация оптимизатора для MZV-2025
#include "stdafx.h"
#include "Optimizer.h"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <stack>

namespace Optimizer
{
    // =========================================================
    // Вспомогательные функции
    // =========================================================

    std::string AsmOptimizer::Trim(const std::string& s)
    {
        size_t start = s.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        size_t end = s.find_last_not_of(" \t\r\n");
        return s.substr(start, end - start + 1);
    }

    void AsmOptimizer::ParseInstruction(const std::string& line, AsmInstruction& instr)
    {
        instr.original = line;
        instr.removed = false;

        std::string trimmed = Trim(line);

        // Пустая строка
        if (trimmed.empty())
        {
            instr.isComment = true;
            return;
        }

        // Комментарий
        if (trimmed[0] == ';')
        {
            instr.isComment = true;
            return;
        }

        // Метка (заканчивается на :)
        size_t colonPos = trimmed.find(':');
        if (colonPos != std::string::npos && colonPos == trimmed.length() - 1)
        {
            instr.isLabel = true;
            instr.opcode = trimmed.substr(0, colonPos);
            return;
        }

        // Метка с инструкцией после неё
        if (colonPos != std::string::npos)
        {
            // Проверяем, не является ли это DWORD PTR [ebp-8] и т.д.
            size_t ptrPos = trimmed.find("PTR");
            if (ptrPos == std::string::npos || ptrPos > colonPos)
            {
                instr.isLabel = true;
                instr.opcode = trimmed.substr(0, colonPos);
                return;
            }
        }

        // Обычная инструкция
        // Удаляем комментарий в конце строки
        size_t commentPos = trimmed.find(';');
        if (commentPos != std::string::npos)
        {
            trimmed = Trim(trimmed.substr(0, commentPos));
        }

        // Парсим opcode и операнды
        std::istringstream iss(trimmed);
        iss >> instr.opcode;

        // Преобразуем opcode в нижний регистр
        std::transform(instr.opcode.begin(), instr.opcode.end(), instr.opcode.begin(), ::tolower);

        // Читаем остаток строки как операнды
        std::string rest;
        std::getline(iss, rest);
        rest = Trim(rest);

        if (!rest.empty())
        {
            // Разделяем по запятой
            size_t commaPos = rest.find(',');
            if (commaPos != std::string::npos)
            {
                instr.operand1 = Trim(rest.substr(0, commaPos));
                instr.operand2 = Trim(rest.substr(commaPos + 1));
            }
            else
            {
                instr.operand1 = rest;
            }
        }
    }

    // =========================================================
    // Загрузка и сохранение
    // =========================================================

    bool AsmOptimizer::LoadFile(const std::string& path)
    {
        instructions.clear();

        std::ifstream in(path);
        if (!in.is_open())
            return false;

        std::string line;
        while (std::getline(in, line))
        {
            AsmInstruction instr;
            ParseInstruction(line, instr);
            instructions.push_back(instr);
        }

        in.close();
        return true;
    }

    bool AsmOptimizer::SaveFile(const std::string& path)
    {
        std::ofstream out(path);
        if (!out.is_open())
            return false;

        for (const auto& instr : instructions)
        {
            if (!instr.removed)
            {
                out << instr.original << "\n";
            }
        }

        out.close();
        return true;
    }

    // =========================================================
    // Статистика
    // =========================================================

    int AsmOptimizer::GetInstructionCount() const
    {
        int count = 0;
        for (const auto& instr : instructions)
        {
            if (!instr.isComment && !instr.isLabel && !instr.removed)
                count++;
        }
        return count;
    }

    int AsmOptimizer::GetRemovedCount() const
    {
        int count = 0;
        for (const auto& instr : instructions)
        {
            if (instr.removed)
                count++;
        }
        return count;
    }

    // =========================================================
    // Peephole паттерны
    // =========================================================

    // Паттерн: push reg / pop reg (где reg одинаковый) -> удалить оба
    bool AsmOptimizer::PatternPushPop(int i)
    {
        if (i + 1 >= (int)instructions.size())
            return false;

        AsmInstruction& i1 = instructions[i];
        AsmInstruction& i2 = instructions[i + 1];

        if (i1.removed || i2.removed)
            return false;

        if (i1.opcode == "push" && i2.opcode == "pop")
        {
            // push eax / pop eax -> удалить оба
            if (i1.operand1 == i2.operand1)
            {
                i1.removed = true;
                i2.removed = true;
                return true;
            }

            // push eax / pop ebx -> mov ebx, eax
            // Модифицируем первую инструкцию, удаляем вторую
            // Но это более сложная оптимизация, пока не реализуем
        }

        return false;
    }

    // Паттерн: mov reg, X / mov reg, Y -> удалить первую mov
    bool AsmOptimizer::PatternMovMov(int i)
    {
        if (i + 1 >= (int)instructions.size())
            return false;

        AsmInstruction& i1 = instructions[i];
        AsmInstruction& i2 = instructions[i + 1];

        if (i1.removed || i2.removed)
            return false;

        if (i1.opcode == "mov" && i2.opcode == "mov")
        {
            // Если обе mov пишут в один регистр, первая бесполезна
            if (i1.operand1 == i2.operand1)
            {
                // Но только если вторая не читает из первой
                if (i2.operand2.find(i1.operand1) == std::string::npos)
                {
                    i1.removed = true;
                    return true;
                }
            }
        }

        return false;
    }

    // Паттерн: push reg / mov reg, X / pop reg -> push X
    bool AsmOptimizer::PatternPushMovPop(int i)
    {
        if (i + 2 >= (int)instructions.size())
            return false;

        AsmInstruction& i1 = instructions[i];
        AsmInstruction& i2 = instructions[i + 1];
        AsmInstruction& i3 = instructions[i + 2];

        if (i1.removed || i2.removed || i3.removed)
            return false;

        if (i1.opcode == "push" && i2.opcode == "mov" && i3.opcode == "pop")
        {
            // push eax / mov eax, 5 / pop eax -> push 5
            if (i1.operand1 == i2.operand1 && i2.operand1 == i3.operand1)
            {
                // Заменяем первую на push X
                i1.original = "    push " + i2.operand2;
                i1.operand1 = i2.operand2;
                i2.removed = true;
                i3.removed = true;
                return true;
            }
        }

        return false;
    }

    // =========================================================
    // Основные оптимизации
    // =========================================================

    int AsmOptimizer::PeepholeOptimize()
    {
        int totalOptimized = 0;
        bool changed;

        // Повторяем пока есть изменения
        do
        {
            changed = false;

            for (int i = 0; i < (int)instructions.size(); i++)
            {
                if (instructions[i].isComment || instructions[i].isLabel || instructions[i].removed)
                    continue;

                // Пробуем все паттерны
                if (PatternPushPop(i))
                {
                    totalOptimized += 2;
                    changed = true;
                    continue;
                }

                if (PatternMovMov(i))
                {
                    totalOptimized++;
                    changed = true;
                    continue;
                }

                if (PatternPushMovPop(i))
                {
                    totalOptimized += 2;
                    changed = true;
                    continue;
                }
            }
        } while (changed);

        return totalOptimized;
    }

    int AsmOptimizer::ConstantFolding()
    {
        // Свёртка констант на уровне ассемблера ограничена
        // Основная свёртка делается на уровне ПОЛИЗ
        return 0;
    }

    int AsmOptimizer::Optimize()
    {
        if (level == O0)
            return 0;

        int optimized = 0;

        if (level >= O1)
        {
            optimized += PeepholeOptimize();
        }

        if (level >= O2)
        {
            optimized += ConstantFolding();
        }

        return optimized;
    }

    // =========================================================
    // Свёртка констант на уровне таблицы лексем
    // =========================================================

    namespace ConstFold
    {
        bool IsLiteralInt(LT::LexTable& lt, IT::IdTable& it, int pos)
        {
            if (pos >= lt.size) return false;
            if (lt.table[pos].lexema != LEX_LITERAL) return false;

            int idx = lt.table[pos].idxTI;
            if (idx == LT_TI_NULLIDX || idx >= it.size) return false;

            return it.table[idx].datatype == IT_INT && it.table[idx].idtype == IT_L;
        }

        int GetLiteralValue(LT::LexTable& lt, IT::IdTable& it, int pos)
        {
            int idx = lt.table[pos].idxTI;
            return it.table[idx].value.vint;
        }

        bool CanFold(LT::LexTable& lextable, int start, int end)
        {
            // Проверяем, содержит ли выражение только литералы и операторы
            for (int i = start; i < end; i++)
            {
                char lex = lextable.table[i].lexema;
                if (lex == LEX_POLIZ_NULL) continue;

                // Разрешены: литералы, бинарные операции, унарные операции
                if (lex != LEX_LITERAL && lex != LEX_BINOP && lex != LEX_UNARY)
                    return false;
            }
            return true;
        }

        int EvaluateConstExpr(LT::LexTable& lextable, IT::IdTable& idtable, int start, int end)
        {
            std::stack<int> evalStack;

            for (int i = start; i < end; i++)
            {
                char lex = lextable.table[i].lexema;
                if (lex == LEX_POLIZ_NULL) continue;

                if (lex == LEX_LITERAL)
                {
                    if (IsLiteralInt(lextable, idtable, i))
                    {
                        evalStack.push(GetLiteralValue(lextable, idtable, i));
                    }
                }
                else if (lex == LEX_BINOP)
                {
                    if (evalStack.size() < 2) return 0;

                    int b = evalStack.top(); evalStack.pop();
                    int a = evalStack.top(); evalStack.pop();

                    char op = lextable.table[i].op;
                    int result = 0;

                    switch (op)
                    {
                    case '+': result = a + b; break;
                    case '-': result = a - b; break;
                    case '*': result = a * b; break;
                    case '/': result = (b != 0) ? a / b : 0; break;
                    case '%': result = (b != 0) ? a % b : 0; break;
                    default: result = 0;
                    }

                    evalStack.push(result);
                }
                else if (lex == LEX_UNARY)
                {
                    if (evalStack.empty()) return 0;

                    int a = evalStack.top(); evalStack.pop();
                    char op = lextable.table[i].op;
                    int result = 0;

                    switch (op)
                    {
                    case '+': result = a + 1; break;  // ++
                    case '-': result = a - 1; break;  // --
                    case '~': result = ~a; break;     // битовое отрицание
                    default: result = a;
                    }

                    evalStack.push(result);
                }
            }

            return evalStack.empty() ? 0 : evalStack.top();
        }

        int FoldConstants(LT::LexTable& lextable, IT::IdTable& idtable)
        {
            int folded = 0;

            // Ищем выражения после = (присваивание)
            for (int i = 0; i < lextable.size; i++)
            {
                if (lextable.table[i].lexema == LEX_ASSIGN)
                {
                    int exprStart = i + 1;
                    int exprEnd = exprStart;

                    // Находим конец выражения
                    while (exprEnd < lextable.size && lextable.table[exprEnd].lexema != LEX_SEMICOLON)
                        exprEnd++;

                    // Проверяем, можно ли свернуть
                    if (CanFold(lextable, exprStart, exprEnd))
                    {
                        // Вычисляем значение
                        int value = EvaluateConstExpr(lextable, idtable, exprStart, exprEnd);

                        // Создаём новый литерал
                        char litName[32];
                        static int foldedLitCount = 0;
                        sprintf_s(litName, "FOLD%d", foldedLitCount++);

                        IT::Entry entry = IT::CreateEntry(litName, "", IT_INT, IT_L, lextable.table[exprStart].line);
                        entry.value.vint = value;
                        IT::Add(idtable, entry);
                        int litIdx = idtable.size - 1;

                        // Заменяем выражение на один литерал
                        lextable.table[exprStart].lexema = LEX_LITERAL;
                        lextable.table[exprStart].idxTI = litIdx;
                        lextable.table[exprStart].op = CHAR_NULL;

                        // Остальные помечаем как NULL
                        for (int j = exprStart + 1; j < exprEnd; j++)
                        {
                            lextable.table[j].lexema = LEX_POLIZ_NULL;
                            lextable.table[j].idxTI = LT_TI_NULLIDX;
                        }

                        folded++;
                        cout << "    [Optimizer] Folded constant expression at line "
                             << lextable.table[exprStart].line << " -> " << value << endl;
                    }
                }
            }

            return folded;
        }
    }
}
