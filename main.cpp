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
	paramManager.addParam("-f", "--formats", "Shows available formats", "");
	paramManager.addParam("-of", "--outformat", "Sets format of output layers files", "FORMAT");
	paramManager.addParam("-d", "--display", "Displays output product (WIP)", "SCALE");
	paramManager.addParam("-r", "--reverse", "Reverses process, uses OUTPUT to generate INPUT", "");
	paramManager.addParam(
		"-ts",
		"--tilesize", "Use with -r and -o set to specific file, for help use -hts, max value 126",
		""
	);
	paramManager.addParam("-hts", "--help-ts", "Help message for usage of -ts flag", "");
	if(paramManager.process(argc, argv) == false) {
		return 1;
	}

	bool			outputReversed	= paramManager.getValueOf("-r") not_eq "";
	bool			displayResult	= paramManager.getValueOf("-d") != "";

	int	tileSize		= 0;

	if(paramManager.getValueOf("-ts") != "") {
		tileSize	= stoi(paramManager.getValueOf("-ts"));
		//Naive limit
		if(tileSize > 126) {	//Limit forced by MagicaVoxel VOX format
			cerr	<< "Size of tile is too big! Aborting..." << endl;
			return 1;
		}
		if(tileSize < 0) {
			cerr	<< "Size cannot be negative number! Aborting..." << endl;
			return 1;
		} else if(tileSize < 1) {
			cerr	<< "Size of tile is too small! Aborting..." << endl;
			return 1;
		}
	}

	//Formats
	vector<string>	formats;
	formats.push_back("png");
	formats.push_back("gif");
	formats.push_back("bmp");
	formats.push_back("asc");
	formats.push_back("ppm");
	formats.push_back("pgm");
	//formats.push_back("cpp");
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
		if(outputReversed) {
			cerr	<< "Final file has not a VOX extension! Aborting..." << endl;
		} else {
			cerr	<< "Input file is not a VOX file! Aborting..." << endl;
		}
		return 1;
	}

	//Checking if out (in when reversed) directory/file exists
	bool	outIsFile	= endsWith(outputPath, "." + outFormat);
	if(dirExists(outputPath) == false) {
		if(outIsFile) {
			if(outputReversed and not fileExists(outputPath)) {
				cerr	<< "Incorrect layers file path (does not exists)! Aborting..." << endl;
				return 1;
			}
		} else {
			if(outputReversed) {
				cerr	<< "Incorrect layers directory path! Aborting..." << endl;
			} else {
				cerr	<< "Incorrect output path! Aborting..." << endl;
			}
			return 1;
		}
	}
	if(not outIsFile and outputPath.back() != '/' and outputPath.back() != '\\') {
		outputPath.append(1, '/');
	}
	
	//Loading files
	VOXModel vox;
	vector<CImg<unsigned char>*>	layers;

	vec4	tempColor;

	if(outputReversed) {
		//From layer files to VOX file
		vector<string>	layerFile;
		if(outIsFile) {
			if(tileSize == 0) {
				cerr	<< "Tile size cannot be 0! Aborting..." << endl;
				return 1;
			}
			layerFile.push_back(outputPath);
		} else {
			DIR*	inDir	= nullptr;
			dirent*	entry	= nullptr;

			//Listing files in input directory
			inDir	= opendir(outputPath.c_str());
			if(inDir == nullptr) {
				cerr	<< "Layers directory incorrect! Aborting..."
						<< "Additional message: " << strerror(errno)
				<< endl;
				return 1;
			}

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
		}

		if(layerFile.size() > 0) {
			//Opening files and reading them as layers
			if(layerFile.size() > 1) {
				sort(layerFile.begin(), layerFile.end());
			}
			try {
				if(outIsFile) {
					layers.reserve(1);
					layers.push_back(new CImg<unsigned char>(layerFile.front().c_str()));
					if(layers.front()->width() < tileSize) {
						cerr	<< "Too big tile size for that input sprite sheet! Aborting..." << endl;
						wipeVector(layers);
						return 1;
					}
					if(layers.front()->height() not_eq tileSize) {
						cerr	<< "Height of input sprite sheet is not equal to tile size! Aborting..." << endl;
						wipeVector(layers);
						return 1;
					}
					if(layers.front()->width()%tileSize > 0) {
						cerr	<< "Tile size is not a multiple of the size of input sprite sheet! Aborting..." << endl;
						wipeVector(layers);
						return 1;
					}
				} else {
					layers.reserve(layerFile.size());
					for(string& file : layerFile) {
						layers.push_back(new CImg<unsigned char>((outputPath + file).c_str()));
					}
				}
			} catch(CImgIOException& ex) {
				cerr	<< "No layer file found [Searched for all: '"
						<< outFormat << "' files]! Aborting...\n"
						<< "Details: " << ex.what()
				<< endl;
				wipeVector(layers);
				return 1;
			}

			//Reading layer sizes and palette
			unsigned int	z			= 0;
			unsigned int	paletteID	= 0;
			unsigned short	tempIdx		= 0;

			for(CImg<unsigned char>* layer : layers) {
				//Checking sizes of layers
				if(z == 0) {
					if(outIsFile) {
						vox.SetSize(tileSize, layer->height(), layer->width() / tileSize);
					} else {
						vox.SetSize(layer->width(), layer->height(), layers.size());
					}
					processingBar(0, vox.SizeZ());
				}
				if(not outIsFile and (
					layer->width() not_eq vox.SizeX()
				or layer->height() not_eq vox.SizeY()
				)) {
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
							if(tempIdx == 0xFFFF) {
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
									wipeVector(layers);
									return 1;
								}
							} else {
								//Moving Idx by one, due to VOX palette format
								++tempIdx;
							}
							//Writing voxel with determined color from palette
							if(outIsFile) {
								vox.SetVoxel(x % tileSize, y, x / tileSize, tempIdx);
								if(x % tileSize == 0) {
									processingBar(x / tileSize, vox.SizeZ());	
								}
							} else {
								vox.SetVoxel(x, y, z, tempIdx);
							}
						}
					}
					++z;
					/////////////////////////////////////////////
					if(not outIsFile) processingBar(z, vox.SizeZ());
					/////////////////////////////////////////////
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
		if(vox.SizeZ() == 0) {
			cerr	<< "VOX file is empty! Aborting..." << endl;
			return 1;
		}

		layers.reserve(vox.SizeZ());

		//Copying voxels into layer(s) and saving it as file in specified format
		if(outIsFile) {
			layers.push_back(new CImg<unsigned char>(
				vox.SizeX() * vox.SizeZ(), vox.SizeY(), 1, 4, 0
			));
		}
		//LoL => Material conditional
		bool	implyLayers	= not outIsFile or displayResult;
		for(int z = 0; z < vox.SizeZ(); ++z) {
			/////////////////////////////////////////////
			processingBar(z, vox.SizeZ());
			/////////////////////////////////////////////

			if(implyLayers) {
				layers.push_back(new CImg<unsigned char>(vox.SizeX(), vox.SizeY(), 1, 4, 0));
			}
			
			for(int y = 0; y < vox.SizeY(); ++y) {
				for(int x = 0; x < vox.SizeX(); ++x) {
					if(unsigned char paletteID = vox.VoxelColorID(x, y, z); paletteID == 0) {
						tempColor.a	= 0;
					} else {
						tempColor	= vox.PaletteColor(paletteID);
						tempColor.a	= 255;
					}
					if(outIsFile) {
						layers.front()->draw_point(z * vox.SizeX() + x, y, 0, tempColor.raw, tempColor.a);
					}
					if(implyLayers) {
						layers.back()->draw_point(x, y, 0, tempColor.raw, tempColor.a);
					}
				}
			}
			if(not outIsFile) {
				layers.back()->save((outputPath + "LAYER." + outFormat).c_str(), z, 3);
			}
		}
		if(outIsFile) {
			layers.front()->save(outputPath.c_str());
			if(displayResult) {
				delete	layers.front();
				layers.erase(layers.begin());
			}
		}
		processingBar(vox.SizeZ(), vox.SizeZ());
	}

	cout	<< "\nDone (" << vox.SizeZ() << " layers)!" << endl;

	if(displayResult) {
		Preview::Show("Display of: " + input, layers, stoi(paramManager.getValueOf("-d")), vox.SizeX(), vox.SizeY(), outputPath);
	}

	//Clean out and peace out
	wipeVector(layers);
	return 0;
}