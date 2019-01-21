//================================
//  MagicaVoxel [12/09/2013]
//  Copyright(c) 2013 @ ephtracy. All rights reserved.
//================================

//================================
// Notice that this code is neither robust nor complete.
// It is only a sample code demonstrating
//	 how to load current version .vox model from MagicaVoxel.
//================================

#ifndef __MV_MODEL__
#define __MV_MODEL__

#include "stdio.h"

// magic number
int MV_ID( int a, int b, int c, int d ) {
	return ( a ) | ( b << 8 ) | ( c << 16 ) | ( d << 24 );
}

//================
// RGBA
//================
class MV_RGBA {
public :
	unsigned char r, g, b, a;
};

//================
// Voxel
//================
class MV_Voxel {
public :
	unsigned char x, y, z, colorIndex;
};

//================
// Model
//================
class MV_Model {
public :
	// size
	int sizex, sizey, sizez;
	
	// voxels
	int numVoxels;
	MV_Voxel *voxels;

	// palette
	bool isCustomPalette;
	MV_RGBA palette[ 256 ];
	
	// version
	int version;
	
public :
	~MV_Model() {
		Free();
	}
	
	MV_Model() :
		sizex( 0 ),
		sizey( 0 ),
		sizez( 0 ),
		numVoxels( 0 ),
		voxels( NULL ),
		isCustomPalette( false ),
		version( 0 )
	{
	}
	
	void Free( void ) {
		if ( voxels ) {
			delete[] voxels;
			voxels = NULL;
		}
		numVoxels = 0;
		
		sizex = sizey = sizez = 0;
		
		isCustomPalette = false;

		version = 0;
	}
	
	bool LoadModel( const char *path ) {
		// free old data
		Free();
		
		// open file
		FILE *fp = fopen( path, "rb" );
		if ( !fp  ){
			Error( "failed to open file" );
			return false;
		}
		
		// read file
		bool success = ReadModelFile( fp );
		
		// close file
		fclose( fp );
		
		// if failed, free invalid data
		if ( !success ) {
			Free();
		}
		
		return success;
	}
	
private :
	struct chunk_t {
		int id;
		int contentSize;
		int childrenSize;
		long end;
	};
	
private :
	bool ReadModelFile( FILE *fp ) {
		const int MV_VERSION = 150;
		
		const int ID_VOX  = MV_ID( 'V', 'O', 'X', ' ' );
		const int ID_MAIN = MV_ID( 'M', 'A', 'I', 'N' );
		const int ID_SIZE = MV_ID( 'S', 'I', 'Z', 'E' );
		const int ID_XYZI = MV_ID( 'X', 'Y', 'Z', 'I' );
		const int ID_RGBA = MV_ID( 'R', 'G', 'B', 'A' );
	   
		// magic number
		int magic = ReadInt( fp );
		if ( magic != ID_VOX ) {
			Error( "magic number does not match" );
			return false;
		}
		
		// version
		version = ReadInt( fp );
		if ( version != MV_VERSION ) {
			Error( "version does not match" );
			return false;
		}
		
		// main chunk
		chunk_t mainChunk;
		ReadChunk( fp, mainChunk );
		if ( mainChunk.id != ID_MAIN ) {
			Error( "main chunk is not found" );
			return false;
		}
		
		// skip content of main chunk
		fseek( fp, mainChunk.contentSize, SEEK_CUR );
		
		// read children chunks
		while ( ftell( fp ) < mainChunk.end ) {
			// read chunk header
			chunk_t sub;
			ReadChunk( fp, sub );
			
			if ( sub.id == ID_SIZE ) {
				// size
				sizex = ReadInt( fp );
				sizey = ReadInt( fp );
				sizez = ReadInt( fp );
			}
			else if ( sub.id == ID_XYZI ) {
				// numVoxels
				numVoxels = ReadInt( fp );
				if ( numVoxels < 0 ) {
					Error( "negative number of voxels" );
					return false;
				}
				
				// voxels
				if ( numVoxels > 0 ) {
					voxels = new MV_Voxel[ numVoxels ];
					fread( voxels, sizeof( MV_Voxel ), numVoxels, fp );
				}
			}
			else if ( sub.id == ID_RGBA ) {
				// last color is not used, so we only need to read 255 colors
				isCustomPalette = true;
				fread( palette + 1, sizeof( MV_RGBA ), 255, fp );

				// NOTICE : skip the last reserved color
				MV_RGBA reserved;
				fread( &reserved, sizeof( MV_RGBA ), 1, fp );
			}

			// skip unread bytes of current chunk or the whole unused chunk
			fseek( fp, sub.end, SEEK_SET );
		}
		
		// print model info
		printf( "[Log] MV_VoxelModel :: Model : %d %d %d : %d\n",
			   sizex, sizey, sizez, numVoxels
			   );
		
		return true;
	}
	
	void ReadChunk( FILE *fp, chunk_t &chunk ) {
		// read chunk
		chunk.id = ReadInt( fp );
		chunk.contentSize  = ReadInt( fp );
		chunk.childrenSize = ReadInt( fp );
		
		// end of chunk : used for skipping the whole chunk
		chunk.end = ftell( fp ) + chunk.contentSize + chunk.childrenSize;
		
		// print chunk info
		const char *c = ( const char * )( &chunk.id );
		printf( "[Log] MV_VoxelModel :: Chunk : %c%c%c%c : %d %d\n",
			   c[0], c[1], c[2], c[3],
			   chunk.contentSize, chunk.childrenSize
			   );
	}
	
	int ReadInt( FILE *fp ) {
		int v = 0;
		fread( &v, 4, 1, fp );
		return v;
	}
	
	void Error( const char *info ) const {
		printf( "[Error] MV_VoxelModel :: %s\n", info );
	}
};

#endif // __MV_MODEL__
