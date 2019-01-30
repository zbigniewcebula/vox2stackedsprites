#ifndef __PARAMS_MANAGER__
#define __PARAMS_MANAGER__

#include <vector>
#include <iostream>
#include <algorithm>

#include "helpFunctions.h"

using namespace std;

class ParamsManager {
	public:
		class Param {
			public:
				string small;
				string big;
				string description;
				string valueDescription;

				string value	= "";

				Param() {}
				Param(string s, string b, string d, string vd)
					:	small(s), big(b), description(d), valueDescription(vd)
				{}
				Param(const Param& org) {
					small				= org.small;
					big					= org.big;
					description			= org.description;
					valueDescription	= org.valueDescription;
					value				= org.value;
				}

				void print() {
					cout	<< right << setw(7) << small << ", " << left << setw(14) << big
							<< setw(20) << valueDescription
							<< '\t' << setw(40) << description << '\n';
				}

				bool operator==(const Param& rhs) {
					return rhs.small == small and rhs.big == big and description == rhs.description;
				}
				bool operator==(const string& rhs) {
					return (small == rhs) or (big == rhs);
				}
		};
	private:
		vector<Param>	params;

	public:
		ParamsManager() {}

		void addParam(string small, string big, string description, string valueDescription) {
			if(find(params.begin(), params.end(), small) == params.end()
			and find(params.begin(), params.end(), big) == params.end()
			) {
				params.emplace_back(small, big, description, valueDescription);
			}
		}

		vector<Param>::iterator exists(string flag) {
			return find_if(params.begin(), params.end(), [&](Param& p) -> bool {
				return p == flag;
			});
		}

		string getValueOf(string flag) {
			auto	it = exists(flag);
			if(it != badParameter()) {
				return (*it).value;
			}
			return "";
		}

		void printList() {
			for(Param p : params) {
				p.print();
			}
			cout	<< flush;
		}

		inline vector<Param>::iterator badParameter() {
			return params.end();
		}

		void printHelp() {
			cout	<< "Usage: vox2ss [OPTION [VALUE]]\n\n"
					<< "Break up VOX file into separated PNG files as layers along Z-axis\n\n"
					<< "Available options:\n";
			printList();
			cout	<< "\n\n\nFor more visit: https://github.com/zbigniewcebula/vox2stackedsprites\n"
					<< endl;
		}

		void printHelpTileSize() {
			cout	<< "Usage: vox2ss -i Output.vox -o sheet.png -ts 32\n\n"
					<< "===== HELP FOR -ts FLAG =====\n"
					<< "Let's assume that we have image \"sheet.png\" with size of 512x32\n"
					<< "	that means there is 16 sprites, one for each layer of Z level (32 * 16 = 512).\n"
					<< "Vox2ss will load whole image, chop it into 16 smaller parts and treat every part as layer file.\n"
					<< "Remember that width of image HAVE TO be multiple of given \"-ts\" value\n"
					<< "	to let vox2ss split it without problems, also \"-ts\" value cannot be smaller than 1.\n"
					<< "	Value of image width is going to generate \"flat\" model of Z height equal to 1.\n"
					<< "Size of tile is limited to 126 due to VOX format.\n"
					<< "This flag HAVE TO be used with \"-r\" that reverses process and uses images to generate VOX\n"
					<< "	and also \"-o\" flag is mandatory to specify input file of sheet to get data from.\n"

					<< "\n\nFor more visit: https://github.com/zbigniewcebula/vox2stackedsprites\n"
			<< endl;
		}

		bool process(int argc, char** argv) {
			string		tempStr;
			bool		paramOverload	= false;
			auto		lastParam		= badParameter();
			for(int i = 1; i < argc; ++i) {
				tempStr	= argv[i];
				if(startsWith(tempStr, "-")) {
					lastParam	= exists(tempStr);
					if(lastParam == badParameter()) {
						cerr	<< "Unknown parameter \"" << tempStr << "\"! Aborting..." << endl;
						return false;
					} else if((*lastParam) == "-h") {
						printHelp();
						return false;
					} else if((*lastParam) == "-hts") {
						printHelpTileSize();
						return false;
					} else if((*lastParam) == "-f"
						or (*lastParam) == "-r"
						or (*lastParam) == "-d"
					) {
						(*lastParam).value	= "1";
					} else if((*lastParam).value != "") {
						cerr	<< "Parameter \"" << tempStr << "\" used multiple times! Aborting..." << endl;
						return false;
					}
				} else {
					if(lastParam != badParameter()) {
						(*lastParam).value	= tempStr;
					} else {
						paramOverload	= true;
					}
					lastParam	= badParameter();
				}
			}
			if(paramOverload) {
				cout	<< "WARNING! Too much parameters, ignoring not used..." << endl;
			}
			return true;
		}
};

#endif