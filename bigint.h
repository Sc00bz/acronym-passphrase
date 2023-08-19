#pragma once

#include <stdint.h>

class BigInt
{
public:
	BigInt();
	BigInt(uint32_t n);
	BigInt(const BigInt &n);
	~BigInt();

	BigInt &operator=(const BigInt &n);
	void    set(uint32_t n);
	double  get() const;
	void    print() const;
	void    mul(uint32_t n);
	void    add(const BigInt &n);
	int     sub(const BigInt &n);
	int     comp(const BigInt &n) const;
	BigInt  randomModSelf() const;

private:
	uint32_t *m_num;
	size_t    m_numSize;
};
