/*
 Copyright (c) 2004 Markus Niemist?

 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or
 sell copies of the Software, and to permit persons to whom
 the Software is furnished to do so, subject to the following
 conditions:

 The above copyright notice and this permission notice shall
 be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
*/
/*
 * Modified by Sergey Khorev
 */
#include <FL/fl_draw.H>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "Table.h"

#define DAMAGE_HEADER	FL_DAMAGE_USER1
#define DAMAGE_ROWS	FL_DAMAGE_USER2

int stringcompare(const char *s1, const char *s2)
{
    if (!s1)
	return (s2 ? -1 : 0);
    else if (!s2)
	return (s1 ? 1 : 0);
    else
#ifdef _MSC_VER
    return stricmp(s1, s2);
#else
    return strcasecmp(s1, s2);
#endif
}

/*
 * ======================================
 *  void Table.drawHeader(int x, int y);
 * ======================================
 *
 * Draws header buttons starting at (x, y).
 */
void Table::drawHeader(int x, int y)
{
    int w;
    struct ColumnInfo col;

    fl_font(labelfont(), headerHeight - 4);

    /*
     * Draw all header cells that aren't clipped.
     */
    for (int i = leftCol; i <= rightCol; i++)
    {
	col = header[i];

	// Draw box
	if (pushed != i)
	    fl_draw_box(FL_THIN_UP_BOX, x, y, w = col.width,
		    headerHeight, FL_GRAY);
	else
	    fl_draw_box(FL_THIN_DOWN_BOX, x, y, w = col.width,
		    headerHeight, FL_GRAY);

	fl_color(labelcolor());

	// Draw labels
	if (col.title != NULL)
	    fl_draw(col.title, x + 2, y - 1, w - 2, headerHeight,
		    col.align);
	x += w;

	// Draw "the sort arrow", if any.
	if (sortColumn == i)
	{
	    int mod = headerHeight - 10;

	    if (!ascent)
		fl_polygon(x - mod - 6, y + 5, x - mod / 2 - 6,
			y + mod + 5, x - 6, y + 5);
	    else
		fl_polygon(x - mod - 6, y + mod + 5,
			x - mod / 2 - 6, y + 5, x - 6, y + mod + 5);
	}
    }
}


/*
 * =============================================================
 *  void Table.drawRow(int row, char *rowData[], int x, int y);
 * =============================================================
 *
 * Draws all items in the row. Starts drawing from (x, y).
 */
void Table::drawRow(int row, char *rowData[], int x, int y)
{
    int w;
    ColumnInfo col;

    fl_font(labelfont(), rowHeight - 3);

    // Draw background box.
    if (row != selected)
    {
	fl_rectf(iX, y, tableWidth - hScroll->value(), rowHeight, color());
	fl_color(labelcolor());
    }
    else if (Fl::focus() == this)
    {
	fl_rectf(iX, y, tableWidth - hScroll->value(), rowHeight, selection_color());
	fl_color(color());

	// Draw focus
	fl_line_style(FL_DOT);
	fl_rect(iX, y, tableWidth - hScroll->value(), rowHeight);
	fl_line_style(FL_SOLID);
    }
    else
    {
	fl_rectf(iX, y, tableWidth - hScroll->value(), rowHeight, selection_color());
	fl_color(color());
    }

    const char *str;

    // Draw the data.
    for (int i = leftCol; i <= rightCol; i++)
    {
	w = (col = header[i]).width;

	if ((str = rowData[i]) != NULL)
	    fl_draw(str, x, y - 1, w - 2, rowHeight + 1, col.align);
	x += w;
    }
}


/*
 * =======================================================
 *  Table.Table(int x, int y, int w, int h, char *label);
 * =======================================================
 *
 * This is standard FLTK constructor. See FLTK documentation for
 * more information.
 */
    Table::Table(int x, int y, int w, int h, char *label)
