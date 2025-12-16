// Lexer.cpp - FST-based лексический анализатор MZV-2025
#include "stdafx.h"

#define WORD_MAX_SIZE   256
#define NOT_FOUND       -1

namespace Lexer
{
    // ===========================================================
    // FST-функции для распознавания ключевых слов
    // ===========================================================

    bool fstInteger(const char* word) {
        FST::FST fst;
        fst.string = word;
        fst.nodeCount = 8;
        fst.nodes = new FST::NODE[8]{
            FST::CreateNode(1, FST::CreateRelation('i', 1)),
            FST::CreateNode(1, FST::CreateRelation('n', 2)),
            FST::CreateNode(1, FST::CreateRelation('t', 3)),
            FST::CreateNode(1, FST::CreateRelation('e', 4)),
            FST::CreateNode(1, FST::CreateRelation('g', 5)),
            FST::CreateNode(1, FST::CreateRelation('e', 6)),
            FST::CreateNode(1, FST::CreateRelation('r', 7)),
            FST::CreateNode(0)
        };
        bool result = FST::Execute(fst);
        delete[] fst.nodes;
        return result;
    }

    bool fstChar(const char* word) {
        FST::FST fst;
        fst.string = word;
        fst.nodeCount = 5;
        fst.nodes = new FST::NODE[5]{
            FST::CreateNode(1, FST::CreateRelation('c', 1)),
            FST::CreateNode(1, FST::CreateRelation('h', 2)),
            FST::CreateNode(1, FST::CreateRelation('a', 3)),
            FST::CreateNode(1, FST::CreateRelation('r', 4)),
            FST::CreateNode(0)
        };
        bool result = FST::Execute(fst);
        delete[] fst.nodes;
        return result;
    }

    bool fstFunction(const char* word) {
        FST::FST fst;
        fst.string = word;
        fst.nodeCount = 9;
        fst.nodes = new FST::NODE[9]{
            FST::CreateNode(1, FST::CreateRelation('f', 1)),
            FST::CreateNode(1, FST::CreateRelation('u', 2)),
            FST::CreateNode(1, FST::CreateRelation('n', 3)),
            FST::CreateNode(1, FST::CreateRelation('c', 4)),
            FST::CreateNode(1, FST::CreateRelation('t', 5)),
            FST::CreateNode(1, FST::CreateRelation('i', 6)),
            FST::CreateNode(1, FST::CreateRelation('o', 7)),
            FST::CreateNode(1, FST::CreateRelation('n', 8)),
            FST::CreateNode(0)
        };
        bool result = FST::Execute(fst);
        delete[] fst.nodes;
        return result;
    }

    bool fstDeclare(const char* word) {
        FST::FST fst;
        fst.string = word;
        fst.nodeCount = 8;
        fst.nodes = new FST::NODE[8]{
            FST::CreateNode(1, FST::CreateRelation('d', 1)),
            FST::CreateNode(1, FST::CreateRelation('e', 2)),
            FST::CreateNode(1, FST::CreateRelation('c', 3)),
            FST::CreateNode(1, FST::CreateRelation('l', 4)),
            FST::CreateNode(1, FST::CreateRelation('a', 5)),
            FST::CreateNode(1, FST::CreateRelation('r', 6)),
            FST::CreateNode(1, FST::CreateRelation('e', 7)),
            FST::CreateNode(0)
        };
        bool result = FST::Execute(fst);
        delete[] fst.nodes;
        return result;
    }

    bool fstMain(const char* word) {
        FST::FST fst;
        fst.string = word;
        fst.nodeCount = 5;
        fst.nodes = new FST::NODE[5]{
            FST::CreateNode(1, FST::CreateRelation('m', 1)),
            FST::CreateNode(1, FST::CreateRelation('a', 2)),
            FST::CreateNode(1, FST::CreateRelation('i', 3)),
            FST::CreateNode(1, FST::CreateRelation('n', 4)),
            FST::CreateNode(0)
        };
        bool result = FST::Execute(fst);
        delete[] fst.nodes;
        return result;
    }

