/* bb-support.cpp */


#include <string.h>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Dial.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Multiline_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Counter.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Roller.H>
#include <FL/Fl_Adjuster.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Select_Browser.H>
#include <FL/fl_ask.H>
#include <FL/fl_message.H>
#include <FL/fl_show_colormap.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Tile.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_Clock.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Color_Chooser.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Menu_.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Help_View.H>

#ifdef BB_USE_FLU
# include <FLU/Flu_Tree_Browser.h>
#endif

#include "Table.h"

#define BBEXPORT       extern "C"

#define ___bool         bool
#define ___safe
#define WIDGET         void **
#define widget_type(x) ((size_t)((x)[ 2 ]) >> 1)
#define widget_ptr(x)  (((void **)((x)[ 3 ]))[ 1 ])
#define widget_rlist(x) (((void ***)((x)[ 5 ]))[ 1 ])
#define widget_children(x) (((void ***)((x)[ 7 ]))[ 1 ])

typedef void (*BBCallback)(Fl_Widget *, void *);
typedef int (*BBHandlerCallback)(Fl_Widget *, int event);

#include "bb.h"

BBEXPORT void BBSetCallback(WIDGET widget, BBCallback callback);
BBEXPORT void *BBAddMenuItem(WIDGET menu, char *label, char *shortcut, BBCallback callback);
BBEXPORT char *BBGetSelection(WIDGET widget);
BBEXPORT void BBSetHandler(WIDGET widget, BBHandlerCallback callback);

static void BBRedrawGLWindow(Fl_Gl_Window *);
static void *BBAddResource(WIDGET widget, void *r, int i);


class BBGLWindow: public Fl_Gl_Window
{
public:
  void *cbid;
  BBCallback cb;

  BBGLWindow(int x, int y, int w, int h, const char *label = 0): Fl_Gl_Window(x, y, w, h, label) {}
  void draw() { BBRedrawGLWindow(this); }
};


class BBLiveImage: public Fl_Widget
{
private:
  unsigned char *data_;
  int d_, w_, h_;

public:
  BBLiveImage(int x, int y, int w, int h, char *label = 0): Fl_Widget(x, y, w, h, label) {
    d_ = 0;
    data_ = 0;
  }

  void setData(unsigned char *data) { data_ = data; }
  void setDepth(int d) { d_ = d; }
  void setWidth(int w) { w_ = w; }
  void setHeight(int h) { h_ = h; }
  int width() { return w_; }
  int height() { return h_; }

protected:
  void draw() {
    if(data_ != 0) 
      if(d_ == 1) fl_draw_image_mono(data_, x(), y(), w_, h_);
      else fl_draw_image(data_, x(), y(), w_, h_, d_);
  }

  int handle(int event) {
    switch(event) {
    case FL_PUSH: do_callback(); return 1;
    default: return Fl_Widget::handle(event);
    }
  }
};

template <class Widget>
class BBHandler: public Widget
{
  BBHandlerCallback handler;
public:
  BBHandler(int w, int h) : Widget(w, h), handler(0) {}
  BBHandler(int x, int y, int w, int h) : Widget(x, y, w, h), handler(0) {}

  virtual int handle(int event)
  {
    if(handler)
    {
      int ret = handler(this, event);
      if(ret >= 0)	// ret < 0 => call ancestor handle
	  return ret;
    }
    return Widget::handle(event);
  }
  void set_handler(BBHandlerCallback cb)
  {
      handler = cb;
  }
};


class BBImage
{
  bool shared;
  Fl_Image *img;
  static bool img_regged;
public:
  BBImage(const char *name) : shared(true)
  {
    if(!img_regged)
    {
      fl_register_images();
      img_regged = true;
    }
    img = Fl_Shared_Image::get(name);
  }
  BBImage(Fl_Image *img_) : shared(false)
  {
    img = img_;
  }
  ~BBImage()
  {
    if(shared)
      ((Fl_Shared_Image *)img)->release();
    else
      delete img;
  }
  Fl_Image *get() { return img; }
};

bool BBImage::img_regged = false;

static unsigned int cb_count = 1;


double BBStart(char *scm)
{
  Fl::visual(FL_RGB|FL_DOUBLE);
  Fl::scheme(scm);
  return Fl::version();
}


void BBStop()
{
}


void *BBCreateWidget(int type, int x, int y, int w, int h)
{
  switch(type) {
  case BB_WINDOW:
    { 
      Fl_Window *win = new BBHandler<Fl_Window>(w, h);
      
      if(x != -1) win->position(x, y);

      return win;
    }

  case BB_GLWINDOW:
    { 
      BBGLWindow *win = new BBHandler<BBGLWindow>(x, y, w, h);
      
      return win;
    }

  case BB_DOUBLEWINDOW:
    return new BBHandler<Fl_Double_Window>(x, y, w, h);

  case BB_BUTTON:
    return new BBHandler<Fl_Button>(x, y, w, h);

  case BB_RETURNBUTTON:
    return new BBHandler<Fl_Return_Button>(x, y, w, h);

  case BB_CHECKBOX:
    return new BBHandler<Fl_Check_Button>(x, y, w, h);

  case BB_CHOICEBUTTON:
    return new BBHandler<Fl_Choice>(x, y, w, h);

  case BB_MENUBUTTON:
    return new BBHandler<Fl_Menu_Button>(x, y, w, h);

  case BB_ENTRY:
    {
      Fl_Input *inp = new BBHandler<Fl_Input>(x, y, w, h);
      inp->when(FL_WHEN_ENTER_KEY_ALWAYS);
      return inp;
    }

  case BB_INTENTRY:
    {
      Fl_Int_Input *inp = new BBHandler<Fl_Int_Input>(x, y, w, h);
      inp->when(FL_WHEN_ENTER_KEY_ALWAYS);
      return inp;
    }

  case BB_FLOATENTRY:
    {
      Fl_Float_Input *inp = new BBHandler<Fl_Float_Input>(x, y, w, h);
      inp->when(FL_WHEN_ENTER_KEY_ALWAYS);
      return inp;
    }

  case BB_EDIT:
    return new BBHandler<Fl_Multiline_Input>(x, y, w, h);

  case BB_TEXTEDITOR:
    {
      Fl_Text_Editor *ed = new BBHandler<Fl_Text_Editor>(x, y, w, h);
      ed->buffer(new Fl_Text_Buffer);
      return ed;
    }

  case BB_COUNTER:
    return new BBHandler<Fl_Counter>(x, y, w, h);

  case BB_DIAL:
    return new BBHandler<Fl_Dial>(x, y, w, h);

  case BB_LABEL:
    {
      Fl_Box *b = new BBHandler<Fl_Box>(x, y, w, h);
      b->box(FL_FLAT_BOX);
      return b;
    }

  case BB_SLIDER:
    return new BBHandler<Fl_Slider>(x, y, w, h);

  case BB_ADJUSTER:
    return new BBHandler<Fl_Adjuster>(x, y, w, h);

  case BB_ROLLER:
    return new BBHandler<Fl_Roller>(x, y, w, h);

  case BB_LIST:
    return new BBHandler<Fl_Select_Browser>(x, y, w, h);

  case BB_RADIOBUTTON:
    return new BBHandler<Fl_Round_Button>(x, y, w, h);

  case BB_MENUBAR:
    return new BBHandler<Fl_Menu_Bar>(x, y, w, h);

  case BB_PROGRESS:
    return new BBHandler<Fl_Progress>(x, y, w, h);

  case BB_TABS:
    return new BBHandler<Fl_Tabs>(x, y, w, h);

  case BB_GROUP:
    return new BBHandler<Fl_Group>(x, y, w, h);

  case BB_TILE:
    return new BBHandler<Fl_Tile>(x, y, w, h);

  case BB_PACK:
    return new BBHandler<Fl_Pack>(x, y, w, h);

  case BB_SCROLL:
    return new BBHandler<Fl_Scroll>(x, y, w, h);

  case BB_LIGHTBUTTON:
    return new BBHandler<Fl_Light_Button>(x, y, w, h);

  case BB_CLOCK:
    return new BBHandler<Fl_Clock>(x, y, w, h);

  case BB_LIVEIMAGE:
    return new BBHandler<BBLiveImage>(x, y, w, h);

#ifdef BB_USE_FLU
  case BB_TREE:
  {
    Flu_Tree_Browser *tree = new BBHandler<Flu_Tree_Browser>(x, y, w, h);
    tree->allow_dnd(false);
    tree->double_click_opens(true);
    tree->insertion_mode(FLU_INSERT_BACK);
    tree->open_without_children(false);
    tree->open_on_select(false);
    tree->selection_drag_mode(FLU_DRAG_TO_SELECT);
    tree->selection_mode(FLU_SINGLE_SELECT);
    tree->show_branches(true);
    tree->show_connectors(true);
    tree->show_root(true);
    tree->show_leaves(true);
    return tree;
  }
#endif

  case BB_TABLE:
    return new BBHandler<Table>(x, y, w,h);

  case BB_HTMLVIEW:
    return new Fl_Help_View(x, y, w,h);

  default:
    return 0;
  }
}


