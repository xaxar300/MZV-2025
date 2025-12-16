// Parm.cpp - реализация параметров командной строки MZV-2025
#include "stdafx.h"

namespace Parm
{
    PARM getparm(int argc, _TCHAR* argv[])
    {
        PARM p;
        wcscpy_s(p.in, L"");
        wcscpy_s(p.out, L"out.txt");
        wcscpy_s(p.log, L"log.txt");
        p.optLevel = 1;  // По умолчанию -O1 (peephole оптимизации)

        bool hasIn = false;

        for (int i = 1; i < argc; i++)
        {
            if (wcsstr(argv[i], L"-in:") == argv[i])
            {
                wcscpy_s(p.in, argv[i] + 4);
                hasIn = true;
            }
            else if (wcsstr(argv[i], L"-out:") == argv[i])
            {
                wcscpy_s(p.out, argv[i] + 5);
            }
            else if (wcsstr(argv[i], L"-log:") == argv[i])
            {
                wcscpy_s(p.log, argv[i] + 5);
            }
            else if (wcscmp(argv[i], L"-O0") == 0)
            {
                p.optLevel = 0;
            }
            else if (wcscmp(argv[i], L"-O1") == 0)
            {
                p.optLevel = 1;
            }
            else if (wcscmp(argv[i], L"-O2") == 0)
            {
                p.optLevel = 2;
            }
        }

        if (!hasIn)
            throw ERROR_THROW(2);

        return p;
    }
};