    bool fstReturn(const char* word) {
        FST::FST fst;
        fst.string = word;
        fst.nodeCount = 7;
        fst.nodes = new FST::NODE[7]{
            FST::CreateNode(1, FST::CreateRelation('r', 1)),
            FST::CreateNode(1, FST::CreateRelation('e', 2)),
            FST::CreateNode(1, FST::CreateRelation('t', 3)),
            FST::CreateNode(1, FST::CreateRelation('u', 4)),
            FST::CreateNode(1, FST::CreateRelation('r', 5)),
            FST::CreateNode(1, FST::CreateRelation('n', 6)),
            FST::CreateNode(0)
        };
        bool result = FST::Execute(fst);
        delete[] fst.nodes;
        return result;
    }

    bool fstOutput(const char* word) {
        FST::FST fst;
        fst.string = word;
        fst.nodeCount = 7;
        fst.nodes = new FST::NODE[7]{
            FST::CreateNode(1, FST::CreateRelation('o', 1)),
            FST::CreateNode(1, FST::CreateRelation('u', 2)),
            FST::CreateNode(1, FST::CreateRelation('t', 3)),
            FST::CreateNode(1, FST::CreateRelation('p', 4)),
            FST::CreateNode(1, FST::CreateRelation('u', 5)),
            FST::CreateNode(1, FST::CreateRelation('t', 6)),
            FST::CreateNode(0)
        };
        bool result = FST::Execute(fst);
        delete[] fst.nodes;
        return result;
    }

    bool fstIf(const char* word) {
        FST::FST fst;
        fst.string = word;
        fst.nodeCount = 3;
        fst.nodes = new FST::NODE[3]{
            FST::CreateNode(1, FST::CreateRelation('i', 1)),
            FST::CreateNode(1, FST::CreateRelation('f', 2)),
            FST::CreateNode(0)
        };
        bool result = FST::Execute(fst);
        delete[] fst.nodes;
        return result;
    }

    bool fstElse(const char* word) {
        FST::FST fst;
        fst.string = word;
        fst.nodeCount = 5;
        fst.nodes = new FST::NODE[5]{
            FST::CreateNode(1, FST::CreateRelation('e', 1)),
            FST::CreateNode(1, FST::CreateRelation('l', 2)),
            FST::CreateNode(1, FST::CreateRelation('s', 3)),
            FST::CreateNode(1, FST::CreateRelation('e', 4)),
            FST::CreateNode(0)
        };
        bool result = FST::Execute(fst);
        delete[] fst.nodes;
        return result;
    }

    // ===========================================================
    // FST-функция для идентификаторов: [a-zA-Z_][a-zA-Z0-9_]*
    // ===========================================================

