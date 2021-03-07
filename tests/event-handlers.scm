;;;; event-handlers.scm


(use srfi-17 bb)

(bb:init)

(define w (bb:make-widget 'window 200 100))
(define e (bb:make-widget 'edit 5 5 150 25))
(set! (bb:property e 'callback) (lambda () (display "edit was changed") (newline) ) )
(set! (bb:property e 'when) 'changed)
(set! (bb:property e 'handler)
  (lambda (e)
    (case e
      [(move)
       (printf "mouse was moved over edit at (~A, ~A)~%" (bb:event 'x) (bb:event 'y))
       #t]
      [(push)
       (printf "mouse was clicked on edit, clicks = 1 + ~A~%" (bb:event 'clicks))
       (set! (bb:event 'clicks) 0)
       (printf "clicks after setting: ~A~%" (bb:event 'clicks))
       #t]
      [(keydown)
       (printf "key was pressed: ~A~%" (bb:event 'key))
       -1]
      [else -1] ) ; pass other events to the base class
    ) )

(bb:show w)
(bb:run)
