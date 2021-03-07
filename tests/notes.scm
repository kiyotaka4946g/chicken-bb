;;;; notes.scm

(use utils srfi-17 bb)

(bb:init)

(define (switch-note)
  (let ([item (bb:property item-list 'value)])
    (when (> item 0)
      (let ([note (list-ref all-notes (sub1 item))])
	(note-hidden?-set! note #f)
	(bb:show (note-window note)) ) ) ) )

(bb:render
 `(window (@ (width 200) (height 300) (resizable) (id w) (resizable-widget lst))
	  (menu-bar (@ (height 30) (id m)))
	  (list (@ (y 30) (width 200) (height 270) (id lst) (callback switch-note))) ) )

(define item-list (bb:find-widget 'lst))
(define menu-bar (bb:find-widget 'm))
(define notes-window (bb:find-widget 'w))
(define default-color 'yellow)
(define all-notes '())

(define-record note window edit title color hidden?)

(define (note-hider note)
  (lambda ()
    (note-hidden?-set! note #t)
    (set! (bb:property (note-window note) 'visible) #f) ) )

(define (add-note . title)
  (let* ([name (optional title (->string (gensym 'note)))]
	 [x (random 600)]
	 [y (random 400)]
	 [w (bb:make-widget 'window x y 200 150)] 
	 [e (bb:group w (lambda () (bb:make-widget 'edit 0 0 200 150)))] 
	 [note (make-note w e name default-color #f)] )
    (set! (bb:property w 'resizable) #t)
    (set! (bb:property w 'resizable-widget) e)
    (set! (bb:property e 'color) default-color)
    (set! (bb:property w 'callback) (note-hider note))
    (bb:show w)
    (bb:add! item-list (sprintf "@.~A" name))
    (set! all-notes (append all-notes (list note))) ) )

(define (choose-color)
  (and-let* ([col (bb:select-color default-color)])
    (set! default-color col) ) )

(define notesfile (make-pathname (get-environment-variable "HOME") ".bbnotes"))

(define (load-notes . file)
  (with-input-from-file (optional file notesfile)
    (lambda ()
      (match (read)
	[(x y w h)
	 (set! (bb:property notes-window 'x) x)
	 (set! (bb:property notes-window 'y) y)
	 (set! (bb:property notes-window 'width) w)
	 (set! (bb:property notes-window 'height) h) ] )
      (let loop ()
	(match (read)
	  [(? eof-object?) #f]
	  [(title color pos text hidden?)
	   (let* ([w (apply bb:make-widget 'window pos)]
		  [e (bb:group w (lambda () (apply bb:make-widget 'edit 0 0 (cddr pos))))] 
		  [note (make-note w e title color hidden?)] )
	     (set! (bb:property e 'color) color)
	     (set! (bb:property w 'resizable) #t)
	     (set! (bb:property w 'resizable-widget) e)
	     (set! (bb:property w 'text) title)
	     (set! (bb:property w 'callback) (note-hider note))
	     (bb:add! e text)
	     (unless hidden? (bb:show w))
	     (set! all-notes (append all-notes (list note)))
	     (bb:add! item-list (string-append "@." title))
	     (loop) ) ] ) ) ) ) )

(define (save-notes . file)
  (with-output-to-file (optional file notesfile)
    (lambda ()
      (write (map (cut bb:property notes-window <>) '(x y width height)))
      (newline)
      (for-each
       (lambda (note)
	 (let ([w (note-window note)]
	       [e (note-edit note)] )
	   (write 
	    (list 
	     (note-title note)
	     (note-color note)
	     (list (bb:property w 'x) (bb:property w 'y) (bb:property w 'width) (bb:property w 'height))
	     (bb:property e 'text) 
	     (note-hidden? note) ) )
	   (newline) ) )
       all-notes) ) ) )

(define (change-title)
  (let ([i (bb:property item-list 'value)])
    (when (> i 0)
      (let* ([note (list-ref all-notes (sub1 i))]
	     [title (bb:get-input "Enter new title" (note-title note))] )
	(set! (bb:property (note-window note) 'text) title)
	(note-title-set! note title) 
	(update-list) ) ) ) )

(define (update-list)
  (bb:remove! item-list #t)
  (for-each (lambda (note) (bb:add! item-list (string-append "@." (note-title note)))) all-notes) )

(define (change-color)
  (let ([i (bb:property item-list 'value)])
    (when (> i 0)
      (let* ([note (list-ref all-notes (sub1 i))]
	     [col (bb:select-color (note-color note))] )
	(set! (bb:property (note-edit note) 'color) col)
	(bb:redraw (note-edit note))
	(note-color-set! note col) ) ) ) )

(define (fini) 
  (save-notes)
  (exit) )

(set! (bb:property notes-window 'callback) fini) ; when ESC is pressed

(bb:add! menu-bar "File/Quit" fini "^q")
(bb:add! menu-bar "Notes/New" add-note "^n")
(bb:add! menu-bar "Notes/Set default color..." choose-color)
(bb:add! menu-bar "Change/Title..." change-title)
(bb:add! menu-bar "Change/Color..." change-color)

(bb:show notes-window)

(when (file-exists? notesfile) (load-notes))

(bb:run)
