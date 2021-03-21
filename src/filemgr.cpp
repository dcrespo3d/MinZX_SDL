#include "filemgr.h"
#include "minzx.h"

#define _CRT_SECURE_NO_WARNINGS 1

#include <stdio.h>

#define TRACE  printf
#define DEBUG  printf
#define LOG    printf
#define INFO   printf
#define NOTICE printf
#define WARN   printf
#define ERROR  printf
#define FATAL  printf

#define SNA_48K_SIZE 49179

static uint16_t fgetWordLE(FILE* pf) {
	uint8_t lo = fgetc(pf);
	uint8_t hi = fgetc(pf);
	return lo | (hi << 8);
}

bool FileMgr::loadSNA(const char* filename, MinZX* targetEmulator)
{
	if (NULL == filename || '\0' == *filename) {
		WARN("FileMgr::loadSNA: no filename specified\n");
		return false;
	}

	if (NULL == targetEmulator) {
		WARN("FileMgr::loadSNA: no target emulator\n");
		return false;
	}

	FILE* pf = fopen(filename, "rb");
	if (pf == NULL) {
		WARN("FileMgr::loadSNA: cannot open file %s\n", filename);
		return false;
	}

	fseek(pf, 0, SEEK_END);
	size_t filesize = ftell(pf);
	if (filesize != SNA_48K_SIZE) {
		WARN("FileMgr::loadSNA: bad size %u for file %s, should be %u", filesize, filename, SNA_48K_SIZE);
		return false;
	}
	fseek(pf, 0, SEEK_SET);

	Z80* z80 = targetEmulator->getCPU();
	uint8_t* mem = targetEmulator->getMemory();

	targetEmulator->reset();

	// Read in the registers
	z80->setRegI(fgetc(pf));
	z80->setRegLx(fgetc(pf));
	z80->setRegHx(fgetc(pf));
	z80->setRegEx(fgetc(pf));
	z80->setRegDx(fgetc(pf));
	z80->setRegCx(fgetc(pf));
	z80->setRegBx(fgetc(pf));
	z80->setRegFx(fgetc(pf));
	z80->setRegAx(fgetc(pf));

	//_zxCpu.i = lhandle.read();
	//_zxCpu.registers.byte[Z80_L] = lhandle.read();
	//_zxCpu.registers.byte[Z80_H] = lhandle.read();
	//_zxCpu.registers.byte[Z80_E] = lhandle.read();
	//_zxCpu.registers.byte[Z80_D] = lhandle.read();
	//_zxCpu.registers.byte[Z80_C] = lhandle.read();
	//_zxCpu.registers.byte[Z80_B] = lhandle.read();
	//_zxCpu.registers.byte[Z80_F] = lhandle.read();
	//_zxCpu.registers.byte[Z80_A] = lhandle.read();

	//_zxCpu.alternates[Z80_HL] = _zxCpu.registers.word[Z80_HL];
	//_zxCpu.alternates[Z80_DE] = _zxCpu.registers.word[Z80_DE];
	//_zxCpu.alternates[Z80_BC] = _zxCpu.registers.word[Z80_BC];
	//_zxCpu.alternates[Z80_AF] = _zxCpu.registers.word[Z80_AF];

	z80->setRegL(fgetc(pf));
	z80->setRegH(fgetc(pf));
	z80->setRegE(fgetc(pf));
	z80->setRegD(fgetc(pf));
	z80->setRegC(fgetc(pf));
	z80->setRegB(fgetc(pf));
	z80->setRegIY(fgetWordLE(pf));
	z80->setRegIX(fgetWordLE(pf));

	//_zxCpu.registers.byte[Z80_L] = lhandle.read();
	//_zxCpu.registers.byte[Z80_H] = lhandle.read();
	//_zxCpu.registers.byte[Z80_E] = lhandle.read();
	//_zxCpu.registers.byte[Z80_D] = lhandle.read();
	//_zxCpu.registers.byte[Z80_C] = lhandle.read();
	//_zxCpu.registers.byte[Z80_B] = lhandle.read();
	//_zxCpu.registers.byte[Z80_IYL] = lhandle.read();
	//_zxCpu.registers.byte[Z80_IYH] = lhandle.read();
	//_zxCpu.registers.byte[Z80_IXL] = lhandle.read();
	//_zxCpu.registers.byte[Z80_IXH] = lhandle.read();

	uint8_t inter = fgetc(pf);
	z80->setIFF2(inter & 0x04 ? 1 : 0);
	z80->setRegR(fgetc(pf));

	z80->setFlags(fgetc(pf));
	z80->setRegA(fgetc(pf));

	z80->setRegSP(fgetWordLE(pf));

	z80->setIM((Z80::IntMode)fgetc(pf));
	targetEmulator->setBorderColor(fgetc(pf));

	z80->setIFF1(z80->isIFF2());
	
	//byte inter = lhandle.read();
	//_zxCpu.iff2 = (inter & 0x04) ? 1 : 0;
	//_zxCpu.r = lhandle.read();

	//_zxCpu.registers.byte[Z80_F] = lhandle.read();
	//_zxCpu.registers.byte[Z80_A] = lhandle.read();

	//sp_l = lhandle.read();
	//sp_h = lhandle.read();
	//_zxCpu.registers.word[Z80_SP] = sp_l + sp_h * 0x100;

	//_zxCpu.im = lhandle.read();
	//byte bordercol = lhandle.read();

	//ESPectrum::borderColor = bordercol;

	//_zxCpu.iff1 = _zxCpu.iff2;

	uint16_t bytesToRead = 0xC000;
	uint16_t offset = 0x4000;
	while (bytesToRead--) {
		mem[offset++] = fgetc(pf);
	}
	

	uint16_t SP = z80->getRegSP();
	uint16_t retaddr = targetEmulator->peek16(SP);

	SP += 2;
	z80->setRegSP(SP);

	z80->setRegPC(retaddr);

	fclose(pf);

	//uint16_t buf_p = 0x4000;
	//while (lhandle.available()) {
	//	writebyte(buf_p, lhandle.read());
	//	buf_p++;
	//}

	//uint16_t thestack = _zxCpu.registers.word[Z80_SP];
	//retaddr = readword(thestack);
	//Serial.printf("%x\n", retaddr);
	//_zxCpu.registers.word[Z80_SP]++;
	//_zxCpu.registers.word[Z80_SP]++;

	//lhandle.close();

	//_zxCpu.pc = retaddr;

}