    bool fstIdentifier(const char* word) {
        if (word[0] == CHAR_NULL) return false;

        // FST для идентификатора с переходами на буквы и подчеркивание
        FST::FST fst;
        fst.string = word;
        fst.nodeCount = 2;

        // Первый узел: буквы и подчеркивание -> 0 или 1
        // Второй узел: буквы, цифры, подчеркивание -> 1 (финальный)
        FST::RELATION* rels = new FST::RELATION[63]; // 26*2 + 10 + 1 = 63
        int idx = 0;

        // Маленькие буквы
        for (char c = 'a'; c <= 'z'; c++) {
            rels[idx++] = FST::CreateRelation(c, 0);
        }
        // Большие буквы
        for (char c = 'A'; c <= 'Z'; c++) {
            rels[idx++] = FST::CreateRelation(c, 0);
        }
        // Цифры (только для продолжения, первый символ не может быть цифрой)
        for (char c = '0'; c <= '9'; c++) {
            rels[idx++] = FST::CreateRelation(c, 0);
        }
        // Подчеркивание
        rels[idx++] = FST::CreateRelation('_', 0);

        // Добавляем переходы в финальное состояние
        FST::RELATION* relsFirst = new FST::RELATION[53]; // без цифр для первого символа
        int idx2 = 0;
        for (char c = 'a'; c <= 'z'; c++) {
            relsFirst[idx2++] = FST::CreateRelation(c, 1);
        }
        for (char c = 'A'; c <= 'Z'; c++) {
            relsFirst[idx2++] = FST::CreateRelation(c, 1);
        }
        relsFirst[idx2++] = FST::CreateRelation('_', 1);

        // Проверка вручную - FST не подходит для такого случая
        // Используем простую проверку
        delete[] rels;
        delete[] relsFirst;

        char c = word[0];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'))
            return false;

        for (int i = 1; word[i] != CHAR_NULL; i++) {
            c = word[i];
            if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                  (c >= '0' && c <= '9') || c == '_'))
                return false;
        }
        return true;
    }

    // ===========================================================
    // FST-функция для целочисленных литералов: [0-9]+
    // ===========================================================

    bool fstIntegerLiteral(const char* word) {
        if (word[0] == CHAR_NULL) return false;

        FST::FST fst;
        fst.string = word;
        fst.nodeCount = 2;
        fst.nodes = new FST::NODE[2]{
            FST::CreateNode(20,
                FST::CreateRelation('0', 0), FST::CreateRelation('1', 0),
                FST::CreateRelation('2', 0), FST::CreateRelation('3', 0),
                FST::CreateRelation('4', 0), FST::CreateRelation('5', 0),
                FST::CreateRelation('6', 0), FST::CreateRelation('7', 0),
                FST::CreateRelation('8', 0), FST::CreateRelation('9', 0),
                FST::CreateRelation('0', 1), FST::CreateRelation('1', 1),
                FST::CreateRelation('2', 1), FST::CreateRelation('3', 1),
                FST::CreateRelation('4', 1), FST::CreateRelation('5', 1),
                FST::CreateRelation('6', 1), FST::CreateRelation('7', 1),
                FST::CreateRelation('8', 1), FST::CreateRelation('9', 1)),
            FST::CreateNode(0)
        };
        bool result = FST::Execute(fst);
        delete[] fst.nodes;
        return result;
    }

    // ===========================================================
    // FST-функция для двоичных литералов: 0b[01]+
    // ===========================================================

    bool fstBinaryLiteral(const char* word) {
        if (word[0] != '0' || (word[1] != 'b' && word[1] != 'B')) return false;
        if (word[2] == CHAR_NULL) return false;

        for (int i = 2; word[i] != CHAR_NULL; i++) {
            if (word[i] != '0' && word[i] != '1') return false;
        }
        return true;
    }

    // ===========================================================
    // Структура флагов для отслеживания состояния анализа
    // ===========================================================

    struct LexerFlags {
        bool inFunction = false;      // Внутри объявления функции
        bool inParams = false;        // Внутри параметров ()
        bool afterType = false;       // После типа данных
        bool afterFunction = false;   // После ключевого слова function

        void reset() {
            inFunction = false;
            inParams = false;
            afterType = false;
            afterFunction = false;
        }
    };

    // ===========================================================
    // Глобальные переменные состояния
    // ===========================================================

    static int currentLine = 1;
    static int currentCol = 1;       // Позиция в текущей строке
    static int lineStartPos = 0;     // Позиция начала текущей строки в тексте
    static char currentScope[IT_STR_MAXSIZE] = "";
    static int literalCount = 0;
    static LexerFlags flags;

    // ===========================================================
    // Вспомогательные функции
    // ===========================================================

    inline bool IsSpace(char c) {
        return c == CHAR_SPACE || c == CHAR_TAB || c == CHAR_RETURN || c == CHAR_NEWLINE;
    }

    int ReadWord(const char* text, int pos, char* word) {
        int i = 0;
        char c = text[pos];
        while ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
               (c >= '0' && c <= '9') || c == '_') {
            if (i < WORD_MAX_SIZE - 1) word[i++] = text[pos++];
            else pos++;
            c = text[pos];
        }
        word[i] = CHAR_NULL;
        return pos;
    }

    int ReadNumber(const char* text, int pos, char* word, bool& isBinary) {
        int i = 0;
        isBinary = (text[pos] == '0' && (text[pos + 1] == 'b' || text[pos + 1] == 'B'));

        if (isBinary) {
            word[i++] = text[pos++]; // '0'
            word[i++] = text[pos++]; // 'b'
        }

        while (isBinary ? (text[pos] == '0' || text[pos] == '1') :
                          (text[pos] >= '0' && text[pos] <= '9')) {
            if (i < WORD_MAX_SIZE - 1) word[i++] = text[pos++];
            else pos++;
        }
        word[i] = CHAR_NULL;
        return pos;
    }

    // Возвращает позицию после строки, hasNonAscii устанавливается в true если найдены не-ASCII символы
    int ReadString(const char* text, int pos, char* word, bool& hasNonAscii) {
        hasNonAscii = false;
        char quote = text[pos++];
        int i = 0;
        while (text[pos] != quote && text[pos] != CHAR_NULL && text[pos] != CHAR_NEWLINE) {
            unsigned char c = (unsigned char)text[pos];
            if (c > 127) {
                hasNonAscii = true;  // Кириллица или другие не-ASCII символы
            }
            if (i < WORD_MAX_SIZE - 1) word[i++] = text[pos++];
            else pos++;
        }
        word[i] = CHAR_NULL;
        if (text[pos] == quote) pos++;
        return pos;
    }

    int ReadOperator(const char* text, int pos, char* word) {
        int i = 0;
        char c = text[pos];
        // Операторы: + - * / % = < > ! ~
        if (c == '+' || c == '-') {
            word[i++] = text[pos++];
            if (text[pos] == c) word[i++] = text[pos++]; // ++ или --
        } else if (c == '<' || c == '>' || c == '=' || c == '!') {
            word[i++] = text[pos++];
            if (text[pos] == '=') word[i++] = text[pos++]; // <= >= == !=
        } else if (c == '*' || c == '/' || c == '%' || c == '~') {
            word[i++] = text[pos++];
        }
        word[i] = CHAR_NULL;
        return pos;
    }

    int BinaryToDecimal(const char* binary) {
        int result = 0;
        // Пропускаем префикс 0b
        int start = (binary[0] == '0' && (binary[1] == 'b' || binary[1] == 'B')) ? 2 : 0;
        for (int i = start; binary[i]; i++)
            result = result * 2 + (binary[i] - '0');
        return result;
    }

    int FindBuiltinFunc(IT::IdTable& it, const char* name) {
        for (int i = 0; i < it.size; i++)
            if (it.table[i].idtype == IT_F && strcmp(it.table[i].id, name) == 0)
                return i;
        return NOT_FOUND;
    }

    // ===========================================================
    // Вспомогательные функции добавления токенов
    // ===========================================================

    void AddLexeme(LT::LexTable& lt, char lexType, int line, int col, int idxTI = LT_TI_NULLIDX, char op = CHAR_NULL) {
        LT::Add(lt, LT::CreateEntry(lexType, line, col, idxTI, op));
    }

    void AddIdentifier(LT::LexTable& lt, IT::IdTable& it, const char* word,
                       IDDATATYPE dataType, IDTYPE idType, int line, int col, const char* scope) {
        IT::Add(it, IT::CreateEntry(word, scope, dataType, idType, line));
        AddLexeme(lt, LEX_ID, line, col, it.size - 1);
    }

    void AddLiteral(LT::LexTable& lt, IT::IdTable& it, const char* word,
                    IDDATATYPE dataType, int line, int col, const char* scope, int intVal, char chrVal, const char* strVal) {
        char litName[32];
        sprintf_s(litName, "L%d", literalCount++);

        IT::Entry ie = IT::CreateEntry(litName, scope, dataType, IT_L, line);

        if (dataType == IT_INT) {
            ie.value.vint = intVal;
        } else if (dataType == IT_CHR) {
            ie.value.vchr = chrVal;
        } else if (dataType == IT_STR) {
            ie.value.vstr.len = (unsigned char)strlen(strVal);
            strcpy_s(ie.value.vstr.str, strVal);
        }

        IT::Add(it, ie);
        AddLexeme(lt, LEX_LITERAL, line, col, it.size - 1);
    }

    // ===========================================================
    // Главная функция FST-анализа
    // ===========================================================

    LEXRESULT Analyze(Input::INDATA input)
    {
        LEXRESULT result;
        result.lextable = LT::Create(LT_MAXSIZE - 1);
        result.idtable = IT::Create(IT_MAXSIZE - 1);
        result.errors = 0;

        // Сброс состояния
        currentLine = 1;
        currentCol = 1;
        lineStartPos = 0;
        currentScope[0] = CHAR_NULL;
        literalCount = 0;
        flags.reset();

        // Добавление стандартных функций из библиотеки
        // strcmp - сравнение строк
        {
            IT::Entry entry = IT::CreateEntry(STDLIB_STRCMP, "", IT_INT, IT_F, 0);
            entry.paramCount = 2;
            IT::Add(result.idtable, entry);
        }
        // strcpy - копирование строк
        {
            IT::Entry entry = IT::CreateEntry(STDLIB_STRCPY, "", IT_INT, IT_F, 0);
            entry.paramCount = 2;
            IT::Add(result.idtable, entry);
        }
        cout << "    [StdLib] Added builtin functions: strcmp, strcpy" << endl;

        const char* text = input.text;
        int pos = 0, len = input.size;
        char word[WORD_MAX_SIZE];
        IDDATATYPE lastType = IT_INT;

        // ===========================================================
        // Основной цикл FST-анализа
        // ===========================================================

        while (pos < len)
        {
            // Пропуск пробелов
            while (pos < len && IsSpace(text[pos])) {
                if (text[pos] == CHAR_NEWLINE) {
                    currentLine++;
                    lineStartPos = pos + 1;
                }
                pos++;
            }
            if (pos >= len) break;

            // Вычисляем текущую колонку (позицию в строке)
            currentCol = pos - lineStartPos + 1;

            char c = text[pos];
            int tokenStartCol = currentCol;  // Запоминаем позицию начала токена
            int tokenEndCol;                 // Позиция конца токена (для ошибок)

            // -----------------------------------------------------------
            // FST: Идентификаторы и ключевые слова
            // -----------------------------------------------------------
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
            {
                pos = ReadWord(text, pos, word);
                tokenEndCol = pos - lineStartPos + 1;  // Позиция после токена (1-based)

                // Проверяем ключевые слова через FST
                if (fstInteger(word)) {
                    AddLexeme(result.lextable, LEX_TYPE, currentLine, tokenEndCol);
                    lastType = IT_INT;
                    flags.afterType = true;
                }
                else if (fstChar(word)) {
                    AddLexeme(result.lextable, LEX_TYPE, currentLine, tokenEndCol);
                    lastType = IT_CHR;
                    flags.afterType = true;
                }
                else if (fstFunction(word)) {
                    AddLexeme(result.lextable, LEX_FUNCTION, currentLine, tokenEndCol);
                    flags.afterFunction = true;
                }
                else if (fstDeclare(word)) {
                    AddLexeme(result.lextable, LEX_DECLARE, currentLine, tokenEndCol);
                }
                else if (fstMain(word)) {
                    AddLexeme(result.lextable, LEX_MAIN, currentLine, tokenEndCol);
                    strcpy_s(currentScope, "main");
                }
                else if (fstReturn(word)) {
                    AddLexeme(result.lextable, LEX_RETURN, currentLine, tokenEndCol);
                }
                else if (fstOutput(word)) {
                    AddLexeme(result.lextable, LEX_OUTPUT, currentLine, tokenEndCol);
                }
                else if (fstIf(word)) {
                    AddLexeme(result.lextable, LEX_IF, currentLine, tokenEndCol);
                }
                else if (fstElse(word)) {
                    AddLexeme(result.lextable, LEX_ELSE, currentLine, tokenEndCol);
                }
                // Встроенные функции
                else if (StdLib::IsBuiltinFunction(word)) {
                    int idx = FindBuiltinFunc(result.idtable, word);
                    if (idx >= 0)
                        AddLexeme(result.lextable, LEX_ID, currentLine, tokenEndCol, idx);
                }
                // Идентификатор
                else if (fstIdentifier(word)) {
                    int idx = IT::IsId(result.idtable, word, currentScope);
                    if (idx == NOT_FOUND) idx = IT::IsId(result.idtable, word, "");

                    if (idx == NOT_FOUND) {
                        IDTYPE itype = IT_V;

                        if (flags.afterFunction && !flags.inParams) {
                            itype = IT_F;
                            strcpy_s(currentScope, word);
                            flags.afterFunction = false;
                        } else if (flags.inParams) {
                            itype = IT_P;
                        }

                        IT::Add(result.idtable, IT::CreateEntry(word, currentScope, lastType, itype, currentLine));
                        idx = result.idtable.size - 1;
                        flags.afterType = false;
                    }
                    AddLexeme(result.lextable, LEX_ID, currentLine, tokenEndCol, idx);
                }
                continue;
            }

            // -----------------------------------------------------------
            // FST: Числовые литералы
            // -----------------------------------------------------------
            if (c >= '0' && c <= '9')
            {
                bool isBinary;
                pos = ReadNumber(text, pos, word, isBinary);
                tokenEndCol = pos - lineStartPos + 1;  // Позиция после токена (1-based)

                int value;
                if (isBinary && fstBinaryLiteral(word)) {
                    value = BinaryToDecimal(word);
                } else if (fstIntegerLiteral(word)) {
                    value = atoi(word);
                } else {
                    value = atoi(word);
                }

                AddLiteral(result.lextable, result.idtable, word, IT_INT,
                          currentLine, tokenEndCol, currentScope, value, 0, "");
                continue;
            }

            // -----------------------------------------------------------
            // FST: Строковые литералы
            // -----------------------------------------------------------
            if (c == CHAR_QUOTE || c == CHAR_DQUOTE)
            {
                bool hasNonAscii = false;
                pos = ReadString(text, pos, word, hasNonAscii);
                tokenEndCol = pos - lineStartPos + 1;  // Позиция после токена (1-based)

                // Кириллица и другие не-ASCII символы разрешены в строках
                // Конвертация UTF-8 -> CP866 происходит в генераторе кода

                int strLen = (int)strlen(word);

                if (strLen == 1) {
                    AddLiteral(result.lextable, result.idtable, word, IT_CHR,
                              currentLine, tokenEndCol, currentScope, 0, word[0], "");
                } else {
                    AddLiteral(result.lextable, result.idtable, word, IT_STR,
                              currentLine, tokenEndCol, currentScope, 0, 0, word);
                }
                continue;
            }

            // -----------------------------------------------------------
            // FST: Операторы (через таблицу диспетчеризации)
            // -----------------------------------------------------------
            if (strchr("+-*/%=<>!~", c))
            {
                pos = ReadOperator(text, pos, word);
                tokenEndCol = pos - lineStartPos + 1;  // Позиция после токена (1-based)

                // Таблица операторов: {pattern, lexType, opChar}
                static const struct { const char* pat; char lex; char op; } operators[] = {
                    {"++", LEX_UNARY,   '+'},  {"--", LEX_UNARY,   '-'},  {"~",  LEX_UNARY,   '~'},
                    {"<=", LEX_COMPARE, '<'},  {">=", LEX_COMPARE, '>'},  {"==", LEX_COMPARE, '='},
                    {"!=", LEX_COMPARE, '!'},  {"<",  LEX_COMPARE, '<'},  {">",  LEX_COMPARE, '>'},
                    {"=",  LEX_ASSIGN,  '\0'}, {"+",  LEX_BINOP,   '+'},  {"-",  LEX_BINOP,   '-'},
                    {"*",  LEX_BINOP,   '*'},  {"/",  LEX_BINOP,   '/'},  {"%",  LEX_BINOP,   '%'},
                    {nullptr, 0, 0}
                };

                bool found = false;
                for (int i = 0; operators[i].pat; i++) {
                    if (strcmp(word, operators[i].pat) == 0) {
                        AddLexeme(result.lextable, operators[i].lex, currentLine, tokenEndCol, LT_TI_NULLIDX, operators[i].op);
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    // Неизвестный оператор (например, одиночный '!')
                    throw ERROR_THROW_IN(111, currentLine, tokenStartCol);
                }
                continue;
            }

            // -----------------------------------------------------------
            // FST: Разделители (через таблицу диспетчеризации)
            // -----------------------------------------------------------
            if (strchr(";,{}()", c))
            {
                // Таблица разделителей: {symbol, lexType, flagAction}
                // flagAction: 0=none, 1=setInParams, -1=clearInParams
                static const struct { char sym; char lex; int flagAction; } separators[] = {
                    {';', LEX_SEMICOLON,  0},  {',', LEX_COMMA,      0},
                    {'{', LEX_LEFTBRACE,  0},  {'}', LEX_RIGHTBRACE, 0},
                    {'(', LEX_LEFTHESIS,  1},  {')', LEX_RIGHTHESIS, -1},
                    {0, 0, 0}
                };

                for (int i = 0; separators[i].sym; i++) {
                    if (c == separators[i].sym) {
                        AddLexeme(result.lextable, separators[i].lex, currentLine, tokenStartCol + 1);  // Для одиночных символов +1
                        if (separators[i].flagAction == 1) flags.inParams = true;
                        else if (separators[i].flagAction == -1) flags.inParams = false;
                        break;
                    }
                }
                pos++;
                continue;
            }

            // Неизвестный символ
            cout << "Warning: unknown character '" << c << "' at line " << currentLine << endl;
            result.errors++;
            pos++;
        }

        return result;
    }
};
