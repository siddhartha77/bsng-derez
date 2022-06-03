#include <Files.h>

#include "utils.h"

typedef struct CIconHeader {
	PixMap	        iconPMap;
	BitMap	        iconMask;
	BitMap	        iconBMap;
	Handle	        iconData;
} CIconHeader;

typedef struct ColorTable256 {
	long            ctSeed;
	short 	        ctFlags;
	short 		    ctSize;
	ColorSpec       ctTable[256];
} ColorTable256;

typedef struct CIconResource {
	CIconHeader     cicn;
	UInt32          theMask[32];
	UInt32          theBitMap[32];
	ColorTable256   theCTable;
	UInt8           thePixels[32 * 32];
} CIconResource;

void GetVersion(StringPtr outString);
OSErr GenerateHeader(Str255 filename, FSSpecPtr myFSSPtr, Str255 inVersion);
OSErr GenerateRez(Str255 filename, Str255 headerFilename, 
                    FSSpecPtr myFSSPtr, CIconResource* inCicn);
CIconResource* MakeCICN(FSSpecPtr inSpec);

CIconResource* CreateCICN(void);
void FillCICNClut(CIconResource* ioCicn);
OSType GetFileType(FSSpecPtr inSpec);
short FindIconID(OSType inFileType);
long FillCICNBitMap(CIconResource* ioCICN, short inResID);
long FillCICNPixMap(CIconResource* ioCICN, short inResID);