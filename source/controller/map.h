#ifndef CB_MAP
#define CB_MAP

#include <fstream>
#include <vector>

using namespace std;

class MapRow {
private:
	vector<string> row;
public:
	void init(int len, string def) {
		row.clear();
		for (int c = 0; c < len; c++) {
			row.push_back(def);
		}
	}
	MapRow(int len, string def) {
		init(len,def);
	}
	MapRow() {}
	bool valid(int index) {
		if (index < 0 || index >= row.size()) {
			return false;
		}
		return true;
	}
	bool write(int index, string key) {
		if (!valid(index)) {return false;}
		row.at(index) = key;
		return true;
	}
	string read(int index) {
		if (!valid(index)) {return " NULL";}
		return row.at(index);
	}
};

class Map {
private:
	vector<MapRow> map;
	int max_x; int max_y;
public:
	int get_max_x() {return max_x;}
	int get_max_y() {return max_y;}
	bool valid(int y, int x) {
		if (y < 0 || x < 0) {
			return false;
		}
		if (y >= max_y || x >= max_x) {
			return false;
		}
		return true;
	}
	
	string read(int y, int x) {
		if (!valid(y,x)) {
			return " NULL";
		}
		return map.at(y).read(x);
	}
	bool write(int y, int x, string key) {
		if (!valid(y,x)) {
			return false;
		}
		map.at(y).write(x,key);
		return true;
	}
	void init(int y, int x) {
		max_x = x;
		max_y = y;
		map.clear();
		MapRow tmp;
		for (int y = 0; y < max_y; y++) {
			map.push_back(tmp);
			map.back().init(max_x, " NULL");
		}
	}
	bool loadfromfile(string path) {
		ifstream ifile;
		ifile.open(path.c_str());
		if (!ifile) {return false;}
		vector<string> file; string tmpline;
		int maxlen = 0;
		while (!ifile.eof()) {
			getline(ifile, tmpline);
			file.push_back(tmpline);
			if (tmpline.length() > maxlen) maxlen = tmpline.length();
		}
		init(file.size(), maxlen);
		for (int y = 0; y < file.size(); y++) {
			for (int x = 0; x < file.at(y).length(); x++) {
				tmpline = file.at(y).at(x);
				write(y,x,tmpline);
			}
		}
		return true;
	}
	string stringify_view(int sy, int sx, int hlim, int vlim) {
		string out = "", tline = "";
		for (int y = sy; y < max_y; y++) {
			if (y >= LINES - hlim) {break;}
			for (int x = sx; x < max_x; x++) {
				if (x >= COLS - vlim) {break;}
				out += read(y,x).at(0);
			}
			out += "\n";
		}
		return out;
	}
};

#endif
