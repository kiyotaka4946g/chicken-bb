;;;; bb.scm
;
; Copyright (c) 2000-2004, Felix L. Winkelmann
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following
; conditions are met:
;
;   Redistributions of source code must retain the above copyright notice, this list of conditions and the following
;     disclaimer. 
;   Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following
;     disclaimer in the documentation and/or other materials provided with the distribution. 
;   Neither the name of the author nor the names of its contributors may be used to endorse or promote
;     products derived from this software without specific prior written permission. 
;
; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
; OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
; AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR
; CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
; CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
; SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
; THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
; OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
; POSSIBILITY OF SUCH DAMAGE.
;
; Send bugs, suggestions and ideas to: 
;
; felix@call-with-current-continuation.org
;
; Felix L. Winkelmann
; Steinweg 1A
; 37130 Gleichen, OT Weissenborn
; Germany


(module bb (bb:make-widget
	    bb:make-widgets bb:property bb:widget? bb:message bb:add! bb:remove!
	    bb:get-input bb:init bb:set-menu-item-active!
	    bb:group bb:run bb:redraw bb:show bb:select-file bb:select-directory
	    bb:rgb bb:image bb:select-color
	    bb:event bb:image-data
	    bb:root-element bb:find-element bb:find-widget bb:element-content
	    bb:render bb:element? bb:element-widget bb:element-children bb:element-parent bb:element-id
	    bb:element-tag bb:element-attributes)

(import scheme chicken foreign)

(use extras srfi-1 utils srfi-4 easyffi data-structures matchable)

(foreign-parse/declare #<<EOF

#define BBEXPORT

#ifdef CHICKEN
___declare(substitute, "BB_?;bb:")
#endif

typedef ___scheme_value WIDGET;

#include "bb.h"

void BBSetCallback(WIDGET widget, void *callback);
void *BBAddMenuItem(WIDGET menu, char *label, char *shortcut, void *callback);
int BBAddTreeItem(WIDGET tree, char *text, int parent, int pos, WIDGET widget);
void BBSetHandler(WIDGET widget, void *callback);

EOF
)

(define-record widget type ptr name resources parent children)

(define bb:widget? widget?)

(define-record-printer (widget w port)
  (fprintf port "#<widget ~A>" (##sys#slot w 3)) )

(define bb:init
  (let ([initd #f])
    (lambda scheme
      (unless initd
	(set! initd (bb:Start (optional scheme #f))) )
      initd) ) )

(define widget-ids
  `((window ,bb:WINDOW)
    (glwindow ,bb:GLWINDOW)
    (double-window ,bb:DOUBLEWINDOW)
    (button ,bb:BUTTON)
    (return-button ,bb:RETURNBUTTON)
    (choice-button ,bb:CHOICEBUTTON)
    (menu-button ,bb:MENUBUTTON)
    (check-box ,bb:CHECKBOX)
    (entry ,bb:ENTRY)
    (edit ,bb:EDIT)
    (text-editor ,bb:TEXTEDITOR)
    (int-entry ,bb:INTENTRY)
    (float-entry ,bb:FLOATENTRY)
    (counter ,bb:COUNTER)
    (dial ,bb:DIAL)
    (label ,bb:LABEL)
    (slider ,bb:SLIDER)
    (adjuster ,bb:ADJUSTER)
    (roller ,bb:ROLLER)
    (list ,bb:LIST)
    (radio-button ,bb:RADIOBUTTON)
    (progress ,bb:PROGRESS)
    (tabs ,bb:TABS)
    (tile ,bb:TILE)
    (pack ,bb:PACK)
    (group ,bb:GROUP)
    (scroll ,bb:SCROLL)
    (clock ,bb:CLOCK)
    (light-button ,bb:LIGHTBUTTON)
    (live-image ,bb:LIVEIMAGE)
    (menu-bar ,bb:MENUBAR)
    (tree ,bb:TREE)
    (table ,bb:TABLE)
    (html-view ,bb:HTMLVIEW) ) )

(define property-ids
  `((x ,bb:X)
    (y ,bb:Y)
    (width ,bb:WIDTH)
    (height ,bb:HEIGHT)
    (text ,bb:TEXT)
    (value ,bb:VALUE)
    (box ,bb:BOX)
    (type ,bb:TYPE)
    (resizable ,bb:RESIZABLE)
    (modal ,bb:MODAL)
    (direction ,bb:DIRECTION)
    (color ,bb:COLOR)
    (focus ,bb:FOCUS)
    (callback ,bb:CALLBACK)
    (image ,bb:IMAGE)
    (when ,bb:WHEN)
    (spacing ,bb:SPACING)
    (maximum ,bb:MAXIMUM)
    (minimum ,bb:MINIMUM)
    (label-color ,bb:LABELCOLOR)
    (label-size ,bb:LABELSIZE)
    (label-font ,bb:LABELFONT)
    (x-position ,bb:XPOSITION)
    (y-position ,bb:YPOSITION)
    (text-color ,bb:TEXTCOLOR)
    (text-size ,bb:TEXTSIZE)
    (text-font ,bb:TEXTFONT)
    (selection-color ,bb:SELECTIONCOLOR)
    (tooltip ,bb:TOOLTIP)
    (visible ,bb:VISIBLE)
    (position ,bb:POSITION)
    (read-only ,bb:READONLY)
    (valid-context ,bb:VALIDCONTEXT)
    (mark ,bb:MARK)
    (image-width ,bb:IMAGEWIDTH)
    (image-height ,bb:IMAGEHEIGHT)
    (size ,bb:SIZE)
    (align ,bb:ALIGN)
    (handler ,bb:HANDLER)
    (resizable-widget ,bb:RESIZABLEWIDGET)
    (callback-reason ,bb:CALLBACK_REASON)
    (callback-node ,bb:CALLBACK_NODE) ) )

(define item-ids
  `((root ,bb:ROOT)
    (branch ,bb:BRANCH)
    (leaf ,bb:LEAF)
    (connector ,bb:CONNECTOR) ) )

(define special-bool-properties
  '(resizable modal visible read-only valid-context) )

(define special-int-properties
  '(box type direction when text-font callback-reason align label-font) )

(define special-int-property-values0
  '(no-box scroll-horizontal vertical never helvetica hilighted center) )

(define value-ids
  `((gray0 ,bb:GRAY0)
    (dark3 ,bb:DARK3)
    (dark2 ,bb:DARK2)
    (dark1 ,bb:DARK1)
    (light1 ,bb:LIGHT1)
    (light2 ,bb:LIGHT2)
    (light3 ,bb:LIGHT3)
    (gray ,bb:GRAY)
    (black ,bb:BLACK)
    (red ,bb:RED)
    (green ,bb:GREEN)
    (yellow ,bb:YELLOW)
    (blue ,bb:BLUE)
    (magenta ,bb:MAGENTA)
    (cyan ,bb:CYAN)
    (dark-red ,bb:DARK_RED)
    (dark-green ,bb:DARK_GREEN)
    (dark-yellow ,bb:DARK_YELLOW)
    (dark-blue ,bb:DARK_BLUE)
    (dark-magenta ,bb:DARK_MAGENTA)
    (dark-cyan ,bb:DARK_CYAN)
    (white ,bb:WHITE)
    (scroll-horizontal ,bb:SCROLL_HORIZONTAL)
    (scroll-vertical ,bb:SCROLL_VERTICAL)
    (scroll-both ,bb:SCROLL_BOTH)
    (scroll-always-on ,bb:SCROLL_ALWAYS_ON)
    (scroll-horizontal-always ,bb:SCROLL_HORIZONTAL_ALWAYS)
    (scroll-vertical-always ,bb:SCROLL_VERTICAL_ALWAYS)
    (scroll-both-always ,bb:SCROLL_BOTH_ALWAYS)
    (vertical ,bb:VERTICAL)
    (horizontal ,bb:HORIZONTAL)
    (vertical-fill-slider ,bb:VERT_FILL_SLIDER)
    (horizontal-fill-slider ,bb:HOR_FILL_SLIDER)
    (vertical-nice-slider ,bb:VERT_NICE_SLIDER)
    (horizontal-nice-slider ,bb:HOR_NICE_SLIDER)
    (normal-dial ,bb:NORMAL_DIAL)
    (line-dial ,bb:LINE_DIAL)
    (fill-dial ,bb:FILL_DIAL)
    (no-box ,bb:NO_BOX)
    (flat-box ,bb:FLAT_BOX)
    (up-box ,bb:UP_BOX)
    (down-box ,bb:DOWN_BOX)
    (up-frame ,bb:UP_FRAME)
    (down-frame ,bb:DOWN_FRAME)
    (thin-up-box ,bb:THIN_UP_BOX)
    (thin-down-box ,bb:THIN_DOWN_BOX)
    (thin-up-frame ,bb:THIN_UP_FRAME)
    (thin-down-frame ,bb:THIN_DOWN_FRAME)
    (engraved-box ,bb:ENGRAVED_BOX)
    (embossed-box ,bb:EMBOSSED_BOX)
    (engraved-frame ,bb:ENGRAVED_FRAME)
    (embossed-frame ,bb:EMBOSSED_FRAME)
    (border-box ,bb:BORDER_BOX)
    (never ,bb:WHEN_NEVER)
    (changed ,bb:WHEN_CHANGED)
    (released ,bb:WHEN_RELEASE)
    (always ,bb:WHEN_NOT_CHANGED)
    (enter ,bb:WHEN_ENTER_KEY)
    (helvetica ,bb:HELVETICA)
    (helvetica-bold ,bb:HELVETICA_BOLD)
    (helvetica-italic ,bb:HELVETICA_ITALIC)
    (helvetica-bold-italic ,bb:HELVETICA_BOLD_ITALIC)
    (courier ,bb:COURIER)
    (courier-bold ,bb:COURIER_BOLD)
    (courier-italic ,bb:COURIER_ITALIC)
    (courier-bold-italic ,bb:COURIER_BOLD_ITALIC)
    (times ,bb:TIMES)
    (times-bold ,bb:TIMES_BOLD)
    (times-italic ,bb:TIMES_ITALIC)
    (times-bold-italic ,bb:TIMES_BOLD_ITALIC)
    (symbol ,bb:SYMBOL)
    (screen ,bb:SCREEN)
    (screen-bold ,bb:SCREEN_BOLD)
    (hilighted ,bb:HILIGHTED)
    (unhilighted ,bb:UNHILIGHTED)
    (selected ,bb:SELECTED)
    (unselected ,bb:UNSELECTED)
    (opened ,bb:OPENED)
    (closed ,bb:CLOSED)
    (double-click ,bb:DOUBLE_CLICK)
    (widget-callback ,bb:WIDGET_CALLBACK)
    (moved-node ,bb:MOVED_NODE)
    (new-node ,bb:NEW_NODE)
    (nothing ,bb:NOTHING)
    (center ,bb:CENTER)
    (top ,bb:TOP)
    (bottom ,bb:BOTTOM)
    (left ,bb:LEFT)
    (right ,bb:RIGHT)
    (inside ,bb:INSIDE)
    (text-over-image ,bb:TEXT_OVER_IMAGE)
    (image-over-text ,bb:IMAGE_OVER_TEXT)
    (clip ,bb:CLIP)
    (wrap ,bb:WRAP) ) )

(define special-int-property-values
  (let ([ids 
	 (lambda (s v) 
	   (cons s (find-tail (lambda (p) (eq? (car p) v)) value-ids)) ) ] )
    (map ids special-int-properties special-int-property-values0) ) )

(define (property id)
  (or (and-let* ([a (assq id property-ids)]) (cadr a))
      (error "invalid property" id) ) )

(define (item widget)
  (if (pair? widget)
    (values
      (car widget)
      (cond
	[(fixnum? (cadr widget)) (cadr widget)]
	[(assq (cadr widget) item-ids) => cadr]
	[else -1])
      (if (and (pair? (cdr widget)) (pair? (cddr widget)) (fixnum? (caddr widget)))
	(caddr widget)
	-1) )
    (values widget -1 -1) ) )

(define (property-value x)
  (or (and-let* ([a (assq x value-ids)]) (cadr a))
      (error "invalid value specifier" x) ) )

(define (make-image x)
  (let ([xx (if (and (pair? x) (null? (cdr x))) (car x) x)])
    (cond
      [(string? xx) (bb:image xx)]
      [(and (not (##sys#immediate? xx)) (##sys#pointer? xx)) xx]
      [else (error "Invalid value for image property")] ) ) )

(define (bb:set! widget prop x)
  (let-values ([(p) (property prop)]
	       [(w i i2) (item widget)])
    (cond [(fixnum? x) (bb:SetIntProperty w i p x)]
	  [(symbol? x) (bb:SetIntProperty w i p (property-value x))]
	  [(eq? x #f) (bb:SetIntProperty w i p 0)]
	  [(eq? x #t) (bb:SetIntProperty w i p 1)]
	  [(number? x) (bb:SetDoubleProperty w p x)]
	  [(or (and (pair? prop) (eq? (car prop) 'image)) (eq? prop 'image))
	   (if (pair? x)
	       (bb:SetImage2 w i (make-image (car x)) (make-image (cdr x)))
	       (bb:SetImage w i (make-image x))) ]
	  [(and (pair? prop) (eq? prop 'selection))
	   (bb:SetSelection w (car prop) (cdr prop)) ]
	  [(string? x) (bb:SetStringProperty w i i2 p x)]
	  [(widget? x) (bb:SetWidgetProperty w p x)]
	  [(procedure? x)
	   (if (eq? prop 'callback)
	       (bb:SetCallback w x)
	       (bb:SetHandler w x) ) ]
	  [(list? x) (bb:SetIntProperty w i p (apply bitwise-ior (map property-value x)))]
	  [(and (not (##sys#immediate? x)) (##sys#pointer? x)) (bb:SetImage w i x)]
	  [else (error "invalid property" prop x)] ) ) )

(define (bb:get widget p0)
  (if (eq? p0 'selection)
      (bb:selection widget)
      (let-values ([(p) (property p0)]
		   [(w i i2) (item widget)])
	(cond [(eq? bb:TEXT p) (bb:GetStringProperty w i i2 p)]
	      [(eq? bb:VALUE p)
	       (cond [(eq? (widget-name w) 'html-view) (bb:GetStringProperty w i i2 p)]
		     [(memq (widget-name w) '(radio-button check-box light-button)) (eq? 1 (bb:GetIntProperty w i p))]
		     [(memq (widget-name w) '(list tree table)) (bb:GetIntProperty w i p)]
		     [else (bb:GetDoubleProperty w p)] ) ]
	      [else
	       (let ([val (bb:GetIntProperty w i p)])
		 (cond [(memq p0 special-bool-properties) (not (zero? val))]
		       [(memq p0 special-int-properties)
			(let ([a (find (lambda (p) (eq? (cadr p) val)) (cdr (assq p0 special-int-property-values)))])
			  (if a
			      (car a)
			      val) ) ]
		       [else val] ) ) ] ) ) ) )

(define bb:property (getter-with-setter bb:get bb:set!))

(define (find-frames str)
  (let* ([frames '()]
	 [strs (list->vector (string-split str "\n"))]
	 [w (string-length (vector-ref strs 0))]
	 [h (vector-length strs)] )
    (define (scan-right c x0 y0)
      (let loop ([x x0])
	(cond [(>= x w) #f]
	      [(char=? c (at x y0))
	       (substring (vector-ref strs y0) x0 x) ]
	      [else (loop (add1 x))] ) ) )
    (define (at x y) (string-ref (vector-ref strs y) x))
    (define (extract-frame c x1 y1)
      (let loop ([x x1])
	(if (or (>= x w) (not (char=? c (at x y1))))
	    (let loop ([y y1])
	      (if (or (>= y h) (not (char=? c (at x1 y))))
		  (vector x1 y1 (- x x1) (- y y1) c '())
		  (loop (add1 y)) ) )
	    (loop (add1 x)) ) ) )
    (define (scan-frame frame)
      (match frame
	[#(x0 y0 w h _ (set! update))
	 (let ([info '()])
	   (do ([x 1 (add1 x)])
	       ((>= x (sub1 w)))
	     (do ([y 1 (add1 y)])
		 ((>= y (sub1 h)))
	       (let ([c (at (+ x0 x) (+ y0 y))])
		 (when (memq c special-chars)
		   (if (memq c delimiters)
		       (and-let* ([s (scan-right c (+ x0 x 1) (+ y0 y))])
			 (set! info (alist-cons c (scan-right c (+ x0 x 1) (+ y0 y)) info)) )
		       (set! info (cons c info)) ) ) ) ) ) 
	   (update info) ) ] ) )
    (do ([y 0 (add1 y)])
	((>= y h))
      (do ([x 0 (add1 x)])
	  ((>= x w))
	(let ([c (at x y)])
	  (cond [(and (or (char-upper-case? c) (memv c (list #\< #\> #\* #\%)))
		      (or (zero? x) (not (char=? c (at (sub1 x) y))))
		      (or (zero? y) (not (char=? c (at x (sub1 y))))) )
		 (let ([frame (extract-frame c x y)])
		   (set! frames (cons frame frames))
		   (scan-frame frame)
		   (set! x (+ x (vector-ref frame 2) -1)) ) ]
		[(memq c delimiters)
		 (and-let* ([s (scan-right c (add1 x) y)])
		   (set! x (+ x (string-length s))) ) ] ) ) ) )
    (values frames w h) ) )

(define delimiters '(#\" #\| #\: #\# #\'))
(define special-chars '(#\" #\| #\: #\# #\'))

(define direction-map
 `(("v" . ,bb:VERTICAL)
   ("h" . ,bb:HORIZONTAL)
   ("vfill" . ,bb:VERT_FILL_SLIDER)
   ("hfill" . ,bb:HOR_FILL_SLIDER)
   ("vnice" . ,bb:VERT_NICE_SLIDER)
   ("hnice" . ,bb:HOR_NICE_SLIDER) ) )

(define box-type-map
  `(("no" . ,bb:NO_BOX) 
    ("flat" . ,bb:FLAT_BOX) 
    ("up" . ,bb:UP_BOX) 
    ("down" . ,bb:DOWN_BOX) 
    ("thinup" . ,bb:THIN_UP_BOX) 
    ("thindown" . ,bb:THIN_DOWN_BOX) 
    ("upframe" . ,bb:UP_FRAME) 
    ("downframe" . ,bb:DOWN_FRAME) 
    ("thinupframe" . ,bb:THIN_UP_FRAME) 
    ("thindownframe" . ,bb:THIN_DOWN_FRAME) 
    ("engraved" . ,bb:ENGRAVED_BOX) 
    ("embossed" . ,bb:EMBOSSED_BOX) 
    ("engravedframe" . ,bb:ENGRAVED_FRAME) 
    ("embossedframe" . ,bb:EMBOSSED_FRAME) 
    ("border" . ,bb:BORDER_BOX) ) )

(define color-map
  `(("gray" . ,bb:GRAY)
    ("black" . ,bb:BLACK)
    ("red" . ,bb:RED)
    ("green" . ,bb:GREEN)
    ("yellow" . ,bb:YELLOW)
    ("blue" . ,bb:BLUE)
    ("magenta" . ,bb:MAGENTA)
    ("cyan" . ,bb:CYAN)
    ("darkred" . ,bb:DARK_RED)
    ("darkgreen" . ,bb:DARK_GREEN)
    ("darkyellow" . ,bb:DARK_YELLOW)
    ("darkblue" . ,bb:DARK_BLUE)
    ("darkmagenta" . ,bb:DARK_MAGENTA)
    ("darkcyan" . ,bb:DARK_CYAN)
    ("white" . ,bb:WHITE) ) )

(define widget-map
  '((#\B . button)
    (#\C . check-xbox)
    (#\E . entry)
    (#\I . edit)
    (#\N . counter)
    (#\M . menu-bar)
    (#\D . dial)
    (#\S . slider)
    (#\A . adjuster)
    (#\L . list)
    (#\W . label)
    (#\O . radio-button)
    (#\P . progress)
    (#\T . tabs)
    (#\X . tile)
    (#\F . glwindow)
    (#\G . group)
    (#\J . clock)
    (#\V . live-image)
    (#\K . pack)
    (#\Z . scroll)
    (#\R . roller)
    (#\H . choice-button)
    (#\< . return-button)
    (#\% . int-entry)
    (#\* . float-entry)
    (#\> . menu-button)
    (#\Y . tree) ) )

(define (bb:make-widgets str fw fh . cmap)
  (let ([cmap (optional cmap '())])
    (let-values ([(frames w h) (find-frames str)])
      (let ([wf (/ fw w)]
	    [hf (/ fh h)] )
	(define (adjust-x x) (inexact->exact (round (* x wf))))
	(define (adjust-y y) (inexact->exact (round (* y hf))))
	(map (match-lambda
	      [#(x y w h c info)
	       (let ([m (assq c cmap)])
		 (when m (set! c (cadr m)))
		 (let ([widget
			(bb:make-widget
			 (or (and-let* ([a (assq c widget-map)]) (cdr a))
			     'label)
			 (adjust-x x) (adjust-y y)
			 (adjust-x w) (adjust-y h) ) ] )
		   (for-each
		    (match-lambda
		     [(#\" . str)
		      (bb:SetStringProperty widget -1 -1 bb:TEXT str) ]
		     [(#\| . str)
		      (for-each
		       (lambda (str)
			 (cond [(assoc str direction-map) =>
				(lambda (a) (bb:SetIntProperty widget -1 bb:DIRECTION (cdr a))) ]
			       [(assoc str box-type-map) =>
				(lambda (a) (bb:SetIntProperty widget -1 bb:BOX (cdr a))) ]
			       [(assoc str color-map) =>
				(lambda (a) (bb:SetIntProperty widget -1 bb:COLOR (cdr a))) ]
			       [else (error "bad widget property spec" str)] ) )
		       (string-split str ",") ) ]
		     [(#\: . str)
		      (bb:SetCallback widget (read (open-input-string str))) ]
		     [(#\' . str)
		      (for-each (cut bb:AddItem widget <> 0) (string-split str ",")) ]
		     [(#\# . str)
		      (bb:SetImage widget -1 (bb:image (string->symbol str))) 
		      (bb:RedrawWidget widget) ]
		     [_ #f] )
		    info)
		   (if m 
		       (cons (caddr m) widget)
		       (cons c widget) ) ) ) ] )
	     frames) ) ) ) )

(define callback-table '())

(define-external (BBGenericCallback (c-pointer w) (c-pointer u)) void
  (let ([cb (assoc w callback-table)])
    (if cb
	((cdr cb)) 
	(let ([cb (assoc u callback-table)])
	  (when cb ((cdr cb))) ) ) ) )

(define bb:SetCallback
  (let ([bb:SetCallback bb:SetCallback])
    (lambda (w cb)
      (let* ([ptr (widget-ptr w)]
	     [a (assoc ptr callback-table)]
	     [cb (cond [(symbol? cb) (eval `(lambda () (,cb)))]
		       [(pair? cb) (eval `(lambda () ,cb))]
		       [else cb] ) ] )
	(bb:SetCallback w (location BBGenericCallback))
	(if a
	    (set-cdr! a cb)
	    (set! callback-table (alist-cons ptr cb callback-table)) ) ) ) ) )

(define handler-table '())
          
(define events	; the order is defined by FL_XXX event values!
  '#(no-event	; 0
      push
      release
      enter
      leave
      drag
      focus
      unfocus
      keydown
      keyup
      close	; 10
      move
      shortcut
      deactivate
      activate
      hide
      show
      paste
      selectionclear
      mousewheel
      dnd-enter ; 20
      dnd-drag
      dnd-leave
      dnd-release ; 23
      ) )

(define-external (BBGenericHandler (c-pointer w) (int event)) int
  (let ([cb (assoc w handler-table)])
    (if cb
	(let ([res ((cdr cb) ; call user-supplied callback-handler
		    (if (fx< event 24)
		      (vector-ref events event)
		      'unknown) ) ] )
	    (if (boolean? res)
	      (if res 1 0)  ; convert boolean to int
	      -1) )	    ; call ancestor handler
	0) ) )

(define-external (BBGenericLink (c-pointer w) (c-string uri)) c-string
  (let ([cb (assoc w handler-table)])
    (if cb
	((cdr cb) uri); call user-supplied link-handler
	0) ) )

(define bb:SetHandler
  (let ([bb:SetHandler bb:SetHandler])
    (lambda (w cb)
      (let* ([ptr (widget-ptr w)]
	     [a (assoc ptr handler-table)]
	     [cb (cond [(symbol? cb) (eval `(lambda () (,cb)))]
		       [(pair? cb) (eval `(lambda () ,cb))]
		       [else cb] ) ] )
	(if (eq? (widget-name w) 'html-view)
	  (bb:SetHandler w (location BBGenericLink))
	  (bb:SetHandler w (location BBGenericHandler)) )
	(if a
	    (set-cdr! a cb)
	    (set! handler-table (alist-cons ptr cb handler-table)) ) ) ) ) )

(define key-codes
  `((,bb:BackSpace backspace)
    (,bb:Tab tab)
    (,bb:Enter enter)
    (,bb:Pause pause)
    (,bb:Scroll_Lock scroll-lock)
    (,bb:Escape escape)
    (,bb:Home home)
    (,bb:Left left)
    (,bb:Up up)
    (,bb:Right right)
    (,bb:Down down)
    (,bb:Page_Up page-up)
    (,bb:Page_Down page-down)
    (,bb:End end)
    (,bb:Print print)
    (,bb:Insert insert)
    (,bb:Menu menu)
    (,bb:Help help)
    (,bb:Num_Lock num-lock)
    (,bb:Shift_L shift-l)
    (,bb:Shift_R shift-r)
    (,bb:Control_L control-l)
    (,bb:Control_R control-r)
    (,bb:Caps_Lock caps-lock)
    (,bb:Meta_L meta-l)
    (,bb:Meta_R meta-r)
    (,bb:Alt_L alt-l)
    (,bb:Alt_R alt-r)
    (,bb:Delete delete) ) )

(define bb:decode-key
  (let ([bb:GetEventInt bb:GetEventInt])
    (lambda (key)
      (let ([val (bb:GetEventInt key)])
	(cond
	  [(fx<= val 255) (integer->char val)]	; ordinary kbd key
	  [(and (fx> val bb:Button) (fx< (- val bb:Button) 4)) ; mouse
	   (string->symbol (string-append "button" (->string (- val bb:Button))))]
	  [(and (fx>= val bb:KP) (fx<= val bb:KP_Last))
	   (cons (integer->char (- val bb:KP)) 'kp)]
	  [(and (fx>= val bb:F) (fx<= val bb:F_Last))
	   (string->symbol (string-append "F" (->string (- val bb:F))))]
	  [(assv val key-codes) => cadr]
	  [else 'unknown]) ) ) ) )

(define event-ids
  `((alt ,bb:E_ALT ,bb:GetEventBool)
    (button1 ,bb:E_BUTTON1 ,bb:GetEventBool)
    (button2 ,bb:E_BUTTON2 ,bb:GetEventBool)
    (button3 ,bb:E_BUTTON3 ,bb:GetEventBool)
    (clicks ,bb:E_CLICKS ,bb:GetEventInt)
    (ctrl ,bb:E_CTRL ,bb:GetEventBool)
    (click? ,bb:E_IS_CLICK ,bb:GetEventBool)
    (key ,bb:E_KEY ,bb:decode-key)
    (length ,bb:E_LENGTH ,bb:GetEventInt)
    (shift ,bb:E_SHIFT ,bb:GetEventBool)
    (x ,bb:E_X ,bb:GetEventInt)
    (y ,bb:E_Y ,bb:GetEventInt)
    (x-root ,bb:E_X_ROOT ,bb:GetEventInt)
    (y-root ,bb:E_Y_ROOT ,bb:GetEventInt)
    (dx ,bb:E_DX ,bb:GetEventInt)
    (dy ,bb:E_DY ,bb:GetEventInt)
    (text ,bb:E_TEXT ,bb:GetEventString) ) )


(define (event id)
  (let ([a (assq id event-ids)])
    (if a
      (values (cadr a) (caddr a))
      (error "invalid event" id) ) ) )

(define (bb:event-get evt)
  (let-values ([(id get) (event evt)])
    (get id) ) )

(define bb:event-set!
  (let ([bb:SetEventInt bb:SetEventInt]
	[bb:SetEventBool bb:SetEventBool])
    (lambda (evt val)
      (case evt
	[(clicks) (bb:SetEventInt bb:E_CLICKS val)]
	[(click?) (bb:SetEventBool bb:E_IS_CLICK val)]
	[else (error "event property is not modifiable")] ) ) ) )

(define bb:event (getter-with-setter bb:event-get bb:event-set!))

(define (widget t)
  (or (and-let* ([a (assq t widget-ids)]) (cadr a))
      (error "invalid widget type" t) ) )

(define current-parent #f)

(define (bb:make-widget id . args)
  (let* ([t (widget id)]
	 [w (make-widget
	     t
	     (match args
	       [(w h) (bb:CreateWidget t -1 -1 w h)]
	       [args (apply bb:CreateWidget t args)] ) 
	     id
	     (foreign-value "C_calloc(sizeof(void *), WIDGET_RESOURCE_MAX)" c-pointer)
	     current-parent
	     '() ) ] )
    (when current-parent 
      (widget-children-set! current-parent (cons w (widget-children current-parent))) )
    w) )

(define message-ids
  `((message ,bb:MESSAGE)
    (alert ,bb:ALERT)
    (ask ,bb:ASK)
    (choice ,bb:CHOICE) ) )

(define bb:message
  (lambda (type . args)
    (if (null? args)
	(bb:Message bb:MESSAGE (->string type) #f #f #f)
	(let-optionals args ([text #f] [s1 #f] [s2 #f] [s3 #f])
	  (bb:Message 
	   (or (and-let* ([a (assq type message-ids)]) (cadr a))
	       (error "invalid message type" type) )
	   (->string text) s1 s2 s3) ) ) ) )

(define (bb:get-input label . def)
  (bb:GetInput label (optional def #f)) )

(define bb:AddMenuItem
  (let ([bb:AddMenuItem bb:AddMenuItem])
    (lambda (menu text cb . sc)
      (let* ([cb (cond [(symbol? cb) (eval `(lambda () (,cb)))]
		       [(pair? cb) (eval `(lambda () ,cb))]
		       [else cb] ) ] 
	     [u (bb:AddMenuItem menu text (optional sc #f) (location BBGenericCallback))] )
	(set! callback-table (alist-cons u cb callback-table)) ) ) ) )

(define bb:AddTreeItem
  (let ([bb:AddTreeItem bb:AddTreeItem])
    (lambda (tree text . more)
      (let-optionals more ([parent -1] [pos -1] [widget #f])
	(bb:AddTreeItem tree text parent pos widget) ) ) ) )

(define bb:AddTableItems
  (let ([bb:AddTableCell bb:AddTableCell]
	[bb:AddTableColumn bb:AddTableColumn])
    (lambda (table text . more)
      (if (eq? text 'column)
	(for-each (cut bb:AddTableColumn table <>) more)
	(for-each (cut bb:AddTableCell table <>) (cons text more)) ) ) ) )

(define bb:set-menu-item-active! bb:ActivateMenuItem)

(define bb:run
  (match-lambda*
   [(or () (#t)) (bb:RunEventLoop #t 0)]
   [(#f) (bb:RunEventLoop #f 0)]
   [((? number? n)) (bb:RunEventLoop #f n)] ) )

(define (bb:add! w text . more)
  (case (widget-name w)
    [(menu-bar choice-button menu-button) (apply bb:AddMenuItem w text more)]
    [(tree) (apply bb:AddTreeItem w text more)]
    [(table) (apply bb:AddTableItems w text more)]
    [else 
     (if (widget? text)
	 (bb:AddWidget w text)
	 (bb:AddItem 
	  w text
	  (let ((m (optional more -1)))
	    (case m
	      ((#t) 1)
	      ((#f) 0)
	      (else m) ) ) ) ) ] ) )

(define (delete-callback ptr tbl)
  (let loop ([tbl tbl])
    (cond [(null? tbl) '()]
	  [(equal? ptr (car tbl)) (cdr tbl)]
	  [else (cons (car tbl) (loop (cdr tbl)))] ) ) )

(define (remove-callbacks w)
  (let ([ptr (widget-ptr w)])
    (set! callback-table (delete-callback ptr callback-table))
    (set! handler-table (delete-callback ptr handler-table))
    (for-each remove-callbacks (widget-children w)) ) )

(define (bb:remove! w . x)
  (if (null? x)
      (if (widget? w)
	  (begin
	    (remove-callbacks w)
	    (release-all-resources w)
	    (bb:RemoveAllItems w) 
	    (bb:DestroyWidget w) )
	  (bb:RemoveImage w) )
      (bb:RemoveItem w (let ([i (car x)]) (if (eq? i #t) -1 i))) ) )

(foreign-parse/declare #<<EOF

static void ReleasePointers(void **lst)
{
  int i;
  void *ptr;

  for(i = 0; i < WIDGET_RESOURCE_MAX; ++i) {
    if((ptr = lst[ i ]) != NULL) C_free(ptr); 
  }
}

EOF
)

(define (release-all-resources w)
  (ReleasePointers (widget-resources w))
  (for-each release-all-resources (widget-children w)) )

(define bb:redraw bb:RedrawWidget)

(define (bb:show w . args)
  (cond
    [(null? args) 
     (match (argv)
       ((name . _) 
	(bb:SetArg 0 name)
	(bb:ShowWindow w 1) )
       (_ (bb:ShowWindow w 0)) ) ]
    [(not (car args)) (bb:ShowWindow w 0)]
    [else
      (do ([c args (cdr c)] [i 0 (+ i 1)])
	  ((or (null? c) (fx= i bb:MAX_SHOW_ARGS)) (bb:ShowWindow w i))
	  (bb:SetArg i (car c)) ) ] ) )

(define bb:selection (foreign-lambda c-string* "BBGetSelection" scheme-object))

(define (bb:select-file m p . f)
  (bb:SelectFile m p (optional f #f)) )

(define (bb:select-directory m . f)
  (bb:SelectDir m (optional f #f)) )

(define bb:rgb
  (let ([split (foreign-lambda* scheme-object ([unsigned-long rgb] [scheme-object lst])
		 "C_word lst0 = lst;"
		 "C_set_block_item(lst, 0, C_fix((rgb >> 24) & 0xff));"
		 "lst = C_block_item(lst, 1);"
		 "C_set_block_item(lst, 0, C_fix((rgb >> 16) & 0xff));"
		 "lst = C_block_item(lst, 1);"
		 "C_set_block_item(lst, 0, C_fix((rgb >> 8) & 0xff));"
		 "return(lst0);") ] )
    (lambda (r . gb)
      (if (pair? gb)
	  (apply bb:RGB r gb)
	  (split r (list 0 0 0)) ) ) ) )

(define (bb:group w thunk)
  (let ([p0 #f])
    (dynamic-wind
	(lambda ()
	  (set! p0 current-parent)
	  (set! current-parent w)
	  (bb:BeginGroup w) )
	thunk
	(lambda ()
	  (set! current-parent p0)
	  (bb:EndGroup w) ) ) ) )

(define (bb:image x . more)
  (or (if (null? more)
	  (cond [(and (not (##sys#immediate? x)) (##sys#pointer? x)) (bb:Pixmap x)]
		[(string? x) (bb:LoadImage x)]
		[else (error "invalid image type" x)] )
	  (match more
	    [(w h d) (bb:RawImage x w h d)]
	    [_ (error "bad number of arguments" more)] ) )
      (error "can't access image" x) ) )

(define (bb:image-data i)
  (let ([w (bb:ImageDim i bb:IMAGEWIDTH)]
	[h (bb:ImageDim i bb:IMAGEHEIGHT)]
	[d (bb:ImageDim i bb:COLOR)]
	[c (bb:ImageDim i bb:SIZE)])
      (values 
	(let loop ([p '()] [c (- c 1)])
	  (if (fx< c 0)
	    p
	    (loop (cons (bb:ImageData i c) p) (- c 1)) ) )
	w h d) ) )


(define (bb:select-color . more)
  (define (choose title r g b)
    (let ([vec (u8vector r g b)])
      (and (not (zero? (bb:SelectColor title vec)))
	   (apply bb:RGB (u8vector->list vec)) ) ) )
  (let-optionals more ([col 0] [title "Please select a color"])
    (cond [(fixnum? col) (bb:SelectColorIndex col)]
	  [(symbol? col) (bb:SelectColorIndex (property-value col))]
	  [(vector? col) (apply choose title (vector->list col))]
	  [(list? col) (apply choose title col)]
	  [(string? col) (choose col 128 128 128)]
	  [else (choose title 128 128 128)] ) ) )
	  

;;; SXML stuff:

(define-record element
  tag					; symbol
  id					; any (eq?)
  content				; string
  children				; (element ...)
  parent				; element | #f
  widget				; widget
  x y					; int
  width height				; int
  properties				; ((symbol . value) ...)
  attributes)				; ((symbol . string) ...)

(define bb:element? element?)
(define bb:root-element (make-parameter #f))
(define bb:element-widget element-widget)
(define bb:element-parent element-parent)
(define bb:element-children element-children)
(define bb:element-id element-id)
(define bb:element-tag element-tag)
(define bb:element-attributes element-attributes)
(define bb:element-content element-content)

(define (bb:find-element id . root)
  (let loop ([root (optional root (bb:root-element))])
    (if (eq? id (element-id root))
	root
	(any loop (element-children root)) ) ) )

(define (bb:find-widget . args)
  (bb:element-widget (apply bb:find-element args) ) )

(define (bb:render sxml)
  (let ([elt
	 (let render ([sxml sxml] [parent #f])
	   (match sxml
	     [(tag ('@ attrs ...) body ...)
	      (unless (assq tag widget-ids)
		(error "invalid tag" tag) )
	      (let-values ([(content subs) (partition string? body)])
		(let ([elt (make-element 
			    tag #f 
			    (and (pair? content) (string-intersperse content ""))
			    '() parent #f #f #f #f #f '() '()) ] )
		  (element-children-set! elt (map (cut render <> elt) subs))
		  (parse-attributes! elt attrs)
		  elt) ) ]
	     [(tag body ...) (render `(,tag (@) ,@body) parent)]
	     [_ (error "invalid SXML syntax" sxml)] ) ) ] )
    (bb:root-element elt)
    (realize elt) 
    (bb:show (element-widget elt))
    elt) )

(define (parse-attributes! elt attrs)
  (for-each
   (lambda (attr)
     (let loop ([attr attr])
       (match attr
	 [(a val)
	  (cond [(assq a special-attributes) =>
		 (lambda (a)
		   ((third a) elt ((second a) val)) ) ]
		[(eq? a 'callback)
		 (element-properties-set! elt (alist-cons 'callback (>callback val) (element-properties elt))) ]
		[(assq a property-ids) =>
		 (lambda (_)
		   (element-properties-set!
		    elt
		    (alist-cons a (>property a val) (element-properties elt)) ) ) ]
		[else (element-attributes-set! elt (alist-cons a val (element-attributes elt)))] ) ]
	 [(a) (loop `(,a ,(->string a)))] ) ) )
   attrs) )

(define (>callback x)
  (cond [(procedure? x) x]
	[(symbol? x) (>callback (symbol->string x))]
	[(string? x) 
	 (let ([x (read (open-input-string x))])
	   (if (symbol? x)
	       (eval x)
	       (eval `(lambda () ,x)) ) ) ]
	[(list? x) (eval `(lambda () ,x))]
	[else (error "invalid callback value" x)] ) )

(define (>property name x)
  (case name
    [(x y width height spacing maximum minimum x-position y-position text-size position mark image-width image-height label-size)
     (cond [(number? x) x]
	   [(string? x) (string->number x)]
	   [else (error "invalid numeric attribute" x name)] ) ]
    [(text tooltip) (->string x)]
    [(color text-color selection-color label-color)
     (cond [(number? x) x]
	   [(string? x)
	    (if (and (= (string-length x) 7)
		     (char=? #\# (string-ref x 0)) )
		(bb:RGB (string->number (substring x 1 3) 16)
			(string->number (substring x 3 5) 16)
			(string->number (substring x 5 7) 16) )
		(string->symbol x) ) ]
	   [(symbol? x) x]
	   [else (error "invalid color attribute" x name)] ) ]
    [(resizable visible focus modal read-only valid-context)
     (cond [(boolean? x) x]
	   [(string? x) (not (string=? "no" x))]
	   [else #t] ) ]
    [(resizable-widget)
     (cond [(symbol? x) x]
	   [(string? x) (string->symbol x)]
	   [else (string->symbol (->string x))] ) ]
    [(when)
     (cond [(symbol? x) x]
	   [(list? x) x]
	   [else (map string->symbol (string-split (->string x) ", "))] ) ]
    [else
     (cond [(number? x) x]
	   [(string? x) (or (string->number x) x)]
	   [(symbol? x) x]
	   [else (error "invalid property value" x name)] ) ] ) )

(define (>position x)
  (cond [(number? x) (inexact->exact x)]
	[(string? x)
	 (if (and (> (string-length x) 0)
		  (or (memq (string-ref x 0) '(#\+ #\-))
		      (char=? #\% (string-ref x (sub1 (string-length x)))) ) )
	     x
	     (inexact->exact (string->number x)) ) ]
	[else (error "bad positional attribute" x)] ) )

(define (>id x)
  (cond [(string? x) (string->symbol x)]
	[(symbol? x) x]
	[else (string->symbol (->string x))] ) )

(define special-attributes
  `((x ,>position ,element-x-set!)
    (y ,>position ,element-y-set!)
    (width ,>position ,element-width-set!)
    (height ,>position ,element-height-set!)
    (id ,>id ,element-id-set!) ) )

(define (realize elt0)
  (define (getxy n x)
    (cond [(not x) n]
	  [(number? x) x]
	  [(memq (string-ref x 0) '(#\+ #\-)) (+ n (string->number x))]
	  [(char=? #\% (string-ref x (sub1 (string-length x)))) 
	   (inexact->exact (round (* (string->number (substring x 0 (sub1 (string-length x)))) n 0.01))) ]
	  [else (error "invalid dimensional property" elt0 x)] ) )
  (let loop ([elt elt0] [w #f] [h #f])
    (let* ([tag (element-tag elt)]
	   [refx (if (eq? elt elt0) -1 0)]
	   [refy (if (eq? elt elt0) -1 0)]
	   [refw (or w 640)]
	   [refh (or h 480)]
	   [x (or (getxy refx (element-x elt)) refx)]
	   [y (or (getxy refy (element-y elt)) refy)]
	   [w (or (getxy refw (element-width elt)))]
	   [h (or (getxy refh (element-height elt)))]
	   [widget (bb:make-widget tag x y w h)] )
      (element-x-set! elt x)
      (element-y-set! elt y)
      (element-width-set! elt w)
      (element-height-set! elt h)
      (element-widget-set! elt widget)
      (and-let* ([c (element-content elt)])
	(set! (bb:property widget 'text) c) )
      (for-each
       (match-lambda 
	[('resizable-widget . _) #f]
	[('callback . cb) (bb:set! widget 'callback cb)]
	[(a . val) (bb:set! widget a val)] )
       (element-properties elt) )
      (let ([cont (lambda () (for-each (cut loop <> w h) (element-children elt)))])
	(if (memq tag '(window double-window group tabs tile pack scroll))
	    (bb:group widget cont)
	    (cont) ) )
      (and-let* ([a (assq 'resizable-widget (element-properties elt))])
	(bb:set! widget 'resizable-widget
		 (element-widget (or (bb:find-element (cdr a)) (error "element not found" (cdr a))))) ) ) ) )

(implicit-exit-handler
 (let ((old (implicit-exit-handler)))
   (lambda ()
     (bb:Stop)
     (old) ) ) )

)