void BBDestroyWidget(WIDGET widget)
{
  Fl_Widget *w = reinterpret_cast<Fl_Widget *>(widget_ptr(widget));
  Fl_Group *p = w->parent();

  if(p != NULL) p->remove(w);

  delete w;
}


void BBSetIntProperty(WIDGET widget, int item, int property, int value)
{
  static bool setting_int_property = false;

  if(setting_int_property) return;

  Fl_Widget *w = (Fl_Widget *)widget_ptr(widget);

  switch(property) {
  case BB_X: w->position(value, w->y()); break;
  case BB_Y: w->position(w->x(), value); break;
  case BB_WIDTH:
#ifdef BB_USE_FLU
    if(widget_type(widget) == BB_TREE && item == BB_CONNECTOR)
    {
      Flu_Tree_Browser *t = (Flu_Tree_Browser *)widget_ptr(widget);
      t->connector_style(t->connector_color(), t->connector_style(), value);
    }
    else
#endif
    if(widget_type(widget) == BB_TABLE && item >= 0)
      ((Table *)widget_ptr(widget))->columnWidth(item, value);
    else
      w->size(value, w->h());
    break;
  case BB_HEIGHT: w->size(w->w(), value); break;
  case BB_BOX:
#ifdef BB_USE_FLU
    if(widget_type(widget) == BB_TREE)
      ((Flu_Tree_Browser *)w)->box((Fl_Boxtype)value);
    else
#endif
      w->box((Fl_Boxtype)value);
    break;
  case BB_TYPE: w->type(value); break;
  case BB_VALIDCONTEXT: ((Fl_Gl_Window *)widget_ptr(widget))->valid(value); break;

  case BB_VALUE:
    switch(widget_type(widget)) {
    case BB_RADIOBUTTON:
    case BB_CHECKBOX:
      ((Fl_Button *)widget_ptr(widget))->value(value); break;
    case BB_PROGRESS:
      ((Fl_Progress *)widget_ptr(widget))->value(value); break;
    case BB_LIST:
      ((Fl_Select_Browser *)widget_ptr(widget))->value(value); break;
    case BB_CHOICEBUTTON:
      ((Fl_Choice *)widget_ptr(widget))->value(value); break;
#ifdef BB_USE_FLU
    case BB_TREE:
      {
	  Flu_Tree_Browser::Node *n = ((Flu_Tree_Browser *)w)->find(value);
	  if(n)
	  {
	    Flu_Tree_Browser::Node *p = n->parent();
	    while (p && !p->open()) // expand all parents
	    {
		p->open(true);
		p = p->parent();
	    }
	    n->select_only();
	    ((Flu_Tree_Browser *)w)->set_hilighted(n);
	  }
      }
      break;
#endif
    case BB_TABLE:
      ((Table *)w)->value(value);
      break;

    default:
      ((Fl_Valuator *)widget_ptr(widget))->value(value);
    }

    break;

  case BB_RESIZABLE:
    {
      switch(widget_type(widget)) {
      case BB_WINDOW:
	{
	  Fl_Window *g = (Fl_Window *)widget_ptr(widget);
	  g->resizable(value ? g : 0);
	  break;
	}

      case BB_GLWINDOW:
	{
	  Fl_Gl_Window *g = (Fl_Gl_Window *)widget_ptr(widget);
	  g->resizable(value ? g : 0);
	  break;
	}
      }

      break;
    }

  case BB_MODAL: 
    if(value) ((Fl_Window *)widget_ptr(widget))->set_modal(); 

    break;

  case BB_DIRECTION: w->type(value); break;
  case BB_COLOR: 
    if(widget_type(widget) == BB_LIVEIMAGE)
      ((BBLiveImage *)widget_ptr(widget))->setDepth(value);
#ifdef BB_USE_FLU
    else if(widget_type(widget) == BB_TREE)
    {
      Flu_Tree_Browser *t = (Flu_Tree_Browser *)widget_ptr(widget);
      if(item == BB_CONNECTOR)
        t->connector_style((Fl_Color)value, t->connector_style(), t->connector_width());
      else
        t->color(value);
    }
#endif
    else
      w->color(value); 
    break;

  case BB_LABELCOLOR: w->labelcolor(value); break;
  case BB_LABELSIZE: w->labelsize(value); break;
  case BB_LABELFONT: w->labelfont(value); break;
  case BB_FOCUS: w->take_focus(); break;
  case BB_SPACING: ((Fl_Pack *)widget_ptr(widget))->spacing(value); break;
  case BB_XPOSITION: ((Fl_Scroll *)widget_ptr(widget))->position(value, ((Fl_Scroll *)widget_ptr(widget))->yposition()); break;
  case BB_YPOSITION: ((Fl_Scroll *)widget_ptr(widget))->position(((Fl_Scroll *)widget_ptr(widget))->yposition(), value); break;
  case BB_TEXTCOLOR:
    switch(widget_type(widget)) {
#ifdef BB_USE_FLU
    case BB_TREE:
      {
	Flu_Tree_Browser *t = (Flu_Tree_Browser *)widget_ptr(widget);
	switch(item)
	  {
	  case BB_ROOT:
	    t->root_color((Fl_Color)value);
	    break;
	  case BB_BRANCH:
	    t->branch_text((Fl_Color)value, t->branch_font(), t->branch_size());
	    break;
	  case BB_LEAF:
	    t->leaf_text((Fl_Color)value, t->leaf_font(), t->leaf_size());
	    break;
	  default:
	    {
	      Flu_Tree_Browser::Node *n = t->find(item);
	      if(n)
		n->label_color((Fl_Color)value);
	    }
	  }
      }

    break;
#endif

    case BB_ENTRY:
    case BB_EDIT:
    case BB_INTENTRY:
    case BB_FLOATENTRY:
      ((Fl_Input *)widget_ptr(widget))->textcolor(value);
      break;

    case BB_TEXTEDITOR:
      ((Fl_Text_Editor *)widget_ptr(widget))->textcolor(value);
      break;

    case BB_HTMLVIEW:
      ((Fl_Help_View *)widget_ptr(widget))->textcolor((Fl_Color)value);
      break;
    }

    break;

  case BB_TEXTSIZE:
    switch(widget_type(widget)) {
#ifdef BB_USE_FLU
    case BB_TREE:
      {
	Flu_Tree_Browser *t = (Flu_Tree_Browser *)widget_ptr(widget);
	switch(item)
	  {
	  case BB_ROOT:
	    t->root_size(value);
	    break;
	  case BB_BRANCH:
	    t->branch_text(t->branch_color(), t->branch_font(), value);
	    break;
	  case BB_LEAF:
	    t->leaf_text(t->leaf_color(), t->leaf_font(), value);
	    break;
	  default:
	    {
	      Flu_Tree_Browser::Node *n = t->find(item);
	      if(n)
		n->label_size(value);
	    }
	  }
      }

      break;
#endif
    case BB_ENTRY:
    case BB_EDIT:
    case BB_INTENTRY:
    case BB_FLOATENTRY:
      ((Fl_Input *)widget_ptr(widget))->textsize(value);
      break;

    case BB_TEXTEDITOR:
      ((Fl_Text_Editor *)widget_ptr(widget))->textsize(value);
      break;

    case BB_HTMLVIEW:
      ((Fl_Help_View *)widget_ptr(widget))->textsize(value);
      break;
    }

    break;

  case BB_TEXTFONT:
    switch(widget_type(widget)) {
#ifdef BB_USE_FLU
    case BB_TREE:
      {
	Flu_Tree_Browser *t = (Flu_Tree_Browser *)widget_ptr(widget);
	switch(item)
	  {
	  case BB_ROOT:
	    t->root_font((Fl_Font)value);
	    break;
	  case BB_BRANCH:
	    t->branch_text(t->branch_color(), (Fl_Font)value, t->branch_size());
	    break;
	  case BB_LEAF:
	    t->leaf_text(t->leaf_color(), (Fl_Font)value, t->leaf_size());
	    break;
	  default:
	    {
	      Flu_Tree_Browser::Node *n = t->find(item);
	      if(n)
		n->label_font((Fl_Font)value);
	    }
	  }
      }

      break;
#endif
    case BB_ENTRY:
    case BB_EDIT:
    case BB_INTENTRY:
    case BB_FLOATENTRY:
      ((Fl_Input *)widget_ptr(widget))->textfont(value);
      break;
      
    case BB_TEXTEDITOR:
      ((Fl_Text_Editor *)widget_ptr(widget))->textfont(value);
      break;

    case BB_HTMLVIEW:
      ((Fl_Help_View *)widget_ptr(widget))->textfont(value);
      break;
    }

    break;

  case BB_SELECTIONCOLOR:
#ifdef BB_USE_FLU
    if(widget_type(widget) == BB_TREE)
      ((Flu_Tree_Browser *)widget_ptr(widget))->selection_color(value);
    else
#endif
      w->selection_color(value);
    break;
  case BB_VISIBLE:
    switch(widget_type(widget)) {
    case BB_WINDOW: 
    case BB_DOUBLEWINDOW:
      {
	Fl_Window *win = (Fl_Window *)widget_ptr(widget);
	if(value) win->show(); else win->hide(); 
	break;
      }
    default:
      if(value) w->show(); else w->hide(); 
    }

    break;

  case BB_WHEN: w->when(value); break;
  case BB_POSITION: 
    switch(widget_type(widget)) {
    case BB_ENTRY:
    case BB_EDIT:
    case BB_INTENTRY:
    case BB_FLOATENTRY:
      {
	Fl_Input_ *inp = (Fl_Input_ *)widget_ptr(widget);
	int n = value == -1 ? inp->size() : value;
	inp->position(n, inp->mark());
	break;
      }

    case BB_TEXTEDITOR:
      {
	Fl_Text_Editor *ep = (Fl_Text_Editor *)widget_ptr(widget);
	ep->insert_position(value);
	break;
      }
    }

    break;

  case BB_MARK:
    switch(widget_type(widget)) {
    case BB_ENTRY:
    case BB_INTENTRY:
    case BB_FLOATENTRY:
    case BB_EDIT:
      {
	Fl_Input_ *inp = (Fl_Input_ *)widget_ptr(widget);
	int n = value == -1 ? inp->size() : value;
	inp->mark(n);
	break;
      }

    case BB_TEXTEDITOR:
      {
	Fl_Text_Editor *ep = (Fl_Text_Editor *)widget_ptr(widget);
	int n = value == -1 ? ep->buffer()->length() : value;
	ep->insert_position(n);
	break;
      }
    }

    break;

  case BB_READONLY: ((Fl_Input_ *)widget_ptr(widget))->readonly(value); break;
  case BB_IMAGEWIDTH: ((BBLiveImage *)widget_ptr(widget))->setWidth(value); break;
  case BB_IMAGEHEIGHT: ((BBLiveImage *)widget_ptr(widget))->setHeight(value); break;
  case BB_SIZE: break;
  case BB_ALIGN:
    if(widget_type(widget) == BB_TABLE && item >= 0)
      ((Table *)widget_ptr(widget))->columnAlign(item, (Fl_Align)value);
    else
      w->align(value);
    break;
  default: 
    setting_int_property = true;
    BBSetDoubleProperty(widget, property, value);
    setting_int_property = false;
  }
}


