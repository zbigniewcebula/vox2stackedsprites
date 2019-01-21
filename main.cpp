#include <iostream>
#include <sstream>
#include <algorithm>
#include <string>
#include <cctype>
#include <cstdio>

#include "CImg.h"
#include "MagicaVoxel.h"

using namespace std;
using namespace cimg_library;

bool fileExists(const char* path);
string tostring(int number);

int main(int argc, char** argv) {
	//Checking args
	if(argc == 1) {
		cerr	<< "No file path given! Aborting..." << endl;
		return 1;
	}
	if(not fileExists(argv[1])) {
		cerr	<< "Input file does not exists! Aborting..." << endl;
		return 1;
	}

	//Completing pats
	string	path		= argv[1];
	string	appPath		= argv[0];
	size_t	slash		= appPath.find_last_of('\\');
	if(slash == -1) {
		slash	= appPath.find_last_of('/');
		if(slash == -1) {
			appPath.clear();
		} else {
			appPath	= appPath.substr(0, slash - 1);
		}
	} else {
		appPath	= appPath.substr(0, slash - 1);
	}
	
	//Cuz I'm lazy
	string	naiveType	= path.substr(path.length() - 3);
	transform(
		naiveType.begin(), naiveType.end(),
		naiveType.begin(), [](unsigned char c) -> unsigned char {
			return tolower(c);
		}
	);
	if(naiveType != "vox") {
		cerr	<< "Input file is not a VOX file! Aborting..." << endl;
		return 1;
	}

	//Loading files
	MV_Model vox;
	if(not vox.LoadModel(path.c_str())) {
		cerr	<< "VOX file is incorrect! Aborting..." << endl;
		return 1;
	}

	//Copying voxels into layer and saving it as PNG file
	cout	<< "Processing[";
	for(int z = 0; z < vox.sizez; ++z) {
		cout	<< '_';
	}
	cout	<< ']' << flush;
	MV_RGBA			pixel;
	unsigned char	paletteIdx;
	for(int z = 0; z < vox.sizez; ++z) {
		/////////////////////////////////////////////
		cout	<< "\rProcessing[";
		for(int _z = 0; _z < (z + 1); ++_z) {
			cout	<< '|';
		}
		/////////////////////////////////////////////
		CImg<unsigned char> layer(vox.sizex, vox.sizey, 1, 3, 0);
		for(int y = 0; y < vox.sizey; ++y) {
			for(int x = 0; x < vox.sizex; ++x) {
				paletteIdx	= vox.ReadVoxel(x, y, z);
				if(paletteIdx == 0) {
					pixel.a	= 0;
				} else {
					pixel.r	= vox.palette[paletteIdx].r;
					pixel.g	= vox.palette[paletteIdx].g;
					pixel.b	= vox.palette[paletteIdx].b;
					pixel.a	= 1;
				}
				layer.draw_point(x, y, 0, pixel.raw, pixel.a);
			}
		}
		cout << flush;
		layer.save((appPath + ".png").c_str(), z, 3);
	}

	cout	<< "\nDone (" << vox.sizez << " layers)!" << endl;
	return 0;
}

bool fileExists(const char* path) {
	FILE*	handle	= fopen(path, "r");
	if(handle == nullptr) {
		return false;
	}
	fclose(handle);
	return true;
}

string tostring(int number) {
	stringstream ss;
	ss	<< number;
	return ss.str();
}