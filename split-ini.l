/*!
\file split.l

The IniFile is read and take into lexical pieces. Than an
deterministig finite automata with 6 states analyse the text.
Every variable entry has at most three parts. The help part, thats a
lot of lines starting with '#', if a line starts with '#-' the help-part
is cleared, so you can add some internal informations, like version,
mail-address and so on. The second part ist the selectionlist, Thats a
list of selections which are separated by , and start with '[[' and end
with ']]' (for example [[Yes,NO]]). You can have more the one selection
part for an variable. The first selection value is used as default
value and is set when you call DefaultSettings.
The last part is the (variable,value) pair separated by an '='-sign. 
You can also add categories simple by writing '\#C <name>' at an line and
all following variables are in the subtree of this category. An
category is ended by '\#C end' at an line.
There is no limit to this categories. 
*/

%{
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "split.h"

#define SSEL  1000
#define ESEL  1001
#define SCAT  1002
#define ECAT  1003
#define HELP  1004
#define NEWL  1005
#define WORD  1006
#define CHAR  1007
#define DELIM 1008
#define SDLM  1009
#define WSPC  1010
#define IGNO  1011
#define HCLR  1012
#define FCAT  1013

/*! Thats are the names of the possible lexical pieces of input */
string inpSTR[14] = {
	"StartSelection",
	"EndSelection",
	"StartCategory",
	"EndCategory",
	"Help",
	"NewLine",
	"Word",
	"Character",
	"Delimiter",
	"SelectionDelimiter",
	"Whitespace (space or tab)",
	"Ignore this",
	"Clear Help Text",
	"Finish categories"
};

string suggestedExtension = "ini";
//! this variable holds the input
string word;
//! the line we read from input file
int line;

%}

%option noyywrap
%option c++

startsel	\[\[
endsel	\]\]
newline	\n
startcat #C[ \t]*
endcat	#C\ end
fincat	#C\ finish
help		#[ \t]*
word		[a-zA-Z_0-9+-]+
delim 	[ \t]*=[ \t]*
seldelim	,[ \t]*
space		[ \t] 
skipnl	\\\n
skipcmt	#-.*\n

%%
^$\n			{ line++; return IGNO; }
{startsel}	{ word = yytext; return SSEL; }
{endsel}		{ word = yytext; return ESEL; }
^{startcat}	{ return SCAT; }
^{endcat}$	{ return ECAT; }
^{fincat}$	{ return FCAT; }
^{help}		{ return HELP; }
{newline}	{ line++; word = "\n"; return NEWL; }
{word}		{ word = yytext; return WORD; }
{delim}		{ word = yytext; return DELIM; }
{seldelim}	{ word = yytext; return SDLM; }
{space}		{ word = " "; return WSPC; }
{skipnl}		{ line++; return IGNO; }
^{skipcmt}	{ line++; return HCLR; }
.				{ word = yytext[0]; return CHAR; }
%%

//! we have this number of rules
#define RULES 34
//! our DFA has this state, where VAR is the start state
enum state {VAR, HLP, SEL, CAT, VAL1, VAL2};
//! the long names of the states
string stateSTR[6] = {
	"Variable",
	"Help",
	"Selection",
	"Category",
	"Value1",
	"Value2"};

state aktstate = VAR; //!< the actual state of the automata
string variable; //!< the name of the variable
string value; //!< the value of the variable
string help; //!< the helptext for an variable or category
string cat; //!< name of an category
vector<string> sel; //!< a vector of string holding the selections

//! holds the information for one state-change rule
struct dfa_struct {
	state oldst; //!< the actual state of the automata
	int inp;     //!< the input, returned by yylex()
	char *func;  /*!< List of functions each identified by an character. 
		The possible functions are
			- i - ignore
			- C - insert new category
			- a - add input to actual category name
			- c - end of category
			- f - finish, closes all open categories
			- R - clear the help text
			- H - add input to the help text
			- N - start new selection entry
			- S - add input to the active selection entry
			- V - add variable name
			- v - add input to the variable value
			- E - insert new variable and reset help, variable, value and
			  selection list
	*/

	state newst; //!< the new state of the automata 
};