void BBSetStringProperty(WIDGET widget, int item, int item2, int property, char *value)
{
  Fl_Widget *w = (Fl_Widget *)widget_ptr(widget);

  switch(property) {
  case BB_TEXT:
    switch(widget_type(widget)) {
    case BB_EDIT:
    case BB_INTENTRY:
    case BB_FLOATENTRY:
    case BB_ENTRY: ((Fl_Input *)widget_ptr(widget))->value(value); break;
    case BB_WINDOW:
    case BB_DOUBLEWINDOW:
    case BB_GLWINDOW: ((Fl_Window *)widget_ptr(widget))->label(value); break;
#ifdef BB_USE_FLU
    case BB_TREE:
    {
      Flu_Tree_Browser *t = (Flu_Tree_Browser *)widget_ptr(widget);
      if(item == BB_ROOT)
	t->label(value);
      else
      {
	Flu_Tree_Browser::Node *n = t->find(item);
	if(n)
	  n->label(value);
      }
    }
    break;
#endif
    case BB_TEXTEDITOR:
      ((Fl_Text_Editor *)widget_ptr(widget))->buffer()->text(value);
      break;

    case BB_HTMLVIEW:
      ((Fl_Help_View *)w)->value(value);
      break;

    case BB_TABLE:
      if(item2 > -1)
      {
	if(item > -1)
	  ((Table *)w)->valueAt(item, item2, value);
	else
	  ((Table *)w)->columnTitle(item2, value);
	break;
      } /* fall through! */

    default: w->label((char *)BBAddResource(widget, strdup(value), WRES_LABEL));
    }

    break;

  case BB_TOOLTIP: w->tooltip((char *)BBAddResource(widget, strdup(value), WRES_TOOLTIP)); break;

  case BB_VALUE:
    if(widget_type(widget) == BB_HTMLVIEW)
      ((Fl_Help_View *)w)->load(value);
  }
}


