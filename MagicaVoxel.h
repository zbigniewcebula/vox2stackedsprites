//================================
//  MagicaVoxel [12/09/2013]
//  Copyright(c) 2013 @ ephtracy. All rights reserved.
//================================

//================================
// Notice that this code is neither robust nor complete.
// It is only a sample code demonstrating
//	 how to load current version .vox model from MagicaVoxel.
//================================

//Refactored by zbigniewcebula

#ifndef __MV_MODEL__
#define __MV_MODEL__

#include <cstdio>
#include <string>
#include <fstream>

using namespace std;
typedef unsigned char	uchar;
typedef unsigned short	ushort;
typedef unsigned int	uint;

//Magic number
int MV_ID(int a, int b, int c, int d) {
	return a | (b << 8) | (c << 16) | (d << 24);
}

//================
// RGBA
//================
class MV_RGBA {
	public:
		union {
			struct {
				unsigned char r, g, b, a;
			};
			unsigned char raw[4];
		};

		MV_RGBA(unsigned char R = 0, unsigned char G = 0, unsigned char B = 0, unsigned char A = 0) {
			r = R; b = B; g = G; a = A;
		}
		MV_RGBA(const MV_RGBA& org) {
			r = org.r; b = org.b; g = org.g; a = org.a;
		}

		bool operator==(const MV_RGBA& cmp) {
			return	cmp.r == r
			and		cmp.g == g
			and		cmp.b == b
			and		cmp.a == a
			;
		}
};

//================
// Voxel
//================
class MV_Voxel {
	public:
		unsigned char x, y, z, colorIndex;
};

//================
// Model
//================
class MV_Model {
	public:
		int sizex, sizey, sizez;
		
		int			numVoxels	= 0;
		MV_Voxel*	voxels		= nullptr;

		bool	isCustomPalette;
		MV_RGBA palette[256];
		
		int version;
		
		~MV_Model() {
			Free();
		}
		
		MV_Model() : 
			sizex(0), sizey(0), sizez(0),
			numVoxels(0), voxels(nullptr),
			isCustomPalette(false),
			version(0)
		{}
		
		bool LoadModel(string path) {
			return LoadModel(path.c_str());
		}
		bool LoadModel(const char* path) {
			// free old data
			Free();
			
			// open file
			FILE* fp = fopen(path, "rb");
			if(fp == nullptr){
				Error("failed to open file");
				return false;
			}
			
			// read file
			bool success = ReadModelFile(fp);
			
			// close file
			fclose(fp);
			
			// if failed, free invalid data
			if(!success) {
				Free();
			}
			
			return success;
		}

		bool IsEmpty() {
			return sizex == 0 && sizey == 0 && sizez == 0;
		}

		void SetSize(int x, int y, int z) {
			sizex		= x;
			sizey		= y;
			sizez		= z;
			numVoxels	= 0;

			if(voxels not_eq nullptr) {
				delete[]	voxels;
			}
			voxels	= new MV_Voxel[sizex * sizey * sizez];
			
			isCustomPalette	= true;
			version			= 150;
		}

		unsigned char ReadVoxel(int x, int y, int z) {
			for(int i = 0; i < numVoxels; ++i) {
				MV_Voxel&	v = voxels[i];
				if(v.x == x && v.y == y && v.z == z) {
					return v.colorIndex;
				}
			}
			return 0;
		}

		void WriteVoxel(int x, int y, int z, int idx) {
			if(x > -1 and x < sizex
			and y > -1 and y < sizey
			and z > -1 and z < sizez
			) {
				if(numVoxels > 0) {
					for(int i = 0; i < numVoxels; ++i) {
						MV_Voxel&	v = voxels[i];
						if(v.x == x and v.y == y and v.z == z) {
							v.colorIndex	= i;
							return;
						}
					}
				}
				voxels[numVoxels].colorIndex	= idx;
				voxels[numVoxels].x				= x;
				voxels[numVoxels].y				= y;
				voxels[numVoxels].z				= z;
				++numVoxels;
			}
		}

		void WritePalette(int index, uchar r, uchar g, uchar b, uchar a = 255) {
			if(index > -1 and index < 256) {
				palette[index].r	= r;
				palette[index].g	= g;
				palette[index].b	= b;
				palette[index].a	= a;
			}
		}
		void WritePalette(int index, const MV_RGBA& clr) {
			if(index > -1 and index < 256) {
				palette[index].r	= clr.r;
				palette[index].g	= clr.g;
				palette[index].b	= clr.b;
				palette[index].a	= clr.a;
			}
		}
		MV_RGBA& ReadPalette(int index) {
			if(index >= 256 or index < 0) {
				index	= 0;
			}
			return palette[index];
		}

		uint FindPaletteColorIndex(uchar r, uchar g, uchar b, uchar a) {
			for(uint i = 0; i < 256; ++i) {
				if(palette[i].r == r
				and	palette[i].g == g
				and	palette[i].b == b
				and	palette[i].a == a
				) {
					 return i;
				}
			}
			return 0xFFFFFFFF;
		}
		uint FindPaletteColorIndex(const MV_RGBA& clr) {
			return FindPaletteColorIndex(clr.r, clr.g, clr.b, clr.a);
		}

