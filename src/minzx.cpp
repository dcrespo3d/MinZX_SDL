#include "minzx.h"
#include "z80.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#define TRACE  printf
#define DEBUG  printf
#define LOG    printf
#define INFO   printf
#define NOTICE printf
#define WARN   printf
#define ERROR  printf
#define FATAL  printf


#define VAL_BRIGHT 255
#define VAL_NO_BRIGHT 176

uint32_t speColors[16];

static void createSpectrumColors()
{
	uint32_t A = 0xFF000000;
	uint32_t r = VAL_NO_BRIGHT << 16;
	uint32_t g = VAL_NO_BRIGHT << 8;
	uint32_t b = VAL_NO_BRIGHT;
	uint32_t R = VAL_BRIGHT << 16;
	uint32_t G = VAL_BRIGHT << 8;
	uint32_t B = VAL_BRIGHT;
	speColors[ 0] = A            ;
	speColors[ 1] = A |         b;
	speColors[ 2] = A | r        ;
	speColors[ 3] = A | r |     b;
	speColors[ 4] = A |     g    ;
	speColors[ 5] = A |     g | b;
	speColors[ 6] = A | r | g    ;
	speColors[ 7] = A | r | g | b;
	speColors[ 8] = A;
	speColors[ 9] = A |         B;
	speColors[10] = A | R        ;
	speColors[11] = A | R |     B;
	speColors[12] = A |     G    ;
	speColors[13] = A |     G | B;
	speColors[14] = A | R | G    ;
	speColors[15] = A | R | G | B;
}

static uint32_t zxColor(int c, bool b)
{
	c = c & 7;
	if (b) c += 8;
	return speColors[c];
}


void MinZX::init()
{
	z80   = new Z80(this);
	mem   = new uint8_t[0x10000];
	ports = new uint8_t[0x10000];

	memset(mem, 0x00, 0x10000);
	memset(ports, 0xFF, 0x10000);

	cycleTstates = 69888;
	loadROM();

	//loadDump();
	createSpectrumColors();
}

void MinZX::update(uint8_t* screen)
{
	tstates = 0;
	uint32_t prevTstates = 0;

	while (tstates < cycleTstates)
	{
		z80->execute();
		prevTstates = tstates;
	}

	generateScreen(screen);

	tstates -= cycleTstates;
}

void MinZX::destroy()
{
	delete z80;
	delete[] mem;
}

void MinZX::loadROM()
{
	FILE* pf = fopen("zx48.rom", "rb");
	if (pf == NULL) {
		ERROR("Cannot load ROM from file\n");
		return;
	}

	size_t elemsRead = fread(mem, 0x4000, 1, pf);
}

void MinZX::loadDump()
{
	FILE* pf = fopen("mm_mem_dump", "rb");
	if (pf == NULL) {
		ERROR("Cannot load dump from file\n");
		return;
	}

	size_t elemsRead = fread(mem, 0x10000, 1, pf);
}

#define WSCR 320
#define HSCR 240
#define WBMP 256
#define HBMP 192
#define WBOR 32
#define HBOR 24

// BGRA

#define ULA_SWAP(y) ((y & 0xC0) | ((y & 0x38) >> 3) | ((y & 0x07) << 3))