void BBSetCallback(WIDGET widget, BBCallback callback)
{
  Fl_Widget *w = (Fl_Widget *)widget_ptr(widget);

  switch(widget_type(widget)) {
  case BB_GLWINDOW: 
    {
      BBGLWindow *glw = (BBGLWindow *)widget_ptr(widget);
      glw->cb = callback;
      glw->cbid = (void *)cb_count++;
      break;
    }

  default: w->callback(callback, (void *)cb_count++);
  }
}

void BBSetHandler(WIDGET widget, BBHandlerCallback cb)
{
  switch(widget_type(widget)) {
  case BB_WINDOW:
    ((BBHandler<Fl_Window>*)widget_ptr(widget))->set_handler(cb);
    break;
  case BB_DOUBLEWINDOW:
    ((BBHandler<Fl_Double_Window>*)widget_ptr(widget))->set_handler(cb);
    break;
  case BB_GLWINDOW:
    ((BBHandler<BBGLWindow>*)widget_ptr(widget))->set_handler(cb);
    break;
  case BB_BUTTON:
    ((BBHandler<Fl_Button>*)widget_ptr(widget))->set_handler(cb);
    break;
  case BB_RETURNBUTTON:
    ((BBHandler<Fl_Return_Button>*)widget_ptr(widget))->set_handler(cb);
    break;
  case BB_CHOICEBUTTON:
    ((BBHandler<Fl_Choice>*)widget_ptr(widget))->set_handler(cb);
    break;
  case BB_MENUBUTTON:
    ((BBHandler<Fl_Menu_Button>*)widget_ptr(widget))->set_handler(cb);
    break;
  case BB_CHECKBOX:
    ((BBHandler<Fl_Check_Button>*)widget_ptr(widget))->set_handler(cb);
    break;
  case BB_ENTRY:
    ((BBHandler<Fl_Input>*)widget_ptr(widget))->set_handler(cb);
    break;
  case BB_EDIT:
    ((BBHandler<Fl_Multiline_Input>*)widget_ptr(widget))->set_handler(cb);
    break;
  case BB_TEXTEDITOR:
    ((BBHandler<Fl_Text_Editor>*)widget_ptr(widget))->set_handler(cb);
    break;
  case BB_INTENTRY:
    ((BBHandler<Fl_Int_Input>*)widget_ptr(widget))->set_handler(cb);
    break;
  case BB_FLOATENTRY:
    ((BBHandler<Fl_Float_Input>*)widget_ptr(widget))->set_handler(cb);
    break;
  case BB_COUNTER:
    ((BBHandler<Fl_Counter>*)widget_ptr(widget))->set_handler(cb);
    break;
  case BB_DIAL:
    ((BBHandler<Fl_Dial>*)widget_ptr(widget))->set_handler(cb);
    break;
  case BB_LABEL:
    ((BBHandler<Fl_Box>*)widget_ptr(widget))->set_handler(cb);
    break;
  case BB_SLIDER:
    ((BBHandler<Fl_Slider>*)widget_ptr(widget))->set_handler(cb);
    break;
  case BB_ADJUSTER:
    ((BBHandler<Fl_Adjuster>*)widget_ptr(widget))->set_handler(cb);
    break;
  case BB_ROLLER:
    ((BBHandler<Fl_Roller>*)widget_ptr(widget))->set_handler(cb);
    break;
  case BB_LIST:
    ((BBHandler<Fl_Select_Browser>*)widget_ptr(widget))->set_handler(cb);
    break;
  case BB_RADIOBUTTON:
    ((BBHandler<Fl_Round_Button>*)widget_ptr(widget))->set_handler(cb);
    break;
  case BB_MENUBAR:
    ((BBHandler<Fl_Menu_Bar>*)widget_ptr(widget))->set_handler(cb);
    break;
  case BB_PROGRESS:
    ((BBHandler<Fl_Progress>*)widget_ptr(widget))->set_handler(cb);
    break;
  case BB_TABS:
    ((BBHandler<Fl_Tabs>*)widget_ptr(widget))->set_handler(cb);
    break;
  case BB_GROUP:
    ((BBHandler<Fl_Group>*)widget_ptr(widget))->set_handler(cb);
    break;
  case BB_TILE:
    ((BBHandler<Fl_Tile>*)widget_ptr(widget))->set_handler(cb);
    break;
  case BB_PACK:
    ((BBHandler<Fl_Pack>*)widget_ptr(widget))->set_handler(cb);
    break;
  case BB_SCROLL:
    ((BBHandler<Fl_Scroll>*)widget_ptr(widget))->set_handler(cb);
    break;
  case BB_LIGHTBUTTON:
    ((BBHandler<Fl_Light_Button>*)widget_ptr(widget))->set_handler(cb);
    break;
  case BB_CLOCK:
    ((BBHandler<Fl_Clock>*)widget_ptr(widget))->set_handler(cb);
    break;
  case BB_LIVEIMAGE:
    ((BBHandler<BBLiveImage>*)widget_ptr(widget))->set_handler(cb);
    break;
#ifdef BB_USE_FLU
  case BB_TREE:
    ((BBHandler<Flu_Tree_Browser>*)widget_ptr(widget))->set_handler(cb);
    break;
#endif
  case BB_TABLE:
    ((BBHandler<Table>*)widget_ptr(widget))->set_handler(cb);
    break;
  case BB_HTMLVIEW:
    ((Fl_Help_View*)widget_ptr(widget))->link((Fl_Help_Func *)cb);
    break;
  }
}

