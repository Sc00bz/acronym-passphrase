#include <iostream>
#include "bigint.h"
#include "csprng.h"

using namespace std;

BigInt::BigInt()
{
	m_num = new uint32_t[1];
	m_num[0] = 0;
	m_numSize = 1;
}

BigInt::BigInt(uint32_t n)
{
	m_num = new uint32_t[1];
	m_num[0] = n;
	m_numSize = 1;
}

BigInt::BigInt(const BigInt &n)
{
	size_t numSize = n.m_numSize;

	m_num = new uint32_t[numSize];
	for (size_t i = 0; i < numSize; i++)
	{
		m_num[i] = n.m_num[i];
	}
	m_numSize = numSize;
}

BigInt::~BigInt()
{
	delete [] m_num;
}

BigInt &BigInt::operator=(const BigInt &n)
{
	size_t numSize = n.m_numSize;

	delete [] m_num;
	m_num = new uint32_t[numSize];
	for (size_t i = 0; i < numSize; i++)
	{
		m_num[i] = n.m_num[i];
	}
	m_numSize = numSize;
	return *this;
}

void BigInt::set(uint32_t n)
{
	m_num[0] = n;
	m_numSize = 1;
}

double BigInt::get() const
{
	double          next = 4294967296.0;
	double          ret  = 0.0;
	const uint32_t *num  = m_num;
	size_t          i    = m_numSize;

	while (i != 0)
	{
		i--;
		ret *= next;
		ret += num[i];
	}
	return ret;
}

void BigInt::print() const
{
	const uint32_t *num = m_num;
	size_t          i   = m_numSize - 1;
	ios             oldState(NULL);

	oldState.copyfmt(cout);
	cout << hex << num[i];
	cout.fill('0');
	cout.width(8);
	while (i != 0)
	{
		i--;
		cout << num[i];
	}
	cout.copyfmt(oldState);
}

void BigInt::mul(uint32_t n)
{
	uint32_t *num      = m_num;
	size_t    numSize  = m_numSize;
	uint32_t  overflow = 0;

	for (size_t i = 0; i < numSize; i++)
	{
		uint64_t tmp = (uint64_t) n * num[i] + overflow;
		num[i] = (uint32_t) tmp;
		overflow = (uint32_t) (tmp >> 32);
	}
	if (overflow != 0)
	{
		uint32_t *tmp = new uint32_t[numSize + 1];
		for (size_t i = 0; i < numSize; i++)
		{
			tmp[i] = num[i];
		}
		tmp[numSize] = overflow;
		delete [] num;
		m_num     = tmp;
		m_numSize = numSize + 1;
	}
}

void BigInt::add(const BigInt &n)
{
	uint32_t       *numA     = m_num;
	const uint32_t *numB     = n.m_num;
	size_t          numSizeA = m_numSize;
	size_t          numSizeB = n.m_numSize;
	uint32_t        overflow = 0;

	if (numSizeA < numSizeB)
	{
		uint32_t *tmp = new uint32_t[numSizeB];
		for (size_t i = 0; i < numSizeA; i++)
		{
			tmp[i] = numA[i];
		}
		for (size_t i = numSizeA; i < numSizeB; i++)
		{
			tmp[i] = 0;
		}
		delete [] numA;
		numA      = tmp;
		m_num     = tmp;
		m_numSize = numSizeB;
	}
	for (size_t i = 0; i < numSizeB; i++)
	{
		uint64_t tmp = (uint64_t) numA[i] + numB[i] + overflow;
		numA[i] = (uint32_t) tmp;
		overflow = (uint32_t) (tmp >> 32);
	}
	for (size_t i = numSizeB; i < numSizeA && overflow != 0; i++)
	{
		uint64_t tmp = (uint64_t) numA[i] + overflow;
		numA[i] = (uint32_t) tmp;
		overflow = (uint32_t) (tmp >> 32);
	}
	if (overflow != 0)
	{
		uint32_t *tmp = new uint32_t[numSizeA + 1];
		for (size_t i = 0; i < numSizeA; i++)
		{
			tmp[i] = numA[i];
		}
		tmp[numSizeA] = overflow;
		delete [] numA;
		m_num     = tmp;
		m_numSize = numSizeA + 1;
	}
}

int BigInt::sub(const BigInt &n)
{
	uint32_t       *numA     = m_num;
	const uint32_t *numB     = n.m_num;
	size_t          numSizeA = m_numSize;
	size_t          numSizeB = n.m_numSize;
	uint32_t        overflow = 0;

	if (numSizeA < numSizeB)
	{
		uint32_t *tmp = new uint32_t[1];
		tmp[0] = 0;
		delete [] numA;
		m_num     = tmp;
		m_numSize = 1;
		return 1;
	}
	for (size_t i = 0; i < numSizeB; i++)
	{
		uint64_t tmp = (uint64_t) numA[i] - numB[i] - overflow;
		numA[i] = (uint32_t) tmp;
		overflow = 0 - (uint32_t) (tmp >> 32);
	}
	for (size_t i = numSizeB; i < numSizeA && overflow != 0; i++)
	{
		uint64_t tmp = (uint64_t) numA[i] + overflow;
		numA[i] = (uint32_t) tmp;
		overflow = (uint32_t) (tmp >> 32);
	}
	if (overflow != 0)
	{
		uint32_t *tmp = new uint32_t[1];
		tmp[0] = 0;
		delete [] numA;
		m_num     = tmp;
		m_numSize = 1;
		return 1;
	}
	while (numSizeA > 1 && numA[numSizeA - 1] == 0)
	{
		numSizeA--;
	}
	m_numSize = numSizeA;

	return 0;
}

int BigInt::comp(const BigInt &n) const
{
	const uint32_t *numA     = m_num;
	const uint32_t *numB     = n.m_num;
	size_t          numSizeA = m_numSize;
	size_t          numSizeB = n.m_numSize;

	if (numSizeA < numSizeB)
	{
		return -1;
	}
	if (numSizeA > numSizeB)
	{
		return 1;
	}
	size_t i = numSizeA;
	while (i != 0)
	{
		i--;
		if (numA[i] < numB[i])
		{
			return -1;
		}
		if (numA[i] > numB[i])
		{
			return 1;
		}
	}
	return 0;
}

BigInt BigInt::randomModSelf() const
{
	BigInt ret;

	// Init ret
	delete [] ret.m_num;
	ret.m_num = new uint32_t[m_numSize];
	ret.m_numSize = m_numSize;

	// Find mask
	uint32_t mask = 0xffffffff;
	uint32_t hi   = m_num[m_numSize - 1];
	while (mask >> 1 > hi)
	{
		mask >>= 1;
	}

	// Find random number
	do
	{
		Csprng::get(ret.m_num, ret.m_numSize * sizeof(uint32_t));
		ret.m_num[ret.m_numSize - 1] &= mask;
	} while (ret.comp(*this) >= 0);

	return ret;
}
