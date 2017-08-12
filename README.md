# DSL-Lisp
A lightweight and extensible Lisp script framework for DSL (Domain-Specific Language) development.

DSL-Lisp is a tiny interpreter that evaluates the lisp expression. It's easy to embed the framework into other projects. Besides, there is a generic `stream layout` inside the framework, so it can read codes from arbitrary media: RAM, disk, network and so on if the interface is implemented accordingly.

It's based on `scheme-lisp`, which is a dialect of lisp. Scheme is different from common-lisp, concise and efficient on some particular occasions.

We have implemented some primitive functions of schema, there followed a list.

```scheme
;; primitive-funcs
set!            ; (set! [target] [value])
set-car!        ; (set-car! [target] [value])
set-cdr!        ; (set-cdr! [target] [value])
define          ; (define [variable] [inital-value])
lambda          ; (lambda (param0, param1, ..., paramN) list)
if              ; (if [predicate] [then-value] [else-value])
begin           ; (begin [list0] [list1] .. [listN])
cond            ; (cond ([predicate0] [clause0]) ([predicate1] [clause1]) ... ([else] [else_clause]) )
cons            ; (cons [value0] [value1])
car             ; (car [list])
cdr             ; (cdr [list])
quote           ; (quote [expression])
display         ; (display [value])
eval            ; (eval [expression])
append          ; (append [list1] [list2])
;; extened
print           ; (print [value])

;; predicates
boolean?        ; (boolean?[value])
number?         ; (number? [value])
char?           ; (char? [value])
string?         ; (string? [value])
+               ; (+ [operand0] [operand1])
-               ; (- [operand0] [operand1])
*               ; (* [operand0] [operand1])
/               ; (/ [operand0] [operand1])
=               ; (= [operand0] [operand1])
>               ; (> [operand0] [operand1])
<               ; (< [operand0] [operand1])
>=              ; (>= [operand0] [operand1])
<=              ; (<= [operand0] [operand1])
```

This project is still in a very alpha stage, I'm planing to implement the `garbage collection` and `macro system` before long, and there may be also some bugs in it.

## Build this project:
Requirement:
GNU developing environment(such as Linux with GCC. Windows with MinGW, CygWin, etc.)

Do the configuration in Makefile, set the variable named CONFIG_* as "y" (yes) or "n" (no)

```
make all
make install
```

you will find an executable file and an archive file that stores static objects.

## example

```scheme
(

(define x
    (lambda (n)
        (cond ((= n 0) "is zero")
              ((= n 1) "is one")
              (else "else."))))

(begin
    ;
    ; call the procedure
    ;
    (x 0)

    ;
    ; string
    ;
    (display "Hello world!")

    ;
    ; list
    ;
    (display
        (cdr
            (quote (1 .2 .3 1.4 2.5 6 7))))

    ;
    ; float operation
    ;
    (display (- (* 2 (+ 3.14 6.28 12.56)) 3))
)

)
```
