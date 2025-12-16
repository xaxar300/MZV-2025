// LT.h - ������� ������ MZV-2025
#pragma once

#define LT_MAXSIZE      4096
#define LT_TI_NULLIDX   0xffffffff

// ���� ������
#define LEX_TYPE        't'     // integer, char

// �������� � ��������������
#define LEX_LITERAL     'l'
#define LEX_ID          'i'

// �������� �����
#define LEX_FUNCTION    'f'
#define LEX_DECLARE     'd'
#define LEX_MAIN        'm'
#define LEX_RETURN      'r'

// ��������� MZV-2025
#define LEX_OUTPUT      'o'     // output
#define LEX_IF          '?'     // if
#define LEX_ELSE        ':'     // else

// �����������
#define LEX_SEMICOLON   ';'
#define LEX_COMMA       ','
#define LEX_LEFTBRACE   '{'
#define LEX_RIGHTBRACE  '}'
#define LEX_LEFTHESIS   '('
#define LEX_RIGHTHESIS  ')'

// ��������
#define LEX_ASSIGN      '='
#define LEX_BINOP       'v'     // +, -, *, /, %
#define LEX_UNARY       'u'     // ++, --, ~
#define LEX_COMPARE     'c'     // <, >, <=, >=, ==, !=

namespace LT
{
    struct Entry
    {
        char lexema;
        int  line;
        int  col;       // Позиция (колонка) в строке
        int  idxTI;
        char op;
        int  priority;
    };

    struct LexTable
    {
        int    maxsize;
        int    size;
        Entry* table;
    };

    LexTable Create(int size);
    void Add(LexTable& lt, Entry e);
    Entry GetEntry(LexTable& lt, int idx);
    void Delete(LexTable& lt);
    Entry CreateEntry(char lex, int line, int col = 0, int idxTI = LT_TI_NULLIDX, char op = CHAR_NULL, int pr = 0);
};