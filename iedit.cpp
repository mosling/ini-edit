#include <iostream>
#include "vw-curses.h"
#include "vw-menu.h"
#include "vw-input.h"
#include "split.h"

using namespace std;

void ShowLine (int w, int line, TreeList l)
{
	MvWAddStr (w, line, 3+l.GetDepth(), l.GetVar());
	if (l.HasSubTree() && !l.IsOpen())
		MvWAddCH (w, line, 0, '+');
	else if (l.HasSubTree() && l.IsOpen())
		MvWAddCH (w, line, 0, '-');
	else
		MvWAddCH (w, line, 0, '=');
}

/*!
Show the list-item from iterator to the end of the screen starting at
line pos. The heigth of the screen-part is in h.
*/
void ShowList (int w, TreeList l, int h, int pos)
{
	int tpos = pos;

	MarkPos (w, pos, -1, 1); // alte Markierung loeschen
	// iterator is set to the list-item at position pos
	while (!l.End() && tpos <= h)
	{
		WMove (w, tpos, 0);
		WClrtoEOL (w);
		ShowLine (w, tpos, l);
		l.SuccIterator ();
		tpos++;
	}
	// clear the empty lines in the window
	while (tpos <= h)
	{
		WMove (w, tpos++, 0);
		WClrtoEOL (w);
	}
	MarkPos (w, -1, pos, 1);
}

void KeyUP (int w, TreeList& l, int h, int& pos)
{
	if (l.First() == true) return ;
	l.PrevIterator ();
	if (pos == 0)
	{
		WScrl (w, -1); //! nach unten scrollen
		ShowLine (w, pos, l);
		MarkPos (w, pos+1, pos, 1);
	}
	else
	{
		MarkPos (w, pos, (pos-1), 1);
		pos--;
	}
}

void KeyDOWN (int w, TreeList& l, int h, int& pos)
{
	if (l.Last() == true) return;
	l.SuccIterator();
	if (pos == h)
	{
		WScrl (w, 1); //! nach oben scrollen
		ShowLine (w, pos, l);
		MarkPos (w, pos-1, pos, 1);
	}
	else
	{
		MarkPos (w, pos, pos+1, 1);
		pos++;
	}
}
		
void HelpText ()
{
	cout << "Editor for INI files" << endl;
	cout << "Usage:" << endl;
	cout << "  iedit [-h|-c] [ini-template] ini-file" << endl; 
	exit (1);
}

