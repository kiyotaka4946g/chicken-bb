/* bb.h */

#define WIDGET_RESOURCE_MAX      2

#define WRES_LABEL     0
#define WRES_TOOLTIP   1

enum {
  BB_WINDOW,
  BB_DOUBLEWINDOW,
  BB_BUTTON,
  BB_RETURNBUTTON,
  BB_CHECKBOX,
  BB_CHOICEBUTTON,
  BB_MENUBUTTON,
  BB_ENTRY,
  BB_EDIT,
  BB_INTENTRY,
  BB_FLOATENTRY,
  BB_COUNTER,
  BB_DIAL,
  BB_LABEL,
  BB_SLIDER,
  BB_ADJUSTER,
  BB_ROLLER,
  BB_LIST,
  BB_RADIOBUTTON,
  BB_MENUBAR,
  BB_PROGRESS,
  BB_TABS,
  BB_GROUP,
  BB_TILE,
  BB_PACK,
  BB_SCROLL,
  BB_LIGHTBUTTON,
  BB_GLWINDOW,
  BB_CLOCK,
  BB_LIVEIMAGE,
  BB_TREE,
  BB_TEXTEDITOR,
  BB_TABLE,
  BB_HTMLVIEW
};

enum {
  BB_X,
  BB_Y,
  BB_WIDTH,
  BB_HEIGHT,
  BB_TEXT,
  BB_VALUE,
  BB_BOX,
  BB_TYPE,
  BB_RESIZABLE,
  BB_MODAL,
  BB_DIRECTION,
  BB_COLOR,
  BB_FOCUS,
  BB_RESIZABLEWIDGET,
  BB_MAXIMUM,
  BB_MINIMUM,
  BB_LABELCOLOR,
  BB_LABELSIZE,
  BB_LABELFONT,
  BB_SPACING,
  BB_POSITION,
  BB_XPOSITION,
  BB_YPOSITION,
  BB_TEXTCOLOR,
  BB_TEXTSIZE,
  BB_TEXTFONT,
  BB_SELECTIONCOLOR,
  BB_TOOLTIP,
  BB_VISIBLE,
  BB_MARK,
  BB_READONLY,
  BB_WHEN,
  BB_VALIDCONTEXT,
  BB_CALLBACK,
  BB_IMAGE,
  BB_IMAGEWIDTH,
  BB_IMAGEHEIGHT,
  BB_SIZE,
  BB_ALIGN,
  BB_HANDLER,
  BB_CALLBACK_REASON, // for Tree
  BB_CALLBACK_NODE
};

/* flags for item-properties */
enum
{
  BB_ROOT = -1,
  BB_BRANCH = -2,
  BB_LEAF = -3,
  BB_CONNECTOR = -4
};

enum {
  BB_GRAY0		= 32,	// 'A'
  BB_DARK3		= 39,	// 'H'
  BB_DARK2		= 45,   // 'N'
  BB_DARK1		= 47,	// 'P'
  BB_LIGHT1		= 50,	// 'S'
  BB_LIGHT2		= 52,	// 'U'
  BB_LIGHT3		= 54,	// 'W'

  BB_GRAY               = 49,

  BB_BLACK		= 56,
  BB_RED		= 88,
  BB_GREEN		= 63,
  BB_YELLOW		= 95,
  BB_BLUE		= 216,
  BB_MAGENTA		= 248,
  BB_CYAN		= 223,
  BB_DARK_RED		= 72,

  BB_DARK_GREEN		= 60,
  BB_DARK_YELLOW	= 76,
  BB_DARK_BLUE		= 136,
  BB_DARK_MAGENTA	= 152,
  BB_DARK_CYAN		= 140,

  BB_WHITE		= 255
};

#define BB_NORMAL_DIAL	0
#define BB_LINE_DIAL	1
#define BB_FILL_DIAL	2

enum {
  BB_VERTICAL,
  BB_HORIZONTAL,
  BB_VERT_FILL_SLIDER,
  BB_HOR_FILL_SLIDER,
  BB_VERT_NICE_SLIDER,
  BB_HOR_NICE_SLIDER
};