		bool SaveModel(string path) {
			return SaveModel(path.c_str());
		}
		bool SaveModel(const char* name = "") {
			ofstream	hFile(name, ios::trunc | ios::out | ios::binary);

			if(hFile.fail()) {
				hFile.close();
				return false;
			}

			//Temp
			int	chunkSize	= 0;

			//Nagłówki
			hFile.write("VOX ", 4);
			hFile.write((char*)&version, 4);
			hFile.write("MAIN", 4);
			hFile.write("\0\0\0\0", 4);
			chunkSize	= numVoxels * sizeof(MV_Voxel) + 0x434;	
			hFile.write((char*)&chunkSize, 4);

			hFile.write("SIZE", 4);
			chunkSize	= 12;
			hFile.write((char*)&chunkSize, 4);
			hFile.write("\0\0\0\0", 4);
			hFile.write((char*)&sizex, 4);
			hFile.write((char*)&sizey, 4);
			hFile.write((char*)&sizez, 4);

			//Voxele
			hFile.write("XYZI", 4);
			chunkSize	= 4 + (numVoxels * sizeof(MV_Voxel));
			hFile.write((char*)&chunkSize, 4);	
			hFile.write("\0\0\0\0", 4);
			hFile.write((char*)&numVoxels, 4);
			
			for(int i = 0; i < numVoxels; ++i) {
				hFile.write((char*)&voxels[i].x, 1);
				hFile.write((char*)&voxels[i].y, 1);
				hFile.write((char*)&voxels[i].z, 1);
				hFile.write((char*)&voxels[i].colorIndex, 1);
			}

			//Paleta
			hFile.write("RGBA", 4);
			chunkSize	= 0x400;
			hFile.write((char*)&chunkSize, 4);
			hFile.write("\0\0\0\0", 4);

			for(int i = 0; i < 256; ++i) {
				hFile.write((char*)&palette[i], 4);
			}

			hFile.flush();
			hFile.close();

			return true;
		}

	private:
		struct chunk_t {
			int			id;
			int			contentSize;
			int			childrenSize;
			long int	end;
		};

		void Free() {
			if(voxels != nullptr) {
				delete[]	voxels;
				voxels		= nullptr;
			}
			numVoxels	= 0;
			
			sizex = sizey = sizez = 0;
			
			isCustomPalette = false;

			version = 0;
		}
		
		bool ReadModelFile(FILE* fp) {
			const int MV_VERSION = 150;
			
			const int ID_VOX  = MV_ID('V', 'O', 'X', ' ');
			const int ID_MAIN = MV_ID('M', 'A', 'I', 'N');
			const int ID_SIZE = MV_ID('S', 'I', 'Z', 'E');
			const int ID_XYZI = MV_ID('X', 'Y', 'Z', 'I');
			const int ID_RGBA = MV_ID('R', 'G', 'B', 'A');
		   
			// magic number
			int magic = ReadInt(fp);
			if(magic != ID_VOX) {
				Error("magic number does not match");
				return false;
			}
			
			// version
			version = ReadInt(fp);
			if(version != MV_VERSION) {
				Error("version does not match");
				return false;
			}
			
			// main chunk
			chunk_t mainChunk;
			ReadChunk(fp, mainChunk);
			if(mainChunk.id != ID_MAIN) {
				Error("main chunk is not found");
				return false;
			}
			
			// skip content of main chunk
			fseek(fp, mainChunk.contentSize, SEEK_CUR);
			
			// read children chunks
			while(ftell(fp) < mainChunk.end) {
				// read chunk header
				chunk_t sub;
				ReadChunk(fp, sub);
				
				if(sub.id == ID_SIZE) {
					// size
					sizex = ReadInt(fp);
					sizey = ReadInt(fp);
					sizez = ReadInt(fp);
				}
				else if(sub.id == ID_XYZI) {
					// numVoxels
					numVoxels = ReadInt(fp);
					if(numVoxels < 0) {
						Error("negative number of voxels");
						return false;
					}
					
					// voxels
					if(numVoxels > 0) {
						voxels = new MV_Voxel[ numVoxels ];
						fread(voxels, sizeof(MV_Voxel), numVoxels, fp);
					}
				} else if(sub.id == ID_RGBA) {
					// last color is not used, so we only need to read 255 colors
					isCustomPalette = true;
					fread(palette + 1, sizeof(MV_RGBA), 255, fp);

					// NOTICE : skip the last reserved color
					MV_RGBA reserved;
					fread(&reserved, sizeof(MV_RGBA), 1, fp);
				}

				// skip unread bytes of current chunk or the whole unused chunk
				fseek(fp, sub.end, SEEK_SET);
			}
			
			return true;
		}
		
		void ReadChunk(FILE* fp, chunk_t& chunk) {
			// read chunk
			chunk.id			= ReadInt(fp);
			chunk.contentSize 	= ReadInt(fp);
			chunk.childrenSize	= ReadInt(fp);
			
			// end of chunk : used for skipping the whole chunk
			chunk.end = ftell(fp) + chunk.contentSize + chunk.childrenSize;
		}
		
		int ReadInt(FILE* fp) {
			int v = 0;
			fread(&v, 4, 1, fp);
			return v;
		}
		
		void Error(const char* info) const {
			printf("[Error] MV_VoxelModel :: %s\n", info);
		}
};

#endif // __MV_MODEL__