int main (int argc, char **argv)
{
	int sl, hlp, valW, qm, hW; //!< Selektionsliste
	int i, pos, ch, lheight, res, mh;
	int lines, cols, l, c, arg;
	char tmpinp[512];
	bool treeopen, hasOption=false;
	string saveas;
	TreeList vlist;

	if (argc < 2) HelpText ();

	for (i=1; i<argc; i++)
	{
		if (argv[i][0] == '-')
		{
			hasOption=true;
			switch (argv[i][1])
			{
				case 'h':
					HelpText();
				case 'c':
					SetBC ('|','|','-','-','+','+','+','+');
					SetMarker ('*');
			}
		}
	}

	hasOption?arg=2:arg=1;

	if (!ReadIniFile (argv[arg], vlist))
	{
		cerr << "Can't read file: " << argv[1] << endl;
		exit (1);
	}
	saveas = argv[arg++];
	if (arg<argc)
	{
		if (ReadIniFile (argv[arg], vlist))
		{
			saveas = argv[arg];
		}
		//! an error is ignored
	}

	vlist.AddTreePointer ();
	vlist.NotChanged (); //! because all changes at this time are redoable	

	InitCurses ();
	lines = CLines(); cols = CCols();

	//! Fenster fuer das Schnellmenu
	qm = NewWin (1,cols,lines-1,0);
	MvWAddStr (qm, 0, 0, 
		"  F1:help win  F2:save  F3:merge  F4:change  F5:(un)fold  F10:quit");
	//! Erzeugen des Fensters fuer die Variablenliste
	l = lines/2-2; c = cols/2-5;
	lheight = l-3;
	sl = BoxWin (l, c, 0, 0, "parameter window");
	ScrollOK (sl, 1);
	WRefresh (sl);
	//! Erzeugen des Wertefensters
	valW = BoxWin (l, cols-c, 0, c, "parameter value");
	WRefresh (valW);
	//! Erzeugen des Hilfefensters
	hlp = BoxWin (lines-l-1, cols, l, 0, "help window");
	WRefresh (hlp);

	pos = 0;
	vlist.SetIterator (0);
	treeopen = true;
	vlist.TreeOpen (treeopen);
	ShowList (sl, vlist, lheight, pos);
	ch = -42;
	//! MainLoop
	do
	{
		
		//! key down key pressed --> next entry if exists
		if (ch==CKEY_DOWN || ch=='j')
		{
			KeyDOWN (sl, vlist, lheight, pos);
		}
		//! key up key pressed --> previous entry if exists
		else if (ch==CKEY_UP || ch=='k')
		{
			KeyUP (sl, vlist, lheight, pos);
		}
		//! enter key pressed --> show/hide subtrees
		else if (ch==CKEY_ENTER )
		{
			if (vlist.HasSubTree())
			{
				vlist.IsOpen (!vlist.IsOpen());
			}
			ShowList (sl, vlist, lheight, pos);
		}
		//! show the help text as extra text on an big screen
		else if (ch==CKEY_F1 || ch=='h')
		{
			hW = BoxWin (lines, cols, 0, 0, "Help Window (any key to close)");
			WClear (hW); MvWAddStr (hW, 0, 0, vlist.GetHelp ());
			ch = WGetCH (hW);
			DelWin (hW);
			DelWin (hW-1); // remove the box
		}
		//! space pressed, set next selection value
		else if (ch==CKEY_SPACE)
		{
			vlist.SetNextSelection ();
		}
		//! set the default value if exists
		else if (ch=='d')
		{
			if (vlist.NumSelections() > 0)
			{
				vlist.SetVal (0);
			}
		}
		//! change the value, input box or menu if selection list exists
		else if (ch==CKEY_F4 || ch=='c')
		{
			if (vlist.HasSubTree())
				goto NEXTLOOP;

			if (vlist.HasSelection())
			{
				mh = vlist.NumSelections ();
				if (mh > 10) mh = 10;
				res = menu (vlist.GetVar(), mh, 40, vlist.SelectionList());
				if (res >= 0)
				{
					vlist.SetVal (res);
				}
			}
			else
			{
				if (strlen (vlist.GetVal()) > 511)
					goto NEXTLOOP;
				strcpy (tmpinp, vlist.GetVal());
				if (InputBox (vlist.GetVar(), 50, tmpinp, 512))
				{
					vlist.SetVal(string(tmpinp));
				}
			}
			RefreshAll ();
		}
		//! load a file and merge with the exisiting tree
		else if (ch==CKEY_F3 || ch=='m')
		{
			strcpy (tmpinp, "");
			if (InputBox ("Read file", 30, tmpinp, 512))
			{
				if (ReadIniFile (tmpinp, vlist))
				{
					saveas = tmpinp;
					vlist.AddTreePointer ();
					// neu anzeigen
					pos = 0;
					vlist.SetIterator (0);
					ShowList (sl, vlist, lheight, pos);
				}
			}
		}
		//! show/hide all subtrees
		else if (ch==CKEY_F5 || ch=='f')
		{
			treeopen = !treeopen;
			vlist.TreeOpen (treeopen);
			pos = 0;
			vlist.SetIterator (0);
			ShowList (sl, vlist, lheight, pos);
		}
		//! exact search for an variable
		else if (ch == '/')
		{
			strcpy (tmpinp, "");
			if (InputBox("Search variable", 40, tmpinp, 512))
			{
				if (vlist.SetIterator (tmpinp))
				{
					vlist.OpenParents ();
					pos = 0;
					ShowList (sl, vlist, lheight, pos);
				}
			}
		}
		//! save the tree as Ini-File
		else if (ch==CKEY_F2 || ch=='s')
		{
			strcpy (tmpinp, saveas.c_str());
			if (InputBox ("Save to file:", 50, tmpinp, 512))
			{
				vlist.WriteIniFile (tmpinp, YesNo ("save with comments?"));
				vlist.NotChanged();
			}
		}
		WClear (valW); MvWAddStr (valW, 0, 0, vlist.GetVal());
		WClear (hlp); MvWAddStr (hlp, 0, 0, vlist.GetHelp ());

		UpdatePanels ();
		DoUpdate ();
		WMove (sl, pos, 2);
NEXTLOOP:
		ch = WGetCH(sl);
	}
	while (ch!=CKEY_F10 && ch!='q' && ch!='Q');

	if (vlist.IsChanged ())
	{
		strcpy (tmpinp, saveas.c_str());
		if (InputBox ("something changed - save to:", 50, tmpinp, 512))
		{
			vlist.WriteIniFile (tmpinp, YesNo("save with comments?"));
		}
	}

	CloseCurses ();
	return 0;
}

/*!
\mainpage

This program offers an simple, but usefull method for editing ini- or
rc-files. This files contains a list of variables with an value (i.e.
an pair separeted by '=' or anything other). You can add helptext and
possible selections for every variable. Also it is possible to add
categories, thats are meta-variables wich contain other variables
(i.e. category address contains name, street, state, email, ...).
IEdit works with ncurses for viewing the data, so it should be run on
every unix-machine ( !!! insert some win-comment ). The model and the
view are strictely separated, so you can add your own file-structure
by changing the lexical rules (and the DFA) and the TreeList::WriteIniFile()
method. Here is a picture, where I change Doxyfile.

\htmlonly
<img alt="Beispiel" align="center" height="316" widht="501" src="iedit.gif">
<p>
\endhtmlonly
*/
