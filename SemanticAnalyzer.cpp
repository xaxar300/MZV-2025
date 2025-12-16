// SemanticAnalyzer.cpp - реализация семантического анализатора MZV-2025
#include "stdafx.h"

#define MAX_FUNCTIONS 64
#define MAX_PARAMS 16

namespace Semantic
{
    // Глобальный кэш типов выражений
    std::unordered_map<int, IDDATATYPE> expressionTypeCache;

    // Кэш информации о функциях (объявление в начале для доступа из ClearCache)
    static FunctionInfo cachedFunctions[MAX_FUNCTIONS];
    static int cachedFuncCount = -1;  // -1 означает кэш не заполнен

    // Очистка всех кэшей (вызывать перед каждым анализом)
    void ClearCache()
    {
        expressionTypeCache.clear();
        cachedFuncCount = -1;
    }

    // ��������� ����� ���� ��� ��������� �� �������
    const char* GetTypeName(IDDATATYPE type)
    {
        switch (type)
        {
        case IT_INT: return "integer";
        case IT_CHR: return "char";
        case IT_STR: return "string";
        default:     return "unknown";
        }
    }

    // �������� ������������� �����
    // integer � char ���������� (char ����� ������������ ��� �����)
    bool AreTypesCompatible(IDDATATYPE left, IDDATATYPE right)
    {
        // ���������� ���� ������ ����������
        if (left == right) return true;

        // integer � char ���������� ��� ����������
        if ((left == IT_INT || left == IT_CHR) &&
            (right == IT_INT || right == IT_CHR))
            return true;

        return false;
    }

    // ����������� ������������� ������
    void LogSemanticError(Log::LOGDATA& log, int errorCode, int line, const char* message)
    {
        char buf[512];
        sprintf_s(buf, "Semantic error %d (line %d): %s", errorCode, line, message);
        Log::writeline(log, buf);
        // ��������� ����� � �������
        cout << "\n      [ERROR " << errorCode << "] Line " << line << ": " << message << endl;
    }

    // ����������� �������� ��������
    void LogCheckOK(const char* checkName, const char* details = nullptr)
    {
        cout << "OK";
        if (details != nullptr)
            cout << " (" << details << ")";
        cout << endl;
    }

    // �������� ������� ������� main
    bool CheckMainExists(IT::IdTable& idtable)
    {
        // ������� ���� main ��� ������� � ������� ���������������
        for (int i = 0; i < idtable.size; i++)
        {
            if (idtable.table[i].idtype == IT_F &&
                strcmp(idtable.table[i].id, "main") == 0)
            {
                return true;
            }
        }

        // ���� �� ����� ��� IT_F, ��������� ���� �� scope "main"
        // (main ����� ���� ������� ������ ��� scope, ��� ������ � IT)
        for (int i = 0; i < idtable.size; i++)
        {
            if (strcmp(idtable.table[i].scope, "main") == 0)
            {
                return true;
            }
        }

        return false;
    }

    // �������� ��������� ���������� � ����� ������� ���������
    bool CheckRedeclarations(IT::IdTable& idtable, Log::LOGDATA& log, int& errorCount)
    {
        bool success = true;

        for (int i = 0; i < idtable.size; i++)
        {
            // ���������� ��������
            if (idtable.table[i].idtype == IT_L)
                continue;

            for (int j = i + 1; j < idtable.size; j++)
            {
                // ���������� ��������
                if (idtable.table[j].idtype == IT_L)
                    continue;

                // ��������� ���������� ���� � ������� ���������
                if (strcmp(idtable.table[i].id, idtable.table[j].id) == 0 &&
                    strcmp(idtable.table[i].scope, idtable.table[j].scope) == 0)
                {
                    // ��� ��������� ����������
                    char msg[256];
                    sprintf_s(msg, "��������� ���������� �������������� '%s' � ������� '%s'",
                        idtable.table[j].id,
                        idtable.table[j].scope[0] ? idtable.table[j].scope : "global");

                    LogSemanticError(log, Error::ERR_SEM_REDECLARED,
                        idtable.table[j].firstLine, msg);
                    errorCount++;
                    success = false;
                }
            }
        }

        return success;
    }