enum {
  BB_NO_BOX,
  BB_FLAT_BOX,
  BB_UP_BOX,		BB_DOWN_BOX,
  BB_UP_FRAME,		BB_DOWN_FRAME,
  BB_THIN_UP_BOX,	BB_THIN_DOWN_BOX,
  BB_THIN_UP_FRAME,	BB_THIN_DOWN_FRAME,
  BB_ENGRAVED_BOX,	BB_EMBOSSED_BOX,
  BB_ENGRAVED_FRAME,	BB_EMBOSSED_FRAME,
  BB_BORDER_BOX
};

enum {
  BB_MESSAGE,
  BB_ALERT,
  BB_ASK,
  BB_CHOICE
};

enum {
  BB_SCROLL_HORIZONTAL = 1,
  BB_SCROLL_VERTICAL = 2,
  BB_SCROLL_BOTH = 3,
  BB_SCROLL_ALWAYS_ON = 4,
  BB_SCROLL_HORIZONTAL_ALWAYS = 5,
  BB_SCROLL_VERTICAL_ALWAYS = 6,
  BB_SCROLL_BOTH_ALWAYS = 7
};

enum {
  BB_HELVETICA		= 0,
  BB_HELVETICA_BOLD,
  BB_HELVETICA_ITALIC,
  BB_HELVETICA_BOLD_ITALIC,
  BB_COURIER,
  BB_COURIER_BOLD,
  BB_COURIER_ITALIC,
  BB_COURIER_BOLD_ITALIC,
  BB_TIMES,
  BB_TIMES_BOLD,
  BB_TIMES_ITALIC,
  BB_TIMES_BOLD_ITALIC,
  BB_SYMBOL,
  BB_SCREEN,
  BB_SCREEN_BOLD
};

enum {
  BB_CENTER		= 0,
  BB_TOP		= 1,
  BB_BOTTOM		= 2,
  BB_LEFT		= 4,
  BB_RIGHT		= 8,
  BB_INSIDE		= 16,
  BB_TEXT_OVER_IMAGE	= 32,
  BB_IMAGE_OVER_TEXT	= 0,
  BB_CLIP		= 64,
  BB_WRAP		= 128
};

enum {
  BB_WHEN_NEVER		= 0,
  BB_WHEN_CHANGED	= 1,
  BB_WHEN_RELEASE	= 4,
  BB_WHEN_RELEASE_ALWAYS= 6,
  BB_WHEN_ENTER_KEY	= 8,
  BB_WHEN_NOT_CHANGED	= 2 // modifier bit to disable changed() test
};

#define BBMAX_SHOW_ARGS 32

/* event properties */
enum {
  BBE_ALT,
  BBE_BUTTON1,
  BBE_BUTTON2,
  BBE_BUTTON3,
  BBE_CLICKS,
  BBE_CTRL,
  BBE_IS_CLICK,
  BBE_KEY,
  BBE_LENGTH,
  BBE_SHIFT,
  BBE_X,
  BBE_Y,
  BBE_X_ROOT,
  BBE_Y_ROOT,
  BBE_DX,
  BBE_DY,
  BBE_TEXT
};

/* keyboard codes */
enum {
  BB_Button	    = 0xfee8, // use Fl_Button + n for button n
  BB_BackSpace	    = 0xff08,
  BB_Tab	    = 0xff09,
  BB_Enter	    = 0xff0d,
  BB_Pause	    = 0xff13,
  BB_Scroll_Lock    = 0xff14,
  BB_Escape	    = 0xff1b,
  BB_Home	    = 0xff50,
  BB_Left	    = 0xff51,
  BB_Up		    = 0xff52,
  BB_Right	    = 0xff53,
  BB_Down	    = 0xff54,
  BB_Page_Up	    = 0xff55,
  BB_Page_Down	    = 0xff56,
  BB_End	    = 0xff57,
  BB_Print	    = 0xff61,
  BB_Insert	    = 0xff63,
  BB_Menu	    = 0xff67, // the "menu/apps" key on XFree86
  BB_Help	    = 0xff68, // the 'help' key on Mac keyboards
  BB_Num_Lock	    = 0xff7f,
  BB_KP		    = 0xff80, // use FL_KP+'x' for 'x' on numeric keypad
  BB_KP_Enter	    = 0xff8d, // same as Fl_KP+'\r'
  BB_KP_Last	    = 0xffbd, // use to range-check keypad
  BB_F		    = 0xffbd, // use FL_F+n for function key n
  BB_F_Last	    = 0xffe0, // use to range-check function keys
  BB_Shift_L	    = 0xffe1,
  BB_Shift_R	    = 0xffe2,
  BB_Control_L	    = 0xffe3,
  BB_Control_R	    = 0xffe4,
  BB_Caps_Lock	    = 0xffe5,
  BB_Meta_L	    = 0xffe7, // the left MSWindows key on XFree86
  BB_Meta_R	    = 0xffe8, // the right MSWindows key on XFree86
  BB_Alt_L	    = 0xffe9,
  BB_Alt_R	    = 0xffea,
  BB_Delete	    = 0xffff
};

