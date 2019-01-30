#ifndef __VOX_MODEL__
#define __VOX_MODEL__

#include <string>
#include <cstring>
#include <fstream>
#include <vector>

using namespace std;
typedef uint8_t		uchar;
typedef uint16_t	ushort;
typedef uint32_t	uint;

class vec4 {
	public:
		union {
			struct {
				uchar r;
				uchar g;
				uchar b;
				uchar a;
			};
			struct {
				uchar x;
				uchar y;
				uchar z;
				uchar w;
			};
			uchar	raw[4];
			uint	rawInt;
		};

		vec4(uchar R = 0, uchar G = 0, uchar B = 0, uchar A = 0)
			:	r(R), g(G), b(B), a(A)
		{}
		vec4(const vec4& org)
			:	r(org.r), g(org.g), b(org.b), a(org.a)
		{}
		vec4(uint value)
			:	rawInt(value)
		{}

		//Methods
		void Set(uchar R, uchar G, uchar B, uchar A) {
			r	= R;
			G	= G;
			B	= B;
			A	= A;
		}
		void Set(uchar R, uchar G, uchar B) {
			r	= R;
			G	= G;
			B	= B;
		}

		//Assigment
		vec4& operator=(const vec4& org) {
			rawInt	= org.rawInt;
			return *this;
		}
		vec4& operator+=(vec4& org) {
			rawInt			= ((*this) + org).rawInt; 
			return *this;
		}
		vec4& operator-=(vec4& org) {
			rawInt			= ((*this) - org).rawInt; 
			return *this;
		}

		//Grouping
		vector<vec4> operator,(vec4& neigh) {
			vector<vec4>	lst;
			lst.emplace_back(rawInt);
			lst.emplace_back(neigh.rawInt);
			return lst;
		}
		vector<vec4> operator,(vector<vec4>& neighList) {
			vector<vec4>	lst;
			lst.reserve(1 + neighList.size());
			lst.emplace_back(rawInt);
			lst.insert(lst.begin() + 1, neighList.begin(), neighList.end());
			return lst;
		}

		//Comparation
		inline bool operator==(vec4& cmp) const {
			return rawInt == cmp.rawInt;
		}
		inline bool operator!=(vec4& cmp) const {
			return rawInt not_eq cmp.rawInt;
		}

		//Effects
		inline vec4 operator ~() const {
			return vec4(compl rawInt);
		}

		//Artmetic
		vec4 operator*(vec4& org) const {
			float	compounds[8] = {
				r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f,
				org.r / 255.0f, org.g / 255.0f, org.b / 255.0f, org.a / 255.0f
			};
			return vec4(
				(compounds[0] * compounds[4]) * 255.0f,
				(compounds[1] * compounds[5]) * 255.0f,
				(compounds[2] * compounds[6]) * 255.0f,
				(compounds[3] * compounds[7]) * 255.0f
			);
		}
		vec4 operator+(vec4& org) const {
			return vec4(
				vec4::Clamp(r + org.r), vec4::Clamp(g + org.g), vec4::Clamp(b + org.b), vec4::Clamp(a + org.a)
			);
		}
		vec4 operator-(vec4& org) const {
			return vec4(
				vec4::Clamp(r - org.r), vec4::Clamp(g - org.g), vec4::Clamp(b - org.b), vec4::Clamp(a - org.a)
			);
		}

	private:
		static uchar Clamp(int val) {
			return val < 0? 0: (val > 255? 255: val);
		}
};

//Additional operators for vector of vec4's
vector<vec4> operator,(vector<vec4>& a, vector<vec4>& b) {
	vector<vec4>	lst;
	lst.reserve(a.size() + b.size());
	lst.insert(lst.end(), a.begin(), a.end());
	lst.insert(lst.end(), b.begin(), b.end());
	return lst;
}
vector<vec4> operator,(vector<vec4>& a, vec4& add) {
	vector<vec4>	lst(a);
	lst.emplace_back(add.rawInt);
	return lst;
}
inline vector<vec4> operator+(vector<vec4>& a, vector<vec4>& b) {
	return (a, b);
}
inline vector<vec4> operator+(vector<vec4>& a, vec4& add) {
	return (a, add);
}

