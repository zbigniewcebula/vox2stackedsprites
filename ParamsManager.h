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
			cout	<< "Usage: vox2stackedsprites INPUT_FILE [OPTION [VALUE]]\n"
					<< "Break up VOX file into separated PNG files as layers along Z-axis\n\n"
					<< "Available options:\n";
			printList();
			cout	<< "\n\nFor more visit: https://github.com/zbigniewcebula/vox2stackedsprites"
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
					} else if((*lastParam) == "-f" || (*lastParam) == "-r") {
						(*lastParam).value	= ".";
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