: Fl_Group(x, y, w, h, label)
{
    labelsize(18);

    // Setup variables.
    align((Fl_Align)(FL_ALIGN_LEFT | FL_ALIGN_CLIP));
    box(FL_THIN_DOWN_FRAME);
    color(FL_BACKGROUND2_COLOR, FL_SELECTION_COLOR);
    when(FL_WHEN_CHANGED);

    // Create scrollbars.
    vScroll = new Fl_Scrollbar(x + w - scrollbarSize, y, scrollbarSize, h + scrollbarSize);
    vScroll->type(FL_VERTICAL);
    vScroll->linesize(3 * rowHeight);
    vScroll->callback(scrollCallback, (void*)this);
    vScroll->hide();

    hScroll = new Fl_Scrollbar(x, y + h - scrollbarSize, w, scrollbarSize);
    hScroll->type(FL_HORIZONTAL);
    hScroll->callback(scrollCallback, (void*)this);
    hScroll->hide();

    Fl_Group::end();

    // Setup the rest of the variables to reasonable defaults.
    nCols = nRows = 0;
    cPos = 0;

    dragX = 0;
    resizing = -1;
    pushed = -1;

    curRow = NULL;

    sortColumn = -1;
    selected = -1;
    canSort = true;
    canResize = true;
    ascent = false;
    noMoreColumns = false;
    dimensionsChanged = false;
    toBeSorted = false;
    headerEnabled = true;

}


/*
 * =================
 *  Table.~Table();
 * =================
 *
 * Destructor.
 */
Table::~Table()
{
    clear();
}


/*
 * ====================================
 *  bool Table.headerOn();
 *  void Table.headerOn(bool enabled);
 * ====================================
 *
 * These methods get or set the value of variable controlling
 * whether header buttons are displayed.
 */
bool Table::headerOn() const
{
    return headerEnabled;
}

void Table::headerOn(bool enabled)
{
    headerEnabled = enabled;
    dimensionsChanged = true;
    redraw();
}


/*
 * =====================================
 *  bool Table.allowResize();
 *  void Table.allowResize(bool allow);
 * =====================================
 *
 * These methods get or set the value of variable controlling
 * whether user may resize columns by dragging the column border.
 */
bool Table::allowResize() const
{
    return canResize;
}

void Table::allowResize(bool allow)
{
    canResize = allow;
}


/*
 * ===================================
 *  bool Table.allowSort();
 *  void Table.allowSort(bool allow);
 * ===================================
 *
 * These methods get or set the value of variable controlling
 * whether user can determine how data on table is sorted by
 * clicking on the header buttons.
 */
bool Table::allowSort() const
{
    return canSort;
}

void Table::allowSort(bool allow)
{
    canSort = allow;
}


/*
 * ==================================
 *  int Table.headerSize();
 *  void Table.headerSize(int size);
 * ==================================
 *
 * These methods get or set the value of variable controlling
 * the height of header buttons.
 */
int Table::headerSize() const
{
    return headerHeight;
}

void Table::headerSize(int height)
{
    headerHeight = height;
    dimensionsChanged = true;
    redraw();
}


/*
 * ===============================
 *  int Table.rowSize();
 *  void Table.rowSize(int size);
 * ===============================
 *
 * These methods get or set the value of variable controlling
 * the height of rows.
 */
int Table::rowSize() const
{
    return rowHeight;
}

void Table::rowSize(int height)
{
    rowHeight = height;
    vScroll->linesize(3 * height);
    dimensionsChanged = true;
    redraw();
}


/*
 * ===================================
 *  int Table.scrollbSize();
 *  void Table.scrollbSize(int size);
 * ===================================
 *
 * These methods get or set the value of variable controlling
 * the size (width) of the scrollbars.
 */
int Table::scrollbSize() const
{
    return scrollbarSize;
}

void Table::scrollbSize(int size)
{
    scrollbarSize = size;
    dimensionsChanged = true;
    redraw();
}


/*
 * =====================================================
 *  Fl_Align Table.columnAlign(int column);
 *  void Table.columnAlign(int column, Fl_Align align);
 * =====================================================
 *
 * These methods get or set the value of variable controlling
 * the alignment of the specified column.
 */
Fl_Align Table::columnAlign(int column) const
{
    if ((column < 0) && (column >= nCols))
	return (Fl_Align)(FL_ALIGN_LEFT | FL_ALIGN_CLIP);
    /* NOT REACHED */

    return header[column].align;
}

void Table::columnAlign(int column, Fl_Align align)
{
    if ((column < 0) && (column >= nCols))
	return;
    /* NOT REACHED */

    header[column].align = (Fl_Align)(align | FL_ALIGN_CLIP);
    redraw();
}


/*
 * =====================================================
 *  int Table.columnWidth(int column);
 *  void Table.columnWidth(int column, int width);
 * =====================================================
 *
 * These methods get or set the value of variable controlling
 * the width of the specified column.
 */
int Table::columnWidth(int column) const
{
    if ((column < 0) && (column >= nCols))
	return 0;
    /* NOT REACHED */

    return header[column].width;
}

void Table::columnWidth(int column, int width)
{
    if ((column < 0) && (column >= nCols))
	return;
    /* NOT REACHED */

    header[column].width = width;
    dimensionsChanged = true;
    redraw();
}


