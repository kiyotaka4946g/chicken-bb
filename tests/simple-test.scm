;;;; simple-test.scm


(use bb)

(bb:init)

(define w (bb:make-widget 'window 200 100))

(bb:group
 w
 (lambda ()
   (let ([lbl (bb:make-widget 'label 200 100)])
     (set! (bb:property lbl 'box) 'engraved-box)
     (set! (bb:property lbl 'text-size) 32)
     (set! (bb:property lbl 'text-font) 'times-bold-italic)
     (set! (bb:property lbl 'text) "Hello, World") ) ) )

(bb:show w)

;An alternative, simpler way of doing the above:
;
;(bb:render 
; '(window (@ (width 200) (height 100))
;	  (label (@ (box engraved-box) (text-size 32) (text-font times-bold-italic))
;		 "Hello, World")))

(bb:run)
