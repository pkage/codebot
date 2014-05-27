#include <curses.h>
#include <iostream>
#include <sstream>
#include "internal/transfer.h"
#include "interpreter/pkb.h"
#include "controller/game.h"
#include <SFML/Window.hpp>
#ifdef __WIN32
#include <random>
#else
#include <ranlib.h>
#endif
#include <ctime>

using namespace std;



// oneline defs
void xmvaddstr(int y, int x, string s) {mvaddstr(y,x,s.c_str());}
string xmvgetstr(int y, int x) {string tmp; char c[9999]; mvgetstr(y,x,c); tmp = c; return tmp;}
string itos(int i) {stringstream ss; ss << i; return ss.str();}
string ftos(float f) {stringstream ss; ss << f; return ss.str();}
void clearline(int r) {if (r < 0 || r >= COLS) return; for (int c = 0; c < COLS; c++) {mvaddch(r,c, ' ');}}
// function prototypes
void game();
StringPairBundle getbotlist();
void rendermap(Game &gm, int sx, int sy, int hlim, int vlim);
void loadcolorpairs();
int selectacolor();
void inspector(Game &gm);
void botinspector(Game &gm, int index);
int selectbottarget(Game &gm);
void pollMemSlot(Game &gm, int target, int len);
float selectSpeed(float prev);
void winmsg(Game &gm, int tstate);

int main() {
	srand(time(0));
	initscr(); cbreak(); keypad(stdscr, TRUE); noecho(); raw();
	if (!has_colors()) {xmvaddstr(0,0,"Your terminal does not support color!\nRobots may be hard to differentiate.\n\n[press any key to continue]");getch();clear();}
	else {start_color(); loadcolorpairs();}
	game();
	noraw();
	endwin();
	return 0;
}

void game() {
	Game gm;
	StringPairBundle botl = getbotlist();
	for (int c = 0; c < botl.bundle.size(); c++) {
		if (!gm.addbot(botl.bundle.at(c).str1, botl.bundle.at(c).str2, botl.bundle.at(c).freeloader)) {
			clear(); xmvaddstr(0,0,"unable to open file \"" + botl.bundle.at(c).str1 + "\"!\n[press q to quit, press any other key to ignore.]");
			if (getch() == 'q') {return;}
		}
	}
	bool paused = false, tmpbool = false; int tmpi = 0, tstate;
	do {
		clear();
		echo();
		xmvaddstr(0,0,"mapfile to load: ");
		if (tmpi > 0) {
			clearline(1);
			xmvaddstr(1,0,"could not load map! try again? [y/n]");
			refresh();
			noecho();
			switch (getch()) {
				case 'n':
				case 'N':
					endwin();
					return;
					break;
				
				case 'y':
				case 'Y':
					tmpi = 0;
					break;;
			}
			clearline(1);
		}
		tmpi++;
		echo();
	} while (!gm.loadmap(xmvgetstr(1,0)));
	gm.initialize_bots();
	clear();
	float tickdelay = 0.02;
	while (true) {
		clear();
		rendermap(gm,0,0,0,gm.bots.size() + 1);
		for (int c = 0; c < gm.bots.size(); c++) {
			clearline((LINES - gm.bots.size()) + c);
			xmvaddstr((LINES - gm.bots.size()) + c, 0, "Bot name: " + gm.bots.at(c).get_id() + "; health: [" + itos(gm.bots.at(c).get_health()) + "]; coords " + itos(gm.bots.at(c).get_y()) + "," + itos(gm.bots.at(c).get_x()) + "; exec: {" + itos(gm.bots.at(c).pkb.posinfile) + "}");
		}
		for (int c = 0; c < gm.bots.size(); c++) { // live code window
			xmvaddstr(LINES - ((2 * (c + 1)) + 1), COLS - (gm.bots.at(c).get_id().length() + 6), gm.bots.at(c).get_id() + " exec:");
			xmvaddstr(LINES - ((2 * (c + 1))), COLS - gm.bots.at(c).get_exec().length(), gm.bots.at(c).get_exec()); 
		}
		refresh();
		if (!paused) {
			sf::sleep(sf::seconds(tickdelay));
		} else {
			curs_set(0);
			while (true) {
				xmvaddstr(0, COLS - 6, "PAUSED");
				xmvaddstr(1, COLS - 6, "------");
				xmvaddstr(3, COLS - 10, "resume [r]");
				xmvaddstr(4, COLS - 13, "inspector [i]");
				xmvaddstr(5, COLS - 16, "change speed [c]");
				xmvaddstr(6, COLS - 8, "step [s]");
				refresh();
				noecho();
				tmpbool = false;
				switch (getch()) {
					case 'r':
						paused = false;
						tmpbool = true;
						break;	
					case 'i':
						inspector(gm);
						break;
					case 'c':
						tickdelay = selectSpeed(tickdelay);
						break;
					case 's':
						tmpbool = true;
						break;
				}
				if (tmpbool) {break;}
			}
			curs_set(1);
			echo();clear();
		}
		for (int c = 0; c < gm.bots.size(); c++) { // live code window
			xmvaddstr(LINES - ((2 * c) + 1), COLS - (gm.bots.at(c).get_id().length() + 6), gm.bots.at(c).get_id() + " exec:");
			xmvaddstr(LINES - (2 * c), COLS - gm.bots.at(c).get_exec().length(), gm.bots.at(c).get_exec()); 
		}
		refresh();
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {break;}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::P)) {paused = true;}
		tstate = gm.tick();
		if (tstate != -1) {
			winmsg(gm, tstate);
		}
	}
	noraw();
	endwin(); 
	return;
}