class VOXModel {
	private:
		//Static
		static constexpr const int MV_VERSION	= 150;
		static constexpr const int ID_VOX		= 0x20584F56;	//VOX 

		//
		vec4		size;
		
		int			numVoxels	= 0;
		
		vec4		palette[256];
		vec4*		voxels		= nullptr;
		
		int			version		= 0;
	public:
		~VOXModel() {
			FreeMemory();
		}
		
		VOXModel() : 
			size(0, 0, 0), numVoxels(0), voxels(nullptr), version(0)
		{}
		
		inline bool Load(string path) {
			return Load(path.c_str());
		}
		bool Load(const char* path) {
			//Free old data
			FreeMemory();
			
			//Open file
			ifstream	hFile(path, ios::in bitor ios::binary);
			
			if(hFile.fail()) {
				cerr	<< "[VOX] Failed to open file!" << endl;
				return false;
			}
			bool success = ReadModelFile(hFile);
			
			hFile.close();
			
			if(not success) {
				FreeMemory();
			}
			
			return success;		
		}

		constexpr bool IsEmpty() const {
			return size.x == 0 and size.y == 0 and size.z == 0;
		}

		void SetSize(int x, int y, int z) {
			size.Set(x, y, z);
			numVoxels	= 0;
			version		= MV_VERSION;

			if(voxels not_eq nullptr) {
				delete[]	voxels;
			}
			voxels	= new vec4[size.x * size.y * size.z];
		}

		uchar VoxelColorID(int x, int y, int z) const {
			for(int i = 0; i < numVoxels; ++i) {
				vec4&	v = voxels[i];
				if(v.x == x and v.y == y and v.z == z) {
					return v.w;
				}
			}
			return 0;
		}

		void SetVoxel(int x, int y, int z, int idx) {
			if(x > -1 and x < size.x
			and y > -1 and y < size.y
			and z > -1 and z < size.z
			) {
				if(numVoxels > 0) {
					for(int i = 0; i < numVoxels; ++i) {
						vec4&	v = voxels[i];
						if(v.x == x and v.y == y and v.z == z) {
							v.w	= i;
							return;
						}
					}
				}
				voxels[numVoxels].Set(x, y, z, idx);
				++numVoxels;
			}
		}

		void SetPaletteColor(int index, uchar r, uchar g, uchar b, uchar a = 255) {
			if(index > -1 and index < 256) {
				palette[index].Set(r, g, b, a);
			}
		}
		void SetPaletteColor(int index, const vec4& clr) {
			if(index > -1 and index < 256) {
				palette[index]	= clr;
			}
		}
		vec4& PaletteColor(int index) {
			if(index >= 256 or index < 0) {
				index	= 0;
			}
			return palette[index];
		}

		ushort FindPaletteColorIndex(uchar r, uchar g, uchar b, uchar a) const {
			for(ushort i = 0; i < 256; ++i) {
				if(palette[i].r == r
				and	palette[i].g == g
				and	palette[i].b == b
				and	palette[i].a == a
				) {
					 return i;
				}
			}
			return 0xFFFF;
		}
		inline ushort FindPaletteColorIndex(const vec4& clr) {
			return FindPaletteColorIndex(clr.r, clr.g, clr.b, clr.a);
		}

