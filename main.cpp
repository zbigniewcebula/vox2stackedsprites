#include <iostream>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <thread>

#include <cctype>
#include <cstdio>

#define cimg_use_png
#include "CImg.h"
#include "MagicaVoxel.h"
#include "png.h"

#include "helpFunctions.h"
#include "ParamsManager.h"

using namespace std;
using namespace cimg_library;

void help();

int main(int argc, char** argv) {
	//Checking args
	if(argc <= 1) {
		cerr	<< "No parameters given! Aborting..." << endl;
		return 1;
	}

	//Listing args and proccessing parameters/flags
	ParamsManager	paramManager;
	paramManager.addParam("-h", "--help", "Shows help", "");
	paramManager.addParam("-o", "--out", "Sets path for output files", "DIRECTORY");
	paramManager.addParam("-i", "--in", "Sets path for input file", "INTPUT_PATH");
	paramManager.addParam("-of", "--outformat", "Sets format of output layers files", "FORMAT");
	paramManager.addParam("-f", "--formats", "Shows available formats", "");
	paramManager.addParam("-d", "--display", "Displays output product (WIP)", "SCALE");
	paramManager.addParam("-r", "--reverse", "Reverses process, uses OUTPUT ot generate INPUT", "");
	if(paramManager.process(argc, argv) == false) {
		return 1;
	}

	//Formats
	vector<string>	formats;
	formats.push_back("png");
	formats.push_back("gif");
	formats.push_back("bmp");
	formats.push_back("asc");
	formats.push_back("ppm");
	formats.push_back("pgm");
	if(paramManager.getValueOf("-f") != "") {
		cout	<< "Avaiable formats: " << '\n';
		for(string format : formats) {
			cout	<< '\t' << format << '\n';
		}
		cout	<< flush;
		return 0;
	}

	//Checking input file
	string	input		= paramManager.getValueOf("-i");
	if(not fileExists(input)) {
		cerr	<< "Input file does not exists! Aborting..." << endl;
		return 1;
	}

	//Completing paths
	string	outputPath	= paramManager.getValueOf("-o");
	if(outputPath == "") {
		outputPath	= argv[0];
		size_t	slash		= outputPath.find_last_of('\\');
		if(slash == string::npos) {
			slash	= outputPath.find_last_of('/');
			if(slash == string::npos) {
				outputPath.clear();
			} else {
				outputPath	= outputPath.substr(0, slash - 1);
			}
		} else {
			outputPath	= outputPath.substr(0, slash - 1);
		}
	} else {
		if(dirExists(outputPath) == false) {
			cerr	<< "Incorrect output path! Aborting..." << endl;
			return 1;
		}
		if(outputPath.back() != '/' && outputPath.back() != '\\') {
			outputPath.append(1, '/');
		}
	}

	//Output format check
	string	outFormat	= paramManager.getValueOf("-of");
	if(outFormat == "") {
		outFormat	= formats[0];
	} else {
		transform(
			outFormat.begin(), outFormat.end(), outFormat.begin(), 
			[](unsigned char c) -> unsigned char { return tolower(c); }
		);
		if(find(formats.begin(), formats.end(), outFormat) == formats.end()) {
			cerr	<< "Incorrect output format! Aborting..." << endl;
			return 1;
		}
	}

	//Cuz I'm lazy
	string	naiveType	= input.substr(input.length() - 3);
	transform(
		naiveType.begin(), naiveType.end(),
		naiveType.begin(), [](unsigned char c) -> unsigned char {
			return tolower(c);
		}
	);
	if(naiveType != "vox") {
		if(paramManager.getValueOf("-r") != "") {
			cerr	<< "Final file has not a VOX extension! Aborting..." << endl;
		} else {
			cerr	<< "Input file is not a VOX file! Aborting..." << endl;
		}
		return 1;
	}
	
	//Loading files
	MV_Model vox;
	if(not vox.LoadModel(input.c_str())) {
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
	vector<CImg<unsigned char>*>	layers;
	for(int z = 0; z < vox.sizez; ++z) {
		/////////////////////////////////////////////
		cout	<< "\rProcessing[";
		for(int _z = 0; _z < (z + 1); ++_z) {
			cout	<< '|';
		}
		/////////////////////////////////////////////
		layers.push_back(new CImg<unsigned char>(vox.sizex, vox.sizey, 1, 4, 0));
		CImg<unsigned char>& layer	= *(layers.back());
		for(int y = 0; y < vox.sizey; ++y) {
			for(int x = 0; x < vox.sizex; ++x) {
				paletteIdx	= vox.ReadVoxel(x, y, z);
				if(paletteIdx == 0) {
					pixel.a	= 0;
				} else {
					pixel.r	= vox.palette[paletteIdx].r;
					pixel.g	= vox.palette[paletteIdx].g;
					pixel.b	= vox.palette[paletteIdx].b;
					pixel.a	= 255;
				}
				layer.draw_point(x, y, 0, pixel.raw, pixel.a);
			}
		}
		cout << flush;
		layer.save((outputPath + "LAYER." + outFormat).c_str(), z, 3);
	}

	cout	<< "\nDone (" << vox.sizez << " layers)!" << endl;


	if(paramManager.getValueOf("-d") != "") {
		//Preparing helping variables
		int	baseZoom		= stoi(paramManager.getValueOf("-d"));
		int pxZoom			= 0;
		if(baseZoom < 1) {
			baseZoom = 1;
		}
		if(baseZoom > 10) {
			baseZoom = 10;
		}
		int	winSizeX	= vox.sizex * 2 * baseZoom;
		int	winSizeY	= vox.sizey * 2 * baseZoom;

		//(*Screen) buffer and window
		CImg<unsigned char>* screen	= new CImg<unsigned char>(winSizeX, winSizeY, 1, 3, 0);
		CImgDisplay	window(*screen, string("Display of: " + input).c_str(), 3, false, false);

		//Temp variables
		const unsigned int saveShortcut[] = {
			cimg::keyCTRLLEFT, cimg::keyS
		};
		int	z;
		int	stopZ	= layers.size() - 1;

		int	angle	= 0;
		int	tempX	= 0;
		int	tempY	= 0;

		int	offX	= 0;
		int	offY	= 0;

		bool displayHelp	= false;
		int helpMenuX		= 1;
		int helpMenuY		= winSizeY - 80;

		struct OptionKeyPair
		{
			public:
				string	option;
				string	key;

				OptionKeyPair() {}
				OptionKeyPair(string opt, string k)
					:	option(opt), key(k)
				{}
		};
		vector<OptionKeyPair>	options;
		options.emplace_back("Help", "[H][F1]");
		options.emplace_back("Rotate", "[SPACE]");
		options.emplace_back("Move", "[ARROWS]");
		options.emplace_back("Zoom", "[SCROLLWHEEL]");
		options.emplace_back("Layer", "[PGUP/PGDOWN]");
		options.emplace_back("Screenshot", "[CTRL+S]");
		options.emplace_back("Exit", "[ESC]");

		//Main loop of window
		while(true) {
			//Clear
			screen->fill(255);

			//Display layers
			z	= 0;
			for(CImg<unsigned char>* l : layers) {
				CImg<unsigned char>	layer(*l);
				layer.rotate(angle, layer.width() / 2, layer.height() / 2);
				for(int y = 0; y < layer.height(); ++y) {
					for(int x = 0; x < layer.width(); ++x) {
						pixel.r	= layer.atXY(x, y, 0, 0);
						pixel.g	= layer.atXY(x, y, 0, 1);
						pixel.b	= layer.atXY(x, y, 0, 2);
						pixel.a	= layer.atXY(x, y, 0, 3);
						if(pixel.a == 0) {
							continue;
						}
						for(int Y = 0; Y < pxZoom; ++Y) {
							for(int X = 0; X < pxZoom; ++X) {
								tempX	= (winSizeX / 2) - ((layer.width() * pxZoom) / 2) + (
									x * pxZoom
								) + X + offX;
								tempY	= (3 * winSizeY / 4) + ((layer.height() * pxZoom) / 2) - (
									(y * pxZoom) + (z * pxZoom) + Y
								) + offY;
								if(tempX > -1 && tempY > -1 && tempX < winSizeX && tempY < winSizeY) {
									screen->draw_point(tempX, tempY, 0, pixel.raw, 1);
								}
							}
						}
					}
				}
				++z;
				if(z >= stopZ) {
					break;
				}
			}
			//Display help
			if(displayHelp) {
				pixel.r	= 0;
				pixel.g	= 0;
				pixel.b	= 0;
				pixel.a	= 255;
				screen->draw_text(0, 0, string("Zoom: " + tostring(pxZoom)).c_str(), pixel.raw, 0, 1, 12);
				screen->draw_text(0, 14, string("Z: " + tostring(stopZ)).c_str(), pixel.raw, 0, 1, 12);
				screen->draw_text(0, 28, string(
						"Offset: (" + tostring(offX) + "; " + tostring(offY) + ")"
					).c_str(), pixel.raw, 0, 1, 12
				);

				helpMenuY		= winSizeY - options.size() * 12;
				int	yMenu		= 0;
				for(OptionKeyPair o : options) {
					screen->draw_text(helpMenuX, helpMenuY + (yMenu * 11), o.option.c_str(), pixel.raw, 0, 1, 10);
					screen->draw_text(helpMenuX + 80, helpMenuY + (yMenu * 11), o.key.c_str(), pixel.raw, 0, 1, 10);
					++yMenu;
				}
			}

			//Render
			window.display(*screen);
			window.paint();

			//Rotation
			if(window.is_key(cimg::keySPACE)) {
				angle += 45;
				if(angle >= 360) {
					angle	= 0;
				}

			}

			//Zoom control
			pxZoom	= baseZoom + window.wheel(); 
			if(pxZoom < 1) {
				pxZoom = 1;
			}
			if(pxZoom > 20) {
				pxZoom = 20;
			}

			//Help
			if(window.is_key(cimg::keyH) || window.is_key(cimg::keyF1)) {
				displayHelp	= !displayHelp;
			}

			//Layers control
			if(window.is_key(cimg::keyPAGEUP)) {
				stopZ += 1;
				if(size_t(stopZ) >= layers.size()) {
					stopZ	= layers.size() - 1;
				}
			}
			if(window.is_key(cimg::keyPAGEDOWN)) {
				stopZ -= 1;
				if(stopZ < 1) {
					stopZ	= 1;
				}
			}

			//Offset
			if(window.is_key(cimg::keyARROWUP)) {
				offY	-= winSizeY / 20;
			}
			if(window.is_key(cimg::keyARROWDOWN)) {
				offY	+= winSizeY / 20;
			}
			if(window.is_key(cimg::keyARROWLEFT)) {
				offX	-= winSizeX / 20;
			}
			if(window.is_key(cimg::keyARROWRIGHT)) {
				offX	+= winSizeX / 20;
			}
			if(offX < -(winSizeX / 2)) {
				offX = -(winSizeX / 2);
			}
			if(offX > (winSizeX / 2)) {
				offX = (winSizeX / 2);
			}
			if(offY < -(winSizeY / 2)) {
				offY = -(winSizeY / 2);
			}
			if(offY > (winSizeY / 2)) {
				offY = (winSizeY / 2);
			}

			//Window resize lock
			if(window.is_resized()) {
				winSizeX	= window.window_width();
				winSizeY	= window.window_width();
				window.resize(winSizeX, winSizeY, true);

				delete	screen;
				screen		= new CImg<unsigned char>(winSizeX, winSizeY, 1, 3, 0);
			}

			//Rest
			if(window.is_keyESC() || window.is_closed()) {
				break;
			}
			if(window.is_key_sequence(saveShortcut, 2)) {
				CImg<unsigned char>	screenShot(winSizeX, winSizeY, 0, 3);
				window.snapshot(screenShot);
				screenShot.save((outputPath + "SNAPSHOT.png").c_str());
			}

			//Delaying window event reaction
			this_thread::sleep_for(50ms);
		}
		delete	screen;
	}

	//Clean out and peace out
	for(CImg<unsigned char>* layer : layers) {
		delete	layer;
	}
	return 0;
}