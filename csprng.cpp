#include <stdio.h>
#include "csprng.h"

#ifdef _WIN32
	HCRYPTPROV Csprng::m_hCryptProv = 0;
#else
	FILE *Csprng::m_fin = NULL;
#endif
int Csprng::m_init = Csprng::init();

int Csprng::getInt(uint8_t &value, uint8_t min, uint8_t max)
{
	uint8_t randInt;
	uint8_t maxRand;
	uint8_t range = max - min + 1;

	if (min > max)
	{
		return 1;
	}
	else if (min == max)
	{
		value = min;
		return 0;
	}

	if (Csprng::get(&randInt, sizeof(randInt)))
	{
		return 1;
	}
	if (((range - 1) & range) == 0)
	{
		value = (randInt & (range - 1)) + min;
		return 0;
	}

	maxRand = UINT8_MAX - UINT8_MAX % range - 1;
	while (randInt > maxRand)
	{
		if (Csprng::get(&randInt, sizeof(randInt)))
		{
			return 1;
		}
	}

	value = randInt % range + min;
	return 0;
}

int Csprng::getInt(uint16_t &value, uint16_t min, uint16_t max)
{
	uint16_t randInt;
	uint16_t maxRand;
	uint16_t range = max - min + 1;

	if (min > max)
	{
		return 1;
	}
	else if (min == max)
	{
		value = min;
		return 0;
	}

	if (Csprng::get(&randInt, sizeof(randInt)))
	{
		return 1;
	}
	if (((range - 1) & range) == 0)
	{
		value = (randInt & (range - 1)) + min;
		return 0;
	}

	maxRand = UINT16_MAX - UINT16_MAX % range - 1;
	while (randInt > maxRand)
	{
		if (Csprng::get(&randInt, sizeof(randInt)))
		{
			return 1;
		}
	}

	value = randInt % range + min;
	return 0;
}

int Csprng::getInt(uint32_t &value, uint32_t min, uint32_t max)
{
	uint32_t randInt;
	uint32_t maxRand;
	uint32_t range = max - min + 1;

	if (min > max)
	{
		return 1;
	}
	else if (min == max)
	{
		value = min;
		return 0;
	}

	if (Csprng::get(&randInt, sizeof(randInt)))
	{
		return 1;
	}
	if (((range - 1) & range) == 0)
	{
		value = (randInt & (range - 1)) + min;
		return 0;
	}

	maxRand = UINT32_MAX - UINT32_MAX % range - 1;
	while (randInt > maxRand)
	{
		if (Csprng::get(&randInt, sizeof(randInt)))
		{
			return 1;
		}
	}

	value = randInt % range + min;
	return 0;
}

int Csprng::getInt(uint64_t &value, uint64_t min, uint64_t max)
{
	uint64_t randInt;
	uint64_t maxRand;
	uint64_t range = max - min + 1;

	if (min > max)
	{
		return 1;
	}
	else if (min == max)
	{
		value = min;
		return 0;
	}

	if (Csprng::get(&randInt, sizeof(randInt)))
	{
		return 1;
	}
	if (((range - 1) & range) == 0)
	{
		value = (randInt & (range - 1)) + min;
		return 0;
	}

	maxRand = UINT64_MAX - UINT64_MAX % range - 1;
	while (randInt > maxRand)
	{
		if (Csprng::get(&randInt, sizeof(randInt)))
		{
			return 1;
		}
	}

	value = randInt % range + min;
	return 0;
}

#ifdef _WIN32

int Csprng::get(void *buffer, size_t size)
{
	if (size <= 0 || !CryptGenRandom(m_hCryptProv, size, (BYTE*) buffer))
	{
		fprintf(stderr, "Error CryptGenRandom\n");
		exit(1);
		return 1;
	}
	return 0;
}

int Csprng::init()
{
	static bool first = true;

	if (first)
	{
		first = false;
		if (!CryptAcquireContext(&m_hCryptProv, NULL, NULL, PROV_RSA_FULL, 0))
		{
			fprintf(stderr, "Error CryptAcquireContext\n");
			exit(1);
		}
	}
	return 0;
}

#else

int Csprng::get(void *buffer, size_t size)
{
	if (size <= 0 || fread(buffer, size, 1, m_fin) != 1)
	{
		fprintf(stderr, "fread  \"/dev/urandom\"\n");
		exit(1);
		return 1;
	}
	return 0;
}

int Csprng::init()
{
	static bool first = true;

	if (first)
	{
		first = false;
		m_fin = fopen("/dev/urandom", "rb");
		if (m_fin == NULL)
		{
			perror("fopen \"/dev/urandom\"");
			exit(1);
		}
	}
	return 0;
}

#endif