void BBSetDoubleProperty(WIDGET widget, int property, double value)
{
  static bool setting_double_property = false;

  if(setting_double_property) return;

  switch(property) {
  case BB_VALUE:
    switch(widget_type(widget)) {
    case BB_RADIOBUTTON:
    case BB_CHECKBOX:
    case BB_TABS:
    case BB_CHOICEBUTTON:
      BBSetIntProperty(widget, -1, property, (int)value);
      break;

    case BB_PROGRESS:
      ((Fl_Progress *)widget_ptr(widget))->value(value); break;

    case BB_CLOCK:
      ((Fl_Clock *)widget_ptr(widget))->value((unsigned long)value); break;

    default:
      ((Fl_Valuator *)widget_ptr(widget))->value(value);
    }

    break;
  case BB_MAXIMUM:
    switch(widget_type(widget)) {
    case BB_PROGRESS: ((Fl_Progress *)widget_ptr(widget))->maximum(value); break;
    case BB_DIAL: ((Fl_Dial *)widget_ptr(widget))->angle2((short)value); break;
    }

    break;

  case BB_MINIMUM:
    switch(widget_type(widget)) {
    case BB_PROGRESS: ((Fl_Progress *)widget_ptr(widget))->minimum(value); break;
    case BB_DIAL: ((Fl_Dial *)widget_ptr(widget))->angle1((short)value); break;
    }

    break;

  default:
    setting_double_property = true;
    BBSetIntProperty(widget, -1, property, (int)value);
    setting_double_property = false;
  }
}


int BBGetIntProperty(WIDGET widget, int item, int property)
{
  static int getting_int_property = false;

  if(getting_int_property) return 0;

  Fl_Widget *w = (Fl_Widget *)widget_ptr(widget);
  int result;

  switch(property) {
  case BB_X: result = w->x(); break;
  case BB_Y: result = w->y(); break;
  case BB_WIDTH:
#ifdef BB_USE_FLU
    if(widget_type(widget) == BB_TREE && item == BB_CONNECTOR)
      result = ((Flu_Tree_Browser *)widget_ptr(widget))->connector_width();
    else
#endif
      if(widget_type(widget) == BB_TABLE && item >= 0)
	result = ((Table *)widget_ptr(widget))->columnWidth(item);
      else result = w->w();

    break;
  case BB_HEIGHT: result = w->h(); break;
  case BB_BOX:
#ifdef BB_USE_FLU
    if(widget_type(widget) == BB_TREE)
      result = ((Flu_Tree_Browser *)w)->box();
    else
#endif
      result = w->box();
    break;

  case BB_TYPE: result = w->type(); break;
  case BB_VALIDCONTEXT: result = ((Fl_Gl_Window *)widget_ptr(widget))->valid(); break;
  case BB_RESIZABLE: result = ((Fl_Group *)widget_ptr(widget))->resizable() != 0; break;
  case BB_MODAL: result = ((Fl_Window *)widget_ptr(widget))->modal(); break;
  case BB_DIRECTION: result = w->type(); break;
  case BB_COLOR:
#ifdef BB_USE_FLU
    if(widget_type(widget) == BB_TREE && item == BB_CONNECTOR)
      result = ((Flu_Tree_Browser *)widget_ptr(widget))->connector_color();
    else
#endif
      result = w->color();

    break;
  case BB_LABELCOLOR: result = w->labelcolor(); break;
  case BB_LABELSIZE: result = w->labelsize(); break;
  case BB_LABELFONT: result = w->labelfont(); break;
  case BB_VALUE:
    switch(widget_type(widget)) {
    case BB_RADIOBUTTON:
    case BB_CHECKBOX:
      result = ((Fl_Button *)widget_ptr(widget))->value(); break;
    case BB_PROGRESS:
      result = (int)((Fl_Progress *)widget_ptr(widget))->value(); break;
    case BB_LIST:
      result = ((Fl_Select_Browser *)widget_ptr(widget))->value(); break;
    case BB_CHOICEBUTTON:
      result = ((Fl_Choice *)widget_ptr(widget))->value(); break;
#ifdef BB_USE_FLU
    case BB_TREE:
    {
      Flu_Tree_Browser::Node *n = ((Flu_Tree_Browser *)widget_ptr(widget))->get_selected(1);
      if(n)
	  result = n->id();
      else
	  result = -1;

      break;
    }
#endif
    case BB_TABLE:
      result = ((Table *)w)->value(); break;
    default:
      result = (int)((Fl_Valuator *)widget_ptr(widget))->value(); break;
    }

    break;

  case BB_SPACING: result = ((Fl_Pack *)widget_ptr(widget))->spacing(); break;
  case BB_XPOSITION: result = ((Fl_Scroll *)widget_ptr(widget))->xposition(); break;
  case BB_YPOSITION: result = ((Fl_Scroll *)widget_ptr(widget))->yposition(); break;
  case BB_TEXTCOLOR:
    result = 0;

    switch(widget_type(widget)) {
#ifdef BB_USE_FLU
    case BB_TREE:
      {
	Flu_Tree_Browser *t = (Flu_Tree_Browser *)widget_ptr(widget);
	switch(item)
	  {
	  case BB_ROOT: result = t->root_color(); break;
	  case BB_BRANCH: result = t->branch_color(); break;
	  case BB_LEAF: result = t->leaf_color(); break;
	  default:
	    {
	      Flu_Tree_Browser::Node *n = t->find(item);
	      if(n)
		n->label_color();

	      break;
	    }
	  }
      }

      break;
#endif
    case BB_ENTRY:
    case BB_EDIT:
    case BB_INTENTRY:
    case BB_FLOATENTRY:
      result = ((Fl_Input *)widget_ptr(widget))->textcolor(); break;

    case BB_TEXTEDITOR:
      result = ((Fl_Text_Editor *)widget_ptr(widget))->textcolor(); break;

    case BB_HTMLVIEW:
      result = ((Fl_Help_View *)widget_ptr(widget))->textcolor(); break;
    }

    break;

  case BB_TEXTSIZE:
    result = 0;

    switch(widget_type(widget)) {
#ifdef BB_USE_FLU
    case BB_TREE:
      {
	Flu_Tree_Browser *t = (Flu_Tree_Browser *)widget_ptr(widget);
	switch(item)
	  {
	  case BB_ROOT: result = t->root_size(); break;
	  case BB_BRANCH: result = t->branch_size(); break;
	  case BB_LEAF: result = t->leaf_size(); break;
	  default:
	    {
	      Flu_Tree_Browser::Node *n = t->find(item);
	      if(n)
		n->label_size();

	      break;
	    }
	  }
      }

      break;
#endif
    case BB_ENTRY:
    case BB_EDIT:
    case BB_INTENTRY:
    case BB_FLOATENTRY:
      result = ((Fl_Input *)widget_ptr(widget))->textsize(); break;

    case BB_TEXTEDITOR:
      result = ((Fl_Text_Editor *)widget_ptr(widget))->textsize(); break;

    case BB_HTMLVIEW:
      result = ((Fl_Help_View *)widget_ptr(widget))->textsize(); break;
    }

    break;

  case BB_TEXTFONT:
    result = 0;

    switch(widget_type(widget)) {
#ifdef BB_USE_FLU
    case BB_TREE:
      {
	Flu_Tree_Browser *t = (Flu_Tree_Browser *)widget_ptr(widget);
	switch(item)
	  {
	  case BB_ROOT: result = t->root_font(); break;
	  case BB_BRANCH: result = t->branch_font(); break;
	  case BB_LEAF: result = t->leaf_font(); break;
	  default:
	    {
	      Flu_Tree_Browser::Node *n = t->find(item);
	      if(n)
		n->label_font();
	      break;
	    }
	  }
      }

      break;

#endif
    case BB_ENTRY:
    case BB_EDIT:
    case BB_INTENTRY:
    case BB_FLOATENTRY:
      result = ((Fl_Input *)widget_ptr(widget))->textfont(); break;

    case BB_TEXTEDITOR:
      result = ((Fl_Text_Editor *)widget_ptr(widget))->textfont(); break;

    case BB_HTMLVIEW:
      result = ((Fl_Help_View *)widget_ptr(widget))->textfont(); break;
    }

    break;

  case BB_SELECTIONCOLOR:
#ifdef BB_USE_FLU
    if(widget_type(widget) == BB_TREE)
      result = ((Flu_Tree_Browser *)widget_ptr(widget))->selection_color();
    else
#endif
    result = w->selection_color();
    break;

  case BB_VISIBLE: result = w->visible(); break;
  case BB_WHEN: result = w->when(); break;

  case BB_POSITION: 
    result = 0;

    switch(widget_type(widget)) {
    case BB_EDIT:
    case BB_ENTRY:
    case BB_INTENTRY:
    case BB_FLOATENTRY:
      result = ((Fl_Input_ *)widget_ptr(widget))->position();
      break;

    case BB_TEXTEDITOR:
      result = ((Fl_Text_Editor *)widget_ptr(widget))->insert_position();
      break;
    }

    break;

  case BB_MARK: 
    result = 0;

    switch(widget_type(widget)) {
    case BB_EDIT:
    case BB_ENTRY:
    case BB_INTENTRY:
    case BB_FLOATENTRY:
      result = ((Fl_Input_ *)widget_ptr(widget))->mark(); break;

    case BB_TEXTEDITOR:
      result = ((Fl_Text_Editor *)widget_ptr(widget))->insert_position(); break;
    }

    break;

  case BB_READONLY: result = ((Fl_Input_ *)widget_ptr(widget))->readonly(); break;
  case BB_IMAGEWIDTH: result = ((BBLiveImage *)widget_ptr(widget))->width(); break;
  case BB_IMAGEHEIGHT: result = ((BBLiveImage *)widget_ptr(widget))->height(); break;
  case BB_FOCUS: result = 0; break;
  case BB_SIZE: result = ((Fl_Menu_ *)widget_ptr(widget))->size(); break;
  case BB_ALIGN:
    if(widget_type(widget) == BB_TABLE && item >= 0)
      result = ((Table *)widget_ptr(widget))->columnAlign(item);
    else
      result = w->align();

    break;

#ifdef BB_USE_FLU
  case BB_CALLBACK_REASON:
    if(widget_type(widget) == BB_TREE)
      result = ((Flu_Tree_Browser *)widget_ptr(widget))->callback_reason();
    else
      result = -1;

    break;
  case BB_CALLBACK_NODE:
    if(widget_type(widget) == BB_TREE)
    {
      Flu_Tree_Browser::Node *n = ((Flu_Tree_Browser *)widget_ptr(widget))->callback_node();
      if(n)
	result = n->id();
    }
    else result = -1;

    break;
#endif
  default: 
    getting_int_property = true;
    result = (int)BBGetDoubleProperty(widget, property);
    getting_int_property = false;
  }

  return result;
}