/* callback reasons */
enum {
  BB_HILIGHTED,
  BB_UNHILIGHTED,
  BB_SELECTED,
  BB_UNSELECTED,
  BB_OPENED,
  BB_CLOSED,
  BB_DOUBLE_CLICK,
  BB_WIDGET_CALLBACK,
  BB_MOVED_NODE,
  BB_NEW_NODE,
  BB_NOTHING
};

BBEXPORT double BBStart(char *);
BBEXPORT void BBStop();
BBEXPORT void *BBCreateWidget(int type, int x, int y, int w, int h);
BBEXPORT ___safe void BBDestroyWidget(WIDGET widget);
BBEXPORT ___safe void BBSetIntProperty(WIDGET widget, int item, int property, int value);
BBEXPORT ___safe void BBSetDoubleProperty(WIDGET widget, int property, double value);
BBEXPORT ___safe void BBSetStringProperty(WIDGET widget, int item, int item2, int property, char *value);
BBEXPORT int BBGetIntProperty(WIDGET widget, int item, int property);
BBEXPORT double BBGetDoubleProperty(WIDGET widget, int property);
BBEXPORT char *BBGetStringProperty(WIDGET widget, int item, int item2, int property);
BBEXPORT ___safe int BBRunEventLoop(___bool wait, double secs);
BBEXPORT ___safe void BBShowWindow(WIDGET widget, int argc);
BBEXPORT void BBRedrawWidget(WIDGET widget);
BBEXPORT void BBBeginGroup(WIDGET widget);
BBEXPORT void BBEndGroup(WIDGET widget);
BBEXPORT void *BBPixmap(void *xpm);
BBEXPORT void BBSetImage(WIDGET widget, int item, void *img);
BBEXPORT void BBSetImage2(WIDGET widget, int item, void *iclosed, void *iopen);
BBEXPORT void BBAddItem(WIDGET widget, char *str, int pos);
BBEXPORT int BBAddTreeItem(WIDGET tree, char *text, int parent, int pos, WIDGET widget);
BBEXPORT void BBRemoveItem(WIDGET widget, int n);
BBEXPORT int BBMessage(int type, char *text, char *c1, char *c2, char *c3);
BBEXPORT char *BBSelectFile(char *message, char *pattern, char *fname);
BBEXPORT char *BBSelectDir(char *message, char *fname);
BBEXPORT void BBSetWidgetProperty(WIDGET w1, int property, WIDGET w2);
BBEXPORT void BBRemoveAllItems(WIDGET widget);
BBEXPORT void BBAddWidget(WIDGET w, WIDGET c);
BBEXPORT unsigned long BBRGB(int r, int g, int b);
BBEXPORT char *BBGetSelection(WIDGET widget);
BBEXPORT void *BBLoadImage(char *name);
BBEXPORT void *BBRawImage(void *data, int w, int h, int d);
BBEXPORT void BBRemoveImage(void *img);
BBEXPORT int BBSelectColor(char *title, unsigned char *rgb);
BBEXPORT int BBSelectColorIndex(int col);
BBEXPORT char *BBGetInput(char *label, char *def);
BBEXPORT void BBActivateMenuItem(WIDGET m, int index, ___bool flag);
BBEXPORT int BBGetEventInt(int event);
BBEXPORT ___bool BBGetEventBool(int event);
BBEXPORT char *BBGetEventString(int event);
BBEXPORT void BBSetEventInt(int event, int val);
BBEXPORT void BBSetEventBool(int event, ___bool val);
BBEXPORT void BBSetArg(int i, char *a);
BBEXPORT int BBImageDim(void *data, int dim);
BBEXPORT void *BBImageData(void *image, int i);
BBEXPORT void BBAddTableColumn(WIDGET widget, char *text);
BBEXPORT void BBAddTableCell(WIDGET widget, char *text);
BBEXPORT void BBSetSelection(WIDGET widget, int start, int end);