    // Сбор информации о функциях
    // Оптимизировано: кэширует результат для повторных вызовов
    void CollectFunctionInfo(LT::LexTable& lextable, IT::IdTable& idtable,
        FunctionInfo* functions, int& funcCount)
    {
        // Если кэш уже заполнен, используем его
        if (cachedFuncCount >= 0)
        {
            funcCount = cachedFuncCount;
            memcpy(functions, cachedFunctions, sizeof(FunctionInfo) * funcCount);
            return;
        }

        funcCount = 0;

        for (int i = 0; i < idtable.size; i++)
        {
            if (idtable.table[i].idtype == IT_F && funcCount < MAX_FUNCTIONS)
            {
                strcpy_s(functions[funcCount].name, idtable.table[i].id);
                functions[funcCount].returnType = idtable.table[i].datatype;
                functions[funcCount].declarationLine = idtable.table[i].firstLine;

                // Считаем параметры по записям IT_P в scope этой функции
                int paramIdx = 0;
                for (int j = 0; j < idtable.size && paramIdx < MAX_PARAMS; j++)
                {
                    if (idtable.table[j].idtype == IT_P &&
                        strcmp(idtable.table[j].scope, idtable.table[i].id) == 0)
                    {
                        functions[funcCount].paramTypes[paramIdx] = idtable.table[j].datatype;
                        paramIdx++;
                    }
                }
                functions[funcCount].paramCount = paramIdx;

                funcCount++;
            }
        }

        // Сохраняем в кэш
        cachedFuncCount = funcCount;
        memcpy(cachedFunctions, functions, sizeof(FunctionInfo) * funcCount);
    }

    // Получение типа выражения начиная с индекса startIdx
    // Оптимизировано: использует кэш для повторных запросов O(1)
    IDDATATYPE GetExpressionType(LT::LexTable& lextable, IT::IdTable& idtable,
        int startIdx, int& endIdx)
    {
        // Проверяем кэш
        auto cached = expressionTypeCache.find(startIdx);
        if (cached != expressionTypeCache.end())
        {
            // Нужно пересчитать endIdx даже при попадании в кэш
            int parenDepth = 0;
            for (int i = startIdx; i < lextable.size; i++)
            {
                char lex = lextable.table[i].lexema;
                if (lex == LEX_LEFTHESIS) parenDepth++;
                else if (lex == LEX_RIGHTHESIS) { parenDepth--; if (parenDepth < 0) { endIdx = i - 1; break; } }
                else if ((lex == LEX_SEMICOLON || lex == LEX_COMMA) && parenDepth == 0) { endIdx = i - 1; break; }
                endIdx = i;
            }
            return cached->second;
        }

        IDDATATYPE resultType = IT_INT; // тип по умолчанию
        int parenDepth = 0;
        endIdx = startIdx;

        for (int i = startIdx; i < lextable.size; i++)
        {
            char lex = lextable.table[i].lexema;

            if (lex == LEX_LEFTHESIS)
            {
                parenDepth++;
            }
            else if (lex == LEX_RIGHTHESIS)
            {
                parenDepth--;
                if (parenDepth < 0)
                {
                    endIdx = i - 1;
                    // Сохраняем в кэш
                    expressionTypeCache[startIdx] = resultType;
                    return resultType;
                }
            }
            else if (lex == LEX_SEMICOLON || lex == LEX_COMMA)
            {
                if (parenDepth == 0)
                {
                    endIdx = i - 1;
                    // Сохраняем в кэш
                    expressionTypeCache[startIdx] = resultType;
                    return resultType;
                }
            }
            else if (lex == LEX_ID || lex == LEX_LITERAL)
            {
                int idxTI = lextable.table[i].idxTI;
                if (idxTI != LT_TI_NULLIDX && idxTI < idtable.size)
                {
                    IDDATATYPE type = idtable.table[idxTI].datatype;

                    // Строковый тип имеет приоритет (для output)
                    if (type == IT_STR)
                    {
                        resultType = IT_STR;
                    }
                    else if (resultType != IT_STR)
                    {
                        // Для числовых типов используем первый встретившийся
                        if (i == startIdx || resultType == IT_INT)
                        {
                            resultType = type;
                        }
                    }
                }
            }

            endIdx = i;
        }

        // Сохраняем в кэш
        expressionTypeCache[startIdx] = resultType;
        return resultType;
    }

    // �������� ����� ��� ������������
    bool CheckAssignmentTypes(LT::LexTable& lextable, IT::IdTable& idtable,
        Log::LOGDATA& log, int& errorCount)
    {
        bool success = true;

        for (int i = 0; i < lextable.size; i++)
        {
            // ���� �������� ������������
            if (lextable.table[i].lexema == LEX_ASSIGN)
            {
                // ����� �� '=' ������ ���� �������������
                if (i > 0 && lextable.table[i - 1].lexema == LEX_ID)
                {
                    int leftIdxTI = lextable.table[i - 1].idxTI;
                    if (leftIdxTI == LT_TI_NULLIDX)
                        continue;

                    IDDATATYPE leftType = idtable.table[leftIdxTI].datatype;
                    int line = lextable.table[i].line;

                    // �������� ��� ������ �����
                    int endIdx;
                    IDDATATYPE rightType = GetExpressionType(lextable, idtable, i + 1, endIdx);

                    // ��������� ������������� �����
                    if (!AreTypesCompatible(leftType, rightType))
                    {
                        char msg[256];
                        sprintf_s(msg, "�������������� ����� ��� ������������: '%s' = %s (��������� %s)",
                            idtable.table[leftIdxTI].id,
                            GetTypeName(rightType),
                            GetTypeName(leftType));

                        LogSemanticError(log, Error::ERR_SEM_ASSIGN_TYPE, line, msg);
                        errorCount++;
                        success = false;
                    }

                    // ������ ��������� ������ ����������
                    if (leftType != IT_STR && rightType == IT_STR)
                    {
                        char msg[256];
                        sprintf_s(msg, "������ ��������� ������ ���������� '%s' ���� %s",
                            idtable.table[leftIdxTI].id,
                            GetTypeName(leftType));

                        LogSemanticError(log, Error::ERR_SEM_TYPE_MISMATCH, line, msg);
                        errorCount++;
                        success = false;
                    }
                }
            }
        }

        return success;
    }

