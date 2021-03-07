;;;; shell.scm


(require-extension extras posix srfi-17 bb)

(bb:init)

(define width 300)
(define height 150)

(define w (bb:make-widget 'window width height))

(define str #<<EOF
LLLLLLLLLLLLLJJJJJJJ
LLLLLLLLLLLLLJJJJJJJ
LLLLLLLLLLLLLJJJJJJJ
EEEEEEEEEEEEEEEEEEEE
EOF
)

(define widgets
  (bb:group
   w
   (lambda ()
     (bb:make-widgets str width height) ) ) )

(define lst (cdr (assq #\L widgets)))
(define entry (cdr (assq #\E widgets)))

(set! (bb:property entry 'callback)
  (lambda ()
    (let ([cmd (bb:property entry 'text)])
      (bb:add! lst (string-append "@i@." cmd))
      (set! (bb:property entry 'text) "")
      (with-input-from-pipe 
       cmd
       (lambda ()
	 (let loop ()
	   (let ([ln (read-line)])
	     (unless (eof-object? ln)
	       (bb:add! lst (string-append "@." ln)) 
	       (loop) ) ) ) ) ) ) ) )

(bb:show w)
(bb:run)