double BBGetDoubleProperty(WIDGET widget, int property)
{
  static bool getting_double_property = false;

  if(getting_double_property) return 0;

  double result;

  switch(property) {
  case BB_VALUE:
    switch(widget_type(widget)) {
    case BB_RADIOBUTTON:
    case BB_CHECKBOX:
      result = ((Fl_Button *)widget_ptr(widget))->value(); break;
    case BB_CHOICEBUTTON:
      result = ((Fl_Choice *)widget_ptr(widget))->value(); break;
    case BB_PROGRESS:
      result = ((Fl_Progress *)widget_ptr(widget))->value(); break;
    case BB_CLOCK:
      result = (double)((Fl_Clock *)widget_ptr(widget))->value(); break;
    default:
      result = ((Fl_Valuator *)widget_ptr(widget))->value(); break;
    }

    break;
  case BB_MAXIMUM:
    switch(widget_type(widget)) {
    case BB_PROGRESS: result = ((Fl_Progress *)widget_ptr(widget))->maximum(); break;
    case BB_DIAL: result = ((Fl_Dial *)widget_ptr(widget))->angle2(); break;
    default: 
      getting_double_property = true;
      result = BBGetIntProperty(widget, -1, property);
      getting_double_property = false;
      break;
    }

  case BB_MINIMUM:
    switch(widget_type(widget)) {
    case BB_PROGRESS: result = ((Fl_Progress *)widget_ptr(widget))->minimum(); break;
    case BB_DIAL: result = ((Fl_Dial *)widget_ptr(widget))->angle1(); break;
    default:
      getting_double_property = true;
      result = BBGetIntProperty(widget, -1, property);
      getting_double_property = false;
      break;
    }

  default: 
    getting_double_property = true;
    result = BBGetIntProperty(widget, -1, property);
    getting_double_property = false;
    break;
  }

  return result;
}


char *BBGetStringProperty(WIDGET widget, int item, int item2, int property)
{
  Fl_Widget *w = (Fl_Widget *)widget_ptr(widget);

  switch(property) {
  case BB_TEXT: 
    switch(widget_type(widget)) {
    case BB_EDIT:
    case BB_INTENTRY:
    case BB_FLOATENTRY:
    case BB_ENTRY: return (char *)((Fl_Input *)widget_ptr(widget))->value();
    case BB_GLWINDOW:
    case BB_DOUBLEWINDOW:
    case BB_TEXTEDITOR: return ((Fl_Text_Editor *)widget_ptr(widget))->buffer()->text();
    case BB_WINDOW: return (char *)((Fl_Window *)widget_ptr(widget))->label();
#ifdef BB_USE_FLU
    case BB_TREE:
      if(item == BB_ROOT)
	return (char *)((Flu_Tree_Browser *)widget_ptr(widget))->label();
      else
      {
	Flu_Tree_Browser::Node *n = ((Flu_Tree_Browser *)widget_ptr(widget))->find(item);
	if(n)
	  return (char *)n->label();
	else
	  return 0;
      }
#endif
    case BB_HTMLVIEW: return (char *)((Fl_Help_View *)w)->value();
    case BB_TABLE:
      if(item2 > -1)
      {
	if(item > -1)
	  return ((Table *)w)->valueAt(item, item2);
	else
	  return (char *)((Table *)w)->columnTitle(item2);
      } /* fall through! */
    default: return (char *)w->label();
    }

  case BB_TOOLTIP: return (char *)w->tooltip();

  case BB_VALUE:
    if(widget_type(widget) == BB_HTMLVIEW)
      return (char *)((Fl_Help_View *)w)->filename();

  default: return 0;
  }
}