    // Проверка вызовов функций
    bool CheckFunctionCalls(LT::LexTable& lextable, IT::IdTable& idtable,
        Log::LOGDATA& log, int& errorCount)
    {
        bool success = true;
        int funcCount = 0;

        // Используем статический кэш вместо локального массива (экономия стека)
        CollectFunctionInfo(lextable, idtable, cachedFunctions, funcCount);

        // ���� ������ �������: ������������� + '('
        for (int i = 0; i < lextable.size - 1; i++)
        {
            if (lextable.table[i].lexema == LEX_ID &&
                lextable.table[i + 1].lexema == LEX_LEFTHESIS)
            {
                int idxTI = lextable.table[i].idxTI;
                if (idxTI == LT_TI_NULLIDX)
                    continue;

                // ���������, ��� ��� �������
                if (idtable.table[idxTI].idtype != IT_F)
                    continue;

                const char* funcName = idtable.table[idxTI].id;
                int line = lextable.table[i].line;

                // Находим информацию о функции
                FunctionInfo* funcInfo = nullptr;
                for (int f = 0; f < funcCount; f++)
                {
                    if (strcmp(cachedFunctions[f].name, funcName) == 0)
                    {
                        funcInfo = &cachedFunctions[f];
                        break;
                    }
                }

                if (funcInfo == nullptr)
                    continue;

                // ������������ ����������� ��������� �� �������
                int actualParams = 0;
                IDDATATYPE actualTypes[MAX_PARAMS];
                int parenDepth = 1;
                int j = i + 2; // ����� '('
                bool hasContent = false;
                int currentArgIdx = 0;

                while (j < lextable.size && parenDepth > 0)
                {
                    char lex = lextable.table[j].lexema;

                    if (lex == LEX_LEFTHESIS)
                    {
                        parenDepth++;
                    }
                    else if (lex == LEX_RIGHTHESIS)
                    {
                        parenDepth--;
                        if (parenDepth == 0 && hasContent)
                        {
                            actualParams = currentArgIdx + 1;
                        }
                    }
                    else if (lex == LEX_COMMA && parenDepth == 1)
                    {
                        // ����������� ����������
                        currentArgIdx++;
                    }
                    else if ((lex == LEX_ID || lex == LEX_LITERAL) && parenDepth == 1)
                    {
                        hasContent = true;
                        int argIdxTI = lextable.table[j].idxTI;
                        if (argIdxTI != LT_TI_NULLIDX && currentArgIdx < MAX_PARAMS)
                        {
                            // ���������� ��� ������� �������� ������� ���������
                            if (actualTypes[currentArgIdx] == IT_INT || currentArgIdx >= actualParams)
                            {
                                actualTypes[currentArgIdx] = idtable.table[argIdxTI].datatype;
                            }
                        }
                    }
                    j++;
                }

                // ���� ������ ������ - 0 ����������
                if (!hasContent)
                    actualParams = 0;

                // ��������� ���������� ����������
                if (actualParams != funcInfo->paramCount)
                {
                    char msg[256];
                    sprintf_s(msg, "Function '%s': expected %d params, got %d",
                        funcName, funcInfo->paramCount, actualParams);

                    LogSemanticError(log, Error::ERR_SEM_PARAM_COUNT, line, msg);
                    errorCount++;
                    success = false;
                }
                else
                {
                    // ��������� ���� ����������
                    for (int p = 0; p < actualParams && p < funcInfo->paramCount; p++)
                    {
                        if (!AreTypesCompatible(funcInfo->paramTypes[p], actualTypes[p]))
                        {
                            char msg[256];
                            sprintf_s(msg, "Function '%s': param %d - expected %s, got %s",
                                funcName, p + 1,
                                GetTypeName(funcInfo->paramTypes[p]),
                                GetTypeName(actualTypes[p]));

                            LogSemanticError(log, Error::ERR_SEM_PARAM_TYPE, line, msg);
                            errorCount++;
                            success = false;
                        }
                    }
                }
            }
        }

        return success;
    }