		inline bool Save(string path) {
			return Save(path.c_str());
		}
		bool Save(const char* name = "") {
			//File
			ofstream	hFile(name, ios::trunc bitor ios::out bitor ios::binary);

			if(hFile.fail()) {
				hFile.close();
				return false;
			}

			//Temporary
			int	chunkSize	= 0;

			//Headers
			hFile.write("VOX ", 4);
			hFile.write(reinterpret_cast<char*>(&version), 4);
			hFile.write("MAIN", 4);
			hFile.write("\0\0\0\0", 4);
			chunkSize	= numVoxels * sizeof(vec4) + 0x434;	
			hFile.write(reinterpret_cast<char*>(&chunkSize), 4);

			hFile.write("SIZE", 4);
			chunkSize	= 12;
			hFile.write(reinterpret_cast<char*>(&chunkSize), 4);
			hFile.write("\0\0\0\0", 4);
			hFile.write(reinterpret_cast<char*>(size.raw), 12);

			//Voxels
			hFile.write("XYZI", 4);
			chunkSize	= 4 + numVoxels * sizeof(vec4);
			hFile.write(reinterpret_cast<char*>(&chunkSize), 4);	
			hFile.write("\0\0\0\0", 4);
			hFile.write(reinterpret_cast<char*>(&numVoxels), 4);
			
			for(int i = 0; i < numVoxels; ++i) {
				hFile.write(reinterpret_cast<char*>(voxels[i].raw), 4);
			}

			//Palette
			hFile.write("RGBA", 4);
			chunkSize	= 0x400;
			hFile.write(reinterpret_cast<char*>(&chunkSize), 4);
			hFile.write("\0\0\0\0", 4);

			for(int i = 0; i < 256; ++i) {
				hFile.write(reinterpret_cast<char*>(palette[i].raw), 4);
			}

			hFile.flush();
			hFile.close();

			return true;
		}

		constexpr int SizeX() const {
			return size.x;
		}
		constexpr int SizeY() const {
			return size.y;
		}
		constexpr int SizeZ() const {
			return size.z;
		}
		constexpr int Version() const {
			return version;
		}
		constexpr int VoxelCount() const {
			return numVoxels;
		}

	private:
		class Chunk {
			public:
				enum Type : int {
					MAIN	= 0x4E49414D,
					SIZE	= 0x455A4953,
					XYZI	= 0x495A5958,
					RGBA	= 0x41424752,

					PACK	= 0x4B434150,
					MATT	= 0x5454414D,
					MATL	= 0x4C54414D,
					DICT	= 0x54434944,
					nTRN	= 0x4E52546E,
					nGRP	= 0x5052476E,
					nSHP	= 0x5048536E,
					LAYR	= 0x5259414C,

					rOBJ	= 0x4A424F72
				};

				Type		id;
				int			contentSize;
				int			childrenSize;
				long int	end;

				void ReadFromFile(ifstream& hFile) {
					hFile.read(reinterpret_cast<char*>(this), 12);

					end	= static_cast<int>(hFile.tellg()) + contentSize + childrenSize;
				}
		};

		void FreeMemory() {
			if(voxels != nullptr) {
				delete[]	voxels;
				voxels		= nullptr;
			}
			numVoxels	= 0;
			version		= 0;
			
			size.x = size.y = size.z = 0;
		}
		
