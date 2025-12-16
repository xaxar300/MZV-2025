// MFST.h - Магазинный автомат для синтаксического анализа MZV-2025
// Добавлен прямой доступ к стеку из laba18
#pragma once

#include <stack>
#include <deque>
#include <cstdio>
#include "LT.h"
#include "GRB.h"
#include "Log.h"

// Максимальный размер диагностики
#define MFST_DIAGN_NUMBER   10

// =================================================================
// Хак для прямого доступа к контейнеру std::stack (из laba18)
// Позволяет получить доступ к внутреннему deque без копирования
// =================================================================
template<typename T, typename Container = std::deque<T>>
class StackAccess : public std::stack<T, Container>
{
public:
    // Прямой доступ к внутреннему контейнеру (только для чтения)
    const Container& getContainer() const
    {
        return this->c;
    }

    // Доступ по индексу (0 = дно стека)
    const T& at(size_t index) const
    {
        return this->c[index];
    }

    // Размер стека
    size_t getSize() const
    {
        return this->c.size();
    }
};

namespace MFST
{
    // Коды возврата шага автомата
    enum RC_STEP
    {
        NS_OK,              // Нетерминал успешно заменен
        NS_NORULE,          // Не найдено правило для нетерминала
        NS_NORULECHAIN,     // Не найдена подходящая цепочка
        NS_ERROR,           // Ошибка
        TS_OK,              // Терминал успешно совпал
        TS_NOK,             // Терминал не совпал
        LENTA_END,          // Достигнут конец ленты
        SURPRISE            // Неожиданная ситуация
    };

    // Структура для сохранения состояния автомата (для откатов)
    struct MfstState
    {
        short lenta_position;               // Позиция на ленте
        StackAccess<char> st;               // Копия магазина
        short nrule;                        // Номер правила
        short nrulechain;                   // Номер цепочки в правиле

        MfstState();
        MfstState(short pos, StackAccess<char> s, short rule, short chain);
    };

    // Структура диагностики ошибок
    struct MfstDiagnosis
    {
        short lenta_position;               // Позиция ошибки на ленте
        short nrule;                        // Номер правила
        short nrulechain;                   // Номер цепочки

        MfstDiagnosis();
        MfstDiagnosis(short pos, short rule, short chain);
    };

    // Структура для вывода дерева разбора
    struct Deduction
    {
        short size;                         // Количество шагов в выводе
        short* nrules;                      // Номера правил
        short* nrulechains;                 // Номера цепочек

        Deduction();
        ~Deduction();
    };

    // Магазинный автомат
    struct Mfst
    {
        // Входные данные
        LT::LexTable* lextable;           // Таблица лексем
        GRB::Greibach   greibach;           // Грамматика (копия)

        // Лента (последовательность лексем)
        char* lenta;                      // Лента терминалов
        short   lenta_size;                 // Размер ленты
        short   lenta_position;             // Текущая позиция на ленте

        // Магазин (стек) - используем StackAccess для прямого доступа
        StackAccess<char> st;               // Стек символов с прямым доступом

        // Состояние автомата
        short   nrule;                      // Текущий номер правила
        short   nrulechain;                 // Текущий номер цепочки

        // Стек состояний для откатов
        std::stack<MfstState> storestate;

        // Диагностика
        MfstDiagnosis diagnosis[MFST_DIAGN_NUMBER];
        short diagnosis_count;

        // Дерево вывода
        Deduction deduction;

        // Трассировка
        FILE* traceFile;
        int stepCounter;                    // Счётчик шагов для вывода
        bool traceEnabled;                  // Флаг включения трассировки (по умолчанию true)

        // Конструкторы и деструктор
        Mfst();
        Mfst(LT::LexTable& lt, GRB::Greibach gr);
        ~Mfst();

        // Основные методы
        bool start();                       // Запустить разбор
        RC_STEP step();                     // Выполнить один шаг

        // Вспомогательные методы
        char* getCSt(char* buf);            // Получить содержимое стека
        char* getCLenta(char* buf, short pos, short n = 25);  // Получить часть ленты
        char* getDiagnosis(short n, char* buf);  // Получить строку диагностики

        // Управление состоянием
        bool savestate();                   // Сохранить состояние
        bool reststate();                   // Восстановить состояние
        bool push_chain(GRB::Chain chain);  // Поместить цепочку в стек

        // Диагностика
        bool savediagnosis(RC_STEP rc);

        // Дерево вывода
        bool savededucation();

        // Вывод
        void printStack();
        void printStep();
        void printrules();

        // Управление трассировкой
        void setTraceEnabled(bool enabled) { traceEnabled = enabled; }
        bool isTraceEnabled() const { return traceEnabled; }
    };

    // Выполнить синтаксический анализ
    // traceEnabled - флаг включения детального вывода (по умолчанию true)
    bool Analyze(LT::LexTable& lextable, Log::LOGDATA& log, bool traceEnabled = true);
}