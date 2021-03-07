/*
 Copyright (c) 2004 Markus Niemistö

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
 * Modified by Sergey Khorev to get rid of overkilling STL stuff
 * Some reformatting was done
 * Get rid of unneeded stuff
 * Added binding to system colors
 */

#ifndef __TABLE_HH
#define __TABLE_HH

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Scrollbar.H>
#include <FL/Fl_Widget.H>


#define TABLE_WHEN_DCLICK		16

// very simple replacement to STL vector
// even doesn't call destructors on its items
template <class T>
class minivector
{
    T	    *_data;
    size_t  _size;  // size
    size_t  _asize; // allocated size
public:
    minivector() : _data (0), _size(0), _asize(0) {}
    ~minivector()
    {
	free(_data);
    }

    T& operator[] (size_t i)
    {
	return _data[i];
    }
    const T& operator[] (size_t i) const
    {
	return _data[i];
    }
    size_t erase(size_t i)
    {
	if (i < _size - 1)
	    memmove(_data + i, _data + i + 1, (_size - i - 1) * sizeof(T));
	--_size;
	if (_size * 2 < _asize && _asize > 2)
	{
	    _asize /= 2;
	    _data = (T *)realloc(_data, _asize);
	}
	return i;
    }
    size_t size()
    {
	return _size;
    }
    void push_back(const T &t)
    {
	if (_asize == _size)
	{
	    if (_asize == 0)
		_asize = 2;
	    else
		_asize *= 2; // double allocated size
	    _data = (T *)realloc(_data, _asize* sizeof(T));
	}
	_data[_size++] = t;
    }
    void clear()
    {
	_asize = _size = 0;
	free(_data);
	_data = 0;
    }
};


class Table : public Fl_Group
{
private:
    struct ColumnInfo
    {
	const char *title;
	int width;
	Fl_Align align;
    };

    // Scrollbars
    Fl_Scrollbar *hScroll, *vScroll;

    // Column data
    minivector<struct ColumnInfo> header;

    // Cell data
    minivector<char**> data;

    // Table dimensions
    int tableHeight, tableWidth;
    int oX, oY, oW, oH;	// Outer dimensions (widget - box)
    int iX, iY, iW, iH;	/*
			 * Table area dimensions
			 * (outer dimension - header - scrollbars)
			 */

    // For optimization
    int topRow, bottomRow, leftCol, rightCol;
    int topRowY, leftColX;

    int nCols, nRows;	// Number of rows and columns
    int cPos;		// Column where new entry is added.

    int resizing, dragX;
    int pushed;
    int sortColumn;

    // Object sizes
    int scrollbarSize;
    int headerHeight;
    int rowHeight;

    int selected;
    char **curRow;

    // Various flags
    bool ascent;
    bool canResize, canSort;
    bool noMoreColumns;
    bool toBeSorted;
    bool dimensionsChanged;
    bool headerEnabled;

    void dSort(int start, int end);
    void aSort(int start, int end);

protected:
    virtual int handle(int event);

    virtual void drawHeader(int x, int y);
    virtual void drawRow(int row, char *rowData[], int x, int y);

    virtual void draw();
    virtual void resize(int x, int y, int w, int h);

    void calcDimensions();
    void scrolled();
    void resized();

    static void scrollCallback(Fl_Widget *widget, void *data);

public:
    Table(int x, int y, int w, int h, char *label = NULL);
    ~Table();

    bool headerOn() const;
    void headerOn(bool enabled);
    bool allowResize() const;
    void allowResize(bool allow);
    bool allowSort() const;
    void allowSort(bool allow);

    int headerSize() const;
    void headerSize(int height);
    int rowSize() const;
    void rowSize(int height);
    int scrollbSize() const;
    void scrollbSize(int size);

    Fl_Align columnAlign(int column) const;
    void columnAlign(int column, Fl_Align align);
    int columnWidth(int column) const;
    void columnWidth(int column, int width);
    const char *columnTitle(int column);
    void columnTitle(int column, const char *title);

    void sort();
    void sort(int column, bool ascent);
    void getSort(int &sortColumn, bool &ascent);

    void addColumn(const char *label, int width = 150,
	    Fl_Align align = (Fl_Align)(FL_ALIGN_LEFT | FL_ALIGN_CLIP));

    void addCell(const char *data);
    void removeRow(int row);
    void clear(bool removeColumns = false);

    void where(int x, int y, int &row, int &column, int &resize);
    void scrollTo(int pos);

    int columns();
    int rows();
    void value(int selection);
    int value();
    char *valueAt(int row, int column);
    void valueAt(int row, int column, char *data);

    const char **getRow(int row);

    uchar labelsize() const { return Fl_Group::labelsize(); }
    void labelsize(uchar);
};

#endif