StringPairBundle getbotlist() {
	StringPair tmp;
	char tc;
	StringPairBundle out;
	clear();
	xmvaddstr(0,0,"Entering robots into competition...");
	for (int c = 1; true; c++) {
		echo();
		xmvaddstr(c * 4,0,"Robot #" + itos(c) + ":");
		xmvaddstr(1 + c * 4,4,".pkb file: ");
		tmp.str1 = xmvgetstr(1 + c * 4,16);
		xmvaddstr(2 + c * 4,4,"Robot Name: ");
		tmp.str2 = xmvgetstr(2 + c * 4,17);
		xmvaddstr(3 + c * 4, 4, "select a highlight scheme for your robot below.");
		tmp.freeloader = selectacolor();
		clearline(3 + c * 4);
		attron(COLOR_PAIR(tmp.freeloader));
		xmvaddstr(3 + c * 4, 4, "this scheme selected");
		attroff(COLOR_PAIR(tmp.freeloader));
		out.bundle.push_back(tmp);
		xmvaddstr((1 + c) * 4,4,"[press space to enter another robot]");
		noecho();
		refresh();
		tc = getch();
		if (tc != ' ') {break;}
		clearline((1 + c) * 4);
	}
	return out;
}
void loadcolorpairs() {
	init_pair(0, COLOR_WHITE, COLOR_BLACK);
	init_pair(1, COLOR_BLACK, COLOR_WHITE);
	init_pair(2, COLOR_RED, COLOR_BLACK);
	init_pair(3, COLOR_GREEN, COLOR_BLACK);
	init_pair(4, COLOR_YELLOW, COLOR_BLACK);
	init_pair(5, COLOR_BLUE, COLOR_BLACK);
	init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(7, COLOR_CYAN, COLOR_BLACK);
	init_pair(8, COLOR_BLACK, COLOR_RED);
	init_pair(9, COLOR_BLACK, COLOR_GREEN);
	init_pair(10, COLOR_BLACK, COLOR_YELLOW);
	init_pair(11, COLOR_BLACK, COLOR_BLUE);
	init_pair(12, COLOR_BLACK, COLOR_MAGENTA);
	init_pair(13, COLOR_BLACK, COLOR_CYAN);
}

int selectacolor() {
	int selection = 2;
	int start = LINES - 13;
	xmvaddstr(start, 0, "pick a highlight scheme for your robot");	
	for (int c = 2; c < 14; c++) {
		attron(COLOR_PAIR(c));
		xmvaddstr(c + start, 2, "this scheme");
		attroff(COLOR_PAIR(c));
	}
	refresh();
	int tmp;
	int c = 2;
	noecho(); curs_set(0);
	while (true) {
		xmvaddstr(start + selection, 0, ">");
		tmp = getch();
		xmvaddstr(start + selection, 0, " ");
		if (tmp == KEY_UP) {
			if (selection == 2) {selection = 12;} else {selection--;}
		} else if (tmp == KEY_DOWN) {
			if (selection == 12) {selection = 2;} else {selection++;}
		} else if (tmp == ' ' || tmp == 10) {
			break;
		}
		
	}
	for (int d = start; d < LINES; d++) {
		clearline(d);
	}
	echo();curs_set(1);
	return selection;
}

