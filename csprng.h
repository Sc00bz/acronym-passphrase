#pragma once

#include <type_traits>
#ifdef _WIN32
	#include <windows.h>
#else
	#include <stdio.h>
#endif
#include <stdint.h>

class Csprng
{
public:
	static int get(void *buffer, size_t size);

	static int getInt(uint8_t  &value, uint8_t  min, uint8_t  max);
	static int getInt(uint16_t &value, uint16_t min, uint16_t max);
	static int getInt(uint32_t &value, uint32_t min, uint32_t max);
	static int getInt(uint64_t &value, uint64_t min, uint64_t max);

private:
	static int init();

#ifdef WIN32
	static HCRYPTPROV  m_hCryptProv;
#else
	static FILE       *m_fin;
#endif
	static int         m_init;
};
