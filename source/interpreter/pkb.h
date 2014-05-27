//
// codebot : virtual robotics programming
// by patrick kage
// pkb.h : pkbasic interpreter
//

// this file was created from a mainfile of a different program. it was not designed to do this.
// i am not responsible for any damage this file causes to your psyche.
// it really is that bad.

#ifndef CB_INTRP
#define CB_INTRP

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string.h>
#include <cmath>

#include <cstdlib>
#include <random>

#include "../controller/map.h"

#ifndef PKBASIC_VER
#define PKBASIC_VER 1.0
#endif

#ifndef LIB_LOC
#define LIB_LOC "/usr/local/pkb"
#endif
#ifndef TEMPDIR
#define TEMPDIR "/tmp/"
#endif

#ifndef INSERT_RDEPTH
#define INSERT_RDEPTH 1000
#endif

using namespace std;

class Parsedcmd {
public:
	string cmd, orig;
	vector<string> args;
	void add_arg(string arg) {
		args.push_back(arg);
	}
	int arg_count() {return args.size();}
	string get_arg(int n) {
		if (n > args.size() || n < 0) {
			return "";
		}
		return args.at(n);
	}
};

class PFile {
private:
	int depth;
public:
	vector<string> lines;
	bool get_from_file(string fname) {
		ifstream file; file.open(fname.c_str());
		if (!file) {
			cout << "ERROR: file " << fname << "not found";
			return false; // file not found
		}
		string tmp;
		while (!file.eof()) {
			getline(file, tmp);
			lines.push_back(tmp);
		}
		return true;
	}
	string line(int n) {return lines.at(n - 1);}
};