int BBRunEventLoop(bool wait, double tm)
{
  if(!wait) return (int)Fl::wait(tm);
  else return Fl::run();
}

static char *args[BBMAX_SHOW_ARGS] = {0};
void BBSetArg(int i, char *a)
{
  if(i < BBMAX_SHOW_ARGS)
    args[i] = strdup(a);
}


void BBShowWindow(WIDGET widget, int argc)
{
  Fl_Window *w = (Fl_Window *)widget_ptr(widget);
  if(argc)
  {
    Fl::args(argc, args);
    w->show(argc, args);
    for(int i = 0; i < argc; i++)
      free(args[i]);
  }
  else
    w->show();
}


void BBRedrawWidget(WIDGET widget)
{
  Fl_Widget *w = (Fl_Widget *)widget_ptr(widget);
  w->redraw();
}


void BBBeginGroup(WIDGET widget)
{
  ((Fl_Group *)widget_ptr(widget))->begin();
}


void BBEndGroup(WIDGET widget)
{
  ((Fl_Group *)widget_ptr(widget))->end();  
}


void* BBPixmap(void *xpm)
{
  return (void *)new BBImage(new Fl_Pixmap((char *const *)xpm));
}


void BBSetImage(WIDGET widget, int item, void *img)
{
  switch(widget_type(widget)) {
  case BB_LIVEIMAGE:
    ((BBLiveImage *)widget_ptr(widget))->setData((unsigned char *)img);
    break;
#ifdef BB_USE_FLU
  case BB_TREE:
  {
    Flu_Tree_Browser *t = ((Flu_Tree_Browser *)widget_ptr(widget));
    Fl_Image *im = ((BBImage *)img)->get();
    switch(item)
    {
    case BB_ROOT:
      if(t->get_root())
	t->get_root()->branch_icons(im, im);
      break;
    case BB_BRANCH: t->branch_icons(im, im); break;
    case BB_LEAF: t->leaf_icon(im); break;
    case BB_CONNECTOR: t->collapse_icons(im, im); break;
    default:
    {
      Flu_Tree_Browser::Node *n = t->find(item);
      if(n)
      {
	if(n->is_branch())
	  n->branch_icons(im, im);
        else
	  n->leaf_icon(im);
      }
      break;
    }
    }
    break;
  }
#endif
  default:
    Fl_Image *im = ((BBImage *)img)->get();
    ((Fl_Widget *)widget_ptr(widget))->image(im);
  }
}


void BBSetImage2(WIDGET widget, int item, void *iclosed, void *iopen)
{
#ifdef BB_USE_FLU
  if(widget_type(widget) == BB_TREE)
  {
    Flu_Tree_Browser *t = ((Flu_Tree_Browser *)widget_ptr(widget));
    Fl_Image *i1 = ((BBImage *)iclosed)->get();
    Fl_Image *i2 = ((BBImage *)iopen)->get();
    switch(item)
    {
    case BB_ROOT:
      if(t->get_root())
	t->get_root()->branch_icons(i1, i2);
      break;
    case BB_BRANCH: t->branch_icons(i1, i2); break;
    case BB_CONNECTOR: t->collapse_icons(i1, i2); break;
    default:
    {
      Flu_Tree_Browser::Node *n = t->find(item);
      if(n)
      {
	if(n->is_branch())
	  n->branch_icons(i1, i2);
        else
	  n->leaf_icon(i1);
      }
      break;
    }
    }
  }
#endif
}


void *BBRawImage(void *data, int w, int h, int d)
{
  return new BBImage(new Fl_RGB_Image((const unsigned char *)data, w, h, d));
}


void *BBLoadImage(char *name)
{
  return new BBImage(name);
}

int BBImageDim(void *image, int dim)
{
  Fl_Image *img = ((BBImage *)image)->get();
  switch(dim)
  {
  case BB_IMAGEWIDTH: return img->w();
  case BB_IMAGEHEIGHT: return img->h();
  case BB_COLOR: return img->d();
  case BB_SIZE: return img->count();
  default: return 0;
  }
}

void *BBImageData(void *image, int i)
{
  Fl_Image *img = ((BBImage *)image)->get();
  return (void *)(img->data()[i]);
}


void BBAddItem(WIDGET widget, char *str, int pos)
{
  switch(widget_type(widget)) {
  case BB_LIST:
    {
      Fl_Browser *b = (Fl_Browser *)widget_ptr(widget);
      if(pos > 0)
	b->insert(pos, str);
      else
      {
        b->add(str);
        b->bottomline(b->size());
      }
    }

    break;

  case BB_ENTRY:
  case BB_EDIT:
  case BB_INTENTRY:
  case BB_FLOATENTRY:
    ((Fl_Input *)widget_ptr(widget))->insert(str);
    break;

  case BB_TEXTEDITOR:
    {
      Fl_Text_Editor *ep = (Fl_Text_Editor *)widget_ptr(widget);

      if(pos != 1) ep->buffer()->insert(ep->insert_position(), str);
      else ep->buffer()->replace_selection(str);

      break;
    }
  }
}


void BBRemoveItem(WIDGET widget, int n)
{
  if(widget_type(widget) == BB_LIST) {
    if(n == -1) ((Fl_Browser *)widget_ptr(widget))->clear();
    else ((Fl_Browser *)widget_ptr(widget))->remove(n + 1);
  }
#ifdef BB_USE_FLU
  else if(widget_type(widget) == BB_TREE) {
    ((Flu_Tree_Browser *)widget_ptr(widget))->remove(n);
  }
#endif
  else if(widget_type(widget) == BB_TABLE) {
    ((Table *)widget_ptr(widget))->removeRow(n);
  }
}

int BBMessage(int type, char *text, char *def, char *c1, char *c2)
{
  switch(type) {
  case BB_MESSAGE: fl_message("%s", text); return 0;
  case BB_ALERT: fl_alert("%s", text); return 0;
  case BB_ASK: return fl_ask("%s", text);
  case BB_CHOICE: return fl_choice("%s", def, c1, c2, text);
  default: return 0;
  }
}


char *BBSelectFile(char *message, char *pattern, char *fname)
{
  return fl_file_chooser(message, pattern, fname);
}


char *BBSelectDir(char *message, char *fname)
{
  return fl_dir_chooser(message, fname, 0);
}


