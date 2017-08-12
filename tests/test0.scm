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