		bool ReadModelFile(ifstream& hFile) {
			//Magic number
			int magic;
			hFile.read(reinterpret_cast<char*>(&magic), 4);
			if(magic not_eq this->ID_VOX) {
				cerr	<< "[VOX] Magic number does not match proper one!" << endl;
				return false;
			}
			
			//Version
			hFile.read(reinterpret_cast<char*>(&version), 4);
			if(version not_eq MV_VERSION) {
				cerr	<< "[VOX] Supported version does not match!" << endl;
				return false;
			}
			
			//Main chunk
			Chunk	mainChunk;
			mainChunk.ReadFromFile(hFile);
			if(mainChunk.id not_eq Chunk::Type::MAIN) {
				cerr	<< "[VOX] Main chunk does not exists! Broken file." << endl;
				return false;
			}
			
			//Skip content of main chunk
			if(mainChunk.contentSize > 0) {
				hFile.seekg(static_cast<int>(hFile.tellg()) + mainChunk.contentSize);
			}
			
			bool	customPalette	= false;

			//Read children chunks
			while(hFile.tellg() < mainChunk.end) {
				Chunk childrenChunk;
				childrenChunk.ReadFromFile(hFile);
				
				switch(childrenChunk.id) {
					case(Chunk::Type::SIZE): {
						hFile.read(reinterpret_cast<char*>(&size.raw), 12);
						break;
					}
					case(Chunk::Type::XYZI): {
						hFile.read(reinterpret_cast<char*>(&numVoxels), 4);
						if(numVoxels < 0) {
							cerr	<< "[VOX] Negative voxel number, file broken!" << endl;
							return false;
						}
						if(numVoxels > 0) {
							voxels	= new vec4[numVoxels];
							hFile.read(reinterpret_cast<char*>(voxels), sizeof(vec4) * numVoxels);
						}
						break;
					}
					case(Chunk::Type::RGBA): {
						//Clean old palette
						memset(reinterpret_cast<void*>(palette), 0, sizeof(vec4) * 255);

						//Last color is not used, so we only need to read 255 colors
						hFile.read(reinterpret_cast<char*>(palette + 1), sizeof(vec4) * 255);
						hFile.seekg(static_cast<int>(hFile.tellg()) + sizeof(vec4));

						customPalette	= true;
					}
					case(Chunk::Type::PACK): {
						cout	<< "[VOX] Multiple models are not supported, ignoring rest of model!" << endl;
						break;
					}
					case(Chunk::Type::MATT):
					case(Chunk::Type::MATL):
					case(Chunk::Type::DICT):
					case(Chunk::Type::nTRN):
					case(Chunk::Type::nGRP):
					case(Chunk::Type::nSHP):
					case(Chunk::Type::LAYR): {
						//Unsupported header, silent ignore
						break;
					}
					case(Chunk::Type::rOBJ): {
						//Undocumented header, silent ignore
						break;
					}
					default: {
						cerr	<< "[VOX] Unknown header (at 0x" << hex << hFile.tellg() << dec << "), ignoring!"
								<< endl;
						break;
					}
				}
				hFile.seekg(childrenChunk.end);
			}
			if(not customPalette) {
				SetDefaultPalette();
			}
			
			return true;
		}

