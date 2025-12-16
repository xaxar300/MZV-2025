// LT.cpp - реализация таблицы лексем MZV-2025
#include "stdafx.h"

namespace LT
{
    LexTable Create(int size)
    {
        if (size < 1 || size > LT_MAXSIZE)
            throw ERROR_THROW(113);

        LexTable lt;
        lt.maxsize = size;
        lt.size = 0;
        lt.table = new Entry[size];

        memset(lt.table, 0, sizeof(Entry) * size);
        for (int i = 0; i < size; i++)
        {
            lt.table[i].line = -1;
            lt.table[i].col = 0;
            lt.table[i].idxTI = LT_TI_NULLIDX;
        }
        return lt;
    }

    void Add(LexTable& lt, Entry e)
    {
        if (lt.size >= lt.maxsize)
            throw ERROR_THROW(114);
        lt.table[lt.size++] = e;
    }

    Entry GetEntry(LexTable& lt, int idx)
    {
        if (idx < 0 || idx >= lt.size)
            throw ERROR_THROW(115);
        return lt.table[idx];
    }

    void Delete(LexTable& lt)
    {
        delete[] lt.table;
        lt.table = nullptr;
        lt.size = lt.maxsize = 0;
    }

    Entry CreateEntry(char lex, int line, int col, int idxTI, char op, int pr)
    {
        return { lex, line, col, idxTI, op, pr };
    }
};