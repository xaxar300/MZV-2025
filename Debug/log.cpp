=== MZV-2025 ===
Дата: 10.12.2025 17:53:16

--- Параметры ---
in: full.txt
out: out.txt
log: log.cpp

--- Исходный код (1145 байт., 80 строк) ---
integer function abs(integer x)
{
    declare integer r;
    if (x < 0) {
        r = 0 - x;
    } else {
        r = x;
    };
    return r;
};

integer function max(integer a, integer b)
{
    declare integer r;
    if (a > b) {
        r = a;
    } else {
        r = b;
    };
    return r;
};

integer function calc(integer x, integer y)
{
    declare integer r;
    r = x * y + x - y;
    return r;
};

main
{
    declare integer a;
    declare integer b;
    declare integer c;
    declare integer t;
    declare char ch;

    a = 25;
    b = 0b1010;
    ch = 'X';

    output 'Start';
    output a;
    output b;

    c = a + b * 2;
    output c;

    a++;
    b--;
    c = ~a;

    if (a > b) {
        if (a > 50) {
            output 'big';
        } else {
            output 'medium';
        };
    } else {
        output 'small';
    };

    c = max(a, b);
    output c;

    t = 0 - 15;
    c = abs(t);
    output c;

    c = calc(5, 3);
    output c;

    if (c > 10) {
        output 'ok';
    };

    output ch;
    output 'End';
    return 0;
};


=== Semantic Analysis ===

    Function main found

Semantic analysis completed: 0 error(s), 0 warning(s)