void rendermap(Game &gm, int sx, int sy, int hlim, int vlim) {
	string view = gm.getview(sx,sy,hlim,vlim);
	int y = 0, x = 0;
	for (int c = 0; c < view.size(); c++) {
		while (view.at(c) != '\n') {
			if (view.at(c) == '#') {
				attron(COLOR_PAIR(1));
				mvaddch(y,x,view.at(c));
				attroff(COLOR_PAIR(1));
			} else if (view.at(c) == 'o') {
				attron(COLOR_PAIR(8));
				mvaddch(y,x,view.at(c));
				attroff(COLOR_PAIR(8));
			} else {
				mvaddch(y,x,view.at(c));
			}
			for (int d = 0; d < gm.bots.size(); d++) {
				if (gm.bots.at(d).get_x() == x + sx && gm.bots.at(d).get_y() == y + sy) {
					attron(COLOR_PAIR(gm.bots.at(d).getattr()));
					mvaddch(y,x,'@');
					attroff(COLOR_PAIR(gm.bots.at(d).getattr()));
				} 
			}
			c++;
			x++;
		}
		y++; x = 0;
	}
	
}

void inspector(Game &gm) {
	int target = -1, tmpi;
	bool tmpb = true;
	while (tmpb) { 
		xmvaddstr(0, COLS - 9, "INSPECTOR");
		xmvaddstr(1, COLS - 9, "---------");
		if (target != -1) {
			xmvaddstr(3, (COLS - 8) - gm.bots.at(target).get_id().length(), "target: " + gm.bots.at(target).get_id());
		} else {
			xmvaddstr(3, COLS - 12, "target: none");
		}
		xmvaddstr(4, COLS - 17, "select target [s]");
		xmvaddstr(5, COLS - 22, "scan target memory [S]");
		xmvaddstr(6, COLS - 8, "back [b]");
		refresh();
		switch (getch()) {
			case 's':
				target = selectbottarget(gm);
				break;
			case 'b':
				tmpb = false;
				break;
			case 'S':
				pollMemSlot(gm, target, 10);
				break;
			default:
				break;
			
		}

	}
	string tmps = ""; // cleaning up
	tmpi = 22;
	if (target != -1) {
		if (gm.bots.at(target).get_id().length() + 8 > tmpi) tmpi = gm.bots.at(target).get_id().length() + 8;
	}
	for (int d = 0; d < tmpi + 1; d++) {
		tmps += " ";
	}
	for (int c = 0; c < 7; c++) { 
		xmvaddstr(c, COLS - tmpi, tmps);
	}
}

int selectbottarget(Game &gm) {
	xmvaddstr(8,COLS - 16,"TARGET SELECTION");
	xmvaddstr(9,COLS - 16,"----------------");
	int maxl = 17;
	for (int c = 0; c < gm.bots.size(); c++) {
		xmvaddstr(11 + c, (COLS - gm.bots.at(c).get_id().length()) - 2, gm.bots.at(c).get_id());
		if (gm.bots.at(c).get_id().length() > maxl) {
			maxl = gm.bots.at(c).get_id().length() + 3;
		}
	}
	int sel = 0, tmp; 
	bool tmpb = true;
	while (tmpb) {
		xmvaddstr(11 + sel, COLS - 1, "<");
		tmp = getch();
		xmvaddstr(11 + sel, COLS - 1, " ");
		switch (tmp) {
			case KEY_UP:
				sel = ((sel == 0) ? gm.bots.size() - 1 : sel - 1);
				break;
			case KEY_DOWN:
				sel = ((sel == gm.bots.size() - 1) ? 0 : sel + 1);
				break;
			case KEY_ENTER:
			case 10:
			case ' ':
				tmpb = false;
				break;
		}
	}
	for (int c = 0; c < gm.bots.size() + 4; c++) { // clear the menu
		for (int d = 0; d < maxl; d++) {
			xmvaddstr(c + 8, COLS - d, " ");
		}
	}
	return sel;
}

