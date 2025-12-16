// Lexer.h - лексический анализатор MZV-2025
#pragma once

namespace Lexer
{
    // Результат лексического анализа
    struct LEXRESULT
    {
        LT::LexTable lextable = {};
        IT::IdTable idtable = {};
        int errors = 0;
    };

    // Главная функция анализа
    LEXRESULT Analyze(Input::INDATA input);

    // Вспомогательные функции классификации
    bool IsLetter(char c);
    bool IsDigit(char c);
    bool IsBinaryDigit(char c);
    bool IsSpace(char c);
    bool IsOperator(char c);
    bool IsSeparator(char c);
};
