/*!
\file split-xml.l

An XML-file is read and take into lexical pieces. That pieces are
tags, attributs and text.
*/

%{
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "split.h"

#define STAG  1000
#define ETAG  1001
#define CTAG  1002
#define QTXT  1003
#define NEWL  1004
#define WORD  1005
#define CHAR  1006
#define WSPC  1007
#define IGNO  1008

/*! Thats are the names of the possible lexical pieces of input */
string inpSTR[9] = {
	"StartTag",
	"EndTag",
	"Close Tag",
	"quotet Text",
	"NewLine",
	"Word",
	"Character",
	"Whitespace (space or tab)",
	"Ignore this",
};

string suggestedExtension = ".xml";
//! this variable holds the input
string word;
//! the line we read from input file
int line;
%}

%option noyywrap
%option c++

starttag	<
closetag	>
newline	\n
word		[a-zA-Z_0-9]+
qtext		\"[^"]*\"
delim 	=
space		[ \t] 
skipnl	\\\n
skiptag	<![^>]*>

%%
{skiptag}	{ return IGNO; }
^$\n			{ line++; return IGNO; }
{starttag}	{ return STAG; }
\/{word}>	{ word = yytext; return ETAG; }
{closetag}	{ return CTAG; }
{qtext}		{ word = yytext; return QTXT; }
{newline}	{ line++; word = "\n"; return NEWL; }
{word}		{ word = yytext; return WORD; }
{space}		{ word = " "; return WSPC; }
.				{ word = yytext[0]; return CHAR; }
%%

//! we have this number of rules
#define RULES 21
//! our DFA has this state, where XML is the start state
enum state {STG, ETG, XML, TXT, ATT, TAG};
//! the long names of the states
string stateSTR[6] = {
	"StartTag",
	"EndTag",
	"XML-Text",
	"Text",
	"Attribut",
	"Tagstart"};

state aktstate = XML; //!< the actual state of the automata
string name; //!< the name of the variable
string value; //!< the value of the variable
bool hasAttributes; //! Tag has attributes or not
vector<string> stack; //! stack to check the tag usage

//! holds the information for one state-change rule
struct dfa_struct {
	state oldst; //!< the actual state of the automata
	int inp;     //!< the input, returned by yylex()
	char *func;  /*!< List of functions each identified by an character. 
		The possible functions are
			- i - ignore
			- t - append text to value
			- T - insert an textfield
			- l - insert a new level
			- d - decrease the depth of level
			- a - set the attribute name
			- A - set the attribute value ans save the attribute
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
	{ XML, STAG, "i", TAG},
	{ XML, WORD, "t", TXT},
	{ XML, CHAR, "t", TXT},
	{ XML, NEWL, "i", XML},
	{ XML, WSPC, "i", XML},
	{ TXT, WORD, "t", TXT},
	{ TXT, WSPC, "t", TXT},
	{ TXT, CHAR, "t", TXT},
	{ TXT, QTXT, "t", TXT},
	{ TXT, NEWL, "t", TXT},
	{ TXT, STAG, "T", TAG},
	{ TAG, WORD, "l", STG},
	{ TAG, ETAG, "d", XML},
	{ STG, WSPC, "i", STG},
	{ STG, NEWL, "i", STG},
	{ STG, WORD, "a", ATT},
	{ STG, CTAG, "D", XML},
	{ ATT, CHAR, "i", ATT},
	{ ATT, WSPC, "i", ATT},
	{ ATT, WORD, "A", STG},
	{ ATT, QTXT, "A", STG}
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
	string tmp;
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
						case 't':
							value = value + word;
							break;
						case 'T':
							tmp = "DATA";
							inifile.AddEntry (tmp, string(""), value, 
								vector<string>(0x0), depth);
							value = "";
							break;
						case 'l':
							name = word;
							stack.push_back (name);
							inifile.AddEntry (name, string(""), string(""), 
								vector<string>(0x0), depth);
							depth++;
							break;
						case 'a':
							name = word;
							break;
						case 'A':
							if (hasAttributes == false)
							{
								tmp = "ATTRIBUTES";
								inifile.AddEntry (tmp, string (""),
									string(""), vector<string>(0x0), depth);
								hasAttributes = true;
								depth++;
							}
							value = word;
							inifile.AddEntry (name, string(""), value,
								vector<string>(0x0), depth);
							break;
						case 'D':
							if (hasAttributes)
							{
								hasAttributes = false;
								value = "";
								depth--;
							}
							break;
						case 'd':
							name.assign(word,1,word.size()-2);
#ifdef DEBUG_MODE
							cout << "DEBUG: EndTagName: /" << name << "/" << endl;
#endif
							if (name != stack[stack.size()-1])
							{
								cerr << "line(" << line << ")";
								cerr << "EndTag: " << name << " expected: ";
								cerr << stack[stack.size()-1] << endl;
								exit (1);
							}
							stack.pop_back();
							depth--;
							break;
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
		aktstate = XML;
		hasAttributes = false;
		line = 1;
		inifile.MergeEqual (false);

		std::istream iniStream(&iniFileBuffer);
		lexer->switch_streams(&iniStream, 0);
		while ((yyret = lexer->yylex())>0)
		{
			if (yyret != IGNO)
			{
				dfa (aktstate, yyret, word, inifile, depth, newSel);
			}
		}
	}
	else
	{
		cerr << "ERROR: can't open file '" << fname << "'" << endl; 
	}

	return true;
}
