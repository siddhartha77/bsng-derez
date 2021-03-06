#include "BSNGDeRez.h"

#define CREATOR     'CWIE'
#define FILE_TYPE   'TEXT'
#define HEADER      "\p/* This file was generated by BSNG DeRez. */\r\r"
#define CICN_ID     "1000"

OSErr GenerateHeader(Str255 filename, FSSpecPtr myFSSPtr, Str255 inVersion) {
    Str255      buff;
    Str255      name;
    OSErr       err = noErr;
    short       refNum;
    long        count;
    
    myCopyPStr(myFSSPtr->name, name);
    myAppendPStr(name, "\p ");
    myAppendPStr(name, inVersion);
    
    /* Add ellipses if name too long */
    if (name[0] > 27) {
        name[0] = 27;
        name[27] = '?';
    }
    
    myCopyPStr(HEADER, buff);
    myAppendPStr(buff, "\p#define PROGRAM_NAME \"");   
    myAppendPStr(buff, name);
    myAppendPStr(buff, "\p\"");

    err = HCreate(
        myFSSPtr->vRefNum,
        myFSSPtr->parID,
        filename,
        CREATOR,
        FILE_TYPE
    );
    
    if (err)
        return err;
    
    err = HOpenDF(
        myFSSPtr->vRefNum,
        myFSSPtr->parID,
        filename,
        fsWrPerm,
        &refNum
    );
    
    if (err)
        return err;
        
    SetFPos(refNum, fsFromStart, 0);
    
    count = buff[0];

    err = FSWrite(
        refNum,
        &count,
        &buff[1]
    );

    FSClose(refNum);
    
    return err;
}


OSErr GenerateRez(  Str255 filename, Str255 headerFilename, 
                    FSSpecPtr myFSSPtr, CIconResource* inCicn) {
	Str255      buff;
	Str255      hexPStr;
    OSErr       err = noErr;
    short       refNum;
    long        count;
    long        i, length;
	UInt8*      cicnP;
	
	myCopyPStr(HEADER, buff);
    myAppendPStr(buff, "\p#include \"");  
    myAppendPStr(buff, headerFilename);
    myAppendPStr(buff, "\p\"\r\r");
    myAppendPStr(buff, "\pdata 'cicn' (" CICN_ID ") {\r");
    
    err = HCreate(
        myFSSPtr->vRefNum,
        myFSSPtr->parID,
        filename,
        CREATOR,
        FILE_TYPE
    );
    
    if (err)
        return err;
    
    err = HOpenDF(
        myFSSPtr->vRefNum,
        myFSSPtr->parID,
        filename,
        fsWrPerm,
        &refNum
    );
    
    if (err)
        return err;
        
    SetFPos(refNum, fsFromStart, 0);
    
    count = buff[0];

    err = FSWrite(
        refNum,
        &count,
        &buff[1]
    );
    	
	length = sizeof(CIconResource);
	cicnP = (UInt8*) inCicn;
	for (i = 0; i < length; i++) {
	    myCopyPStr("\p", buff);
	    
		if ((i % 16) == 0) {
			myAppendPStr(buff, "\p	$\"");
		} else if ((i % 2) == 0) {
			myAppendPStr(buff, "\p ");
		}
		
		myUNumToBaseBPStr((unsigned long)cicnP[i], hexPStr, 16, 2);
		myAppendPStr(buff, hexPStr);

		if ((i % 16) == 15) {
			myAppendPStr(buff, "\p\"\r");
		}
		
		count = buff[0];
		
		err = FSWrite(
            refNum,
            &count,
            &buff[1]
        );
	}
	
	myCopyPStr("\p", buff);
	
	if ((i % 16) != 0) {
		myAppendPStr(buff, "\p\"\r");
	}
	
	myAppendPStr(buff, "\p};\r\r");
	
	count = buff[0];
		
	err = FSWrite(
       refNum,
       &count,
       &buff[1]
   );
	
    FSClose(refNum);
    
    return err;
}

void GetVersion(StringPtr s) {
	VersRecHndl     versH;
	UInt8           major;
	UInt8           minor;
	UInt8           bug;
	Str255          buff;
	
	versH = (VersRecHndl)Get1Resource('vers', 2);
	if (versH == NULL)
		versH = (VersRecHndl)Get1Resource('vers', 1);
		
    if (versH == NULL) {
        s[0] = 0;
        return;
    }

	HLock((Handle)versH);
	
	major = (*versH)->numericVersion.majorRev;	
	minor = (*versH)->numericVersion.minorAndBugRev >> 4;
	bug = (*versH)->numericVersion.minorAndBugRev & 0xf;
	
	NumToString(major, buff);	
	myCopyPStr(buff, s);
	myAppendPStr(s, "\p.");
	NumToString(minor, buff);
	myAppendPStr(s, buff);
	myAppendPStr(s, "\p.");
	NumToString(bug, buff);
	myAppendPStr(s, buff);
	
	HUnlock((Handle)versH);
	ReleaseResource((Handle)versH);
}


