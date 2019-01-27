#ifndef __PREVIEW__
#define __PREVIEW__

#include <string>
#include <vector>
#include <thread>

#include "CImg.h"
#include "VOX.h"

#include "helpFunctions.h"

using namespace std;
using namespace cimg_library;

class Preview {
	public:
		static void Show(
			string title,
			vector<CImg<unsigned char>*>& layers, int startZoom,
			int modelSizeX, int modelSizeY,
			string outputPath
		) {
			//Preparing helping variables
			int	baseZoom		= startZoom;
			int pxZoom			= 0;
			if(baseZoom < 1) {
				baseZoom = 1;
			}
			if(baseZoom > 10) {
				baseZoom = 10;
			}
			int	winSizeX	= modelSizeX * 2 * baseZoom;
			int	winSizeY	= modelSizeY * 2 * baseZoom;

			//Screen buffer and window
			CImg<unsigned char>* screen	= new CImg<unsigned char>(winSizeX, winSizeY, 1, 3, 0);
			CImgDisplay	window(*screen, title.c_str(), 3, false, false);

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

			vec4	tempColor;

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
				//Clear with checker
				screen->fill(32);
				for(int y = 0; y < screen->height(); ++y) {
					for(int x = 0; x < screen->width(); ++x) {
						if((y%16 < 8 and x%16 < 8)
						or (y%16 >= 8 and x%16 >= 8)
						) {
							for(int c = 0; c < 3; ++c) {
								screen->atXY(x, y, 0, c)	= 96;
							}
						}
					}
				}

				//Display layers
				z	= 0;
				for(CImg<unsigned char>* l : layers) {
					CImg<unsigned char>	layer(*l);
					layer.rotate(angle, layer.width() / 2, layer.height() / 2);
					for(int y = 0; y < layer.height(); ++y) {
						for(int x = 0; x < layer.width(); ++x) {
							tempColor.r	= layer.atXY(x, y, 0, 0);
							tempColor.g	= layer.atXY(x, y, 0, 1);
							tempColor.b	= layer.atXY(x, y, 0, 2);
							tempColor.a	= layer.atXY(x, y, 0, 3);
							if(tempColor.a == 0) {
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
										screen->draw_point(tempX, tempY, 0, tempColor.raw, 1);
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
					tempColor.rawInt	= 0xFFFFFFFF;
					screen->draw_text(0, 0, string("Zoom: " + tostring(pxZoom)).c_str(), tempColor.raw, 0, 1, 12);
					screen->draw_text(0, 14, string("Z: " + tostring(stopZ)).c_str(), tempColor.raw, 0, 1, 12);
					screen->draw_text(0, 28, string(
							"Offset: (" + tostring(offX) + "; " + tostring(offY) + ")"
						).c_str(), tempColor.raw, 0, 1, 12
					);

					helpMenuY		= winSizeY - options.size() * 12;
					int	yMenu		= 0;
					for(OptionKeyPair o : options) {
						screen->draw_text(helpMenuX, helpMenuY + (yMenu * 11), o.option.c_str(), tempColor.raw, 0, 1, 10);
						screen->draw_text(helpMenuX + 80, helpMenuY + (yMenu * 11), o.key.c_str(), tempColor.raw, 0, 1, 10);
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
				if(window.is_key(cimg::keyH) or window.is_key(cimg::keyF1)) {
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
				if(window.is_keyESC() or window.is_closed()) {
					break;
				}
				if(window.is_key_sequence(saveShortcut, 2)) {
					CImg<unsigned char>	screenShot(winSizeX, winSizeY, 0, 3);
					window.snapshot(screenShot);
					screenShot.save((outputPath + "SNAPSHOT.png").c_str());
				}

				//Delaying window event reaction
				this_thread::sleep_for(33ms);	//naive 30 FPS
			}
			delete	screen;
		}
};

#endif