/*
 * ========================================================
 *  const char *Table.columnTitle(int column);
 *  void Table.columnTitle(int column, const char *title);
 * ========================================================
 *
 * These methods get or set the value of variable controlling
 * the width of the specified column.
 */
const char *Table::columnTitle(int column)
{
    if ((column < 0) && (column >= nCols))
	return NULL;
    /* NOT REACHED */

    return header[column].title;
}

void Table::columnTitle(int column, const char *title)
{
    if ((column < 0) && (column >= nCols))
	return;
    /* NOT REACHED */

    free((void*)header[column].title);
    header[column].title = strdup(title);

    damage(DAMAGE_HEADER);
}

/*
 * ===================
 *  int Table.rows();
 * ===================
 *
 * Returns the number of rows in table.
 */
int Table::rows()
{
    return nRows;
}


/*
 * ======================
 *  int Table.columns();
 * ======================
 *
 * Returns the number of columns in table.
 */
int Table::columns()
{
    return nCols;
}


/*
 * ==================================
 *  void Table.value(int selection);
 * ==================================
 *
 * Sets the currently selected row.
 */
void Table::value(int selection)
{
    if ((selection >= -1) && (selection < nRows))
	selected = selection;
    damage(DAMAGE_ROWS);
}


/*
 * ====================
 *  int Table.value();
 * ====================
 *
 * Returns the number of the currently selected row.
 */
int Table::value()
{
    return selected;
}


/*
 * ====================================================================
 *  void Table.addColumn(const char *label, int width, Fl_Align align);
 * ====================================================================
 *
 * Adds column with label as title, width and align as
 * sort function.
 */
void Table::addColumn(const char *label, int width, Fl_Align align)
{
    if (!noMoreColumns)
    {
	struct ColumnInfo col;

	dimensionsChanged = true;

	col.title = strdup(label);
	col.width = width;
	col.align = (Fl_Align)(align | FL_ALIGN_CLIP);
	header.push_back(col);
	nCols++;
    }
}


/*
 * =================================
 *  void Table.addCell(char *data);
 * =================================
 *
 * Adds a cell with data to the table.
 */
void Table::addCell(const char *data)
{
    if (!noMoreColumns)
	noMoreColumns = true;

    if ((cPos >= nCols) || (curRow == NULL))
    {
	this->data.push_back(curRow = new char*[nCols]);
	for(int i = 0; i < nCols; i++)
	    curRow[i] = NULL;
	dimensionsChanged = true;
	nRows++;
	cPos = 0;
    }

    if (data != NULL)
	curRow[cPos] = strdup(data);
    else
	curRow[cPos] = strdup("");

    if (cPos == sortColumn)
	toBeSorted = true;
    cPos++;
}

/*
 * ================================
 *  void Table.removeRow(int row);
 * ================================
 *
 * Removes row referenced by row.
 */
void Table::removeRow(int row)
{
    if ((row == -1) && (selected >= 0))
	row = selected;

    if ((row >= 0) && (row < nRows))
    {
	char **rowData = data[row];
	if (rowData == curRow)
	    curRow = NULL;
	for (int i = 0; i < nCols; i++)
	    free(rowData[i]);
	delete[] rowData;
	data.erase(row);
	nRows--;
	dimensionsChanged = true;
	toBeSorted = true;
	selected = -1;
    }
}


/*
 * =======================================
 *  void Table.clear(bool removeColumns);
 * =======================================
 *
 * Frees all data in table. If removeColumns is true, frees also header
 * structures.
 */
void Table::clear(bool removeColumns)
{
    nRows = 0;
    curRow = NULL;
    cPos = 0;

    // Delete row data.
    char **row;
    for (int j = 0; j < data.size(); ++j)
    {
	row = data[j];
	for (int i = 0; i < nCols; i++)
	    free(row[i]);
	delete[] row;
    }
    data.clear();

    if (removeColumns)
    {
	// Delete header data.
	for (int i = 0; i < header.size(); ++i)
	    free((void*)header[i].title);

	header.clear();
	nCols = 0;
    }
    selected = -1;
    noMoreColumns = false;
    dimensionsChanged = true;
}


/*
 * ============================================
 *  char *Table.valueAt(int row, int column);
 * ============================================
 *
 * Returns value in cell referenced by row and column.
 */
char *Table::valueAt(int row, int column)
{
    if ((row >= 0) && (row < nRows) && (column >= 0) && (column < nCols))
	return data[row][column];
    else if ((row == -1) && (selected >= 0) && (column >= 0) && (column < nCols))
	return data[selected][column];
    else
	return NULL;
}

