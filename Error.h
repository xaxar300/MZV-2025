// Error.h - обработка ошибок MZV-2025
#pragma once

#define ERROR_THROW(id) Error::geterror(id)
#define ERROR_THROW_IN(id, line, col) Error::geterrorin(id, line, col)
#define ERROR_ENTRY(id, m) {id, m, {-1, -1}}
#define ERROR_MAXSIZE_MESSAGE 200
#define ERROR_ENTRY_NODEF(id) ERROR_ENTRY(-(id), "Неопределённая ошибка")

// ERROR_ENTRY_NODEF10(id) - 10 неопределённых элементов таблицы ошибок
#define ERROR_ENTRY_NODEF10(id) ERROR_ENTRY_NODEF((id)+0), ERROR_ENTRY_NODEF((id)+1), ERROR_ENTRY_NODEF((id)+2), ERROR_ENTRY_NODEF((id)+3), \
                                ERROR_ENTRY_NODEF((id)+4), ERROR_ENTRY_NODEF((id)+5), ERROR_ENTRY_NODEF((id)+6), ERROR_ENTRY_NODEF((id)+7), \
                                ERROR_ENTRY_NODEF((id)+8), ERROR_ENTRY_NODEF((id)+9)

// ERROR_ENTRY_NODEF100(id) - 100 неопределённых элементов таблицы ошибок
#define ERROR_ENTRY_NODEF100(id) ERROR_ENTRY_NODEF10((id)+0), ERROR_ENTRY_NODEF10((id)+10), ERROR_ENTRY_NODEF10((id)+20), ERROR_ENTRY_NODEF10((id)+30), \
                                 ERROR_ENTRY_NODEF10((id)+40), ERROR_ENTRY_NODEF10((id)+50), ERROR_ENTRY_NODEF10((id)+60), ERROR_ENTRY_NODEF10((id)+70), \
                                 ERROR_ENTRY_NODEF10((id)+80), ERROR_ENTRY_NODEF10((id)+90)

#define ERROR_MAX_ENTRY 1000

namespace Error
{
    struct ERRORDATA
    {
        int id;
        char message[ERROR_MAXSIZE_MESSAGE];
        struct IN
        {
            short line;
            short col;
        } inext;
    };

    // Коды синтаксических ошибок (600-699)
    enum SYNTAX_ERROR
    {
        ERR_SYNTAX_PROGRAM = 600,
        ERR_SYNTAX_FUNCSEQ = 601,
        ERR_SYNTAX_PARAMS = 602,
        ERR_SYNTAX_BODY = 603,
        ERR_SYNTAX_EXPR = 604,
        ERR_SYNTAX_EXPRTAIL = 605,
        ERR_SYNTAX_ARGS = 606,
        ERR_SYNTAX_COND = 607
    };

    // Коды семантических ошибок (700-799)
    enum SEMANTIC_ERROR
    {
        ERR_SEM_GENERAL = 700,
        ERR_SEM_UNDECLARED = 701,
        ERR_SEM_REDECLARED = 702,
        ERR_SEM_TYPE_MISMATCH = 703,
        ERR_SEM_NO_MAIN = 704,
        ERR_SEM_PARAM_COUNT = 705,
        ERR_SEM_PARAM_TYPE = 706,
        ERR_SEM_RETURN_TYPE = 707,
        ERR_SEM_INVALID_OPERATION = 708,
        ERR_SEM_ASSIGN_TYPE = 709,
        ERR_SEM_COND_TYPE = 710,
        ERR_SEM_OUTPUT_TYPE = 711,
        ERR_SEM_UNARY_TYPE = 712,
        ERR_SEM_UNDEFINED_VAR = 713,
        ERR_SEM_UNDEFINED_FUNC = 714,
        ERR_SEM_SCOPE_ERROR = 715,
        ERR_SEM_ARITH_TYPE = 716,
        ERR_SEM_COMPARE_TYPE = 717
    };

    ERRORDATA geterror(int id);
    ERRORDATA geterrorin(int id, int line, int col);
}