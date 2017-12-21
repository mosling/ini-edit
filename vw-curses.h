#ifndef _VW_CURSES_
#define _VW_CURSES_

//NCurses Interface methods

// Adding character and strings to the window
void SetBC (char,char,char,char,char,char,char,char);
void WBox (int w);
void MvWAddStr (int w, int l, int c, const char *ch);
void MvWAddNStr (int w, int l, int c, const char *ch, int len);
void WAddStr (int w, const char *ch);
void WAddCH (int w, char ch);
void MvWAddCH (int w, int l, int c, char ch);
void WMove (int w, int r, int c);
void WClear (int w);
void WClrtoEOL (int );
void SetMarker (char ch);
void MarkPos (int w, int o, int n, int c);

// global informations, start and end NCurses, some settings
void InitCurses ();
void CloseCurses ();
int CCols ();
int CLines ();
void Flash ();
void ScrollOK (int w, int t);
void KeyPad (int w, int t);
void WScrl (int, int);

// create new windows, subwindows and boxed windows, delete windows
int NewWin (int, int, int, int);
int BoxWin (int l, int c, int y, int x, const char *text);
int SubWin (int pw, int l, int c, int y, int x);
int BoxSubWin (int pw, int l, int c, int y, int x, const char *text);
void DelWin (int);

// methods for redrawing and updating
void DoUpdate ();
void UpdatePanels ();
void RefreshAll ();
void WRefresh (int w);
void ScrDump (char *);
void ScrRestore (char *);

// get input 
int GetCH ();
int WGetCH (int);

// some defines for simpler using based at Linux
#define StdScr 0
#define CKEY_DOWN 258
#define CKEY_UP 259
#define CKEY_LEFT 260
#define CKEY_RIGHT 261
#define CKEY_ENTER 10
#define CKEY_HOME 262
#define CKEY_END 360
#define CKEY_BACKSPACE 263
#define CKEY_DELETE 330
#define CESCAPE 27
#define CKEY_SPACE 32
#define CTAB 9
#define CKEY_F1 265
#define CKEY_F2 266
#define CKEY_F3 267
#define CKEY_F4 268
#define CKEY_F5 269
#define CKEY_F6 270
#define CKEY_F7 271
#define CKEY_F8 272
#define CKEY_F9 273
#define CKEY_F10 274
#define CKEY_F11 276
#define CKEY_F12 277

#ifdef __APPLE__
#define CKEY_BACKSPACE 127
#endif

#endif
