#include "minzx.h"
#include "z80.h"
#include <stdio.h>

#define TRACE  printf
#define DEBUG  printf
#define LOG    printf
#define INFO   printf
#define NOTICE printf
#define WARN   printf
#define ERROR  printf
#define FATAL  printf


static Z80* z80;

void MinZX::init()
{
	z80   = new Z80(this);
	mem   = new uint8_t[0x10000];
	ports = new uint8_t[0x10000];

	cycleTstates = 69888;
	loadROM();
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

#define WSCR 320
#define HSCR 240
#define WBMP 256
#define HBMP 192
#define WBOR 32
#define HBOR 24

// BGRA

void MinZX::generateScreen(uint8_t* screen)
{
	uint8_t* ptr = screen;

	for (int scry = 0; scry < HSCR; scry++)
	{
		if (scry < HBOR)
		{
			for (int scrx = 0; scrx < WSCR; scrx++)
			{
				*ptr++ = 255;
				*ptr++ = 255;
				*ptr++ = 255;
				*ptr++ = 255;
			}
		}
		else if (scry < HBOR + HBMP)
		{
			for (int scrx = 0; scrx < WSCR; scrx++)
			{
				if (scrx < WBOR)
				{
					*ptr++ = 255;
					*ptr++ = 255;
					*ptr++ = 255;
					*ptr++ = 255;
				}
				else if (scrx < WBOR + WBMP)
				{
					*ptr++ = 0;
					*ptr++ = 0;
					*ptr++ = 0;
					*ptr++ = 255;
				}
				else
				{
					*ptr++ = 255;
					*ptr++ = 255;
					*ptr++ = 255;
					*ptr++ = 255;
				}
			}
		}
		else
		{
			for (int scrx = 0; scrx < WSCR; scrx++)
			{
				*ptr++ = 255;
				*ptr++ = 255;
				*ptr++ = 255;
				*ptr++ = 255;
			}
		}
	}
}

/* Read opcode from RAM */
uint8_t MinZX::fetchOpcode(uint16_t address)
{
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
	this->tstates += tstates;
}

/* Clocks needed for processing INT and NMI */
void MinZX::interruptHandlingTime(int32_t wstates)
{
	this->tstates += tstates;
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
