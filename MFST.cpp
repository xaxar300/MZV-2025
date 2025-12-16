// MFST.cpp - Реализация магазинного автомата для MZV-2025
#include "stdafx.h"

namespace MFST
{
    // ============== MfstState ==============
    MfstState::MfstState()
        : lenta_position(0), nrule(-1), nrulechain(-1) {
    }

    MfstState::MfstState(short pos, StackAccess<char> s, short rule, short chain)
        : lenta_position(pos), st(s), nrule(rule), nrulechain(chain) {
    }

    // ============== MfstDiagnosis ==============
    MfstDiagnosis::MfstDiagnosis()
        : lenta_position(-1), nrule(-1), nrulechain(-1) {
    }

    MfstDiagnosis::MfstDiagnosis(short pos, short rule, short chain)
        : lenta_position(pos), nrule(rule), nrulechain(chain) {
    }

    // ============== Deduction ==============
    Deduction::Deduction() : size(0)
    {
        nrules = new short[1000];
        nrulechains = new short[1000];
    }

    Deduction::~Deduction()
    {
        delete[] nrules;
        delete[] nrulechains;
    }

    // ============== Mfst ==============
    Mfst::Mfst()
        : lextable(nullptr), lenta(nullptr),
        lenta_size(0), lenta_position(0), nrule(-1), nrulechain(-1),
        diagnosis_count(0), traceFile(nullptr), stepCounter(1), traceEnabled(true) {
    }

    Mfst::Mfst(LT::LexTable& lt, GRB::Greibach gr)
        : lextable(&lt), greibach(gr), lenta_position(0),
        nrule(-1), nrulechain(-1), diagnosis_count(0), traceFile(nullptr), stepCounter(1), traceEnabled(true)
    {
        // Формируем ленту из таблицы лексем
        // lenta_size включает все лексемы + маркер конца ($)
        lenta_size = static_cast<short>(lt.size + 1);
        // Выделяем: lenta_size символов + 1 для \0
        lenta = new char[static_cast<size_t>(lenta_size) + 1];

        for (int i = 0; i < lt.size; i++)
            lenta[i] = lt.table[i].lexema;

        lenta[lt.size] = greibach.bottomStack;  // Маркер конца ($)
        lenta[lenta_size] = CHAR_NULL;          // Нуль-терминатор

        // Инициализация стека
        st.push(greibach.bottomStack);
        st.push(greibach.startSymbol);
    }

    Mfst::~Mfst()
    {
        if (lenta != nullptr)
        {
            delete[] lenta;
            lenta = nullptr;
        }
    }

    char* Mfst::getCSt(char* buf)
    {
        // Оптимизированная версия с прямым доступом к контейнеру (из laba18)
        // Вместо копирования стека - прямое чтение из deque
        const auto& container = st.getContainer();
        size_t size = container.size();
        int i = 0;

        // Читаем от вершины (конец) к дну (начало)
        for (size_t j = size; j > 0 && i < 200; j--)
        {
            buf[i++] = container[j - 1];
        }
        buf[i] = CHAR_NULL;
        // Вершина стека слева, дно ($) справа
        return buf;
    }

    char* Mfst::getCLenta(char* buf, short pos, short n)
    {
        int i = 0;
        for (int j = pos; j < lenta_size && i < n; j++, i++)
            buf[i] = lenta[j];
        buf[i] = CHAR_NULL;
        return buf;
    }

    bool Mfst::savestate()
    {
        storestate.push(MfstState(lenta_position, st, nrule, nrulechain));
        return true;
    }

    bool Mfst::reststate()
    {
        if (storestate.empty())
            return false;

        MfstState state = storestate.top();
        storestate.pop();

        lenta_position = state.lenta_position;
        st = state.st;
        nrule = state.nrule;
        nrulechain = state.nrulechain;
        return true;
    }

    bool Mfst::push_chain(GRB::Chain chain)
    {
        for (int i = chain.size - 1; i >= 0; i--)
        {
            if (chain.nt[i] != CHAR_NULL)
                st.push(chain.nt[i]);
        }
        return true;
    }

