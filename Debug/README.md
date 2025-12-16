# Синтаксический анализатор MZV-2025

## Грамматика в нормальной форме Грейбаха

```
S -> tfi(F){N};S | tfi(F){N}; | m{N};
N -> dti;N | i=E;N | oE;N | ?(EcE){N}:{N};N | iu;N | rE;
E -> i(W)K | iK | lK | (E)K | uEK
K -> vEK | ; | ) | c | ,
F -> ti,F | ti | )
W -> i,W | l,W | i | l | )
```

## Соответствие терминалов и лексем

| Терминал | Лексема | Описание |
|----------|---------|----------|
| `t` | LEX_TYPE | Тип данных (integer, char) |
| `f` | LEX_FUNCTION | function |
| `i` | LEX_ID | Идентификатор |
| `l` | LEX_LITERAL | Литерал |
| `d` | LEX_DECLARE | declare |
| `m` | LEX_MAIN | main |
| `r` | LEX_RETURN | return |
| `o` | LEX_OUTPUT | output |
| `?` | LEX_IF | if |
| `:` | LEX_ELSE | else |
| `=` | LEX_ASSIGN | Присваивание |
| `v` | LEX_BINOP | +, -, *, /, % |
| `u` | LEX_UNARY | ++, --, ~ |
| `c` | LEX_COMPARE | ==, !=, <, >, <=, >= |

## Файлы

- `stdafx.h` - ЗАМЕНИТЬ (добавлены константы)
- `Error.h/cpp` - ЗАМЕНИТЬ (исправлены макросы)
- `LT.cpp` - ЗАМЕНИТЬ
- `GRB.h/cpp` - ДОБАВИТЬ
- `MFST.h/cpp` - ДОБАВИТЬ
- `main.cpp` - ЗАМЕНИТЬ

## Коды ошибок

| Код | Описание |
|-----|----------|
| 600 | Ошибка структуры программы |
| 602 | Ошибка параметров функции |
| 603 | Ошибка тела функции |
| 604 | Ошибка выражения |
| 605 | Ошибка хвоста выражения |
| 606 | Ошибка аргументов вызова |

## Константы символов

```cpp
#define CHAR_NULL    '\0'
#define CHAR_NEWLINE '\n'
#define CHAR_TAB     '\t'
#define CHAR_SPACE   ' '
```