/*
 * ======================================================
 *  void Table.valueAt(int row, int column, char *data);
 * ======================================================
 *
 * Sets alue in cell referenced by row and column.
 */
void Table::valueAt(int row, int column, char *data)
{
    if ((row == -1) && (selected >= 0))
	row = selected;

    if ((row >= 0) && (row < nRows) && (column >= 0) && (column < nCols))
    {
	if (column == sortColumn)
	    toBeSorted = true;
	if (this->data[row][column] != NULL)
	    free(this->data[row][column]);
	this->data[row][column] = strdup(data);
    }
}

/*
 * =====================================
 *  const char **Table.getRow(int row);
 * =====================================
 *
 * Returns pointer to the data of the row number row.
 */
const char **Table::getRow(int row)
{
    if ((row == -1) && (selected >= 0))
	row = selected;

    if ((row >= 0) && (row < nRows))
	return (const char**)data[row];
    else
	return NULL;
}


/*
 * ================================================================
 *  Table.where(int x, int y, int &row, int &column, int &resize);
 * ================================================================
 *
 * Finds corresponding row and column for x and y coordinates. This function
 * uses Fl::event_inside() method.
 *
 * row = -1 means header and row = -2 means that coordinates don't
 * correspond any cell.
 */
void Table::where(int x, int y, int &row, int &column, int &resize)
{
    int temp, temp2;

    // Inside the header
    if ((nCols > 0) && headerEnabled &&
	    Fl::event_inside(oX, oY, iW, headerHeight))
    {
	row = -1;
	temp = leftColX + iX - hScroll->value();

	// Scan visible columns until found one that matches.
	for (column = leftCol; column <= rightCol; column++ )
	{
	    temp2 = temp;

	    // Near the left border of the column header
	    if ((x >= temp) && (x <= temp + 3))
	    {
		resize = 1;
		return;
		/* NOT REACHED */
	    }

	    // Near the right border of the column header
	    else if ((x >= (temp += header[column].width) - 3) &&
		    (x < temp))
	    {
		resize = 2;
		return;
		/* NOT REACHED */
	    }

	    // Somewhere else
	    else if ((x >= temp2) && (x < temp))
	    {
		resize = 0;
		return;
		/* NOT REACHED */
	    }
	}
    } // Header


    /*
     * Now the harder one. X and Y lie somewhere in the table.
     * Find correct row and column.
     */
    else if ((nRows > 0) && Fl::event_inside(iX, iY, iW, iH))
    {
	temp = topRowY;
	int yMod = iY - vScroll->value();
	int leftX = leftColX + iX - hScroll->value();

	// Scan rows
	for (row = topRow; row <= bottomRow; row++)
	{
	    int temp2 = leftX;
	    for (column = leftCol; column <= rightCol; column++)
	    {
		if (Fl::event_inside(temp2, temp + yMod,
			    header[column].width, rowHeight))
		    return;
		/* NOT REACHED */
		temp2 += header[column].width;
	    }
	    temp += rowHeight;
	}
    }
    row = column = -2;
}


/*
 * ==============================
 *  int Table.handle(int event);
 * ==============================
 *
 * FLTK internal. Handles incoming events.
 */