    bool Mfst::savediagnosis(RC_STEP)
    {
        // Если лимит достигнут, но текущая позиция дальше самой дальней -
        // заменяем самую дальнюю диагностику
        if (diagnosis_count >= MFST_DIAGN_NUMBER)
        {
            // Находим диагностику с максимальной позицией
            int maxIdx = 0;
            for (int i = 1; i < diagnosis_count; i++)
            {
                if (diagnosis[i].lenta_position > diagnosis[maxIdx].lenta_position)
                    maxIdx = i;
            }

            // Если текущая позиция дальше - обновляем
            if (lenta_position > diagnosis[maxIdx].lenta_position)
            {
                diagnosis[maxIdx] = MfstDiagnosis(lenta_position, nrule, nrulechain);
                return true;
            }
            return false;
        }

        int k = 0;
        while (k < diagnosis_count && diagnosis[k].lenta_position <= lenta_position)
            k++;

        if (k > 0 && diagnosis[k - 1].lenta_position == lenta_position)
            return false;

        for (int j = diagnosis_count; j > k; j--)
            diagnosis[j] = diagnosis[j - 1];

        diagnosis[k] = MfstDiagnosis(lenta_position, nrule, nrulechain);
        diagnosis_count++;

        return true;
    }

    bool Mfst::savededucation()
    {
        deduction.nrules[deduction.size] = nrule;
        deduction.nrulechains[deduction.size] = nrulechain;
        deduction.size++;
        return true;
    }

    void Mfst::printStack()
    {
        if (traceFile == nullptr)
            return;
        char buf[256];
        fprintf(traceFile, "Стек: %s\n", getCSt(buf));
    }

    void Mfst::printStep()
    {
        if (traceFile == nullptr)
            return;
        char bufL[64], bufS[256];
        fprintf(traceFile, "Поз:%d Лента:%s Стек:%s\n",
            lenta_position, getCLenta(bufL, lenta_position, 20), getCSt(bufS));
    }

    char* Mfst::getDiagnosis(short n, char* buf)
    {
        if (n < 0 || n >= diagnosis_count)
        {
            buf[0] = CHAR_NULL;
            return buf;
        }

        MfstDiagnosis& d = diagnosis[n];
        int line = 0;
        if (d.lenta_position < lextable->size)
            line = lextable->table[d.lenta_position].line;

        int errCode = GRB_ERROR_SERIES;
        if (d.nrule >= 0 && d.nrule < greibach.ruleCount)
            errCode = greibach.rules[d.nrule].idError;

        sprintf_s(buf, 256, "Строка %d: ошибка %d", line, errCode);
        return buf;
    }

    void Mfst::printrules()
    {
        if (traceFile == nullptr)
            return;

        fprintf(traceFile, "\n=== ДЕРЕВО ВЫВОДА ===\n");
        char buf[256];

        for (int i = 0; i < deduction.size; i++)
        {
            if (deduction.nrules[i] >= 0 && deduction.nrules[i] < greibach.ruleCount)
            {
                greibach.rules[deduction.nrules[i]].getCRule(buf, deduction.nrulechains[i]);
                fprintf(traceFile, "%d. %s\n", i + 1, buf);
            }
        }
    }

