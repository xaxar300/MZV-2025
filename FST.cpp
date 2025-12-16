// FST.cpp - реализация конечных автоматов MZV-2025
#include "stdafx.h"

namespace FST
{
    // =====================================================
    // Фабричные функции для создания автоматов
    // =====================================================

    RELATION CreateRelation(char c, int node)
    {
        return RELATION(c, node);
    }

    NODE CreateNode(int count, ...)
    {
        NODE n;
        n.relCount = count;
        n.relations = (count > 0) ? new RELATION[count] : nullptr;

        va_list args;
        va_start(args, count);
        for (int i = 0; i < count; i++)
            n.relations[i] = va_arg(args, RELATION);
        va_end(args);

        return n;
    }

    // =====================================================
    // Выполнение автомата
    // =====================================================

    bool Execute(FST& fst)
    {
        int currentState = 0;
        fst.position = 0;

        while (fst.string[fst.position] != CHAR_NULL)
        {
            NODE& node = fst.nodes[currentState];
            char c = fst.string[fst.position];
            bool found = false;

            for (int i = 0; i < node.relCount; i++)
            {
                if (node.relations[i].symbol == c)
                {
                    currentState = node.relations[i].nextNode;
                    found = true;
                    break;
                }
            }

            if (!found) return false;
            fst.position++;
        }

        return (currentState == fst.nodeCount - 1);
    }

    // === Ключевые слова - оптимизировано через strcmp ===
    bool IsInteger(const char* str) { return strcmp(str, "integer") == 0; }
    bool IsChar(const char* str) { return strcmp(str, "char") == 0; }
    bool IsFunction(const char* str) { return strcmp(str, "function") == 0; }
    bool IsDeclare(const char* str) { return strcmp(str, "declare") == 0; }
    bool IsMain(const char* str) { return strcmp(str, "main") == 0; }
    bool IsReturn(const char* str) { return strcmp(str, "return") == 0; }
    bool IsOutput(const char* str) { return strcmp(str, "output") == 0; }
    bool IsIf(const char* str) { return strcmp(str, "if") == 0; }
    bool IsElse(const char* str) { return strcmp(str, "else") == 0; }

    // === Целочисленный литерал (десятичный): [0-9]+ ===
    bool IsIntegerLiteral(const char* str)
    {
        if (str[0] == CHAR_NULL) return false;
        for (int i = 0; str[i] != CHAR_NULL; i++)
            if (str[i] < '0' || str[i] > '9') return false;
        return true;
    }

    // === Двоичный литерал: 0b[01]+ ===
    bool IsBinaryLiteral(const char* str)
    {
        if (str[0] != '0' || (str[1] != 'b' && str[1] != 'B')) return false;
        if (str[2] == CHAR_NULL) return false;
        for (int i = 2; str[i] != CHAR_NULL; i++)
            if (str[i] != '0' && str[i] != '1') return false;
        return true;
    }

    // === Строковый литерал: '...' или "..." ===
    bool IsStringLiteral(const char* str)
    {
        int len = (int)strlen(str);
        if (len < 2) return false;
        return (str[0] == CHAR_QUOTE && str[len - 1] == CHAR_QUOTE) ||
            (str[0] == CHAR_DQUOTE && str[len - 1] == CHAR_DQUOTE);
    }

    // === Идентификатор: [a-zA-Z_][a-zA-Z0-9_]* ===
    bool IsIdentifier(const char* str)
    {
        if (str[0] == CHAR_NULL) return false;

        char c = str[0];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'))
            return false;

        for (int i = 1; str[i] != CHAR_NULL; i++)
        {
            c = str[i];
            if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') || c == '_'))
                return false;
        }
        return true;
    }

    // === Унарные операции ===
    bool IsIncrement(const char* str) { return str[0] == '+' && str[1] == '+' && str[2] == CHAR_NULL; }
    bool IsDecrement(const char* str) { return str[0] == '-' && str[1] == '-' && str[2] == CHAR_NULL; }
    bool IsBitNot(const char* str) { return str[0] == '~' && str[1] == CHAR_NULL; }
};