int Table::handle(int event)
{
    int ret = 0;
    int row, column, resize;

    if (event != FL_KEYDOWN)
	ret = Fl_Group::handle(event);

    /*
     * MAIN SWITCH
     */
    switch (event)
    {

	/*
	 * PUSH event
	 */
    case FL_PUSH:
	// Test if pushed on scrollbars.
	if (vScroll->visible() && Fl::event_inside(vScroll->x(),
		    vScroll->y(), vScroll->w(), vScroll->h()))
	    break;

	if (hScroll->visible() && Fl::event_inside(hScroll->x(),
		    hScroll->y(), hScroll->w(), hScroll->h()))
	    break;

	// Which row/column are we over?
	where(Fl::event_x(), Fl::event_y(), row, column, resize);

	switch (row)
	{
	    // Push on nothing... Not interested
	case -2:
	    if (selected != -1)
	    {
		selected = -1;
		damage(DAMAGE_ROWS);
	    }
	    break;

	    // Push on header.
	case -1:
	    if ((canResize) &&
		    (Fl::event_button() == FL_LEFT_MOUSE) &&
		    (resize != 0))
	    {
		resizing = (resize == 1) ? column - 1 : column;
		dragX = Fl::event_x();
		ret = 1;
	    }
	    else if ((canSort) &&
		    (Fl::event_button() == FL_LEFT_MOUSE))
	    {
		pushed = column;
		damage(DAMAGE_HEADER);
		ret = 1;
	    }
	    break;

	    // Push on cell.
	default:
	    bool changed = selected != row;
	    selected = row;

	    // Create new selection
	    char **tableRow = data[selected];
	    int col;
	    // size: (nCols - 1) tabs + \0
	    int selsize = strlen(tableRow[0]) + nCols;

	    for (col = 1; col < nCols; col++)
	    {
		if (tableRow[col] != NULL)
		    selsize += strlen(tableRow[col]);
	    }

	    char *selection = new char[selsize];
	    strcpy(selection, tableRow[0]);
	    for (col = 1; col < nCols; col++)
	    {
		strcat(selection, "\t");
		if (tableRow[col] != NULL)
		    strcat(selection, tableRow[col]);
	    }
	    Fl::selection(*this, selection,
		    selsize);
	    delete[] selection;

	    // Update view.
	    damage(DAMAGE_ROWS);
	    take_focus();

	    // Callback
	    if ((Fl::event_clicks() != 0) && !changed &&
		    (when() & TABLE_WHEN_DCLICK))
	    {
		Fl::event_is_click(0);
		do_callback();
	    }
	    else if (changed && (when() & FL_WHEN_CHANGED))
		do_callback();
	    else if (when() & FL_WHEN_NOT_CHANGED)
		do_callback();
	    ret = 1;
	    break;
	} // switch(row)
	break;


	/*
	 * DRAG event
	 */
    case FL_DRAG:
	// Resizing...
	if (resizing > -1 )
	{
	    int offset = dragX - Fl::event_x();
	    int newWidth = header[resizing].width - offset;

	    // Width must be at least 1.
	    if (newWidth < 1)
		newWidth = 1;

	    // Test if column really is resized.
	    if (header[resizing].width != newWidth)
	    {
		header[resizing].width = newWidth;
		dragX = Fl::event_x();
		resized();
		redraw();
	    }
	    ret = 1;
	}
	break;


	/*
	 * RELEASE event
	 */
    case FL_RELEASE:
	// Which row/column are we over?
	where(Fl::event_x(), Fl::event_y(), row, column, resize);

	// Restore cursor and end resizing.
	if (Fl::event_button() == FL_LEFT_MOUSE)
	{
	    fl_cursor(FL_CURSOR_DEFAULT, labelcolor(), color());
	    if ((pushed == column) && canSort)
	    {
		if (sortColumn != pushed)
		{
		    sortColumn = pushed;
		    ascent = true;
		}
		else
		    ascent = !ascent;
		sort();
		redraw();
	    }
	    pushed = -1;
	    resizing = -1;
	    ret = 1;
	}

	// Callback.
	if ((row >= 0) && (when() & FL_WHEN_RELEASE))
	    do_callback();
	break;


	/*
	 * MOVE event
	 */
    case FL_MOVE:
	// Which row/column are we over?
	where(Fl::event_x(), Fl::event_y(), row, column, resize);

	// If near header boundary.
	if ((row == -1) && canResize && resize)
	    fl_cursor(FL_CURSOR_WE, labelcolor(), color());
	else
	    fl_cursor(FL_CURSOR_DEFAULT, labelcolor(), color());
	ret = 1;
	break;

    case FL_ENTER:
    case FL_LEAVE:
	if (event == FL_LEAVE)
	    fl_cursor(FL_CURSOR_DEFAULT, labelcolor(), color());
	ret = 1;
	break;

    case FL_FOCUS:
	if ((nRows > 0) && (selected < 0))
	    selected = 0;
    case FL_UNFOCUS:
	if (Fl::visible_focus())
	{
	    damage(DAMAGE_ROWS);
	    ret = 1;
	}
	break;

	/*
	 * KEYDOWN event
	 */
    case FL_KEYDOWN:
	switch(Fl::event_key())
	{
	case FL_Enter:
	    if ((selected > -1) &&  (when() & TABLE_WHEN_DCLICK) ||
		    (when() & FL_WHEN_ENTER_KEY))
		do_callback();
	    ret = 1;
	    break;

	case FL_Up:
	    // Does it make sense to move up?
	    if (selected > 0)
	    {
		selected--;

		// Is scrolling needed
		if ((selected < topRow) ||
			(selected > bottomRow))
		    scrollTo(rowHeight * selected);
		else
		    damage(DAMAGE_ROWS);

		// Callback
		if (when() & FL_WHEN_CHANGED)
		    do_callback();
		else if (when() & FL_WHEN_NOT_CHANGED)
		    do_callback();
	    }
	    ret = 1;
	    break;

	case FL_Down:
	    // Does it make sense to move down?
	    if ((selected >= 0) && (selected < (nRows - 1)))
	    {
		selected++;

		// Scroll if needed
		if ((selected >= bottomRow) ||
			(selected < topRow))
		    scrollTo(rowHeight *
			    (selected + 1) - iH);
		else
		    damage(DAMAGE_ROWS);

		// Callback
		if (when() & FL_WHEN_CHANGED)
		    do_callback();
		else if (when() & FL_WHEN_NOT_CHANGED)
		    do_callback();
	    }
	    ret = 1;
	    break;

	case FL_Page_Up:
	    // Does it make sense to move up?
	    if (selected > 0)
	    {
		// Number of rows on the 'page'
		int step = iH / rowHeight;

		// Change selection
		if (selected >= step)
		    selected -= step;
		else
		    selected = 0;

		// Scroll if needed
		if ((selected < topRow) ||
			(selected > bottomRow))
		    scrollTo(selected * rowHeight);
		else
		    damage(DAMAGE_ROWS);

		// Callback
		if (when() & FL_WHEN_CHANGED)
		    do_callback();
		else if (when() & FL_WHEN_NOT_CHANGED)
		    do_callback();
	    }
	    ret = 1;
	    break;

	case FL_Page_Down:
	    // Does it make sense to move down?
	    if ((selected >= 0) && (selected < (nRows - 1)))
	    {
		// Number of rows on the 'page'
		int step = iH / rowHeight;

		// Change selection
		if ((selected += step) > nRows)
		    selected = nRows - 1;

		// Scroll if needed
		if ((selected >= bottomRow) ||
			(selected < topRow))
		    scrollTo(rowHeight *
			    (selected + 1) - iH);
		else
		    damage(DAMAGE_ROWS);

		// Callback
		if (when() & FL_WHEN_CHANGED)
		    do_callback();
		else if (when() & FL_WHEN_NOT_CHANGED)
		    do_callback();
	    }
	    ret = 1;
	    break;

	case FL_Home:
	    // Does it make sense to move up?
	    if ((selected > 0) && (nRows > 0))
	    {
		selected = 0;
		scrollTo(0);
		damage(DAMAGE_ROWS);

		// Callback
		if (when() & FL_WHEN_CHANGED)
		    do_callback();
		else if (when() & FL_WHEN_NOT_CHANGED)
		    do_callback();
	    }
	    ret = 1;
	    break;

	case FL_End:
	    // Does it make sense to move down?
	    if ((selected >= 0) && (nRows > 0))
	    {
		selected = nRows - 1;
		scrollTo(rowHeight * (selected + 1) - iH);
		damage(DAMAGE_ROWS);

		// Callback
		if (when() & FL_WHEN_CHANGED)
		    do_callback();
		else if (when() & FL_WHEN_NOT_CHANGED)
		    do_callback();
	    }
	    ret = 1;
	    break;
	}
	break;
    }
    return ret;
}


