#include <stdio.h>
#include <curses.h>

main ()
{
	int ch=0;
	char str[80];

	initscr ();
	noecho ();
	keypad (stdscr, TRUE);

	mvaddstr (20,2,"Press 'Q' to finish.");
	//! show the used ACS_-characters
	move (5,60); addstr ("ACS_ULCORNER :");
	move (6,60); addstr ("ACS_LLCORNER :");
	move (7,60); addstr ("ACS_URCORNER :");
	move (8,60); addstr ("ACS_LLCORNER :");
	move (9,60); addstr ("   ACS_HLINE :");
	move (10,60); addstr ("   ACS_VLINE :");
	move (11,60); addstr (" ACS_DIAMOND :");
	move (5,75); addch (ACS_ULCORNER);
	move (6,75); addch (ACS_LLCORNER);
	move (7,75); addch (ACS_URCORNER);
	move (8,75); addch (ACS_LRCORNER);
	move (9,75); addch (ACS_HLINE);
	move (10,75); addch (ACS_VLINE);
	move (11,75); addch (ACS_DIAMOND);
	move (10,2);
	while (ch != 'Q')
	{
		ch = getch();
		sprintf (str, "Key: %d (%s)", ch, keyname(ch));
		move (10,2);
		clrtoeol ();
		addstr (str);
		refresh ();
	}

	endwin();
}
	
