#ifndef CB_TRANSFER
#define CB_TRANSFER

#include <vector>

class StringBundle {
public:
	std::vector<std::string> bundle;
	
	void addstring(std::string str) {
		bundle.push_back(str);
	}
};

class StringPair {
public:
	std::string str1;
	std::string str2;
	int freeloader; // bonus because you're such a good customer
};

class StringPairBundle {
public:
	std::vector<StringPair> bundle;
	void addstring(StringPair sp) {
		bundle.push_back(sp);
	}
};


#endif