    RC_STEP Mfst::step()
    {
        if (lenta_position >= lenta_size)
            return LENTA_END;

        if (st.empty())
            return NS_ERROR;

        char top = st.top();
        char current = lenta[lenta_position];

        // Буферы для вывода
        char bufStack[256];
        char bufTape[64];
        char ruleStr[64] = "";

        // Обработка эпсилон-символа @ (просто удаляем из стека)
        if (top == TS_EPS)
        {
            st.pop();
            return NS_OK;
        }

        // Макрос для условного вывода в лог-файл (используем wide strings для UTF-8)
        #define TRACE_PRINT(...) if (traceEnabled && traceFile) fwprintf(traceFile, __VA_ARGS__)

        // Нетерминал на вершине стека
        if (ISNS(top))
        {
            GRB::Rule rule;
            int ruleIdx = greibach.getRule(top, rule);

            if (ruleIdx < 0)
            {
                // Нет правила для нетерминала
                TRACE_PRINT(L"%-3d : TNS_NORULECHAIN/NS_NORULE\n", stepCounter);

                savediagnosis(NS_NORULE);
                if (reststate())
                {
                    TRACE_PRINT(L"%-3d : RESTATE\n", stepCounter);
                    getCSt(bufStack);
                    getCLenta(bufTape, lenta_position, 25);
                    TRACE_PRINT(L"%-3d : %-25hs %-25hs %hs\n", stepCounter++, "", bufTape, bufStack);
                    return NS_NORULECHAIN;
                }
                return NS_ERROR;
            }

            nrule = ruleIdx;
            int chainIdx = rule.getNextChain(current, nrulechain + 1);

            if (chainIdx >= 0)
            {
                nrulechain = chainIdx;
                GRB::Chain chain = rule.getChain(chainIdx);

                // Формируем строку правила
                sprintf_s(ruleStr, 64, "%c->%s", top, chain.nt);

                // Сохраняем состояние если есть альтернативы
                bool hasAlternatives = (rule.getNextChain(current, chainIdx + 1) >= 0);
                if (hasAlternatives)
                    savestate();

                // Выводим строку с применением правила
                getCSt(bufStack);
                getCLenta(bufTape, lenta_position, 25);
                TRACE_PRINT(L"%-3d : %-25hs %-25hs %hs\n", stepCounter, ruleStr, bufTape, bufStack);

                // Выводим SAVESTATE если было сохранение
                if (hasAlternatives)
                {
                    TRACE_PRINT(L"%-3d : SAVESTATE:             %d\n", stepCounter, (int)storestate.size());
                }

                st.pop();
                push_chain(chain);

                // Выводим новое состояние после применения правила
                getCSt(bufStack);
                getCLenta(bufTape, lenta_position, 25);
                TRACE_PRINT(L"%-3d : %-25hs %-25hs %hs\n", stepCounter++, "", bufTape, bufStack);

                savededucation();

                nrulechain = -1;
                return NS_OK;
            }
            else
            {
                // Проверяем, есть ли эпсилон-правило (цепочка "@")
                for (int i = 0; i < rule.chainCount; i++)
                {
                    if (rule.chains[i].size == 1 && rule.chains[i].nt[0] == TS_EPS)
                    {
                        // Формируем строку правила
                        sprintf_s(ruleStr, 64, "%c->e", top);

                        // Выводим строку таблицы
                        getCSt(bufStack);
                        getCLenta(bufTape, lenta_position, 25);
                        TRACE_PRINT(L"%-3d : %-25hs %-25hs %hs\n", stepCounter, ruleStr, bufTape, bufStack);

                        st.pop();

                        // Выводим новое состояние
                        getCSt(bufStack);
                        TRACE_PRINT(L"%-3d : %-25hs %-25hs %hs\n", stepCounter++, "", bufTape, bufStack);

                        nrulechain = -1;
                        return NS_OK;
                    }
                }

                // Нет подходящей цепочки
                TRACE_PRINT(L"%-3d : TNS_NORULECHAIN/NS_NORULE\n", stepCounter);

                savediagnosis(NS_NORULECHAIN);
                if (reststate())
                {
                    TRACE_PRINT(L"%-3d : RESTATE\n", stepCounter);
                    getCSt(bufStack);
                    getCLenta(bufTape, lenta_position, 25);
                    TRACE_PRINT(L"%-3d : %-25hs %-25hs %hs\n", stepCounter++, "", bufTape, bufStack);
                    return NS_NORULECHAIN;
                }
                return NS_ERROR;
            }
        }
        // Терминал на вершине стека
        else if (top == current)
        {
            // Выводим совпадение терминала (сдвиг) - только лента и стек
            getCSt(bufStack);
            getCLenta(bufTape, lenta_position, 25);
            TRACE_PRINT(L"%-3d : %-25hs %-25hs %hs\n", stepCounter++, "", bufTape, bufStack);

            st.pop();
            lenta_position++;
            nrulechain = -1;
            return TS_OK;
        }
        else
        {
            // Терминал не совпал
            TRACE_PRINT(L"%-3d : TS_NOK/NS_NORULECHAIN\n", stepCounter);

            savediagnosis(TS_NOK);
            if (reststate())
            {
                TRACE_PRINT(L"%-3d : RESTATE\n", stepCounter);
                getCSt(bufStack);
                getCLenta(bufTape, lenta_position, 25);
                TRACE_PRINT(L"%-3d : %-25hs %-25hs %hs\n", stepCounter++, "", bufTape, bufStack);
                return TS_NOK;
            }
            return NS_ERROR;
        }

        #undef TRACE_PRINT
    }

