// GRB.cpp - Реализация грамматики Грейбаха для MZV-2025
#include "stdafx.h"

namespace GRB
{
    // ============== Chain ==============
    Chain::Chain() : size(0) { memset(nt, CHAR_NULL, GRB_MAXSIZE_SYMBOL); }

    Chain::Chain(const char* s)
    {
        size = (short)strlen(s);
        memcpy(nt, s, size);
        nt[size] = CHAR_NULL;
    }

    // ============== Rule ==============
    Rule::Rule() : nn(CHAR_NULL), idError(-1), chainCount(0) {}

    Rule::Rule(char n, int err, int count, ...) : nn(n), idError(err), chainCount(count)
    {
        va_list args;
        va_start(args, count);
        for (int i = 0; i < count; i++)
            chains[i] = Chain(va_arg(args, const char*));
        va_end(args);
    }

    int Rule::getNextChain(char c, int start) const
    {
        for (int i = start; i < chainCount; i++)
            if (chains[i].size > 0 && chains[i].nt[0] == c)
                return i;
        return -1;
    }

    char* Rule::getCRule(char* buf, int nchain) const
    {
        buf[0] = nn; buf[1] = '-'; buf[2] = '>'; buf[3] = CHAR_NULL;
        if (nchain >= 0 && nchain < chainCount)
            strcat_s(buf, 200, chains[nchain].nt);
        return buf;
    }

    // ============== Greibach ==============
    Greibach::Greibach() : ruleCount(0), startSymbol(CHAR_NULL), bottomStack(CHAR_NULL) {}

    Greibach::Greibach(char start, char bottom, int count, ...)
        : startSymbol(start), bottomStack(bottom), ruleCount(count)
    {
        va_list args;
        va_start(args, count);
        for (int i = 0; i < count; i++)
            rules[i] = va_arg(args, Rule);
        va_end(args);
    }

    int Greibach::getRule(char nn, Rule& rule) const
    {
        for (int i = 0; i < ruleCount; i++)
            if (rules[i].nn == nn) { rule = rules[i]; return i; }
        return -1;
    }

    /*
    ========== ГРАММАТИКА MZV-2025 ==========
    S -> tfi(F{N};S | m{N};
    N -> dti;N | iu;N | i=E;N | oE;N | ?(EcE){N}:{N};N | ?(EcE){N};N | rE; | @
    E -> i(WK | iK | lK | (E)K | uEK
    K -> vEK | @
    F -> ti,F | ti) | )
    W -> i,W | l,W | i) | l) | )
    */

    Greibach getGreibach()
    {
        return Greibach(NS_S, TS_END, 6,
            Rule(NS_S, 600, 2, "tfi(F{N};S", "m{N};"),
            Rule(NS_N, 603, 8, "dti;N", "iu;N", "i=E;N", "oE;N",
                "?(EcE){N}:{N};N", "?(EcE){N};N", "rE;", "@"),
            Rule(NS_E, 604, 5, "i(WK", "iK", "lK", "(E)K", "uEK"),
            Rule('K', 605, 2, "vEK", "@"),
            Rule(NS_F, 602, 3, "ti,F", "ti)", ")"),
            Rule(NS_W, 606, 5, "i,W", "l,W", "i)", "l)", ")")
        );
    }
};