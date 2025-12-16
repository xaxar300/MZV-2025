// Optimizer.h - оптимизатор сгенерированного кода MZV-2025
// Реализует:
// - Peephole оптимизацию (удаление push/pop пар)
// - Свёртку констант (constant folding)
#pragma once

#include <string>
#include <vector>
#include <fstream>

namespace Optimizer
{
    // Уровни оптимизации
    enum OptLevel
    {
        O0 = 0,     // Без оптимизаций
        O1 = 1,     // Peephole оптимизации
        O2 = 2      // Peephole + свёртка констант
    };

    // Структура инструкции ассемблера
    struct AsmInstruction
    {
        std::string original;       // Исходная строка
        std::string opcode;         // Мнемоника команды
        std::string operand1;       // Первый операнд
        std::string operand2;       // Второй операнд
        bool isComment;             // Это комментарий
        bool isLabel;               // Это метка
        bool removed;               // Помечена для удаления

        AsmInstruction() : isComment(false), isLabel(false), removed(false) {}
    };

    // Класс оптимизатора
    class AsmOptimizer
    {
    private:
        std::vector<AsmInstruction> instructions;
        OptLevel level;

        // Парсинг инструкций
        void ParseInstruction(const std::string& line, AsmInstruction& instr);
        std::string Trim(const std::string& s);

        // Оптимизации
        int PeepholeOptimize();
        int ConstantFolding();

        // Peephole паттерны
        bool PatternPushPop(int i);
        bool PatternMovMov(int i);
        bool PatternPushMovPop(int i);

    public:
        AsmOptimizer(OptLevel optLevel = O1) : level(optLevel) {}

        // Загрузить ассемблерный файл
        bool LoadFile(const std::string& path);

        // Сохранить оптимизированный файл
        bool SaveFile(const std::string& path);

        // Выполнить оптимизации
        int Optimize();

        // Установить уровень оптимизации
        void SetLevel(OptLevel l) { level = l; }
        OptLevel GetLevel() const { return level; }

        // Получить статистику
        int GetInstructionCount() const;
        int GetRemovedCount() const;
    };

    // Оптимизатор для таблицы лексем (свёртка констант на этапе компиляции)
    namespace ConstFold
    {
        // Выполнить свёртку констант в таблице лексем
        // Возвращает количество свёрнутых выражений
        int FoldConstants(LT::LexTable& lextable, IT::IdTable& idtable);

        // Проверить, можно ли свернуть выражение
        bool CanFold(LT::LexTable& lextable, int start, int end);

        // Вычислить константное выражение
        int EvaluateConstExpr(LT::LexTable& lextable, IT::IdTable& idtable, int start, int end);
    }
}