    bool Mfst::start()
    {
        // Макрос для условного вывода в лог-файл (используем wide strings для UTF-8)
        #define TRACE_PRINT(...) if (traceEnabled && traceFile) fwprintf(traceFile, __VA_ARGS__)
        #define TRACE_COUT(x) if (traceEnabled && traceFile) fwprintf(traceFile, L"%s", x)

        TRACE_COUT(L"\n--- Syntax analysis started ---\n");
        if (traceEnabled && traceFile) fwprintf(traceFile, L"Tape: %hs\n\n", lenta);

        // Заголовок таблицы с фиксированной шириной колонок
        TRACE_PRINT(L"--------------------------------------------------------------------------------------\n");
        TRACE_PRINT(L"%-3hs : %-25hs %-25hs %hs\n", "Step", "Rule", "Input tape", "Stack");
        TRACE_PRINT(L"--------------------------------------------------------------------------------------\n");

        // Применяем первое правило для стартового символа
        GRB::Rule startRule;
        int startRuleIdx = greibach.getRule(greibach.startSymbol, startRule);

        if (startRuleIdx >= 0)
        {
            char current = lenta[lenta_position];
            int chainIdx = startRule.getNextChain(current, 0);

            if (chainIdx >= 0)
            {
                nrule = startRuleIdx;
                nrulechain = chainIdx;
                GRB::Chain chain = startRule.getChain(chainIdx);

                // Формируем строку правила
                char ruleStr[64];
                sprintf_s(ruleStr, 64, "%c->%s", greibach.startSymbol, chain.nt);

                // Сохраняем состояние если есть альтернативы
                bool hasAlternatives = (startRule.getNextChain(current, chainIdx + 1) >= 0);
                if (hasAlternatives)
                    savestate();

                // Выводим начальное состояние
                char bufStack[256], bufTape[64];
                getCSt(bufStack);
                getCLenta(bufTape, lenta_position, 25);
                TRACE_PRINT(L"%-3d : %-25hs %-25hs %hs\n", stepCounter, ruleStr, bufTape, bufStack);

                // Выводим SAVESTATE если было сохранение
                if (hasAlternatives)
                {
                    TRACE_PRINT(L"%-3d : SAVESTATE:             %d\n", stepCounter, (int)storestate.size());
                }

                // Применяем правило
                st.pop();  // Удаляем S
                push_chain(chain);
                savededucation();

                // Выводим новое состояние
                getCSt(bufStack);
                getCLenta(bufTape, lenta_position, 25);
                TRACE_PRINT(L"%-3d : %-25hs %-25hs %hs\n", stepCounter++, "", bufTape, bufStack);

                nrulechain = -1;
            }
        }

        RC_STEP rc = SURPRISE;
        int maxSteps = 100000;
        int stepCount = 0;
        bool result = false;

        while (rc != LENTA_END && rc != NS_ERROR && stepCount++ < maxSteps)
        {
            rc = step();

            // Проверка успешного завершения
            if (st.size() == 1 && st.top() == greibach.bottomStack &&
                lenta_position >= lenta_size - 1)
            {
                result = true;
                break;
            }
        }

        TRACE_PRINT(L"--------------------------------------------------------------------------------------\n");

        if (result)
        {
            TRACE_COUT(L"\n--- Parsing successful ---\n");
            TRACE_COUT(L"\nDerivation rules:\n");
            char buf[256];
            for (int i = 0; i < deduction.size; i++)
            {
                if (deduction.nrules[i] >= 0 && deduction.nrules[i] < greibach.ruleCount)
                {
                    greibach.rules[deduction.nrules[i]].getCRule(buf, deduction.nrulechains[i]);
                    if (traceEnabled && traceFile) fwprintf(traceFile, L"  %d. %hs\n", (i + 1), buf);
                }
            }
        }
        else
        {
            TRACE_COUT(L"\n--- Parsing failed ---\n");
            TRACE_COUT(L"Diagnostics:\n");
            char buf[256];
            for (int i = 0; i < diagnosis_count; i++)
                if (traceEnabled && traceFile) fwprintf(traceFile, L"  %hs\n", getDiagnosis(i, buf));
        }

        #undef TRACE_PRINT
        #undef TRACE_COUT

        return result;
    }

    bool Analyze(LT::LexTable& lextable, Log::LOGDATA& log, bool traceEnabled)
    {
        GRB::Greibach gr = GRB::getGreibach();
        Mfst mfst(lextable, gr);
        mfst.traceFile = log.stream;
        mfst.setTraceEnabled(traceEnabled);

        bool result = mfst.start();

        if (!result && mfst.diagnosis_count > 0)
        {
            // Находим диагностику с максимальной позицией на ленте
            // (это место, где парсер продвинулся дальше всего)
            int maxIdx = 0;
            for (int i = 1; i < mfst.diagnosis_count; i++)
            {
                if (mfst.diagnosis[i].lenta_position > mfst.diagnosis[maxIdx].lenta_position)
                    maxIdx = i;
            }

            MfstDiagnosis& d = mfst.diagnosis[maxIdx];
            int line = 1;
            int col = 1;

            // lenta_position указывает на лексему, где парсер не смог продолжить.
            // Ошибка обычно в предыдущей лексеме (например, пропущенная точка с запятой).
            // Поэтому берем позицию ПРЕДЫДУЩЕЙ лексемы для более точной диагностики.
            int errorPos = d.lenta_position;
            if (errorPos > 0) {
                errorPos--;  // Указываем на предыдущую лексему
            }

            if (errorPos < lextable.size)
            {
                line = lextable.table[errorPos].line;
                col = lextable.table[errorPos].col;
            }

            int errCode = GRB_ERROR_SERIES;
            if (d.nrule >= 0)
                errCode = gr.rules[d.nrule].idError;

            throw ERROR_THROW_IN(errCode, line, col);
        }
        return result;
    }
}