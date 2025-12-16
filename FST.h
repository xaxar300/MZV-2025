// FST.h - Конечные автоматы для лексического анализа MZV-2025
#pragma once

namespace FST
{
    // Ребро графа (переход)
    struct RELATION
    {
        char symbol;        // Символ перехода
        int nextNode;       // Номер следующего узла

        // Конструктор по умолчанию
        RELATION() : symbol(0), nextNode(0) {}

        // Конструктор с параметрами
        RELATION(char c, int node) : symbol(c), nextNode(node) {}
    };

    // Узел графа (состояние)
    struct NODE
    {
        int relCount;           // Количество переходов
        RELATION* relations;    // Массив переходов

        // Конструктор по умолчанию (финальное состояние)
        NODE() : relCount(0), relations(nullptr) {}
    };

    // Конечный автомат
    struct FST
    {
        const char* string;     // Входная строка
        int position;           // Текущая позиция
        int nodeCount;          // Количество состояний
        NODE* nodes;            // Массив состояний

        // Конструктор по умолчанию
        FST() : string(nullptr), position(0), nodeCount(0), nodes(nullptr) {}
    };

    // === Фабричные функции для создания автоматов ===

    // Создание перехода
    RELATION CreateRelation(char c, int node);

    // Создание узла (variadic)
    NODE CreateNode(int count, ...);

    // Выполнение автомата
    bool Execute(FST& fst);

    // === Проверки для ключевых слов ===
    bool IsInteger(const char* str);
    bool IsChar(const char* str);
    bool IsFunction(const char* str);
    bool IsDeclare(const char* str);
    bool IsMain(const char* str);
    bool IsReturn(const char* str);
    bool IsOutput(const char* str);
    bool IsIf(const char* str);
    bool IsElse(const char* str);

    // === Проверки для литералов ===
    bool IsIntegerLiteral(const char* str);
    bool IsBinaryLiteral(const char* str);
    bool IsStringLiteral(const char* str);

    // === Проверка для идентификаторов ===
    bool IsIdentifier(const char* str);

    // === Проверки для унарных операций ===
    bool IsIncrement(const char* str);
    bool IsDecrement(const char* str);
    bool IsBitNot(const char* str);
};