/*!
Thats the automata we use in the form state, input, function, new state.
For an list of possible functions see dfa_struct::func
If an unknown (state,input)-pair is reached, the program stops with
the name of the state and the line where the input was found.
*/
dfa_struct automata[RULES] = {
	{ VAR, ECAT, "c", VAR}, { VAR, FCAT, "f", VAR}, { VAR, HCLR, "R", VAR},
	{ VAR, NEWL, "i", VAR}, { VAR, HELP, "i", HLP}, { VAR, SCAT, "i", CAT},
	{ VAR, WORD, "V", VAL1},
	{ HLP, NEWL, "H", VAR}, { HLP, ESEL, "i", HLP}, { HLP, SCAT, "i", HLP},
	{ HLP, ECAT, "i", HLP}, { HLP, WORD, "H", HLP}, { HLP, CHAR, "H", HLP},
	{ HLP, DELIM,"H", HLP}, { HLP, SDLM, "H", HLP}, { HLP, WSPC, "H", HLP},
	{ HLP, SSEL, "NH", SEL},
	{ SEL, ESEL, "H", HLP}, { SEL, WORD, "SH", SEL}, { SEL, NEWL, "H", SEL},
	{ SEL, HELP, "i", SEL}, { SEL, SDLM, "NH", SEL}, { SEL, WSPC, "SH", SEL},
	{ SEL, CHAR, "SH", SEL},
	{ CAT, NEWL, "C", VAR}, { CAT, WORD, "a", CAT}, { CAT, CHAR, "a", CAT},
	{ CAT, WSPC, "a", CAT},
	{ VAL1,DELIM,"i", VAL2},
	{ VAL2,NEWL, "E", VAR}, { VAL2,WORD, "v", VAL2}, { VAL2,CHAR, "v", VAL2},
	{ VAL2,SDLM, "v", VAL2}, { VAL2,WSPC, "v", VAL2}
};

/*!
This automata change his state in dependency of the given input and do
something, fill values, add selections or finish the reading of an
variable by calling AddEntry.
*/
void dfa (state& st, int input, string word,
	TreeList& inifile, int &depth, bool &newS)
{
	int j,l,i;
#ifdef DEBUG_MODE
	unsigned int k;
#endif
	bool rule=false;

	i=0;
	while (i<RULES && rule==false)
	{
		if (automata[i].oldst == st && automata[i].inp == input)
		{
			// haben eine Regel gefunden
			l = strlen (automata[i].func);
			rule = true;
			for (j=0; j<l; j++)
			{
				if (automata[i].func[j] == ':')
				{
					printf ("Warning: %s\n", &(automata[i].func[j]));
					break;
				}
				else
				{
					switch (automata[i].func[j])
					{
						case 'V':
							variable = word;
							break;
						case 'H':
							help = help + word;
							break;
						case 'R': // remove
							help = "";
							break;
						case 'S':
							if  (newS)
							{
								newS = false;
								sel.push_back (word);
							}
							else
							{
								sel[sel.size()-1] += word;
							}
							break;
						case 'N':
							newS = true;
							break;
						case 'a':
							cat = cat + word;
							break;
						case 'C':
							inifile.AddEntry (cat, string(""), 
								help, vector<string>(0x0), depth);
							cat = "";
							help = "";
							depth++;
							break;
						case 'c':
							help = "";
							if (depth > 0)
								depth--;
							break;
						case 'f':
							help = "";
							depth = 0;
							break;
						case 'v':
							value = value + word;
							break;
						case 'E':
#ifdef DEBUG_MODE
							cout << "Variable vollstaendig:" << endl;
							cout << "Hilfe: " << help << endl;
							cout << "Name: -->" << variable << "<--" << endl;
							cout << "Selektionsliste:" << endl;
							for (k=0; k<sel.size(); k++)
							{
								cout << "  -->" << sel[k] << "<--" << endl;
							}
							cout << "Wert: -->" << value << "<--" << endl;
#endif
							inifile.AddEntry (variable, value, help, sel, depth);
							// Werte wieder loeschen
							sel.erase (sel.begin(), sel.end());
							help = "";
							variable = "";
							value = "";
					}
				}
			}
			st = automata[i].newst;
		}
		i++;
	}
	if (rule == false)
	{
		cout << "have no rule for: ";
		cout << stateSTR[st] << " with " << inpSTR[input-1000] << endl;
		cout << "line: " << line << ";read word: /" << word << "/" << endl;
		exit (1);
	}
}

/*! 
This function reads an IniFile with name fname into the TreeList
inifile. If the file not exists false is returned. You can call this
function with different files. If you have two file, an template with
an internal structure and an plain variable file first read the
structured data, because existing variables are overwritten. If you
load first the plain data you have as result all category names at the
end of your existing list, no structure is added.
*/
bool ReadIniFile (char *fname, TreeList& inifile)
{
	string myFile = fname;
	int sl = suggestedExtension.size();
	if( myFile.substr( myFile.size()-sl,sl) != suggestedExtension)
	{
		string dummy;
		std::cout << "This is possibly the wrong parser, the suggested extension is " 
		          << suggestedExtension << " use it? (y/n)" << std::endl; 
		std::cin >> dummy;
		if (dummy != "y")
		{
			return false;
		}
	}

	filebuf iniFileBuffer;
    if (iniFileBuffer.open(fname, ios::in))
  	{
		FlexLexer* lexer = new yyFlexLexer();
		int yyret;
		int depth = 0;
		bool newSel = false;
		aktstate = VAR;
		line = 1;

		std::istream iniStream(&iniFileBuffer);
		lexer->switch_streams(&iniStream, 0);

		while ((yyret = lexer->yylex())>0)
		{
			if (yyret != IGNO)
			{
				dfa (aktstate, yyret, word, inifile, depth, newSel);
			}
		}

		iniFileBuffer.close();
	}
	else
	{
		cerr << "ERROR: can't open file '" << fname << "'" << endl; 
	}

	return true;
}

