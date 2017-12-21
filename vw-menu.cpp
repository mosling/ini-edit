/*!
Some functions for selecting a value from a scrollable list.
*/
#include <cstring>
#include "vw-curses.h"
#include "vw-menu.h"

/*!
Display a window with the given heigt, width and title on the screen.
The vector of strings contains the possible values for selection. This
function returns the position of the selected value, thats in the
range from 0 to ivals.size()-1.
If no value is selected (Escape pressed) than -1 is returned.
*/
int menu(const char *title, int height, int width, vector<string>& ivals)
{
	int i, x, y;
	int line = 0, fline = 0, key = 0;
	int items=ivals.size();
	int litem=items-1;
	int menu, menub;

	/* center dialog box on screen */
	x = (CCols() - width-2) / 2;
	y = (CLines() - height-2) / 2;

	menub = NewWin (height+2, width+2, y, x);
	WBox (menub);
	menu = NewWin (height, width, y+1, x+1);
	KeyPad (menu, 1);
	ScrollOK (menu, 1);

	if (title != NULL)
	{
		WMove (menub, 0, (width - strlen (title)) / 2 - 1);
		WAddCH (menub, ' ');
		WAddStr (menub, title);
		WAddCH (menub, ' ');
	}

	/* Set up the initial value and show the visible values */
	fline = 0;
	line = 0;
	while ((line<items) && (line<height))
	{
		MvWAddNStr (menu, line, 3, ivals[line].c_str(), width);
		line++;
	}
	line = 0;
	MarkPos (menu, -1, line, 1);
	WRefresh (menub);
	WRefresh (menu);

	while (key != CESCAPE && key != CKEY_ENTER)
	{
		key = WGetCH (menu);
		switch (key)
		{
			case CKEY_DOWN:
				if (fline+line < litem)
				{
					if (line<(height-1))
					{
						// we don't need to scroll anything
						i = line;
						line++;
						MarkPos (menu, i, line, 1);
					}
					else
					{
						// line == height-1
						fline += 1;
						WScrl (menu, 1);
						MvWAddNStr (menu, line, 3, ivals[fline+line].c_str(), width);
						MarkPos (menu, line-1, line, 1);
					}
				}
				break;	
			case CKEY_UP:
				if (fline!=0 || line !=0)
				{
					if (line == 0)
					{
						fline--;
						WScrl (menu, -1);
						MvWAddNStr (menu, line, 3, ivals[fline+line].c_str(), width);
						MarkPos (menu, line+1, line, 1);
					}
					else
					{
						i = line;
						line--;
						MarkPos (menu, i, line, 1);
					}
				}
				break;	
		}
		WRefresh (menu);
	}

	DelWin (menu);
	DelWin (menub);

	if (key == CESCAPE)
		return -1;
	else
		return line+fline;
}

/*!
This simple function shows a list of two values (YES and NO) and
return true if YES is selected and otherwise false. The fact that the
user abort this selection (by pressing Escape) is not included, in
this case false is also returned.
*/
bool YesNo (const char *title)
{
	vector<string> yn;
	int l=strlen(title);
	string yes, no;

	yes = "YES";
	no  = "NO";
	
	yn.push_back (yes);
	yn.push_back (no);

	if (0 == menu(title,2,l+4,yn))
		return true;
	else
		return false;
}
