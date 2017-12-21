# Configuration File Editor
An ini file editor with comment parsing using ncurses for terminals.

## Compile
You need **flex** and **ncurses** installed. If you have to adapt path please 
feel free to change the Makefile.

Please select the rigth lexer for you by setting the **LEXER** variable into the `Makefile`, there exists two lexer definitions for 
* ini files `split-ini.l`
* xml files `split-xml.l`

To compile the program simple type 
* make 
* make keys

## Usage
The  program *keys* checks the correct work of function- and
cursor-keys. It shows also the used box-characters. If some keys
are not working at your system please change the key definition section
at `vw-curses.h`.

For an version using normal characters, no function- and cursor-keys
start iedit with
    iedit -c

After succefull compilation and linking start the program
    iedit first.ini

Here you can read something about the program and using of variables,
comments and categories. An example Doxygen-file (Doxygen.tmpl) is
also added.