    // �������� ���� ������������� ��������
    bool CheckReturnTypes(LT::LexTable& lextable, IT::IdTable& idtable,
        Log::LOGDATA& log, int& errorCount)
    {
        bool success = true;
        char currentFunction[256] = "";
        IDDATATYPE expectedReturnType = IT_INT;

        for (int i = 0; i < lextable.size; i++)
        {
            // ����������� ������� �������
            if (lextable.table[i].lexema == LEX_FUNCTION ||
                lextable.table[i].lexema == LEX_MAIN)
            {
                // ���� ��� �������
                for (int j = i + 1; j < lextable.size; j++)
                {
                    if (lextable.table[j].lexema == LEX_ID)
                    {
                        int idxTI = lextable.table[j].idxTI;
                        if (idxTI != LT_TI_NULLIDX && idtable.table[idxTI].idtype == IT_F)
                        {
                            strcpy_s(currentFunction, idtable.table[idxTI].id);
                            expectedReturnType = idtable.table[idxTI].datatype;
                        }
                        break;
                    }
                    else if (lextable.table[j].lexema == LEX_LEFTBRACE)
                    {
                        // ��� main
                        strcpy_s(currentFunction, "main");
                        expectedReturnType = IT_INT;
                        break;
                    }
                }
            }

            // ��������� return
            if (lextable.table[i].lexema == LEX_RETURN)
            {
                int line = lextable.table[i].line;

                // �������� ��� ������������� ���������
                int endIdx;
                IDDATATYPE actualType = GetExpressionType(lextable, idtable, i + 1, endIdx);

                if (!AreTypesCompatible(expectedReturnType, actualType))
                {
                    char msg[256];
                    sprintf_s(msg, "������� '%s': ������������ %s, ��������� %s",
                        currentFunction,
                        GetTypeName(actualType),
                        GetTypeName(expectedReturnType));

                    LogSemanticError(log, Error::ERR_SEM_RETURN_TYPE, line, msg);
                    errorCount++;
                    success = false;
                }
            }
        }

        return success;
    }

    // �������� ����� � �������� ����������
    bool CheckConditionTypes(LT::LexTable& lextable, IT::IdTable& idtable,
        Log::LOGDATA& log, int& errorCount)
    {
        bool success = true;

        for (int i = 0; i < lextable.size; i++)
        {
            // ���� if
            if (lextable.table[i].lexema == LEX_IF)
            {
                int line = lextable.table[i].line;

                // ����� if ������ ���� '('
                if (i + 1 < lextable.size && lextable.table[i + 1].lexema == LEX_LEFTHESIS)
                {
                    // ��������� ���� � �������
                    int parenDepth = 1;
                    int j = i + 2;
                    bool hasStringInCondition = false;

                    while (j < lextable.size && parenDepth > 0)
                    {
                        char lex = lextable.table[j].lexema;

                        if (lex == LEX_LEFTHESIS)
                            parenDepth++;
                        else if (lex == LEX_RIGHTHESIS)
                            parenDepth--;
                        else if ((lex == LEX_ID || lex == LEX_LITERAL) && parenDepth == 1)
                        {
                            int idxTI = lextable.table[j].idxTI;
                            if (idxTI != LT_TI_NULLIDX &&
                                idtable.table[idxTI].datatype == IT_STR)
                            {
                                hasStringInCondition = true;
                            }
                        }
                        j++;
                    }

                    if (hasStringInCondition)
                    {
                        LogSemanticError(log, Error::ERR_SEM_COND_TYPE, line,
                            "��������� ��� ���������� � ������� if");
                        errorCount++;
                        success = false;
                    }
                }
            }
        }

        return success;
    }

    // �������� ������� ��������
    bool CheckUnaryOperations(LT::LexTable& lextable, IT::IdTable& idtable,
        Log::LOGDATA& log, int& errorCount)
    {
        bool success = true;

        for (int i = 0; i < lextable.size; i++)
        {
            if (lextable.table[i].lexema == LEX_UNARY)
            {
                int line = lextable.table[i].line;
                char op = lextable.table[i].op;

                // ���� ������� (����� ��� ����� ������� ��������)
                int operandIdx = -1;

                // ����������� ++, -- (����� ��������������)
                if (i > 0 && lextable.table[i - 1].lexema == LEX_ID)
                {
                    operandIdx = i - 1;
                }
                // ���������� ~ (����� ���������������)
                else if (op == '~' && i + 1 < lextable.size &&
                    lextable.table[i + 1].lexema == LEX_ID)
                {
                    operandIdx = i + 1;
                }

                if (operandIdx >= 0)
                {
                    int idxTI = lextable.table[operandIdx].idxTI;
                    if (idxTI != LT_TI_NULLIDX)
                    {
                        IDDATATYPE type = idtable.table[idxTI].datatype;

                        // ++, --, ~ �������� ������ � integer � char
                        if (type == IT_STR)
                        {
                            char msg[256];
                            sprintf_s(msg, "������� �������� '%c' ����������� ��� ���������� ����",
                                op ? op : '?');

                            LogSemanticError(log, Error::ERR_SEM_UNARY_TYPE, line, msg);
                            errorCount++;
                            success = false;
                        }
                    }
                }
            }
        }

        return success;
    }