CIconResource* MakeCICN(FSSpecPtr inSpec) {
	CIconResource* cicn = NULL;
	short          id;
	
	cicn = CreateCICN();
	
	FillCICNClut(cicn);
	
	id = FindIconID(GetFileType(inSpec));
	
	if (id != NULL) {
    	FillCICNPixMap(cicn, id);
    	FillCICNBitMap(cicn, id);
    } else {
        return NULL;
    }

	return cicn;
}


OSType GetFileType(FSSpecPtr inSpec) {
	FInfo finderInfo;
	
	FSpGetFInfo(inSpec, &finderInfo);
	return finderInfo.fdType;
}

short FindIconID(OSType inFileType) {
	Str255 name;
	OSType type;
	short  outID = -1;
	long   i, count;
	Handle frefH;
	
	/* Iterate FREF resources until we find one whose type (1st 4 bytes) == the file type. */
	count = Count1Resources('FREF');
	for (i = 1; i <= count; i++) {
		frefH = Get1IndResource('FREF', i);
		
		if (frefH == NULL) {
    	    ReleaseResource(frefH);
    	    return NULL;
    	}
		
		if (**(OSType**) frefH == inFileType) {
			GetResInfo(frefH, &outID, &type, name);
		}
		ReleaseResource(frefH);
	}
	
	return outID;
}

void FillCICNClut(CIconResource* ioCicn) {
	CTabHandle  clutH;
	ColorTable* clutP;
	long        i;
	
	/* CLUT 8 is the standard 8-bit color system color table and is always present */
	clutH = GetCTable(8);

	/* Copy the data into the cicn's color table. */
	HLock((Handle) clutH);
	clutP = *clutH;
	
	ioCicn->theCTable.ctSeed  = 0;
	ioCicn->theCTable.ctFlags = 0;
	ioCicn->theCTable.ctSize  = clutP->ctSize;
	
	for (i = 0; i <= clutP->ctSize; i++) {
        ioCicn->theCTable.ctTable[i].value = i;
        ioCicn->theCTable.ctTable[i].rgb   = clutP->ctTable[i].rgb;
	}
	
	HUnlock((Handle) clutH);
}


long FillCICNPixMap(CIconResource* ioCICN, short inResID) {
    UInt8*  icl8P;
    Handle  icl8H;
    long    size = 0;
    long    i;
	  
	/* Load the 'icl8' resource specified by inResID and paste it into this cicn's
	   pixmap data. */
	
	icl8H = Get1Resource('icl8', inResID);	
	size = GetMaxResourceSize(icl8H);
	
	if (size != 1024) {
	    ReleaseResource(icl8H);
	    return 0;
	}
	
	HLock(icl8H);
	icl8P = (UInt8*) *icl8H;
	for (i = 0; i < size; i++) {
		ioCICN->thePixels[i] = icl8P[i];
	}
	
	HUnlock(icl8H);
	ReleaseResource(icl8H);
	
	return size;
}

long FillCICNBitMap(CIconResource* ioCICN, short inResID) {
    UInt32* iconP;
    Handle  iconH;
    long    size = 0;
    long    i;
    
	/* Load the ICN# resource specified by inResID and paste it into this cicn's
	   pixmap data. */
	
	iconH = Get1Resource('ICN#', inResID);	
	size = GetMaxResourceSize(iconH);
	
	if (size != 256) {
	    ReleaseResource(iconH);
	    return 0;
	}
	
	HLock(iconH);

	iconP = (UInt32*) *iconH;
	for (i = 0; i < 32; i++) {
		ioCICN->theBitMap[i] = iconP[i];
	}
	
	iconP += 32;
	for (i = 0; i < 32; i++) {
		ioCICN->theMask[i] = iconP[i];
	}	
	
	HUnlock(iconH);
	ReleaseResource(iconH);
	
	return size;
}

CIconResource* CreateCICN(void) {
	Rect           theRect;
	int            theImageRowBytes, theBitsPixel;
	CIconResource* outCICN;
	PixMap*        pm;
	BitMap*        bm;
	
	SetRect(&theRect, 0, 0, 32, 32);
	
	/* Use an 8-bit pixel map. */
	theBitsPixel     = 8;
	theImageRowBytes = 0x8020;
	
	outCICN = (CIconResource*) NewPtr(sizeof(CIconResource));
	
	/* Initialize the headers. */
	pm = &outCICN->cicn.iconPMap;
	pm->baseAddr   = 0;
	pm->rowBytes   = theImageRowBytes;
	pm->bounds     = theRect;
	pm->pmVersion  = 0;
	pm->packType   = 0;
	pm->packSize   = 0;
	pm->hRes       = 0x00480000; // 72 dpi in Fixed format
	pm->vRes       = 0x00480000;
	pm->pixelType  = 0;
	pm->pixelSize  = theBitsPixel;
	pm->cmpCount   = 1;
	pm->cmpSize    = theBitsPixel;
	pm->planeBytes = 0;
	pm->pmTable    = 0;
	pm->pmReserved = 0;

    bm = &outCICN->cicn.iconMask;
    bm->baseAddr   = 0;
    bm->rowBytes   = 4;
    bm->bounds     = theRect;

    bm = &outCICN->cicn.iconBMap;
    bm->baseAddr   = 0;
    bm->rowBytes   = 4;
    bm->bounds     = theRect;

    outCICN->cicn.iconData = 0;

	return outCICN;
}