// PolishNotation.h - преобразование выражений в ПОЛИЗ (обратную польскую запись)
// Адаптировано из laba18 для MZV-2025
#pragma once

// Псевдо-лексема для пустых позиций после преобразования
#define LEX_POLIZ_NULL '$'

// Специальный маркер вызова функции с n параметрами
#define LEX_CALL '@'

namespace PolishNotation
{
    // Получить приоритет операции
    // Приоритет 0 - ( и )
    // Приоритет 1 - запятая ,
    // Приоритет 2 - сравнения ==, !=, <, >, <=, >=
    // Приоритет 3 - + и -
    // Приоритет 4 - * и / и %
    // Приоритет 5 - унарные операции ~, ++, --
    int GetPriority(char lexType, char op);

    // Конвертировать одиночное арифметическое выражение в обратную запись
    // lextable_pos - позиция начала выражения в таблице лексем
    // Возвращает true при успехе
    bool Convert(
        int lextable_pos,
        LT::LexTable& lextable,
        IT::IdTable& idtable
    );

    // Конвертировать выражение с явным указанием конца
    // exprStart - начало выражения
    // exprEnd - конец выражения (не включается)
    bool ConvertRange(
        int exprStart,
        int exprEnd,
        LT::LexTable& lextable,
        IT::IdTable& idtable
    );

    // Конвертировать все выражения в программе
    // Обрабатывает:
    // - присваивания (после =)
    // - return (после r)
    // - output (после o)
    // - условия if (после ?)
    void ConvertAllExpressions(
        LT::LexTable& lextable,
        IT::IdTable& idtable
    );

    // Вывести ПОЛИЗ-представление выражения (для отладки)
    void PrintPoliz(
        LT::LexTable& lextable,
        IT::IdTable& idtable,
        int start,
        int end
    );
}