void MinZX::generateScreen(uint8_t* screen)
{
	uint8_t* bptr = screen;
	uint32_t* dptr = (uint32_t*)bptr;

	//               AARRGGBB
	uint32_t blk = 0xFF000000;
	uint32_t whi = 0xFFFFFFFF;

	uint32_t borderColor = zxColor(7, 0);

	for (int scrY = 0; scrY < HSCR; scrY++)
	{
		if (scrY < HBOR)
		{
			for (int scrX = 0; scrX < WSCR; scrX++)
			{
				*dptr++ = borderColor;
			}
		}
		else if (scrY < HBOR + HBMP)
		{
			int speY = scrY - HBOR;
			int ulaY = ULA_SWAP(speY);

			int bmpoff = 0x4000 + ( ulaY       << 5);
			int attoff = 0x5800 + ((speY >> 3) << 5);
			uint8_t* bmpptr = mem + bmpoff;
			uint8_t* attptr = mem + attoff;

			for (int scrX = 0; scrX < WSCR; scrX++)
			{
				if (scrX < WBOR)
				{
					*dptr++ = borderColor;
				}
				else if (scrX < WBOR + WBMP)
				{
					int speX = scrX - WBOR;

					uint8_t bmp = *bmpptr++;
					uint8_t att = *attptr++;

					int ink = (att)      & 7;
					int pap = (att >> 3) & 7;
					int bri = (att >> 6) & 1;
					int fla = (att >> 7);
					uint32_t fore = zxColor(ink, bri);
					uint32_t back = zxColor(pap, bri);

					// B & W
					// uint32_t fore = blk;
					// uint32_t back = whi;

					for (int mask = 0x80; mask > 0; mask >>= 1)
					{
						if (mask & bmp)
							*dptr++ = fore;
						else
							*dptr++ = back;
					}

					scrX += 7;
				}
				else
				{
					*dptr++ = borderColor;
				}
			}
		}
		else
		{
			for (int scrX = 0; scrX < WSCR; scrX++)
			{
				*dptr++ = borderColor;
			}
		}
	}
}

/* Read opcode from RAM */
uint8_t MinZX::fetchOpcode(uint16_t address)
{
	//LOG("tstate: %5d - fetch: 0x%04X - %u\n", tstates, address, address);
	if (address == 4757) {
		address = address;
	}
	// 3 clocks to fetch opcode from RAM and 1 execution clock
	tstates += 4;

	return mem[address];
}

/* Read/Write byte from/to RAM */
uint8_t MinZX::peek8(uint16_t address)
{
	// 3 clocks for read byte from RAM
	tstates += 3;
	return mem[address];
}

void MinZX::poke8(uint16_t address, uint8_t value)
{
	// 3 clocks for write byte to RAM
	tstates += 3;
	mem[address] = value;
}

/* Read/Write word from/to RAM */
uint16_t MinZX::peek16(uint16_t address)
{
	// Order matters, first read lsb, then read msb, don't "optimize"
	uint8_t lsb = peek8(address);
	uint8_t msb = peek8(address + 1);
	return (msb << 8) | lsb;
}

void MinZX::poke16(uint16_t address, RegisterPair word)
{
	// Order matters, first write lsb, then write msb, don't "optimize"
	poke8(address, word.byte8.lo);
	poke8(address + 1, word.byte8.hi);
}

/* In/Out byte from/to IO Bus */
uint8_t MinZX::inPort(uint16_t port)
{
	// 4 clocks for read byte from bus
	tstates += 3;
	return ports[port];
}

void MinZX::outPort(uint16_t port, uint8_t value)
{
	// 4 clocks for write byte to bus
	tstates += 4;
	ports[port] = value;
}

/* Put an address on bus lasting 'tstates' cycles */
void MinZX::addressOnBus(uint16_t address, int32_t wstates)
{
	// Additional clocks to be added on some instructions
	tstates += wstates;
}

/* Clocks needed for processing INT and NMI */
void MinZX::interruptHandlingTime(int32_t wstates)
{
	tstates += wstates;
}

/* Callback to know when the INT signal is active */
bool MinZX::isActiveINT(void)
{
	// Put here the needed logic to trigger an INT
	return false;
}

#ifdef WITH_BREAKPOINT_SUPPORT
/* Callback for notify at PC address */
uint8_t MinZX::breakpoint(uint16_t address, uint8_t opcode)
{
}

#endif

#ifdef WITH_EXEC_DONE
/* Callback to notify that one instruction has ended */
void MinZX::execDone(void)
{
}
#endif