class pkb_instance {
public:
// Core function prototypes

// Variable registers
string sreg[52];
double ireg[52];
const string letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
const string NM = "0123456789";

// Flags
bool ERROR_THROWN = false;
string ERROR_TYPE = "NONE";
bool IPROMPT = false; // goto & lbl directives disabled in liveprompt mode
void throw_error(string err) {ERROR_THROWN = true; ERROR_TYPE = err;}
string LIBRARY_PATH = LIB_LOC;
vector<string> libs;
vector<string> created_files;
const int RFILE_NLEN = 10;
int posinfile;
PFile pf;
bool finished;

// IO FUNCTIONS IMPLENENTED IN HERE
void gets(char var) {
        	int index = chartovarindex(var);
        	if (index != -1) {
        	        getline(cin,sreg[index]);
        	} else {
        	        throw_error("VARIABLE NOT FOUND");
        	}
	}
	void getint(char var) {
        	int index = chartovarindex(var);
        	if (index == -1) {
        	        throw_error("VARIABLE NOT FOUND");
        	        return;
        	}
        	string tmp;
        	cin >> tmp;
        	ireg[index] = atof(tmp.c_str());
        	return;
	}
	void print(string s) {
        	if (s[0] == '\"') {
                	stripQuotes(s);
                	string final = "";
                	for (int c = 0; c < s.length(); c++) {
                	        // HOLY SHIT ESCAPE CHARACTERS
                        	if (s.at(c) == '\\' && (c + 1 != s.length())) {
                                	if (s.at(c+1) == 'n') {
                                        	final += '\n';
                                        	c++;
                                        	continue;
					}
				}
	                        final += s.at(c);
	        	}
	                cout << final;
	        } else if (s[0] == '$') {
	                int index = chartovarindex(s[1]);
	                if (index == -1) {
	                        throw_error("VARIABLE NOT FOUND");
	                } else {
	                        cout << sreg[index];
	                }
	        } else if (s[0] == '#') {
	                int index = chartovarindex(s[1]); if (index == -1) {throw_error("VARIABLE NOT FOUND"); return;}
	                stringstream ss; ss.clear(); ss << ireg[index];
	                cout << ss.str();
	        }
	}

// old entry point
int main(int argc, char* argv[]) {
	srand(time(0));
	initialize_variables();
	if (argc == 1) {
		IPROMPT = true;
		cout << "PKBasic interactive prompt enabled.";
		iprompt();
	}
	if (argc == 2) {
		//cout << "One argument! : " << argv[1] << "\nThis is a file we should run.\n";
//		fprompt(string(argv[1]));
	}
	cout << "\n";
	clean_files();
	return 0;
}

// Interactive prompt
void iprompt() {
	cout << "\ntype \"exit\" to exit";
	string line;
	while (true) {
		ERROR_THROWN = false;
		cin.clear();
		cout << "\n] ";
		getline(cin, line);
		if (line == "exit") {
			break;
		}
		if (!runcmd(parsecmd(line))) {
			cout << "\nCOMMAND NOT FOUND";
		} else {
			//cout << "\nCOMMAND RUN";
		}
		if (ERROR_THROWN) {
			cout << "ERROR: " << ERROR_TYPE;
		}
	}
	return;
}

// Command parser
Parsedcmd parsecmd(string str) {
	int spos = str.find(" ");
	string cmd, arg;
	if (spos == -1) {
		cmd = str;
		arg = "";
	} else {
		cmd = str.substr(0, spos);
		arg = str.substr(spos + 1);
	}
	//cout << "Parsed command as: [" << cmd << "]";
	//cout << "\nParsed argument as [" << arg << "]";
	Parsedcmd pcmd;
	pcmd.cmd = cmd;
	pcmd.orig = str;
	pcmd.add_arg(arg);
	return pcmd;
}

// Command director
bool runcmd(Parsedcmd pcmd) {
	if (pcmd.cmd == "print") {
		print(pcmd.get_arg(0));
		return true;
	}
/*	if (pcmd.cmd == "system") {
		cout << "System commanded!";
		system(pcmd.get_arg(0));
		return true;
 	} */
	if (pcmd.cmd == "get") {
		if (pcmd.get_arg(0)[0] == '#') {
			getint(pcmd.get_arg(0)[1]);
		} else if (pcmd.get_arg(0)[0] == '$') {
			gets(pcmd.get_arg(0)[1]);
		} else {
			throw_error("INVALID GET CALL: NO RECEPTACLE");
		}
		return true;
	}
	if (pcmd.cmd[0] == '$') {
		if (pcmd.arg_count() == 0) {
			ERROR_THROWN = true;
			ERROR_TYPE = "UNUSED_VALUE";
			return true;
		}
		Parsedcmd tmp = parsecmd(pcmd.get_arg(0));
		if (tmp.cmd != "=") {
			throw_error("INVALID OPERAND ON STRING");
		} else {
			string temp = tmp.get_arg(0);
			if (temp[0] != '\"') {
			//	cout << temp[0];
				throw_error("INVALID TYPE");
				return true;
			}
			stripQuotes(temp);
			set_string(pcmd.cmd[1], temp);
			return true;
		}
		return true;
	}
	if (pcmd.cmd == "floor") {
		floor(pcmd.get_arg(0));	
		return true;
	}
	if (pcmd.cmd == "ceil") {
		ceil(pcmd.get_arg(0));	
		return true;
	}
	if (pcmd.cmd[0] == '#') {
		intops(pcmd);
		return true;
	}
	if (pcmd.cmd == "sys") {
		std::system(pcmd.get_arg(0).c_str());
		return true;
	}
        if (pcmd.cmd == "import") {
            import_lib(pcmd.get_arg(0));
            return true;
        }
	if (checklib(pcmd.cmd)) {
		call_lib(pcmd.cmd, pcmd.orig);
		return true;
	}
	if (pcmd.cmd == "skip") {
		return true; // doesn't do anything
	}
	return false;
}

void stripQuotes(string &in) {
	if (in.at(0) == '\"') {in.erase(0,1);}
	if (in.at(in.length() - 1) == '\"') {in.erase(in.length() - 1, 1);}
	return;
}

int chartovarindex(char ch) {
	return letters.find(ch);
}

// BEGIN AUXILLARY DEFINITIONS

void system(string s) {
	stripQuotes(s);
	cout << "\nExecuting: [" << s << "]\n";
	cout.clear();
	system("clear");
	cout << "\nFinished execute.";
}

void set_int(char var, int val) {
	int index = chartovarindex(var);
	if (index != -1) {
		ireg[index] = val;
	} else {
		ERROR_THROWN = true;
		ERROR_TYPE = "VARIABLE NOT FOUND";
	}
}

void set_string(char var, string str) {
	int index = chartovarindex(var);
        if (index != -1) {
                sreg[index] = str;
        } else {
                ERROR_THROWN = true;
                ERROR_TYPE = "STRING NOT FOUND";
        }
}

void intops(Parsedcmd pcmd) {
	int index = chartovarindex(pcmd.cmd[1]);
	if (index == -1) {
		throw_error("VARIABLE NOT FOUND");
		return;
	}
	Parsedcmd dref = parsecmd(pcmd.get_arg(0));
	if (NM.find(dref.get_arg(0)[0]) != -1) {
		if (dref.cmd == "=") {
			ireg[index] = atof(dref.get_arg(0).c_str());
		}
		if (dref.cmd == "+=") {
			ireg[index] = ireg[index] + atof(dref.get_arg(0).c_str());
		}
		if (dref.cmd == "-=") {
			ireg[index] = ireg[index] - atof(dref.get_arg(0).c_str());
		}
		if (dref.cmd == "*=") {
			ireg[index] = ireg[index] * atof(dref.get_arg(0).c_str());
		}
		if (dref.cmd == "floor") {
			ireg[index] = std::floor(ireg[index]);
		}
		if (dref.cmd == "ceil") {
			ireg[index] = std::ceil(ireg[index]);
		}
		if (dref.cmd == "^=") {
			ireg[index] = pow(ireg[index], atof(dref.get_arg(0).c_str()));
		}
		if (dref.cmd == "%=") {
			ireg[index] = (int)ireg[index]%(int)std::floor(atoi(dref.get_arg(0).c_str()));
		}
		if (dref.cmd == "~~") {
			ireg[index] = rand()%atoi(dref.get_arg(0).c_str());;
		}
		if (dref.cmd == "/=") {
                        ireg[index] = ireg[index]/atof(dref.get_arg(0).c_str());
                }
	} else if (dref.get_arg(0)[0] == '#') {
		int i2 = chartovarindex(dref.get_arg(0)[1]);
		if (i2 == -1) {
			throw_error("VARIABLE NOT FOUND");
			return;
		}
		if (dref.cmd == "=") {
                        ireg[index] = ireg[i2];
                }
                if (dref.cmd == "+=") {
                        ireg[index] = ireg[index] + ireg[i2];
                }
                if (dref.cmd == "-=") {
                        ireg[index] = ireg[index] - ireg[i2];
                }
                if (dref.cmd == "*=") {
                        ireg[index] = ireg[index] * ireg[i2];
                }
		if (dref.cmd == "/=") {
			ireg[index] = ireg[index]/ireg[i2];
		}
		if (dref.cmd == "^=") {
                        ireg[index] = pow(ireg[index], ireg[i2]);
                }
                if (dref.cmd == "%=") {
                        ireg[index] = (int)ireg[index]%(int)std::floor(ireg[i2]);
                }
		if (dref.cmd == "~~") {
			ireg[index] = rand()%(int)std::floor(ireg[i2]);
		}
	} else {
		throw_error("INVALID OPERAND ON INTEGER");
	}
}
void fprompt(string file) {
	Parsedcmd pcmd;
	int tmp; bool temp;
//	cout << "GETTING PFile FROM " << file << "\n";
	PFile pf;
	if (!pf.get_from_file(file)) {
		cout << "ERROR: UNABLE TO OPEN FILE\n";
		return;
	}
//	cout << "STARTING MAIN LOOP";
	for (int c = 0; c < pf.lines.size(); c++) {
		pcmd = parsecmd(pf.lines.at(c));
		if (c == 0) {
			if (pf.lines.at(c).at(0) == '#' && pf.lines.at(c).at(1) == '!') {
				continue;
			}
		}
		if (pcmd.cmd == "goto") {
			if (pcmd.get_arg(0)[0] == '#') {
				tmp = get_label(pf, pcmd.get_arg(0)[1]);
				if (tmp == -2) {
					throw_error("VARIABLE NOT FOUND");
				}
			} else {
				tmp = get_label(pf, atoi(pcmd.get_arg(0).c_str()));
			}
			if (tmp == -1) {
				throw_error("LABEL NOT FOUND");
			}
			if (tmp >= 0) {
				c = tmp;
			}
		} else if (pcmd.cmd == "label") {
			// do nothing
		} else if (pcmd.cmd == "if") {
			temp = ifstatement(pcmd.get_arg(0));
			if (!ERROR_THROWN) {
				if (!temp) {
					c++;
				}
			}
		} else if (pcmd.cmd[0] == '/' && pcmd.cmd[1] == '/') {
			// don't do shit
		} else if (pcmd.cmd == "insert") {
			fprompt(pcmd.get_arg(0));
		} else {
			runcmd(pcmd);
		}
		if (ERROR_THROWN) {
			stringstream ss; ss.clear(); ss << posinfile;
			print("error on line " + ss.str());
		}
	}
}
bool init(string file) {
	posinfile = 0;
	if (!pf.get_from_file(file)) {
		return false;
	}
	finished = false;
	return true;
}

int scan(Map &map, Parsedcmd pcmd, int y, int x) {
	int dx = 0, dy = 0, distance = 0;
	if (pcmd.get_arg(0) == "up") {
		dy = -1;
	} else if (pcmd.get_arg(0) == "down") {
		dy = 1;
	} else if (pcmd.get_arg(0) == "right") {
		dx = 1;
	} else if (pcmd.get_arg(0) == "left") {
		dx = -1;
	} else {
		throw_error("Invalid direction!");
		return -1;
	}
	do {
		x += dx;
		y += dy;
		distance++;
	} while (map.read(y, x) == ".");
	string tmp = map.read(y, x);
	if (tmp == "@") {ireg[0] = 1;}
	if (tmp == "#") {ireg[0] = 0;}
	ireg[1] = distance;
	return 0;
}

bool move(Map &map, Parsedcmd pcmd, int &y, int &x) {
	int dx = 0, dy = 0;
	if (pcmd.get_arg(0) == "up") {
		dy = -1;
	} else if (pcmd.get_arg(0) == "down") {
		dy = 1;
	} else if (pcmd.get_arg(0) == "right") {
		dx = 1;
	} else if (pcmd.get_arg(0) == "left") {
		dx = -1;
	} else {
		throw_error("Invalid direction!");
		return false;
	}
	if (map.read(y + dy, x + dx) != ".") {
		return false;
	}
	y += dy;
	x += dx;
	return true;
}

bool fire(Map &map, Parsedcmd pcmd, int y, int x) {
	int dx = 0, dy = 0;
	if (pcmd.get_arg(0) == "up") {
		dy = -1;
	} else if (pcmd.get_arg(0) == "down") {
		dy = 1;
	} else if (pcmd.get_arg(0) == "right") {
		dx = 1;
	} else if (pcmd.get_arg(0) == "left") {
		dx = -1;
	} else {
		throw_error("Invalid direction!");
		return false;
	}
	while (map.valid(y,x)) {
		y += dy;
		x += dx;
		if (map.read(y,x) == "@robot") {
			map.write(y,x,"@hit");
		} else if (map.read(y,x) == ".") {
			map.write(y,x,"o");
		} else if (map.read(y,x) == "#") {
			return true;
		}
	}
	return true;
}

std::string get_exec() {return pf.lines.at(posinfile);}

void tick(Map &map, int &y, int &x) {
	if (finished) {return;}
	if (posinfile >= pf.lines.size()) {finished = true; return;}
	Parsedcmd pcmd = parsecmd(pf.lines.at(posinfile));
	int tmp; bool temp;
		if (posinfile == 0) {
			if (pf.lines.at(posinfile).at(0) == '#' && pf.lines.at(posinfile).at(1) == '!') {
			posinfile++;
			return;
			}
		}
		if (pcmd.cmd == "goto") {
			if (pcmd.get_arg(0)[0] == '#') {
				tmp = get_label(pf, pcmd.get_arg(0)[1]);
				if (tmp == -2) {
					throw_error("VARIABLE NOT FOUND");
				}
			} else {
				tmp = get_label(pf, atoi(pcmd.get_arg(0).c_str()));
			}
			if (tmp == -1) {
				throw_error("LABEL NOT FOUND");
			}
			if (tmp >= 0) {
				posinfile = tmp;
			}
		} else if (pcmd.cmd == "label") {
			// do nothing
		} else if (pcmd.cmd == "if") {
			temp = ifstatement(pcmd.get_arg(0));
			if (!ERROR_THROWN) {
				if (!temp) {
					posinfile++;
				}
			}
		} else if (pcmd.cmd[0] == '/' && pcmd.cmd[1] == '/') {
			// don't do shit
		} else if (pcmd.cmd == "insert") {
			fprompt(pcmd.get_arg(0));
		} else if (pcmd.cmd == "scan") {
			scan(map,pcmd,y,x);
		} else if (pcmd.cmd == "move") {
			move(map,pcmd,y,x);
		} else if (pcmd.cmd == "fire") {
			fire(map,pcmd,y,x);
		} else {
			runcmd(pcmd);
		}
		if (ERROR_THROWN) {
			stringstream ss; ss.clear(); ss << posinfile;
			cout << "\nERROR: " << ERROR_TYPE << "\nLINE: " << ss.str();
			ERROR_THROWN = false;
		}
	posinfile++;
}

int get_label(PFile &pf, int label) {
	Parsedcmd pc;
	for (int c = 0; c < pf.lines.size(); c++) {
		pc = parsecmd(pf.lines.at(c));
		if (pc.cmd == "label") {
			if (label == atoi(pc.get_arg(0).c_str())) {
				return c;
			}
		}
	}
	return -1;
}


int get_label(PFile &pf, char ivar) {
	int index = chartovarindex(ivar);
	if (index == -1) {
		return -2;
	}
	Parsedcmd pc;
        for (int c = 0; c < pf.lines.size(); c++) {
                pc = parsecmd(pf.lines.at(c));
                if (pc.cmd == "label") {
                        if (ireg[index] == atof(pc.get_arg(0).c_str())) {
                                return c;
                        }
                }
        }
        return -1;
}

bool ifstatement(string stmt) {
	Parsedcmd t1, t2;
	t1 = parsecmd(stmt);
	t2 = parsecmd(t1.get_arg(0));
	string bit1, bit2, operand;
	bit1 = t1.cmd;
	bit2 = t2.get_arg(0);
	operand = t2.cmd;
	int i1, i2;
	if (bit1[0] == '$' && bit2[0] == '$') {
		i1 = chartovarindex(bit1[1]);
		i2 = chartovarindex(bit2[1]);
		if (i1 == -1 || i2 == -1) {
			throw_error("VARIABLE NOT FOUND");
			return false;
		}
		if (operand == "==") {
			if (sreg[i1] == sreg[12]) {
				return true;
			} else {
				return false;
			}
		} else if (operand == "!=") {
			return (sreg[i1] != sreg[i2]);
		} else {
			throw_error("ILLEGAL OPERATION ON STRING");
		}
	} else if (bit1[0] == '$' && bit2[0] != '$') {
		i1 = chartovarindex(bit1[1]);
		if (i1 == -1) {
			throw_error("VARIABLE NOT FOUND");
			return false;
		}
		if (operand == "==") {
			stripQuotes(bit2);
//			cout << "{bit1: (" << bit1 << ")[" << i1 << "], bit2: ("<< bit2 << ")}"; // debug, showed var info
			return (sreg[i1] == bit2);
		} else if (operand == "!=") {
			stripQuotes(bit2);
			return (sreg[i1] != bit2);
		} else {
			throw_error("VARIABLE NOT FOUND");
			return false;
		}
	} else
	if (bit1[0] == '#' && bit2[0] == '#') {
		i1 = chartovarindex(bit1[1]);
		i2 = chartovarindex(bit2[1]);
		if (i1 == -1 || i2 == -1) {
			throw_error("VARIABLE NOT FOUND");
			return false;
		} else
		if (operand == "==") {
			return (ireg[i1] == ireg[i2]);
		} else
		if (operand == ">") {
			return (ireg[i1] > ireg[i2]);
		} else
		if (operand == "<") {
			return (ireg[i1] < ireg[i2]);
		} else
		if (operand == ">=") {
			return (ireg[i1] >= ireg[i2]);
		} else
		if (operand == "<=") {
			return (ireg[i1] <= ireg[i2]);
		} else
		if (operand == "!=") {
			return (ireg[i1] != ireg[i2]);
		} else {
			return false;
			throw_error("ILLEGAL OPERAND ON NUMBER");
		}
	}
	if (bit1[0] != bit2[0]) {
		throw_error("TYPE MISMATCH");
	} else {
		throw_error("INVALID TYPE");
	}
	return false;
}

void syscall(string cmd) {
	system(cmd.c_str());
}

string dumpvars() {
	stringstream ss;
	string ofile = TEMPDIR;
	for (int c = 0; c < RFILE_NLEN; c++) {
		if (rand()%2) {
			ofile += letters.at(rand()%52);
		} else {
			ofile += NM.at(rand()%10);
		}
	}
	ofile += ".vd";
//	cout << "OFILE: " << ofile << "\n";
	ofstream of; of.open(ofile.c_str());
	if (!of) {
		throw_error("UNABLE TO OPEN VARDUMP");
		return "";
	}
	for (int c = 0; c < letters.length(); c++) {
		ss.str(""); ss << ireg[chartovarindex(letters.at(c))];
		of << ("#" + letters.substr(c,1) + ss.str()) << '\n';
		of << ("$" + letters.substr(c,1) + sreg[chartovarindex(letters.at(c))]) << '\n';
	}
	of.close();
	return ofile;
}

bool checklib(string lib) {
	for (int c = 0; c < libs.size(); c++) {
		if (libs.at(c) == lib) {
			return true;
		}
	}
	return false;
}

bool call_lib(string lib, string invoc) {
	string vdmp = dumpvars(), cmd, ofile,tmp,td = TEMPDIR;
	for (int c = 0; c < RFILE_NLEN; c++) {
                if (rand()%2) {
                        ofile += letters.at(rand()%52);
                } else {
                        ofile += NM.at(rand()%10);
                }
        }
	cmd = LIBRARY_PATH;
	cmd += "/" + lib + " " + vdmp + " '" + invoc + "' " + TEMPDIR + ofile + ".sc";
	tmp = td + "/" + ofile + ".oc";
	created_files.push_back(tmp);
        created_files.push_back(vdmp);
//	cout << "Calling [" << cmd << "]";
	std::system(cmd.c_str());
	fprompt(TEMPDIR + ofile + ".sc");
	return true;
}

void import_lib(string lib) {
	if (lib.at(0) == '$') {
		int tmp = chartovarindex(lib.at(1));
		if (tmp != -1) {
			libs.push_back(sreg[tmp]);
		} else {
			throw_error("INVALID VARIABLE");
		}
	} else {
		libs.push_back(lib);
	}
}

void initialize_variables() {
	for (int c = 0; c < 52; c++) {
		sreg[c] = " ";
		ireg[c] = 0;
	}
}

void clean_files() {
	string tmp;
	for ( int c = 0; c < created_files.size(); c++ ) {
		tmp = "rm -f " + created_files.at(c);
//		cout << "attempting to execute [" << tmp << "]";
		std::system(tmp.c_str());
	}
}

void floor(string var) {
	if (var[0] != '#') {
		throw_error("INVALID VARIABLE");
	}
	int i = chartovarindex(var[1]);
	if (i == -1) {
		throw_error("INVALID VARIABLE");
	}
	ireg[i] = std::floor(ireg[i]);
}
void ceil(string var) {
	if (var[0] != '#') {
		throw_error("INVALID VARIABLE");
	}
	int i = chartovarindex(var[1]);
	if (i == -1) {
		throw_error("INVALID VARIABLE");
	}
	ireg[i] = std::ceil(ireg[i]);
}
~pkb_instance() {clean_files();}

};	


#endif