void pollMemSlot(Game &gm, int target, int len) {
	xmvaddstr(8,COLS - 16,"POLLING MEMORY");
	xmvaddstr(9,COLS - 16,"--------------");
	xmvaddstr(10, COLS - 10, "return [q]");
	string anms = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	int pos = 0, ptmp;
	string tmp;
	bool exit = false;
	while (!exit) {
		for (int c = 0; c < len; c++) { // A : #() : $()
			tmp = "";
			tmp += anms.at(pos + c);
			tmp += " : #(";
			tmp += itos(gm.bots.at(target).pkb.ireg[pos + c]) + ") : $(" + gm.bots.at(target).pkb.sreg[pos + c] + ")";
			xmvaddstr(12 + c, COLS - tmp.length(), tmp);
		}
		refresh();
		switch (getch()) {
			case KEY_UP:
				ptmp = ((pos == 0) ? 0 : pos - 1 );
				break;
			case KEY_DOWN:
				ptmp = ((pos == anms.length() - 10) ? pos : pos + 1);
				break;
			case 'q':
				exit = true;
				break;
		}
		for (int c = 0; c < len; c++) {
			tmp = "";
			tmp += anms.at(pos + c);
			tmp += " : #(";
			tmp += itos(gm.bots.at(target).pkb.ireg[pos + c]) + ") : $(" + gm.bots.at(target).pkb.sreg[pos + c] + ")";
			for (int d = 0; d < tmp.length() + 4; d++) {
				mvaddch(12 + c, COLS - d, ' ');
			}
		}
		pos = ptmp;
	}
	tmp = "                 ";
	for (int c = 0; c < 4; c++) {
		xmvaddstr(8 + c,COLS - 16, tmp);
	}
	for (int c = 0; c < len; c++) {
		tmp = "";
		tmp += anms.at(pos + c);
		tmp += " : #(";
		tmp += itos(gm.bots.at(target).pkb.ireg[pos + c]) + ") : $(" + gm.bots.at(target).pkb.sreg[pos + c] + ")";
		for (int d = 0; d < tmp.length() + 4; d++) {
			mvaddch(12 + c, COLS - d, ' ');
		}
	}
}

float selectSpeed(float prev) {
	xmvaddstr(8, COLS - 15, "SPEED SELECTION");
	xmvaddstr(9, COLS - 15, "---------------");
	xmvaddstr(10, COLS - 15, "prev: " + ftos(prev));
	xmvaddstr(12, COLS - 15, "delay (in ms): ");
	curs_set(1); echo();
	string s = xmvgetstr(13, COLS - 15);
	curs_set(0); noecho();
	float out;
	try {
		out = atof(s.c_str());
	} catch (int e) {
		xmvaddstr(14, COLS - 18, "invalid selection");
		getch();
		out = prev;
	}
	for (int c = 0; c < 7; c++) {
		xmvaddstr(8 + c, COLS - 19, "                   ");
	}
	return out;
}

void winmsg(Game &gm, int tstate) {
	string border, msg;
	msg = " " + gm.bots.at(tstate).get_id() + " has won the match. ";
	for (int c = 0; c < msg.size(); c++) {
		mvaddch((LINES / 2) - 1, ((COLS/2) - (msg.size()/2)) + c, ACS_HLINE);
		mvaddch(LINES/2, ((COLS/2) - (msg.size()/2)) + c, ACS_VLINE);
		mvaddch((LINES / 2) + 1, ((COLS/2) - (msg.size()/2)) + c, ACS_HLINE);
	}
	mvaddch((LINES / 2) - 1, (COLS / 2) - (msg.size()/2), ACS_ULCORNER);
	mvaddch((LINES / 2) - 1, (COLS / 2) + (msg.size()/2), ACS_URCORNER);
	mvaddch((LINES / 2) + 1, (COLS / 2) - (msg.size()/2), ACS_LLCORNER);
	mvaddch((LINES / 2) + 1, (COLS / 2) + (msg.size()/2), ACS_LRCORNER);
	xmvaddstr(LINES / 2, (COLS / 2) - msg.size()/2, msg);
	curs_set(0);
	getch();
	curs_set(1);
}