    // ========== �������� �������� ��������� (4.2) ==========

    // ��������� ������� ������� ��������� �� ������� � ������� ������
    void GetCurrentScope(LT::LexTable& lextable, IT::IdTable& idtable,
        int lexIdx, char* scope)
    {
        scope[0] = CHAR_NULL;
        int braceDepth = 0;

        for (int i = 0; i <= lexIdx; i++)
        {
            char lex = lextable.table[i].lexema;

            if (lex == LEX_FUNCTION)
            {
                for (int j = i + 1; j < lextable.size; j++)
                {
                    if (lextable.table[j].lexema == LEX_ID)
                    {
                        int idxTI = lextable.table[j].idxTI;
                        if (idxTI != LT_TI_NULLIDX && idtable.table[idxTI].idtype == IT_F)
                        {
                            strcpy_s(scope, 256, idtable.table[idxTI].id);
                        }
                        break;
                    }
                }
            }
            else if (lex == LEX_MAIN)
            {
                strcpy_s(scope, 256, "main");
            }
            else if (lex == LEX_LEFTBRACE)
            {
                braceDepth++;
            }
            else if (lex == LEX_RIGHTBRACE)
            {
                braceDepth--;
            }
        }
    }

    // �������� ��������� �������������� � ������� �������
    bool IsVisibleInScope(IT::IdTable& idtable, const char* id,
        const char* currentScope, int useLine)
    {
        for (int i = 0; i < idtable.size; i++)
        {
            if (strcmp(idtable.table[i].id, id) == 0)
            {
                if (strcmp(idtable.table[i].scope, currentScope) == 0)
                {
                    if (idtable.table[i].firstLine <= useLine)
                        return true;
                }
                else if (idtable.table[i].scope[0] == CHAR_NULL)
                {
                    if (idtable.table[i].firstLine <= useLine)
                        return true;
                }
                else if (idtable.table[i].idtype == IT_P &&
                    strcmp(idtable.table[i].scope, currentScope) == 0)
                {
                    return true;
                }
            }
        }
        return false;
    }

    // �������� ������������� ���������� �� ����������
    bool CheckVariableUsageBeforeDeclaration(LT::LexTable& lextable, IT::IdTable& idtable,
        Log::LOGDATA& log, int& errorCount)
    {
        bool success = true;
        char currentScope[256] = "";
        bool inDeclaration = false;

        for (int i = 0; i < lextable.size; i++)
        {
            char lex = lextable.table[i].lexema;

            if (lex == LEX_FUNCTION)
            {
                for (int j = i + 1; j < lextable.size; j++)
                {
                    if (lextable.table[j].lexema == LEX_ID)
                    {
                        int idxTI = lextable.table[j].idxTI;
                        if (idxTI != LT_TI_NULLIDX && idtable.table[idxTI].idtype == IT_F)
                        {
                            strcpy_s(currentScope, idtable.table[idxTI].id);
                        }
                        break;
                    }
                }
            }
            else if (lex == LEX_MAIN)
            {
                strcpy_s(currentScope, "main");
            }
            else if (lex == LEX_DECLARE)
            {
                inDeclaration = true;
            }
            else if (lex == LEX_SEMICOLON)
            {
                inDeclaration = false;
            }
            else if (lex == LEX_ID && !inDeclaration)
            {
                int idxTI = lextable.table[i].idxTI;
                if (idxTI == LT_TI_NULLIDX)
                    continue;

                if (idtable.table[idxTI].idtype == IT_F ||
                    idtable.table[idxTI].idtype == IT_L)
                    continue;

                int useLine = lextable.table[i].line;
                const char* varName = idtable.table[idxTI].id;
                int declLine = idtable.table[idxTI].firstLine;

                if (idtable.table[idxTI].idtype != IT_P)
                {
                    if (useLine < declLine)
                    {
                        char msg[256];
                        sprintf_s(msg, "Variable '%s' used at line %d before declaration at line %d",
                            varName, useLine, declLine);
                        LogSemanticError(log, Error::ERR_SEM_UNDEFINED_VAR, useLine, msg);
                        errorCount++;
                        success = false;
                    }
                }
            }
        }

        return success;
    }

