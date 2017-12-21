/*!
This file offers an method for reading data using an input box with an
one line input field. You can move arround the text insert/delete
character and scroll to right or left.
*/

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "vw-curses.h"
#include "vw-input.h"

/*!
This function uses vw-curses.
\param title
	An title wich is showed centerd an the top of the input box.
\param width
	The width of the inputbox, should be less than CCols (maximal
	columns of the screen).
\param instr
	An initial String wich is showed first. This parameter is an
	pointer to an character field. The size of this field is also
	maximal input size. The field is changed if the input is changed,
	there are no undo functions.
\param max_len
	The length of instr as number.
\return
	If you finish the input by pressing the Enter-Key true is returned.
	Is the last key an Escape false is returned.
\bug
	The variable \e instr is changed immediatly, so the calling method has
	the task to let the variable unchanged if false is returned. Use a
	local variable to do this.
*/
bool InputBox (const char *title, int width, char *instr, int max_len)
{
	int i, x, y, box_y, box_x, bw, dialog;
	int xpos = 0, st = 0, key = 0;
	int height = 3, len;

	/* center dialog box on screen */
	x = (CCols() - width) / 2;
	y = (CLines() - height) / 2;

	dialog = NewWin (height, width, y, x);
	KeyPad (dialog, 1);
	WBox (dialog);

	if (title != NULL)
	{
		WMove (dialog, 0, (width - strlen (title)) / 2 - 1);
		WAddCH (dialog, ' ');
		WAddStr (dialog, title);
		WAddCH (dialog, ' ');
	}

	/* Set up the initial value */
	box_y = 1;
	box_x = 1;
	bw = width - 3;
	WMove (dialog, box_y, box_x);
		
	len = strlen (instr);
	if (len >= bw)
	{
		st = len - bw + 1;
		xpos = bw - 1;
		for (i = 0; i < bw - 1; i++)
			WAddCH (dialog, instr[st + i]);
	}
	else
	{
		WAddStr (dialog, instr);
		st = 0;
		xpos = len;
	}
	WMove (dialog, box_y, box_x + xpos);

	WRefresh (dialog);

	while (key != CESCAPE && key != CKEY_ENTER)
	{
		key = WGetCH (dialog);

		switch (key)
		{
			case CKEY_HOME:
				st = 0;
				xpos = 0;
				break;
			case CKEY_END:
				if (len > bw)
				{
					st = len-bw;
					xpos = bw;
				}	
				else
				{
					st = 0;
					xpos = len;
				}
				break;
			case CKEY_RIGHT:
				if (st+xpos < len)
				{
					xpos++;
					if (xpos==bw) xpos++;
				}
				break;
			case CKEY_LEFT:
				if ( st!=0 || xpos!=0)
					xpos--;
				break;
			case CKEY_DELETE:
				if (st+xpos < len)
				{
					for (i=st+xpos; i<len; i++)
					{
						instr[i]=instr[i+1];
					}
					len--;
				}
				break;
			case CKEY_BACKSPACE:
				if (len > 0)
				{
					for (i=st+xpos-1; i<len; i++)
					{
						instr[i]=instr[i+1];
					}
					len--;
					xpos--;
				}
				break;
			default:
				if (key < 0x100 && isprint (key))
				{
					if (len+1 < max_len)
					{
						/*! case1: insert at the end */
						if (st+xpos==len)
						{
							instr[len++] = (char)key;
							instr[len] = '\0';
						}
						/*! case2: insert in the middle of the input */
						else
						{
							for (i=len; i>st+xpos; i--)
								instr[i] = instr[i-1];
							instr[st+xpos] = (char)key;
							len++;
							instr[len] = '\0';
						}
						xpos++;
					}
					else
						Flash ();  /* Alarm user about overflow */
				}
		}
		/* Korrektur der Position */
		if (xpos > bw)
		{
			st++; xpos=bw;
		}

		if (xpos < 0)
		{
			if (st > bw)
			{
				st -= bw; xpos = bw-1;
			}
			else
			{
				xpos = st-1; st = 0;
			}
		}

		/* Eingabe komplett neu darstellen */
		WMove (dialog, box_y, box_x);
		for (i=0; i<bw; i++)
		{
			if (st+i < len) 
				WAddCH(dialog, instr[st+i]);
			else
				WAddCH(dialog, ' ');
		}
		WMove (dialog, box_y, box_x+xpos);

		WRefresh (dialog);
	}

	DelWin (dialog);

	if (key == CESCAPE)
		return false;
	else
		return true;
}
