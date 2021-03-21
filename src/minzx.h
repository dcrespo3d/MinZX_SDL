#ifndef _MINZX_H_
#define _MINZX_H_

#include <inttypes.h>
#include "z80.h"

class MinZX : public Z80operations
{
public:
	void init();

	void update(uint8_t* screen);

	void destroy();

protected:
	virtual uint8_t  fetchOpcode (uint16_t address);
	virtual uint8_t  peek8       (uint16_t address);
	virtual void     poke8       (uint16_t address, uint8_t value);
	virtual uint16_t peek16      (uint16_t address);
	virtual void     poke16      (uint16_t address, RegisterPair word);
	virtual uint8_t  inPort      (uint16_t port);
	virtual void     outPort     (uint16_t port, uint8_t value);
	virtual void     addressOnBus(uint16_t address, int32_t wstates);
	virtual void     interruptHandlingTime(int32_t wstates);
	virtual bool     isActiveINT (void);
#ifdef WITH_BREAKPOINT_SUPPORT
	virtual uint8_t  breakpoint  (uint16_t address, uint8_t opcode);
#endif

#ifdef WITH_EXEC_DONE
	virtual void     execDone(void);
#endif



private:
	Z80* z80;
	uint8_t* mem;
	uint8_t* ports;
	uint32_t tstates;

	uint32_t cycleTstates;

	void loadROM();
	void loadDump();

	void generateScreen(uint8_t* screen);

	uint8_t border;
};

#endif // _MINZX_H_