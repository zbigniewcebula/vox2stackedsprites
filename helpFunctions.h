#ifndef __HELP_FUNC__
#define __HELP_FUNC__

#include <vector>
#include <string>
#include <algorithm>

#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>

using namespace std;

bool fileExists(string path) {
	FILE*	handle	= fopen(path.c_str(), "r");
	if(handle == nullptr) {
		return false;
	}
	fclose(handle);
	return true;
}

bool dirExists(string path) {
	struct stat info;
	if(stat(path.c_str(), &info) != 0) {
		return false;
	} else if(info.st_mode & S_IFDIR) {
		return true;
	}
	return false;
}

inline bool isDir(string path) {
	//Lazy wrap for code's beauty sake...
	return dirExists(path);
}

string tostring(int number) {
	stringstream ss;
	ss	<< number;
	return ss.str();
}

bool startsWith(const string& haystack, const string& needle) {
	return	needle.length() <= haystack.length() 
	&&		equal(needle.begin(), needle.end(), haystack.begin());
}
bool endsWith(const string& haystack, const string& needle) {
	return	needle.length() <= haystack.length() 
	&&		equal(needle.rbegin(), needle.rend(), haystack.rbegin());
}

string tolower(string in) {
	string ret	= in;
	transform(in.begin(), in.end(), ret.begin(), 
		[](unsigned char c) -> unsigned char {
			return tolower(c);
		}
	);
	return ret;
}

void processingBar(unsigned int value, unsigned int max) {
	if(value == 0) {
		cout	<< "Processing[";
		for(unsigned int p = 0; p < max; ++p) {
			cout	<< '_';
		}
		cout	<< ']' << flush;
		return;
	}
	cout	<< "\rProcessing[";
	for(unsigned int p = 0; p < value; ++p) {
		cout	<< '|';
	}
	cout << flush;
}

template<class T> void wipeVector(vector<T*>& list) {
	for(T* element : list) {
		delete	element;
	}
	list.clear();
}

#endif