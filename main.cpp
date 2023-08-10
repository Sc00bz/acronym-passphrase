#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <iomanip>
#include <stdint.h>
#include <math.h>
#include <iostream>
#include <list>
#include <string>
#include <vector>
#include "csprng.h"

using namespace std;

class BigInt
{
public:
	BigInt()           {m_num = new uint32_t[1]; m_num[0] = 0; m_numSize = 1;}
	BigInt(uint32_t n) {m_num = new uint32_t[1]; m_num[0] = n; m_numSize = 1;}
	BigInt(const BigInt &n)
	{
		size_t numSize = n.m_numSize;

		m_num = new uint32_t[numSize];
		for (size_t i = 0; i < numSize; i++)
		{
			m_num[i] = n.m_num[i];
		}
		m_numSize = numSize;
	}
	~BigInt() {delete [] m_num;}

	BigInt &operator=(const BigInt &n)
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

	void set(uint32_t n)
	{
		m_num[0] = n;
		m_numSize = 1;
	}

	double get() const
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

	void print() const
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

	void mul(uint32_t n)
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

	void add(const BigInt &n)
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

	int sub(const BigInt &n)
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

	int comp(const BigInt &n) const
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

	BigInt randomModSelf()
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

private:
	uint32_t *m_num;
	size_t    m_numSize;
};

struct wordKeySpace
{
	string word;
	BigInt keySpace;
	BigInt keySpaceSum;
};

bool operator<(const wordKeySpace &a, const wordKeySpace &b)
{
	if (a.keySpaceSum.comp(b.keySpaceSum) < 0)
	{
		return 1;
	}
	return 0;
}

vector<vector<wordKeySpace>> g_byLength;
vector<vector<string>>       g_byLetter(26);
vector<BigInt>               g_byLengthKeySpace;

int readWords(const char *fileName)
{
	vector<list<wordKeySpace>> byLength;
	vector<list<string>>       byLetter(26);
	FILE *fin = fopen(fileName, "r");
	uint32_t count = 0;
	char  word[256];

	if (fin == NULL)
	{
		perror("Error fopen()");
		cerr << "fopen(\"" << fileName << "\")" << endl;
		return 1;
	}

	while (fgets(word, 256, fin) != NULL)
	{
		int len = 0;
		for (; len < 256; len++)
		{
			if (
				(word[len] < 'a' || word[len] > 'z') &&
				(word[len] < 'A' || word[len] > 'Z'))
			{
				break;
			}
		}
		word[len] = 0;
		while (len >= byLength.size())
		{
			list<wordKeySpace> tmp;
			byLength.push_back(tmp);
		}
		wordKeySpace tmp = {word};
		byLength[len].push_back(tmp);
		byLetter[(word[0] | 32) - 97].push_back(word);
		count++;
	}
	if (feof(fin) == 0)
	{
		perror("Error fgets()");
		return 1;
	}

	// Copy to g_byLength and g_byLetter vectors
	size_t size = byLength.size();
	g_byLength.resize(size);
	for (size_t i = 0; i < size; i++)
	{
		size_t size2 = byLength[i].size();
		g_byLength[i].resize(size2);
		for (size_t j = 0; j < size2; j++)
		{
			g_byLength[i][j] = byLength[i].front();
			byLength[i].pop_front();
		}
	}
	g_byLetter.resize(26);
	for (size_t i = 0; i < 26; i++)
	{
		size_t size2 = byLetter[i].size();
		g_byLetter[i].resize(size2);
		for (size_t j = 0; j < size2; j++)
		{
			g_byLetter[i][j] = byLetter[i].front();
			byLetter[i].pop_front();
		}
	}

	cout << "Words: " << count << endl;

	return 0;
}

void getByLengthKeySpaces()
{
	uint32_t byLetterCount[26];

	for (int i = 0; i < 26; i++)
	{
		byLetterCount[i] = (uint32_t) (g_byLetter[i].size());
	}

	// Calculate key spaces
	g_byLengthKeySpace.clear();
	for (auto it = g_byLength.begin(), end = g_byLength.end(); it != end; ++it)
	{
		BigInt byLengthKeySpace;

		for (auto it2 = it->begin(), end2 = it->end(); it2 != end2; ++it2)
		{
			const char *word = it2->word.c_str();
			BigInt *keySpace = &(it2->keySpace);
			char ch = word[0];

			keySpace->set(1);
			for (int i = 0; ch != 0;)
			{
				size_t letter = (ch | 32) - 97;
				if (letter > 25)
				{
					cerr << "Error: Invalid letter... memory coruption?" << endl;
					exit(1);
				}
				keySpace->mul(byLetterCount[letter]);
				ch = word[++i];
			}
			byLengthKeySpace.add(*keySpace);
			it2->keySpaceSum = byLengthKeySpace;
		}
		g_byLengthKeySpace.push_back(byLengthKeySpace);
	}
}

int main(int argc, char *argv[])
{
	// Usage
	if (argc != 2)
	{
		cerr << "Usage: \"" << argv[0] << "\" file-name" << endl;
		return 1;
	}

	// Read word list
	if (readWords(argv[1]))
	{
		return 1;
	}

	// Print by first letter words
	for (int i = 0; i < 26; i++)
	{
		cout << (char) ('a' + i) << ": " << g_byLetter[i].size() << endl;
	}
	cout << endl;

	getByLengthKeySpaces();

	// Print by length key spaces
	uint32_t maxBits = 0;
	for (int i = 0; i < g_byLength.size(); i++)
	{
		double value = g_byLengthKeySpace[i].get();
		if (value != 0.0)
		{
			value = log2(value);
			uint32_t value32 = (uint32_t) floor(value);
			if (maxBits < value32)
			{
				maxBits = value32;
			}
			cout << i << " (" << g_byLength[i].size() << " words): 2^" << value << " (0x";
			g_byLengthKeySpace[i].print();
			cout << ")" << endl;
		}
	}

	while (1)
	{
		uint32_t bits;

		cout << "Passphase entropy bits (max " << maxBits << "):" << endl;
		cin >> bits;

		int length = 0;
		for (; length < g_byLength.size(); length++)
		{
			double value = g_byLengthKeySpace[length].get();
			if (value != 0.0 && bits <= log2(value))
			{
				cout << "Key space: 2^" << log2(value) << endl;
				break;
			}
		}
		if (length >= g_byLength.size())
		{
			cout << "Error: passphase entropy bits too high" << endl;
			continue;
		}

		// Get passphrase acronym
		wordKeySpace searchValue;
		searchValue.keySpaceSum = g_byLengthKeySpace[length].randomModSelf();

		auto acronymIt = lower_bound(g_byLength[length].begin(), g_byLength[length].end(), searchValue);
		string acronym = acronymIt->word;
		cout << acronym << ":";

		// Get passphrase
		for (size_t i = 0, size = acronym.size(); i < size; i++)
		{
			size_t letter = (acronym[i] | 32) - 97;
			if (letter > 25)
			{
				cerr << "Error: Invalid letter... memory coruption?" << endl;
				exit(1);
			}

			uint32_t randomWord;
			Csprng::getInt(randomWord, 0, (uint32_t) (g_byLetter[letter].size() - 1));
			cout << " " << g_byLetter[letter][randomWord];
		}
		cout << endl << endl;
	}

	return 0;
}
