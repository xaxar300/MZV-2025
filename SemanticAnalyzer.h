// SemanticAnalyzer.h - семантический анализатор MZV-2025
// Проверка типов и корректности кода
#pragma once

#include <unordered_map>

namespace Semantic
{
    // Кэш для типов выражений - ускоряет повторные вычисления O(1)
    extern std::unordered_map<int, IDDATATYPE> expressionTypeCache;

    // Очистка кэша (вызывать перед анализом)
    void ClearCache();

    // ��������� �������������� �������
    struct SEMRESULT
    {
        int errors;                         // ���������� ������
        int warnings;                       // ���������� ��������������
        bool hasMain;                       // ������� ������� main
    };

    // ���������� � ������� ��� �������� �������
    struct FunctionInfo
    {
        char name[256];                     // ��� �������
        IDDATATYPE returnType;              // ��� ������������� ��������
        int paramCount;                     // ���������� ����������
        IDDATATYPE paramTypes[16];          // ���� ���������� (����. 16)
        int declarationLine;                // ������ ����������
    };

    // ������� ������� �������������� �������
    SEMRESULT Analyze(LT::LexTable& lextable, IT::IdTable& idtable, Log::LOGDATA& log);

    // === �������� ����� (4.1) ===

    // �������� ������� main
    bool CheckMainExists(IT::IdTable& idtable);

    // �������� ��������� ����������
    bool CheckRedeclarations(IT::IdTable& idtable, Log::LOGDATA& log, int& errorCount);

    // �������� ����� ��� ������������
    bool CheckAssignmentTypes(LT::LexTable& lextable, IT::IdTable& idtable,
        Log::LOGDATA& log, int& errorCount);

    // �������� ������� ������� (���������)
    bool CheckFunctionCalls(LT::LexTable& lextable, IT::IdTable& idtable,
        Log::LOGDATA& log, int& errorCount);

    // �������� ���� ������������� ��������
    bool CheckReturnTypes(LT::LexTable& lextable, IT::IdTable& idtable,
        Log::LOGDATA& log, int& errorCount);

    // �������� ����� � �������� ����������
    bool CheckConditionTypes(LT::LexTable& lextable, IT::IdTable& idtable,
        Log::LOGDATA& log, int& errorCount);

    // �������� ������� ��������
    bool CheckUnaryOperations(LT::LexTable& lextable, IT::IdTable& idtable,
        Log::LOGDATA& log, int& errorCount);

    // === �������� �������� ��������� (4.2) ===

    // �������� ������������� ���������� �� ����������
    bool CheckVariableUsageBeforeDeclaration(LT::LexTable& lextable, IT::IdTable& idtable,
        Log::LOGDATA& log, int& errorCount);

    // �������� ������������� ���������� �������
    bool CheckFunctionExists(LT::LexTable& lextable, IT::IdTable& idtable,
        Log::LOGDATA& log, int& errorCount);

    // �������� ������������ �������� ���������
    bool CheckScopes(LT::LexTable& lextable, IT::IdTable& idtable,
        Log::LOGDATA& log, int& errorCount);

    // === �������� ����� ��������� (4.3) ===

    // �������� ����� ��������� �������������� �������� (+, -, *, /, %)
    bool CheckArithmeticOperands(LT::LexTable& lextable, IT::IdTable& idtable,
        Log::LOGDATA& log, int& errorCount);

    // �������� ����� ��������� �������� ��������� (<, >, <=, >=, ==, !=)
    bool CheckComparisonOperands(LT::LexTable& lextable, IT::IdTable& idtable,
        Log::LOGDATA& log, int& errorCount);

    // === ��������������� ������� ===

    IDDATATYPE GetExpressionType(LT::LexTable& lextable, IT::IdTable& idtable,
        int startIdx, int& endIdx);

    bool AreTypesCompatible(IDDATATYPE left, IDDATATYPE right);

    const char* GetTypeName(IDDATATYPE type);

    // ���� ���������� � ��������
    void CollectFunctionInfo(LT::LexTable& lextable, IT::IdTable& idtable,
        FunctionInfo* functions, int& funcCount);

    // ��������� ������� ������� ��������� �� ������� � ������� ������
    void GetCurrentScope(LT::LexTable& lextable, IT::IdTable& idtable,
        int lexIdx, char* scope);

    // �������� ��������� �������������� � �������
    bool IsVisibleInScope(IT::IdTable& idtable, const char* id,
        const char* currentScope, int useLine);

    // ����������� ������������� ������
    void LogSemanticError(Log::LOGDATA& log, int errorCode, int line, const char* message);
};