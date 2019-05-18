OBJS=TreeList.o iedit.o vw-curses.o lex.yy.o vw-menu.o vw-input.o
CXXFLAGS=-w -DDEBUG_MODE
CFLAGS=-DDEBUG_MODE
CC = gcc
CXX = g++
LIBS = -lmenu -lpanel -lform -lncurses
LEXER=split-ini.l
LEXOUTPUT=lex.yy.cc

all: iedit

$(LEXOUTPUT): $(LEXER)
	flex $(LEXER)

iedit: $(OBJS)
	$(CXX) -g -v -o iedit $(OBJS) $(LIBS)

keys:
	$(CC) -g -o keys keys.c $(LIBS)

clean:
	rm -f *.o $(LEXOUTPUT) keys iedit
	rm -rf html

pack: clean
	tar cvf ini.tar *.C *.c *.h *.l .files Doxyfile Doxyfile.tmpl Makefile
	gzip ini.tar