		constexpr void SetDefaultPalette() {
			memcpy(reinterpret_cast<void*>(palette), defaultPalette, sizeof(vec4) * 255);
		}
		constexpr static const uint defaultPalette[256] = {
			//0 => Unused color
			0x00000000, 0xffffffff, 0xffccffff, 0xff99ffff, 0xff66ffff, 0xff33ffff, 0xff00ffff, 0xffffccff,
			0xffccccff, 0xff99ccff, 0xff66ccff, 0xff33ccff, 0xff00ccff, 0xffff99ff, 0xffcc99ff, 0xff9999ff,
			0xff6699ff, 0xff3399ff, 0xff0099ff, 0xffff66ff, 0xffcc66ff, 0xff9966ff, 0xff6666ff, 0xff3366ff,
			0xff0066ff, 0xffff33ff, 0xffcc33ff, 0xff9933ff, 0xff6633ff, 0xff3333ff, 0xff0033ff, 0xffff00ff,
			0xffcc00ff, 0xff9900ff, 0xff6600ff, 0xff3300ff, 0xff0000ff, 0xffffffcc, 0xffccffcc, 0xff99ffcc,
			0xff66ffcc, 0xff33ffcc, 0xff00ffcc, 0xffffcccc, 0xffcccccc, 0xff99cccc, 0xff66cccc, 0xff33cccc,
			0xff00cccc, 0xffff99cc, 0xffcc99cc, 0xff9999cc, 0xff6699cc, 0xff3399cc, 0xff0099cc, 0xffff66cc,
			0xffcc66cc, 0xff9966cc, 0xff6666cc, 0xff3366cc, 0xff0066cc, 0xffff33cc, 0xffcc33cc, 0xff9933cc,
			0xff6633cc, 0xff3333cc, 0xff0033cc, 0xffff00cc, 0xffcc00cc, 0xff9900cc, 0xff6600cc, 0xff3300cc,
			0xff0000cc, 0xffffff99, 0xffccff99, 0xff99ff99, 0xff66ff99, 0xff33ff99, 0xff00ff99, 0xffffcc99,
			0xffcccc99, 0xff99cc99, 0xff66cc99, 0xff33cc99, 0xff00cc99, 0xffff9999, 0xffcc9999, 0xff999999,
			0xff669999, 0xff339999, 0xff009999, 0xffff6699, 0xffcc6699, 0xff996699, 0xff666699, 0xff336699,
			0xff006699, 0xffff3399, 0xffcc3399, 0xff993399, 0xff663399, 0xff333399, 0xff003399, 0xffff0099,
			0xffcc0099, 0xff990099, 0xff660099, 0xff330099, 0xff000099, 0xffffff66, 0xffccff66, 0xff99ff66,
			0xff66ff66, 0xff33ff66, 0xff00ff66, 0xffffcc66, 0xffcccc66, 0xff99cc66, 0xff66cc66, 0xff33cc66,
			0xff00cc66, 0xffff9966, 0xffcc9966, 0xff999966, 0xff669966, 0xff339966, 0xff009966, 0xffff6666,
			0xffcc6666, 0xff996666, 0xff666666, 0xff336666, 0xff006666, 0xffff3366, 0xffcc3366, 0xff993366,
			0xff663366, 0xff333366, 0xff003366, 0xffff0066, 0xffcc0066, 0xff990066, 0xff660066, 0xff330066,
			0xff000066, 0xffffff33, 0xffccff33, 0xff99ff33, 0xff66ff33, 0xff33ff33, 0xff00ff33, 0xffffcc33,
			0xffcccc33, 0xff99cc33, 0xff66cc33, 0xff33cc33, 0xff00cc33, 0xffff9933, 0xffcc9933, 0xff999933,
			0xff669933, 0xff339933, 0xff009933, 0xffff6633, 0xffcc6633, 0xff996633, 0xff666633, 0xff336633,
			0xff006633, 0xffff3333, 0xffcc3333, 0xff993333, 0xff663333, 0xff333333, 0xff003333, 0xffff0033,
			0xffcc0033, 0xff990033, 0xff660033, 0xff330033, 0xff000033, 0xffffff00, 0xffccff00, 0xff99ff00,
			0xff66ff00, 0xff33ff00, 0xff00ff00, 0xffffcc00, 0xffcccc00, 0xff99cc00, 0xff66cc00, 0xff33cc00,
			0xff00cc00, 0xffff9900, 0xffcc9900, 0xff999900, 0xff669900, 0xff339900, 0xff009900, 0xffff6600,
			0xffcc6600, 0xff996600, 0xff666600, 0xff336600, 0xff006600, 0xffff3300, 0xffcc3300, 0xff993300,
			0xff663300, 0xff333300, 0xff003300, 0xffff0000, 0xffcc0000, 0xff990000, 0xff660000, 0xff330000,
			0xff0000ee, 0xff0000dd, 0xff0000bb, 0xff0000aa, 0xff000088, 0xff000077, 0xff000055, 0xff000044,
			0xff000022, 0xff000011, 0xff00ee00, 0xff00dd00, 0xff00bb00, 0xff00aa00, 0xff008800, 0xff007700,
			0xff005500, 0xff004400, 0xff002200, 0xff001100, 0xffee0000, 0xffdd0000, 0xffbb0000, 0xffaa0000,
			0xff880000, 0xff770000, 0xff550000, 0xff440000, 0xff220000, 0xff110000, 0xffeeeeee, 0xffdddddd,
			0xffbbbbbb, 0xffaaaaaa, 0xff888888, 0xff777777, 0xff555555, 0xff444444, 0xff222222, 0xff111111,
		};
};

#endif
