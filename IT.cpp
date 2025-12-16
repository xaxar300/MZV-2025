// IT.cpp - ���������� ������� ��������������� MZV-2025
#include "stdafx.h"

#define DEFAULT_INT 0
#define NOT_FOUND -1

namespace IT
{
    IdTable Create(int size)
    {
        if (size < 1 || size > IT_MAXSIZE)
            throw ERROR_THROW(116);

        IdTable it;
        it.maxsize = size;
        it.size = 0;
        it.table = new Entry[size];

        // ������������� ����� memset + ����� ������� �������� �� ���������
        memset(it.table, 0, sizeof(Entry) * size);
        for (int i = 0; i < size; i++)
        {
            it.table[i].datatype = IT_INT;
            it.table[i].idtype = IT_V;
            it.table[i].firstLine = NOT_FOUND;
        }
        return it;
    }

    void Add(IdTable& it, Entry e)
    {
        if (it.size >= it.maxsize)
            throw ERROR_THROW(116);

        int idx = it.size;
        it.table[it.size++] = e;

        // Добавляем в хэш-индекс для быстрого поиска O(1)
        it.nameIndex[std::string(e.id)].push_back(idx);
    }

    Entry GetEntry(IdTable& it, int idx)
    {
        if (idx < 0 || idx >= it.size)
            throw ERROR_THROW(116);
        return it.table[idx];
    }

    // Поиск идентификатора - оптимизированный O(1) через хэш-таблицу
    int IsId(IdTable& it, const char* id, const char* scope)
    {
        // Ищем в хэш-индексе
        auto found = it.nameIndex.find(std::string(id));
        if (found == it.nameIndex.end())
            return NOT_FOUND;

        int globalIdx = NOT_FOUND;
        const std::vector<int>& indices = found->second;

        // Перебираем только записи с нужным именем (обычно 1-3 записи)
        for (int i : indices)
        {
            // Точное совпадение по scope - возвращаем сразу
            if (strcmp(it.table[i].scope, scope) == 0)
                return i;
            // Запоминаем глобальную переменную, если найдём
            if (it.table[i].scope[0] == CHAR_NULL)
                globalIdx = i;
        }
        return globalIdx;
    }

    void Delete(IdTable& it)
    {
        delete[] it.table;
        it.table = nullptr;
        it.size = it.maxsize = 0;
        it.nameIndex.clear();
    }

    Entry CreateEntry(const char* id, const char* scope, IDDATATYPE dtype, IDTYPE itype, int line)
    {
        Entry e;
        memset(&e, 0, sizeof(Entry));

        strcpy_s(e.id, id);
        if (scope) strcpy_s(e.scope, scope);

        e.datatype = dtype;
        e.idtype = itype;
        e.firstLine = line;

        return e;
    }
};