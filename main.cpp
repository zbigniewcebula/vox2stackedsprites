#include <iostream>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <iterator>

#include <cctype>
#include <cstdio>
#include <cstring>

#include <dirent.h>
#include <errno.h>

#define cimg_use_png
#include "CImg.h"
#include "Preview.h"
#include "VOX.h"
#include "png.h"

#include "helpFunctions.h"
#include "ParamsManager.h"

using namespace std;
using namespace cimg_library;

void help();

int main(int argc, char** argv) {
	//CIMG Exceptions
	cimg::exception_mode(0);

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
	paramManager.addParam("-r", "--reverse", "Reverses process, uses OUTPUT to generate INPUT", "");
	if(paramManager.process(argc, argv) == false) {
		return 1;
	}

	bool outputReversed	= paramManager.getValueOf("-r") not_eq "";

	//Formats
	vector<string>	formats;
	formats.push_back("png");
	formats.push_back("gif");
	formats.push_back("bmp");
	formats.push_back("asc");
	formats.push_back("ppm");
	formats.push_back("pgm");
	if(paramManager.getValueOf("-f") not_eq "") {
		cout	<< "Avaiable formats: " << '\n';
		for(string& format : formats) {
			cout	<< '\t' << format << '\n';
		}
		cout	<< flush;
		return 0;
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
	}

	//Output format check
	string	outFormat	= paramManager.getValueOf("-of");
	if(outFormat == "") {
		outFormat	= formats[0];
	} else {
		outFormat = tolower(outFormat);
		if(find(formats.begin(), formats.end(), outFormat) == formats.end()) {
			cerr	<< "Incorrect output format! Aborting..." << endl;
			return 1;
		}
	}

	//Checking input file
	string	input		= paramManager.getValueOf("-i");
	if(not outputReversed and not fileExists(input)) {
		cerr	<< "Input file does not exists! Aborting..." << endl;
		return 1;
	}

	//Cuz I'm lazy - naive cheking VOX file type
	if(string naiveType = tolower(input.substr(input.length() - 3)); naiveType != "vox") {
		if(outputReversed == true) {
			cerr	<< "Final file has not a VOX extension! Aborting..." << endl;
		} else {
			cerr	<< "Input file is not a VOX file! Aborting..." << endl;
		}
		return 1;
	}

	//Checking if out directory exists
	if(dirExists(outputPath) == false) {
		if(outputReversed == true) {
			cerr	<< "Incorrect layers directory path! Aborting..." << endl;
		} else {
			cerr	<< "Incorrect output path! Aborting..." << endl;
		}
		return 1;
	}
	if(outputPath.back() != '/' && outputPath.back() != '\\') {
		outputPath.append(1, '/');
	}
	
	//Loading files
	VOXModel vox;
	vector<CImg<unsigned char>*>	layers;

	vec4	tempColor;

	if(outputReversed) {
		//From layer files to VOX file
		DIR*	inDir	= nullptr;
		dirent*	entry	= nullptr;

		//Listing files in input directory
		inDir	= opendir(outputPath.c_str());
		if(inDir == nullptr) {
			cerr	<< "Layers directory incorrect! Aborting..." << endl;
			cerr	<< "Additional message: " << strerror(errno) << endl;
			return 1;
		}

		vector<string>	layerFile;
		while((entry = readdir(inDir)) not_eq nullptr) {
			string	fileName	= entry->d_name;
			if(fileName == "." || fileName == "..")	continue;

			if(not isDir(fileName)
			and endsWith(tolower(fileName), outFormat)
			) {
				layerFile.push_back(fileName);
			}
		}
		closedir(inDir);

		//Opening files and reading them as layers
		if(layerFile.size() > 0) {
			sort(layerFile.begin(), layerFile.end());
			try {
				for(string& file : layerFile) {
					layers.push_back(new CImg<unsigned char>(
						(outputPath + file).c_str()
					));
				}
			} catch(CImgIOException& ex) {
				cerr	<< "No layer file found [Searched for all: '"
						<< outFormat << "' files]! Aborting...\n"
						<< "Details: " << ex.what()
				<< endl;
				return 1;
			}

			//Reading layer sizes and palette
			unsigned int	z			= 0;
			unsigned int	paletteID	= 0;
			unsigned int	tempIdx		= 0;

			for(CImg<unsigned char>* layer : layers) {
				//Checking sizes of layers
				if(z == 0) {
					vox.SetSize(layer->width(), layer->height(), layers.size());
					cout	<< "Processing[";
					for(int _z = 0; _z < vox.SizeZ(); ++_z) {
						cout	<< '_';
					}
					cout	<< ']' << flush;
				}
				if(layer->width() not_eq vox.SizeX()
				or layer->height() not_eq vox.SizeY()
				) {
					cerr	<< "Layer '" << layerFile[z]
							<< "' size is not same as first layer! Aborting..."
					<< endl;
					return 1;
				} else {
					//Searching pixels for palettes and putting them in VOX
					for(int y = 0; y < layer->height(); ++y) {
						for(int x = 0; x < layer->width(); ++x) {
							//Gathering color of pixel
							tempColor.r	= layer->atXY(x, y, 0, 0);
							tempColor.g	= layer->atXY(x, y, 0, 1);
							tempColor.b	= layer->atXY(x, y, 0, 2);
							tempColor.a	= layer->atXY(x, y, 0, 3);
							if(tempColor.a == 0) {
								//Nonexistent voxel
								continue;
							}

							//Searching for index of palette if exists
							tempIdx	= vox.FindPaletteColorIndex(tempColor);
							if(tempIdx == 0xFFFFFFFF) {
								//If exists checking if palette limit is not reached
								if(paletteID < 256) {
									//Adding color to palette
									vox.SetPaletteColor(paletteID, tempColor);
									tempIdx	= ++paletteID;
								} else {
									cerr	<< "Too many colors! "
											<< "VOX format does not support more than 255 colors. "
											<< "Aborting..."
									<< endl;
									return 1;
								}
							} else {
								//Moving Idx by one, due to VOX palette format
								++tempIdx;
							}
							//Writing voxel with determined color from palette
							vox.SetVoxel(x, y, z, tempIdx);
						}
					}
					/////////////////////////////////////////////
					cout	<< "\rProcessing[";
					for(unsigned int _z = 0; _z < (z + 1); ++_z) {
						cout	<< '|';
					}
					cout << flush;
					/////////////////////////////////////////////
					++z;
				}
			}
		} else {
			cerr	<< "No layer files found [Searched for all: '"
					<< outFormat << "' files]! Aborting..."
			<< endl;
			return 1;
		}
		vox.Save(input);
	} else {
		//From VOX into layers
		if(not vox.Load(input.c_str())) {
			cerr	<< "VOX file is incorrect! Aborting..." << endl;
			return 1;
		}

		//Copying voxels into layer and saving it as PNG file
		cout	<< "Processing[";
		for(int z = 0; z < vox.SizeZ(); ++z) {
			cout	<< '_';
		}
		cout	<< ']' << flush;
		
		for(int z = 0; z < vox.SizeZ(); ++z) {
			layers.push_back(new CImg<unsigned char>(vox.SizeX(), vox.SizeY(), 1, 4, 0));
			CImg<unsigned char>& layer	= *(layers.back());
			for(int y = 0; y < vox.SizeY(); ++y) {
				for(int x = 0; x < vox.SizeX(); ++x) {
					if(unsigned char paletteID = vox.VoxelColorID(x, y, z); paletteID == 0) {
						tempColor.a	= 0;
					} else {
						tempColor	= vox.PaletteColor(paletteID);
						tempColor.a	= 255;
					}
					layer.draw_point(x, y, 0, tempColor.raw, tempColor.a);
				}
			}

			/////////////////////////////////////////////
			cout	<< "\rProcessing[";
			for(int _z = 0; _z < (z + 1); ++_z) {
				cout	<< '|';
			}
			cout << flush;
			/////////////////////////////////////////////
			layer.save((outputPath + "LAYER." + outFormat).c_str(), z, 3);
		}
	}

	cout	<< "\nDone (" << vox.SizeZ() << " layers)!" << endl;

	if(paramManager.getValueOf("-d") != "") {
		Preview::Show(
			"Display of: " + input,
			layers,
			stoi(paramManager.getValueOf("-d")), vox.SizeX(), vox.SizeY(),
			outputPath
		);
	}

	//Clean out and peace out
	for(CImg<unsigned char>* layer : layers) {
		delete	layer;
	}
	return 0;
}