    // Проверка существования вызываемых функций
    bool CheckFunctionExists(LT::LexTable& lextable, IT::IdTable& idtable,
        Log::LOGDATA& log, int& errorCount)
    {
        bool success = true;
        int funcCount = 0;
        // Используем статический кэш (экономия стека)
        CollectFunctionInfo(lextable, idtable, cachedFunctions, funcCount);

        for (int i = 0; i < lextable.size - 1; i++)
        {
            if (lextable.table[i].lexema == LEX_ID &&
                lextable.table[i + 1].lexema == LEX_LEFTHESIS)
            {
                int idxTI = lextable.table[i].idxTI;
                if (idxTI == LT_TI_NULLIDX)
                    continue;

                const char* callName = idtable.table[idxTI].id;
                int line = lextable.table[i].line;

                bool found = false;
                int funcDeclLine = 0;

                for (int f = 0; f < funcCount; f++)
                {
                    if (strcmp(cachedFunctions[f].name, callName) == 0)
                    {
                        found = true;
                        funcDeclLine = cachedFunctions[f].declarationLine;
                        break;
                    }
                }

                if (!found && strcmp(callName, "main") == 0)
                {
                    found = true;
                }

                if (!found)
                {
                    char msg[256];
                    sprintf_s(msg, "Call to undefined function '%s'", callName);
                    LogSemanticError(log, Error::ERR_SEM_UNDEFINED_FUNC, line, msg);
                    errorCount++;
                    success = false;
                }
                else if (funcDeclLine > 0 && line < funcDeclLine)
                {
                    char msg[256];
                    sprintf_s(msg, "Function '%s' called at line %d before declaration at line %d",
                        callName, line, funcDeclLine);
                    LogSemanticError(log, Error::ERR_SEM_UNDEFINED_FUNC, line, msg);
                    errorCount++;
                    success = false;
                }
            }
        }

        return success;
    }

    // �������� ������������ �������� ��������� � ����� ����������
    bool CheckScopes(LT::LexTable& lextable, IT::IdTable& idtable,
        Log::LOGDATA& log, int& errorCount)
    {
        bool success = true;
        int globalVars = 0;
        int localVars = 0;
        int params = 0;
        int functions = 0;
        int literals = 0;

        for (int i = 0; i < idtable.size; i++)
        {
            switch (idtable.table[i].idtype)
            {
            case IT_F:
                functions++;
                break;
            case IT_P:
                params++;
                // ��������� ������ ����� �������� scope
                if (idtable.table[i].scope[0] == CHAR_NULL)
                {
                    char msg[256];
                    sprintf_s(msg, "Parameter '%s' has no function scope", idtable.table[i].id);
                    LogSemanticError(log, Error::ERR_SEM_SCOPE_ERROR, idtable.table[i].firstLine, msg);
                    errorCount++;
                    success = false;
                }
                break;
            case IT_L:
                literals++;
                break;
            case IT_V:
                if (idtable.table[i].scope[0] == CHAR_NULL)
                    globalVars++;
                else
                    localVars++;
                break;
            }
        }

        cout << endl;
        cout << "        Functions:   " << functions << endl;
        cout << "        Parameters:  " << params << endl;
        cout << "        Global vars: " << globalVars << endl;
        cout << "        Local vars:  " << localVars << endl;
        cout << "        Literals:    " << literals << endl;
        cout << "        "; // ��� ������������ OK

        return success;
    }

    // ========== �������� ����� ��������� (4.3) ==========

    // ��������� ���� �������� �� ������� � ������� ������
    IDDATATYPE GetOperandType(LT::LexTable& lextable, IT::IdTable& idtable, int lexIdx)
    {
        if (lexIdx < 0 || lexIdx >= lextable.size)
            return IT_INT;

        char lex = lextable.table[lexIdx].lexema;

        if (lex == LEX_ID || lex == LEX_LITERAL)
        {
            int idxTI = lextable.table[lexIdx].idxTI;
            if (idxTI != LT_TI_NULLIDX && idxTI < idtable.size)
            {
                return idtable.table[idxTI].datatype;
            }
        }

        return IT_INT; // �� ���������
    }

    // ����� ������ �������� ��� �������� ��������
    int FindLeftOperand(LT::LexTable& lextable, int opIdx)
    {
        // ��� �����, ��������� ������
        int parenDepth = 0;
        for (int i = opIdx - 1; i >= 0; i--)
        {
            char lex = lextable.table[i].lexema;

            if (lex == LEX_RIGHTHESIS)
                parenDepth++;
            else if (lex == LEX_LEFTHESIS)
                parenDepth--;
            else if (parenDepth == 0 && (lex == LEX_ID || lex == LEX_LITERAL))
                return i;
            else if (parenDepth == 0 && (lex == LEX_SEMICOLON || lex == LEX_ASSIGN ||
                lex == LEX_COMMA || lex == LEX_LEFTBRACE))
                break;
        }
        return -1;
    }