void *BBAddMenuItem(WIDGET menu, char *label, char *shortcut, BBCallback callback)
{
  ((Fl_Menu_ *)widget_ptr(menu))->add(label, shortcut != 0 ? strdup(shortcut) : 0, callback, (void *)cb_count);
  return (void *)(cb_count++);
}


int BBAddTreeItem(WIDGET tree, char *text, int parent, int pos, WIDGET widget)
{
#ifdef BB_USE_FLU
  Flu_Tree_Browser *t = (Flu_Tree_Browser *)widget_ptr(tree);
  Fl_Widget *w = widget && widget != (WIDGET)6 // not #f
      ? (Fl_Widget *)widget_ptr(widget) : 0;
  Flu_Tree_Browser::Node *n = 0;
  Flu_Tree_Browser::Node *p = 0;
  if(parent != -1 && (p = t->find(parent)))
  {
    if(pos != -1) // add with parent and posistion 
    {
      n = p->insert(text, pos);
      if(n)
	n->widget(w);
    }
    else // add with parent
      n = t->add(p, text, w);
  }
  else // add with full path
    n = t->add(text, w);
  if(n)
    return n->id();
  else
    return -1;
#else
  return -1;
#endif
}


void BBAddWidget(WIDGET widget, WIDGET child)
{
  ((Fl_Group *)widget_ptr(widget))->add((Fl_Widget *)widget_ptr(child));
}


void BBSetWidgetProperty(WIDGET w1, int property, WIDGET w2)
{
  switch(property) {
  case BB_RESIZABLEWIDGET: ((Fl_Group *)widget_ptr(w1))->resizable((Fl_Widget *)widget_ptr(w2)); break;
  case BB_TABS: ((Fl_Tabs *)widget_ptr(w1))->value((Fl_Widget *)widget_ptr(w2)); break;
  }
}


void BBRemoveAllItems(WIDGET widget)
{
  switch(widget_type(widget)) {
  case BB_LIST:
    ((Fl_Browser *)widget_ptr(widget))->clear();
    break;
#ifdef BB_USE_FLU
  case BB_TREE:
    ((Flu_Tree_Browser *)widget_ptr(widget))->clear();
    break;
#endif
  case BB_TABLE:
    ((Table *)widget_ptr(widget))->clear(true); // remove columns too
    break;
  }
}


void BBRemoveImage(void *img)
{
  delete (BBImage *)img;
}


unsigned long BBRGB(int r, int g, int b)
{
  return fl_rgb_color(r, g, b);
}


char *BBGetSelection(WIDGET widget)
{
  switch(widget_type(widget)) {
  case BB_ENTRY:
  case BB_EDIT:
  case BB_INTENTRY:
  case BB_FLOATENTRY:
    {
      Fl_Input_ *inp = (Fl_Input_ *)widget_ptr(widget);
      int p = inp->position();
      int m = inp->mark();
      int n = p - m;
      
      if(n < 0) n = -n;
      
      char *buf = (char *)malloc(n + 1);
      
      if(buf == 0) return 0;
      
      strncpy(buf, inp->value() + (p < m ? p : m), n);
      buf[ n ] = '\0';
      return buf;
    }

  case BB_TEXTEDITOR:
    {
      int p1, p2;
      Fl_Text_Buffer *bp = ((Fl_Text_Editor *)widget_ptr(widget))->buffer();
      bp->selection_position(&p1, &p2);

      if(!bp->selected()) return 0;

      int n = p2 - p1;
      char *buf = (char *)malloc(n + 1);
      
      if(buf == 0) return 0;

      memcpy(buf, bp->text_range(p1, p2), n);
      buf[ n ] = '\0';
      return buf;
    }

  default:
    return 0;
  }
}


void BBSetSelection(WIDGET widget, int start, int end)
{
  if(widget_type(widget) == BB_TEXTEDITOR)
    ((Fl_Text_Editor *)widget_ptr(widget))->buffer()->select(start, end);
}


void BBRedrawGLWindow(Fl_Gl_Window *w)
{
  BBGLWindow *glw = (BBGLWindow *)w;
  glw->cb(glw, glw->cbid);
}


void *BBAddResource(WIDGET widget, void *res, int index)
{
  void **rptr = widget_rlist(widget);
  void *old = rptr[ index ];

  if(old != 0) free(old);
  
  rptr[ index ] = res;
  return res;
}


int BBSelectColor(char *title, unsigned char *rgb)
{
  return fl_color_chooser(title, rgb[ 0 ], rgb[ 1 ], rgb[ 2 ]);
}


int BBSelectColorIndex(int col)
{
  return fl_show_colormap((Fl_Color)col);
}


char *BBGetInput(char *label, char *def)
{
  return (char *)fl_input("%s", def, label);
}


void BBActivateMenuItem(WIDGET mbar, int index, bool flag)
{
  if(widget_type(mbar) != BB_MENUBAR) return;

  Fl_Menu_ *m = (Fl_Menu_ *)widget_ptr(mbar);
  int s = m->size();

  if(index < s) {
    const Fl_Menu_Item *item = m->menu();
    int bits = item[ index ].flags;
    m->mode(index, flag ? (bits & ~FL_MENU_INACTIVE) : (bits | FL_MENU_INACTIVE));
  }
}

void BBAddTableColumn(WIDGET widget, char *text)
{
    ((Table *)widget_ptr(widget))->addColumn(text);
}

void BBAddTableCell(WIDGET widget, char *text)
{
    ((Table *)widget_ptr(widget))->addCell(text);
}

int BBGetEventInt(int event)
{
  switch(event)
  {
  case BBE_CLICKS:
    return Fl::event_clicks();
  case BBE_KEY:
    return Fl::event_key();
  case BBE_LENGTH:
    return Fl::event_length();
  case BBE_X:
    return Fl::event_x();
  case BBE_Y:
    return Fl::event_y();
  case BBE_X_ROOT:
    return Fl::event_x_root();
  case BBE_Y_ROOT:
    return Fl::event_y_root();
  case BBE_DX:
    return Fl::event_dx();
  case BBE_DY:
    return Fl::event_dy();
  }
  return 0;
}

bool BBGetEventBool(int event)
{
  switch(event)
  {
  case BBE_ALT:
    return Fl::event_alt();
  case BBE_BUTTON1:
    return Fl::event_button1();
  case BBE_BUTTON2:
    return Fl::event_button2();
  case BBE_BUTTON3:
    return Fl::event_button3();
  case BBE_CTRL:
    return Fl::event_ctrl();
  case BBE_IS_CLICK:
    return Fl::event_is_click();
  case BBE_SHIFT:
    return Fl::event_shift();
  }
  return false;
}

char *BBGetEventString(int event)
{
  if(event == BBE_TEXT)
    return (char *)Fl::event_text();
  return 0;
}

void BBSetEventInt(int event, int val)
{
  if(event == BBE_CLICKS)
    Fl::event_clicks(val);
}

void BBSetEventBool(int event, bool /*val*/)
{
  if(event == BBE_IS_CLICK)
    Fl::event_is_click(0);
}
