// CodeGenerator.h - генератор кода x64 для MZV-2025
// Генерирует ассемблерный код MASM (ml64)
#pragma once

#include <string>
#include <vector>
#include <map>
#include <fstream>

namespace CodeGen
{
    // =========================================================
    // Информация о переменной в стеке
    // =========================================================
    struct StackVar
    {
        std::string name;       // Имя переменной
        int offset;             // Смещение от RBP (отрицательное)
        IDDATATYPE datatype;    // Тип данных
        IDTYPE idtype;          // Тип идентификатора (var, param, lit)
    };

    // =========================================================
    // Информация о функции
    // =========================================================
    struct FunctionInfo
    {
        std::string name;               // Имя функции
        int paramCount;                 // Количество параметров
        int localVarCount;              // Количество локальных переменных
        int stackSize;                  // Размер стекового кадра
        std::vector<StackVar> vars;     // Переменные в стеке
    };

    // =========================================================
    // Генератор кода
    // =========================================================
    class Generator
    {
    private:
        LT::LexTable& lextable;
        IT::IdTable& idtable;
        std::ofstream out;

        std::string outputPath;
        int labelCounter;
        std::string currentFunction;

        // Карта функций: имя -> информация
        std::map<std::string, FunctionInfo> functions;

        // Строковые литералы для секции .data
        std::vector<std::pair<std::string, std::string>> stringLiterals;

        // Временный стек для вычислений
        int tempStackOffset;

    public:
        Generator(LT::LexTable& lt, IT::IdTable& it, const std::string& outPath);
        ~Generator();

        // Главная функция генерации
        bool Generate();

    private:
        // =====================================================
        // Подготовка
        // =====================================================
        void CollectFunctions();
        void CollectStringLiterals();
        void CalculateStackLayout(FunctionInfo& func);

        // =====================================================
        // Генерация секций
        // =====================================================
        void GenerateHeader();
        void GenerateDataSection();
        void GenerateCodeSection();
        void GenerateFooter();

        // =====================================================
        // Генерация функций
        // =====================================================
        void GenerateFunction(int startIdx, const std::string& funcName);
        void GeneratePrologue(const FunctionInfo& func);
        void GenerateEpilogue(const FunctionInfo& func);
        void GenerateMainFunction(int startIdx);

        // =====================================================
        // Генерация операторов
        // =====================================================
        void GenerateStatement(int& idx);
        void GenerateAssignment(int& idx);
        void GenerateReturn(int& idx);
        void GenerateOutput(int& idx);
        void GenerateIf(int& idx);
        void GenerateFunctionCall(int& idx, int funcIdxTI);

        // =====================================================
        // Генерация выражений (ПОЛИЗ)
        // =====================================================
        void GeneratePolizExpression(int startIdx, int endIdx);
        void GenerateBinaryOp(char op);
        void GenerateUnaryOp(char op);
        void GenerateCompareOp(char op);

        // =====================================================
        // Работа с переменными
        // =====================================================
        std::string GetVarAddress(int idxTI);
        std::string GetVarAddressInFunc(int idxTI, const FunctionInfo& func);
        void LoadToRAX(int idxTI);
        void StoreFromRAX(int idxTI);

        // =====================================================
        // Вспомогательные функции
        // =====================================================
        std::string NewLabel(const std::string& prefix);
        void Emit(const std::string& code);
        void EmitLine(const std::string& code);
        void EmitLineRaw(const std::string& code);  // Без конвертации кодировки
        void EmitComment(const std::string& comment);

        // Поиск в таблицах
        int FindFunctionEnd(int startIdx);
        int FindSemicolon(int startIdx);
        int FindMatchingBrace(int startIdx);
        int FindElse(int startIdx);

        // Получение информации
        const FunctionInfo& GetCurrentFunction();
        bool IsParameter(int idxTI, const FunctionInfo& func);
        int GetParamIndex(int idxTI, const FunctionInfo& func);
    };

    // =========================================================
    // Публичная функция генерации
    // =========================================================
    bool GenerateCode(LT::LexTable& lt, IT::IdTable& it, const wchar_t* outputPath);
}
