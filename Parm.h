// Parm.h - параметры командной строки MZV-2025
#pragma once

#define PARM_MAX_SIZE 300

namespace Parm
{
    struct PARM
    {
        wchar_t in[PARM_MAX_SIZE];
        wchar_t out[PARM_MAX_SIZE];
        wchar_t log[PARM_MAX_SIZE];
        int optLevel;   // Уровень оптимизации: 0, 1, 2
    };

    PARM getparm(int argc, _TCHAR* argv[]);
};
