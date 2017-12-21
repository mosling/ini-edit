/*!
This is an mapping of the NCurses-functions, so we can use the
curses-library with C++. Internally this modul has for every window an
panel, so we have no problems with overlapping windows such input
boxes or menu boxes.
*/
#include <iostream>
#include <ncurses.h>
#include <panel.h>
#include "vw-curses.h"

using namespace std;

WINDOW *WindowList[100]; //!< list of windows map int to WINDOW*
PANEL  *PanelList[100];  //!< every window has an corresponding panel
int numwin = 0; //!< number of the window with the highest index
//! the standard elements for boxes
char BC[8] = {ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE,
	ACS_ULCORNER, ACS_URCORNER, ACS_LLCORNER, ACS_LRCORNER};
char marker = '*'; //! the default marker

//! set new box elements
void SetBC (char l, char r, char t, char b, 
	char tl, char tr, char bl, char br)
{
	BC[0] = l;
	BC[1] = r;
	BC[2] = t;
	BC[3] = b;
	BC[4] = tl;
	BC[5] = tr;
	BC[6] = bl;
	BC[7] = br;
}

//! Set the marker to character ch
void SetMarker (char ch)
{
	marker = ch;
}

//! moves an marker in colum c from row o to row n 
void MarkPos (int w, int o, int n, int c)
{
	if (o>=0) mvwaddch (WindowList[w], o, c, ' ');
	if (n>=0) mvwaddch (WindowList[w], n, c, marker);
}

//! move to position l,c in window w and write ch
void MvWAddStr (int w, int l, int c, const char *ch)
{
	mvwaddstr (WindowList[w], l, c, ch);
}

//! same as MvWAddStr, but len characters are written only
void MvWAddNStr (int w, int l, int c, const char *ch, int len)
{
	mvwaddnstr (WindowList[w], l, c, ch, len);
}

//! add an string to window w
void WAddStr (int w, const char *ch)
{
	waddstr (WindowList[w], ch);
}

//! move cursor position to window w at r,c
void WMove (int w, int r, int c)
{
	wmove (WindowList[w], r, c);
}

//! write one character to window w
void WAddCH (int w, char ch)
{
	waddch (WindowList[w], ch);
}

//! same as WAddCH, but writes the character at position l,c
void MvWAddCH (int w, int l, int c, char ch)
{
	mvwaddch (WindowList[w],l,c,ch);
}

//! returns the maximal number of columns of the terminal
int CCols ()
{
	return COLS;
}

//! returns the maximal number of lines of the terminal
int CLines ()
{
	return LINES;
}

/*!
\brief initialize the curses-package

This functions must be called first, before any other curses function
is used. The following things are set
	- init curses
	- disable line buffering (cbreak)
	- disable input echo (noecho)
	- allow the user to use the functions keys, arrow keys, ...

Also the StandardScreen is the windows with index 0.
*/
void InitCurses ()
{
	initscr();
	cbreak();
	noecho ();
	keypad (stdscr, TRUE);
	WindowList[0] = stdscr;
}

//! close all open windows and close curses
void CloseCurses ()
{
	int i;
	for (i=numwin; i>=1; i--)
	{
		if (WindowList[i] != 0)
		{
			del_panel (PanelList[i]);
			delwin (WindowList[i]);
		}
	}
	update_panels();
	refresh ();
	endwin ();
}

/*! 
create a new window with l lines, c columns at position y,x 
an panel for this window is also created, if the window can't be
created an error occurs and the program is aborted
I set the keypad for the new window to TRUE
*/
int NewWin (int l, int c, int y, int x)
{
	numwin++;
	WindowList[numwin] = newwin(l,c,y,x);
	if (WindowList[numwin] == 0)
	{
		fprintf (stderr, "can't display window %d, (%d,%d,%d,%d)\n",
			numwin,l,c,y,x);
		numwin--;
		exit (1);
	}
	PanelList[numwin] = new_panel (WindowList[numwin]);
	keypad (WindowList[numwin], TRUE);
	return (numwin);
}

//! same as NewWin, but it has a parent window pw
int SubWin (int pw, int l, int c, int y, int x)
{
	numwin++;
	WindowList[numwin] = subwin(WindowList[pw], l, c, y, x);
	if (WindowList[numwin] == 0)
	{
		fprintf (stderr, "can't display window %d, (%d,%d,%d,%d)\n",
			numwin,l,c,y,x);
		numwin--;
		exit (1);
	}
	PanelList[numwin] = new_panel (WindowList[numwin]);
	keypad (WindowList[numwin], TRUE);
	return (numwin);
}

/*! 
same as SubWin, additional box and a name (text) is added
for the box we create an extra window, so same output in window can't
destroy the box.
*/
int BoxSubWin (int pw, int l, int c, int y, int x, const char *text)
{
	int tmpwin;

	tmpwin = SubWin (pw, l, c, y, x);
	WBox (tmpwin);
	MvWAddStr (tmpwin, 0, 2, text);
	tmpwin = SubWin (tmpwin, l-2, c-2, y+1, x+1);
	return tmpwin;
}

//! same as NewWin and BoxSubWin, but now subwindows are created 
int BoxWin (int l, int c, int y, int x, const char *text)
{
	int tmpwin;
	
	tmpwin = NewWin (l, c, y, x);
	WBox (tmpwin);
	MvWAddStr (tmpwin, 0, 2, text);
	wrefresh (WindowList[tmpwin]);
	tmpwin = NewWin (l-2, c-2, y+1, x+1);
	return tmpwin;
}

//! update all windows, the internal structure is copied to the screen
void DoUpdate ()
{
	doupdate ();
}

//! update all panels 
void UpdatePanels ()
{
	update_panels ();
}

//! delete a window, update is called internal
void DelWin (int w)
{
	del_panel (PanelList[w]);
	delwin (WindowList[w]);
	if (w == numwin) numwin--;
	WindowList[w] = 0;
	update_panels();
}

//! draw all windows new
void RefreshAll ()
{
	int i;

	for (i=0; i<=numwin; i++)
		wrefresh (WindowList[i]);
}

//! dump the screen into a file
void ScrDump (char *fname)
{
	scr_dump (fname);
}

//! read a screen dump from file fname
void ScrRestore (char *fname)
{
	scr_restore(fname);
	doupdate ();
}

//! draw a border in window using the character from BC
void WBox (int w)
{
	wborder (WindowList[w],BC[0],BC[1],BC[2],BC[3],BC[4],BC[5],BC[6],BC[7]);
}

//! allow a window to scroll its text
void ScrollOK (int w, int t)
{
	scrollok (WindowList[w], t==0?FALSE:TRUE);
}

//! enable/disable the capability to read function keys
void KeyPad (int w, int t)
{
	keypad (WindowList[w], t==0?FALSE:TRUE);
}

//! refresh one window
void WRefresh (int w)
{
	wrefresh (WindowList[w]);
}

//! wait for an keypress in standard screen
int GetCH ()
{
	int ch;
	ch = getch();

	return ch;
}

//! wait for an keypress in window w
int WGetCH (int w)
{
	int ch;
	ch = wgetch(WindowList[w]);

	return ch;
}

//! scroll the window w 
void WScrl (int w, int v)
{
	wscrl (WindowList[w], v);
}

//! clear the window w, is filled with spaces
void WClear (int w)
{
	wclear (WindowList[w]);
}

//! clear to end of line
void WClrtoEOL (int w)
{
	wclrtoeol (WindowList[w]);
}

//! flash the screen is no beep available
void Flash ()
{
	flash ();
}