/*
 * ===============================
 *  void Table.scrollTo(int pos);
 * ===============================
 *
 * Scrolls table to given position.
 */
void Table::scrollTo(int pos)
{
    if (!vScroll->visible())
    {
	damage(DAMAGE_ROWS);
	return;
	/* NOT REACHED */
    }

    int max = (int)vScroll->Fl_Valuator::maximum();

    if (pos < 0)
	pos = 0;
    else if (pos > max)
	pos = max;

    vScroll->Fl_Valuator::value(pos);
    scrolled();

    damage(DAMAGE_ROWS);
}


/*
 * ===========================================
 *  void Table.sort(int column, bool ascent);
 * ===========================================
 *
 * Sets sortColumn and ascent and sorts table. Does not redraw.
 */
void Table::sort(int column, bool ascent)
{
    if ((column < -1) || (column >= nCols))
	return;

    sortColumn = column;
    this->ascent = ascent;
    sort();
}


void Table::aSort(int start, int end)
{
    int i, j;
    const char *x;
    char **temp;

    x = data[(start + end) / 2][sortColumn];
    i = start;
    j = end;

    for (;;)
    {
	while ((i < end) && (stringcompare(data[i][sortColumn], x) < 0))
	    i++;
	while ((j > 0) && (stringcompare(data[j][sortColumn], x) > 0))
	    j--;

	while ((i < end) && (i != j) &&
		(stringcompare(data[i][sortColumn], data[j][sortColumn]) == 0))
	    i++;

	if (i == j)
	    break;

	temp = data[i];
	data[i] = data[j];
	data[j] = temp;
    }

    if (start < --i)
	aSort(start, i);

    if (end > ++j)
	aSort(j, end);
}