    // ����� ������� �������� ��� �������� ��������
    int FindRightOperand(LT::LexTable& lextable, int opIdx)
    {
        // ��� �����, ��������� ������
        int parenDepth = 0;
        for (int i = opIdx + 1; i < lextable.size; i++)
        {
            char lex = lextable.table[i].lexema;

            if (lex == LEX_LEFTHESIS)
                parenDepth++;
            else if (lex == LEX_RIGHTHESIS)
                parenDepth--;
            else if (parenDepth == 0 && (lex == LEX_ID || lex == LEX_LITERAL))
                return i;
            else if (parenDepth == 0 && (lex == LEX_SEMICOLON || lex == LEX_RIGHTBRACE))
                break;
        }
        return -1;
    }

    // �������� ����� ��������� �������������� �������� (+, -, *, /, %)
    bool CheckArithmeticOperands(LT::LexTable& lextable, IT::IdTable& idtable,
        Log::LOGDATA& log, int& errorCount)
    {
        bool success = true;

        for (int i = 0; i < lextable.size; i++)
        {
            if (lextable.table[i].lexema == LEX_BINOP)
            {
                char op = lextable.table[i].op;
                int line = lextable.table[i].line;

                // ������� ����� � ������ ��������
                int leftIdx = FindLeftOperand(lextable, i);
                int rightIdx = FindRightOperand(lextable, i);

                if (leftIdx >= 0)
                {
                    IDDATATYPE leftType = GetOperandType(lextable, idtable, leftIdx);

                    // ������ �� ����� ����������� � �������������� ���������
                    if (leftType == IT_STR)
                    {
                        char msg[256];
                        sprintf_s(msg, "String type not allowed in arithmetic operation '%c'", op);
                        LogSemanticError(log, Error::ERR_SEM_ARITH_TYPE, line, msg);
                        errorCount++;
                        success = false;
                    }
                }

                if (rightIdx >= 0)
                {
                    IDDATATYPE rightType = GetOperandType(lextable, idtable, rightIdx);

                    if (rightType == IT_STR)
                    {
                        char msg[256];
                        sprintf_s(msg, "String type not allowed in arithmetic operation '%c'", op);
                        LogSemanticError(log, Error::ERR_SEM_ARITH_TYPE, line, msg);
                        errorCount++;
                        success = false;
                    }
                }
            }
        }

        return success;
    }

    // �������� ����� ��������� �������� ��������� (<, >, <=, >=, ==, !=)
    bool CheckComparisonOperands(LT::LexTable& lextable, IT::IdTable& idtable,
        Log::LOGDATA& log, int& errorCount)
    {
        bool success = true;

        for (int i = 0; i < lextable.size; i++)
        {
            if (lextable.table[i].lexema == LEX_COMPARE)
            {
                char op = lextable.table[i].op;
                int line = lextable.table[i].line;

                int leftIdx = FindLeftOperand(lextable, i);
                int rightIdx = FindRightOperand(lextable, i);

                IDDATATYPE leftType = IT_INT;
                IDDATATYPE rightType = IT_INT;

                if (leftIdx >= 0)
                    leftType = GetOperandType(lextable, idtable, leftIdx);
                if (rightIdx >= 0)
                    rightType = GetOperandType(lextable, idtable, rightIdx);

                // ������ ����� ���������� ������ �� ��������� (== � !=)
                bool isEqualityOp = (op == '=' || op == '!'); // == ��� !=

                if (leftType == IT_STR || rightType == IT_STR)
                {
                    if (!isEqualityOp)
                    {
                        char msg[256];
                        sprintf_s(msg, "String comparison only allowed with == or !=, not '%c'", op);
                        LogSemanticError(log, Error::ERR_SEM_COMPARE_TYPE, line, msg);
                        errorCount++;
                        success = false;
                    }
                    // ��� �������� ������ ���� �������� ��� ��������� �����
                    else if ((leftType == IT_STR) != (rightType == IT_STR))
                    {
                        char msg[256];
                        sprintf_s(msg, "Cannot compare string with %s",
                            leftType == IT_STR ? GetTypeName(rightType) : GetTypeName(leftType));
                        LogSemanticError(log, Error::ERR_SEM_COMPARE_TYPE, line, msg);
                        errorCount++;
                        success = false;
                    }
                }
            }
        }

        return success;
    }

