// GRB.h - Грамматика Грейбаха для MZV-2025
// Добавлена типизация GRBALPHABET из laba18
#pragma once

#include <cstring>
#include <cstdarg>

#define GRB_MAXSIZE_RULE    64      // Максимальное количество правил
#define GRB_MAXSIZE_CHAIN   32      // Максимальное количество цепочек в правиле
#define GRB_MAXSIZE_SYMBOL  64      // Максимальная длина цепочки

// =================================================================
// GRBALPHABET - типизированный алфавит грамматики (из laba18)
// Терминалы: > 0 (положительные)
// Нетерминалы: < 0 (отрицательные)
// =================================================================
typedef short GRBALPHABET;

// Проверка: символ является нетерминалом
// Старый макрос (для обратной совместимости)
#define ISNS(s) ((s) >= 'A' && (s) <= 'Z')

// Код ошибки по умолчанию
#define GRB_ERROR_SERIES    600

// Нетерминалы грамматики MZV-2025
#define NS_S    'S'     // Стартовый символ (программа)
#define NS_N    'N'     // Тело функции (операторы)
#define NS_E    'E'     // Выражение
#define NS_K    'K'     // Хвост выражения (tail) - для устранения левой рекурсии
#define NS_F    'F'     // Параметры функции
#define NS_W    'W'     // Аргументы вызова функции

// Терминалы грамматики (соответствуют лексемам)
#define TS_T    't'     // Тип (integer, char)
#define TS_I    'i'     // Идентификатор
#define TS_L    'l'     // Литерал
#define TS_F    'f'     // function
#define TS_D    'd'     // declare
#define TS_M    'm'     // main
#define TS_R    'r'     // return
#define TS_O    'o'     // output
#define TS_IF   '?'     // if
#define TS_EL   ':'     // else
#define TS_SC   ';'     // ;
#define TS_CM   ','     // ,
#define TS_LB   '{'     // {
#define TS_RB   '}'     // }
#define TS_LP   '('     // (
#define TS_RP   ')'     // )
#define TS_EQ   '='     // =
#define TS_V    'v'     // Бинарная операция (+, -, *, /, %)
#define TS_U    'u'     // Унарная операция (++, --, ~)
#define TS_C    'c'     // Сравнение (<, >, <=, >=, ==, !=)
#define TS_END  '$'     // Конец ленты / дно стека
#define TS_EPS  '@'     // Эпсилон (пустая цепочка)

namespace GRB
{
    // Цепочка символов правила грамматики
    struct Chain
    {
        short size;                     // Длина цепочки
        char  nt[GRB_MAXSIZE_SYMBOL];   // Символы цепочки

        Chain();
        Chain(const char* s);
        char operator[](int i) const { return nt[i]; }

        // =================================================================
        // Статические методы для работы с GRBALPHABET (из laba18)
        // =================================================================

        // Преобразовать символ терминала в GRBALPHABET (положительный)
        static GRBALPHABET T(char t)
        {
            return static_cast<GRBALPHABET>(t);
        }

        // Преобразовать символ нетерминала в GRBALPHABET (отрицательный)
        static GRBALPHABET N(char n)
        {
            return static_cast<GRBALPHABET>(-n);
        }

        // Проверка: является ли символ терминалом (> 0)
        static bool isT(GRBALPHABET s)
        {
            return s > 0;
        }

        // Проверка: является ли символ нетерминалом (< 0)
        static bool isN(GRBALPHABET s)
        {
            return s < 0;
        }

        // Преобразовать GRBALPHABET обратно в ASCII-символ
        static char alphabet_to_char(GRBALPHABET s)
        {
            return isT(s) ? static_cast<char>(s) : static_cast<char>(-s);
        }

        // Получить цепочку как GRBALPHABET-массив
        void toAlphabet(GRBALPHABET* out, int& outSize) const
        {
            outSize = size;
            for (int i = 0; i < size; i++)
            {
                char c = nt[i];
                // Заглавные буквы A-Z - нетерминалы (отрицательные)
                if (c >= 'A' && c <= 'Z')
                    out[i] = N(c);
                else
                    out[i] = T(c);
            }
        }
    };

    // Правило грамматики A -> α1 | α2 | ... | αn
    struct Rule
    {
        char  nn;                           // Нетерминал левой части
        int   idError;                      // Код ошибки для правила
        short chainCount;                   // Количество альтернативных цепочек
        Chain chains[GRB_MAXSIZE_CHAIN];    // Альтернативные цепочки

        Rule();
        Rule(char n, int err, int count, ...);

        // Поиск цепочки, начинающейся с символа c, начиная с позиции start
        int getNextChain(char c, int start = 0) const;

        // Получить цепочку по номеру
        Chain getChain(int n) const { return chains[n]; }

        // Получить правило в виде строки для вывода
        char* getCRule(char* buf, int nchain) const;

        // =================================================================
        // Дополнительные методы для работы с GRBALPHABET
        // =================================================================

        // Получить нетерминал как GRBALPHABET (отрицательный)
        GRBALPHABET getNonTerminal() const
        {
            return Chain::N(nn);
        }
    };

    // Грамматика Грейбаха
    struct Greibach
    {
        short    ruleCount;                 // Количество правил
        Rule     rules[GRB_MAXSIZE_RULE];   // Правила грамматики
        char     startSymbol;               // Стартовый символ
        char     bottomStack;               // Маркер дна стека

        Greibach();
        Greibach(char start, char bottom, int count, ...);

        // Поиск правила для нетерминала
        int getRule(char nn, Rule& rule) const;

        // Получить правило по номеру
        Rule getRule(int n) const { return rules[n]; }

        // =================================================================
        // Дополнительные методы для GRBALPHABET
        // =================================================================

        // Получить стартовый символ как GRBALPHABET
        GRBALPHABET getStartN() const
        {
            return Chain::N(startSymbol);
        }

        // Получить маркер дна стека как GRBALPHABET
        GRBALPHABET getBottomT() const
        {
            return Chain::T(bottomStack);
        }
    };

    // Получить грамматику MZV-2025
    Greibach getGreibach();
}
