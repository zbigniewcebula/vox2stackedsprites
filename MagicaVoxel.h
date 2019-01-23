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

		MV_RGBA() {
			r = 0; b = 0; g = 0; a = 0;
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
		
		int			numVoxels;
		MV_Voxel*	voxels;

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

		unsigned char ReadVoxel(int x, int y, int z) {
			for(int i = 0; i < numVoxels; ++i) {
				MV_Voxel&	v = voxels[i];
				if(v.x == x && v.y == y && v.z == z) {
					return v.colorIndex;
				}
			}
			return 0;
		}

		void WriteVoxel(int x, int y, int z, int i) {
			for(int i = 0; i < numVoxels; ++i) {
				MV_Voxel&	v = voxels[i];
				if(v.x == x && v.y == y && v.z == z) {
					v.colorIndex	= i;
					break;
				}
			}
		}

		void WritePalette(int index, uchar r, uchar g, uchar b, uchar a = 255) {
			if(index < 256) {
				palette[index].r	= r;
				palette[index].g	= g;
				palette[index].b	= b;
				palette[index].a	= a;
			}
		}
		MV_RGBA ReadPalette(int index) {
			if(index < 256) {
				return palette[index];
			}
			return palette[0];
		}
		MV_RGBA& AccessPalette(int index) {
			if(index < 256) {
				return palette[index];
			}
			return palette[0];
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
			chunkSize	= 4 + numVoxels * sizeof(MV_Voxel);
			hFile.write((char*)&chunkSize, 4);
			hFile.write("\0\0\0\0", 4);
			hFile.write((char*)&numVoxels, 4);

			for(uchar z = 0; z < sizez; ++z) {
				for(uchar y = 0; y < sizey; ++y) {
					for(uchar x = 0; x < sizex; ++x) {
						uchar v	= ReadVoxel(x, y, z);

						hFile.write((char*)&x, 1);
						hFile.write((char*)&y, 1);
						hFile.write((char*)&z, 1);
						hFile.write((char*)&v, 1);
					}
				}
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