    // Главная функция семантического анализа
    SEMRESULT Analyze(LT::LexTable& lextable, IT::IdTable& idtable, Log::LOGDATA& log)
    {
        // Очищаем кэш перед началом анализа
        ClearCache();

        SEMRESULT result;
        result.errors = 0;
        result.warnings = 0;
        result.hasMain = false;

        Log::writeline(log, "\n=== Semantic Analysis ===\n");
        cout << "\n    ============= SEMANTIC ANALYSIS =============" << endl;
        cout << "    Identifiers: " << idtable.size << ", Lexemes: " << lextable.size << endl;
        cout << "    ----------------------------------------------" << endl;

        // 1. �������� ������� main
        cout << "    [1]  Main function............ " << flush;
        result.hasMain = CheckMainExists(idtable);
        if (!result.hasMain)
        {
            LogSemanticError(log, Error::ERR_SEM_NO_MAIN, 0, "Function 'main' not found");
            result.errors++;
        }
        else
        {
            Log::writeline(log, "    Function main found");
            cout << "OK" << endl;
        }

        // 2. �������� ��������� ����������
        cout << "    [2]  Redeclarations........... " << flush;
        int errBefore = result.errors;
        CheckRedeclarations(idtable, log, result.errors);
        if (result.errors == errBefore)
            cout << "OK" << endl;

        // 3. �������� ����� ��� ������������
        cout << "    [3]  Assignment types......... " << flush;
        errBefore = result.errors;
        CheckAssignmentTypes(lextable, idtable, log, result.errors);
        if (result.errors == errBefore)
            cout << "OK" << endl;

        // 4. Проверка вызовов функций (параметры)
        cout << "    [4]  Function params.......... " << flush;
        errBefore = result.errors;

        // Используем статический кэш (уже заполняется в CheckFunctionCalls)
        int funcCount = 0;
        CollectFunctionInfo(lextable, idtable, cachedFunctions, funcCount);

        CheckFunctionCalls(lextable, idtable, log, result.errors);
        if (result.errors == errBefore)
        {
            cout << "OK";
            if (funcCount > 0)
            {
                cout << " [" << funcCount << " func]";
            }
            cout << endl;
        }

        // 5. �������� ����� ������������ ��������
        cout << "    [5]  Return types............. " << flush;
        errBefore = result.errors;
        CheckReturnTypes(lextable, idtable, log, result.errors);
        if (result.errors == errBefore)
            cout << "OK" << endl;

        // 6. �������� �������� ����������
        cout << "    [6]  If conditions............ " << flush;
        errBefore = result.errors;
        CheckConditionTypes(lextable, idtable, log, result.errors);
        if (result.errors == errBefore)
            cout << "OK" << endl;

        // 7. �������� ������� ��������
        cout << "    [7]  Unary operations......... " << flush;
        errBefore = result.errors;
        CheckUnaryOperations(lextable, idtable, log, result.errors);
        if (result.errors == errBefore)
            cout << "OK" << endl;

        // 8. �������� ����� �������������� ��������
        cout << "    [8]  Arithmetic operands...... " << flush;
        errBefore = result.errors;
        CheckArithmeticOperands(lextable, idtable, log, result.errors);
        if (result.errors == errBefore)
            cout << "OK" << endl;

        // 9. �������� ����� �������� ���������
        cout << "    [9]  Comparison operands...... " << flush;
        errBefore = result.errors;
        CheckComparisonOperands(lextable, idtable, log, result.errors);
        if (result.errors == errBefore)
            cout << "OK" << endl;

        // 10. �������� ������������� ���������� �� ����������
        cout << "    [10] Declaration order........ " << flush;
        errBefore = result.errors;
        CheckVariableUsageBeforeDeclaration(lextable, idtable, log, result.errors);
        if (result.errors == errBefore)
            cout << "OK" << endl;

        // 11. �������� ������������� ���������� �������
        cout << "    [11] Function existence....... " << flush;
        errBefore = result.errors;
        CheckFunctionExists(lextable, idtable, log, result.errors);
        if (result.errors == errBefore)
            cout << "OK" << endl;

        // 12. �������� �������� ��������� � ����������
        cout << "    [12] Scope validation......... " << flush;
        errBefore = result.errors;
        CheckScopes(lextable, idtable, log, result.errors);
        if (result.errors == errBefore)
            cout << "OK" << endl;

        // �������� ���������
        cout << "    ----------------------------------------------" << endl;
        if (result.errors == 0)
        {
            cout << "    RESULT: All checks PASSED!" << endl;
        }
        else
        {
            cout << "    RESULT: " << result.errors << " error(s) found" << endl;
        }
        cout << "    ==============================================" << endl;

        char summary[256];
        sprintf_s(summary, "\nSemantic analysis completed: %d error(s), %d warning(s)",
            result.errors, result.warnings);
        Log::writeline(log, summary);

        return result;
    }
};