void Table::dSort(int start, int end)
{
    int i, j;
    const char *x;
    char **temp;

    x = data[(start + end) / 2][sortColumn];
    i = start;
    j = end;

    for (;;)
    {
	while ((i < end) && (stringcompare(data[i][sortColumn], x) > 0))
	    i++;
	while ((j > 0) && (stringcompare(data[j][sortColumn], x) < 0))
	    j--;

	while ((i < end) && (i != j) &&
		(stringcompare(data[i][sortColumn], data[j][sortColumn]) == 0))
	    i++;

	if (i == j)
	    break;

	temp = data[i];
	data[i] = data[j];
	data[j] = temp;
    }

    if (start < --i)
	dSort(start, i);

    if (end > ++j)
	dSort(j, end);
}


/*
 * ====================
 *  void Table.sort();
 * ====================
 *
 * Sorts table according sortColumn and ascent. Does not redraw.
 */
void Table::sort()
{
    if ((sortColumn == -1) || !canSort)
	return;
    /* NOT REACHED */

    toBeSorted = false;

    // Sort in descending order.
    if ((nRows > 1) && ascent)
	aSort(0, nRows - 1);

    // Sort in ascending order.
    else if (nRows > 1)
	dSort(0, nRows - 1);
}


/*
 * ====================================================
 *  void Table.getSort(int &sortColumn, bool &ascent);
 * ====================================================
 *
 * Set sortColumn and ascent according to current sort policy.
 */
void Table::getSort(int &sortColumn, bool &ascent)
{
    sortColumn = this->sortColumn;
    ascent = this->ascent;
}

/*
 * ====================
 *  void Table.draw();
 * ====================
 *
 * FLTK internal. Called when Table widget needs to be drawn.
 */
void Table::draw()
{
    int damage;

    if (dimensionsChanged)
    {
	dimensionsChanged = false;
	resized();
    }

    if (toBeSorted)
	sort();

    damage = Fl_Widget::damage();

    // Draw children.
    if (damage & (FL_DAMAGE_ALL | FL_DAMAGE_CHILD))
    {
	fl_push_clip(oX, oY, oW, oH);
	Fl_Group::draw();
	fl_pop_clip();
    }

    // Draw box.
    if (damage & FL_DAMAGE_ALL)
    {
	// Draw box.
	draw_box(box(), x(), y(), w(), h(), FL_GRAY);

	// Draw label.
	draw_label();
    }

    // Draw header.
    int xPos = leftColX + iX - hScroll->value();
    if (headerEnabled && (damage & (FL_DAMAGE_ALL | DAMAGE_HEADER)) &&
	    (nCols > 0))
    {
	fl_push_clip(iX, oY, iW, headerHeight);
	drawHeader(xPos, oY);
	fl_pop_clip();
    }

    // Draw all the cells.
    if ((damage & (FL_DAMAGE_ALL | DAMAGE_ROWS)) && (nRows > 0) &&
	    (nCols > 0))
    {
	fl_push_clip(iX, iY, iW, iH);

	int yMod = iY - vScroll->value();
	for (int row = topRow, rowY = topRowY; row <= bottomRow;
		row++, rowY += rowHeight)
	    drawRow(row, data[row], xPos, rowY + yMod);
	fl_pop_clip();
    }

    fl_push_clip(oX, oY, oW, oH);

    // Table height smaller than window? Fill remainder with rectangle
    if (tableWidth < iW)
	fl_rectf(iX + tableWidth, oY, iW - tableWidth, oH, FL_GRAY);

    if (tableHeight < iH)
	fl_rectf(iX, iY + tableHeight, iW, iH - tableHeight, FL_GRAY);

    // Both scrollbars? Draw little box in lower right
    if (vScroll->visible() && hScroll->visible())
	fl_rectf(vScroll->x(), hScroll->y(), vScroll->w(), hScroll->h(), FL_GRAY);

    fl_pop_clip();
}


/*
 * ================================================
 *  void Table.resize(int x, int y, int w, int h);
 * ================================================
 *
 * FLTK internal. Called when Table widget is resized.
 */
void Table::resize(int x, int y, int w, int h)
{
    Fl_Widget::resize(x, y, w, h);
    resized();
    damage(FL_DAMAGE_ALL);
}


