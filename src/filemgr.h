#ifndef _FILEMGR_H_
#define _FILEMGR_H_

class MinZX;

class FileMgr
{
public:
	bool loadSNA(const char* filename, MinZX* targetEmulator);
};

#endif