#ifndef CB_GAME
#define CB_GAME

#include "botwrapper.h"
#include "map.h"
#include <vector>
#ifdef __WIN32
#include <random>
#else
#include <ranlib.h>
#endif

using namespace std;

class Game {
private:
	Map map;
public:
	vector<Robot> bots;
	bool loadmap(string path) {
		return map.loadfromfile(path);
	}
	bool addbot(string path, string id, int attr) {
		Robot bot;
		if (!bot.pkb.init(path)) { // feed the pkbasic instance the beginning of the file
			return false;
		}
		bot.setattr(attr);
		bot.id = id;
		bots.push_back(bot);
		return true;
	}
	void initialize_bots() {
		int tx, ty;
		for (int c = 0; c < bots.size(); c++ ) {
			do {
				tx = rand()%map.get_max_x();
				ty = rand()%map.get_max_y();
			} while (map.read(ty,tx) != ".");
			bots.at(c).init(ty, tx, 100, 100, bots.at(c).id);
			bots.at(c).placeMarker(map); // make sure another bot isn't placed on top - ever.
		}
		for (int c = 0; c < bots.size(); c++) {
			bots.at(c).removeMarker(map);
		}
	}
	int tick() {
		for (int y = 0; y < map.get_max_y(); y++) {
			for (int x = 0; x < map.get_max_x(); x++) {
				if (map.read(y,x) == "o") map.write(y,x,"."); 
			}
		}
		for (int c = 0; c < bots.size(); c++) {
			bots.at(c).placeMarker(map);
		}
		for (int c = 0; c < bots.size(); c++) {
			bots.at(c).removeMarker(map);
			if (!bots.at(c).pkb.finished) bots.at(c).tick(map);
			bots.at(c).placeMarker(map);
		}
		for (int c = 0; c < bots.size(); c++) {
			bots.at(c).removeMarker(map);
		}
		int cs = 0;
		for (int c = 0; c < bots.size(); c++) {
			if (bots.at(c).alive()) cs++;
		}
		if (cs == 1) {
			for (int c = 0; c < bots.size(); c++) {
				if (bots.at(c).alive()) return c; 
			}
		}
		return -1;
	}
	string getview(int sy, int sx, int hlim, int vlim) {return map.stringify_view(sy,sx,hlim,vlim);}
};

#endif