/*
 * ==============================
 *  void Table.calcDimensions();
 * ==============================
 *
 * Calculates table dimensions.
 */
void Table::calcDimensions()
{
    // Calculate width and height of the table (in pixels).
    tableWidth = 0;
    for (int i = 0; i < nCols; i++)
	tableWidth +=header[i].width;

    tableHeight = nRows * rowHeight;
    Fl_Boxtype b;

    iX = oX = x() + Fl::box_dx(b = box());
    iY = oY = y() + Fl::box_dy(b);
    iW = oW = w() - Fl::box_dw(b);
    iH = oH = h() - Fl::box_dh(b);

    // Trim inner size if header enabled.
    if (headerEnabled)
    {
	iY += headerHeight;
	iH -= headerHeight;
    }

    // Hide scrollbars if window is large enough
    int hideV = (tableHeight <= iH),
	hideH = (tableWidth <= iW);

    if (!hideH & hideV)
	hideV = (tableHeight - iH - scrollbarSize) <= 0;
    if (!hideV & hideH)
	hideH = (tableWidth - iW + scrollbarSize) <= 0;

    if (hideV)
    {
	vScroll->hide();
	vScroll->Fl_Valuator::value(0);
    }
    else
    {
	vScroll->show();
	iW -= scrollbarSize;
    }

    if (hideH)
    {
	hScroll->hide();
	hScroll->Fl_Valuator::value(0);
    }
    else
    {
	hScroll->show();
	iH -= scrollbarSize;
    }
}


/*
 * ========================
 *  void Table.scrolled();
 * ========================
 *
 * Calculates visible are after scroll or adding data.
 */
void Table::scrolled()
{
    int y, row, voff = vScroll->value();

    // First visible row
    row = voff / rowHeight;
    topRow = (row >= nRows) ? (nRows - 1) : row;
    if ((y = topRow * rowHeight) > voff)
    {
	topRow--;
	y -= rowHeight;
    }
    topRowY = y;

    // Last visible row
    row = (voff + iH) / rowHeight;
    bottomRow = (row >= nRows) ? (nRows - 1) : row;

    // First visible column
    int x, col, h = hScroll->value();
    for (col = x = 0; col < nCols; col++)
    {
	x += header[col].width;
	if (x >= h)
	{
	    x -= header[col].width;
	    break;
	}
    }
    leftCol = (col >= nCols) ? (nCols - 1) : col;
    leftColX = x;

    // Last visible column
    h += iW;
    for (; col < nCols; col++)
    {
	x += header[col].width;
	if (x >= h)
	    break;
    }
    rightCol = (col >= nCols) ? (nCols - 1) : col;
}


/*
 * =======================
 *  void Table.resized();
 * =======================
 *
 * Calculates scrollbar properties after resizing or adding data.
 */
void Table::resized()
{
    calcDimensions();

    // Calculate properties for vertical scrollbar.
    if (vScroll->visible())
    {
	vScroll->bounds(0, tableHeight - iH);
	vScroll->resize(oX + oW - scrollbarSize, oY, scrollbarSize,
		oH - (hScroll->visible() ? scrollbarSize : 0));
	vScroll->Fl_Valuator::value(vScroll->clamp(vScroll->value()));
	vScroll->slider_size(iH > tableHeight ? 1 : (float)iH / tableHeight);
    }

    // Calculate properties for horizontal scrollbar.
    if (hScroll->visible())
    {
	hScroll->bounds(0, tableWidth - iW);
	hScroll->resize(oX, oY + oH - scrollbarSize,
		oW - (vScroll->visible() ? scrollbarSize : 0), scrollbarSize);
	hScroll->Fl_Valuator::value(hScroll->clamp(hScroll->value()));
	hScroll->slider_size(iW > tableWidth ? 1 : (float)iW / tableWidth);
    }
    scrolled();
    dimensionsChanged = false;
}


/*
 * ===========================================================
 *  void Table.scrollCallback(Fl_Widget *widget, void *data);
 * ===========================================================
 *
 * Internal callback for scrollbars. Scrolls view.
 */
void Table::scrollCallback(Fl_Widget *widget, void *data)
{
    Table *me = (Table*)data;

    me->scrolled();

    if (widget == me->vScroll)
	me->damage(DAMAGE_ROWS);
    else
	me->damage(DAMAGE_ROWS | DAMAGE_HEADER);
}

void Table::labelsize(uchar s)
{
    Fl_Group::labelsize(s);
    headerHeight = labelsize();
    rowHeight = labelsize() - 1;
    scrollbarSize = labelsize() - 3;
}

