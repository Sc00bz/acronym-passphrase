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
#include "bigint.h"

using namespace std;

struct wordKeySpace
{
	string word;
	BigInt keySpace;
	BigInt keySpaceSum;
};

/**
 * Used for binary search: lower_bound().
 */
bool operator<(const wordKeySpace &a, const wordKeySpace &b)
{
	if (a.keySpaceSum.comp(b.keySpaceSum) < 0)
	{
		return 1;
	}
	return 0;
}

/**
 * Reads words from file into both byLength and byLetter.
 *
 * All words that of length N go into byLength[N].
 * All words that start with X go into byLetter[lowercase(X) - 'a'].
 *
 * Returns number of words, or 0 if error.
 */
uint32_t readWords(const char *fileName, vector<vector<wordKeySpace>> &byLength, vector<vector<string>> &byLetter)
{
	vector<list<wordKeySpace>> byLengthList;
	vector<list<string>>       byLetterList(26);
	FILE *fin = fopen(fileName, "r");
	uint32_t count = 0;
	char  word[256];

	if (fin == NULL)
	{
		perror("Error fopen()");
		cerr << "fopen(\"" << fileName << "\")" << endl;
		return 0;
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
		while (len >= byLengthList.size())
		{
			list<wordKeySpace> tmp;
			byLengthList.push_back(tmp);
		}
		wordKeySpace tmp = {word};
		byLengthList[len].push_back(tmp);
		byLetterList[(word[0] | 32) - 97].push_back(word);
		count++;
	}
	if (feof(fin) == 0)
	{
		perror("Error fgets()");
		return 0;
	}

	// Copy to byLength and byLetter vectors
	size_t size = byLengthList.size();
	byLength.resize(size);
	for (size_t i = 0; i < size; i++)
	{
		size_t size2 = byLengthList[i].size();
		byLength[i].resize(size2);
		for (size_t j = 0; j < size2; j++)
		{
			byLength[i][j] = byLengthList[i].front();
			byLengthList[i].pop_front();
		}
	}
	byLetter.resize(26);
	for (size_t i = 0; i < 26; i++)
	{
		size_t size2 = byLetterList[i].size();
		byLetter[i].resize(size2);
		for (size_t j = 0; j < size2; j++)
		{
			byLetter[i][j] = byLetterList[i].front();
			byLetterList[i].pop_front();
		}
	}

	return count;
}

/**
 * Sets byLength's .keySpace and .keySpaceSum:
 *   byLength[N][i].keySpace is the key space of the acronym "byLength[N][i].word".
 *   byLength[N][i].keySpaceSum is the sum of all key spaces of the prior acronyms of length N and the
 *     current acronym (i.e. sum of byLength[N][x].keySpace for x = 0 to i).
 *
 * Sets byLengthKeySpace[N] to total key space of all words of length N:
 *   byLengthKeySpace[N] is set to byLength[N]["last"].keySpaceSum or 0 when byLength[N] is empty.
 */
void setByLengthKeySpaces(vector<vector<wordKeySpace>> &byLength, vector<BigInt> &byLengthKeySpace, const vector<vector<string>> &byLetter)
{
	uint32_t byLetterCount[26];

	for (int i = 0; i < 26; i++)
	{
		byLetterCount[i] = (uint32_t) (byLetter[i].size());
	}

	// Calculate key spaces
	byLengthKeySpace.clear();
	byLengthKeySpace.reserve(byLength.size());
	for (auto itByLength = byLength.begin(), end = byLength.end(); itByLength != end; ++itByLength)
	{
		BigInt byLengthKeySpace_;

		for (auto itAcronym = itByLength->begin(), end2 = itByLength->end(); itAcronym != end2; ++itAcronym)
		{
			const char *word = itAcronym->word.c_str();
			char ch = word[0];

			itAcronym->keySpace.set(1);
			for (int i = 0; ch != 0;)
			{
				size_t letter = (ch | 32) - 97;
				if (letter > 25)
				{
					cerr << "Error: Invalid letter... memory corruption?" << endl;
					exit(1);
				}
				itAcronym->keySpace.mul(byLetterCount[letter]);
				ch = word[++i];
			}
			byLengthKeySpace_.add(itAcronym->keySpace);
			itAcronym->keySpaceSum = byLengthKeySpace_;
		}
		byLengthKeySpace.push_back(byLengthKeySpace_);
	}
}

/**
 * Gets the minimum acronym length that meets the entropy requirement.
 *
 * Returns acronym length, or 0 if error.
 */
size_t getAcronymLength(int entropy, const vector<BigInt> &byLengthKeySpace)
{
	int length = 1;

	for (; length < byLengthKeySpace.size(); length++)
	{
		double value = byLengthKeySpace[length].get();
		if (value != 0.0 && entropy <= log2(value))
		{
			break;
		}
	}
	if (length >= byLengthKeySpace.size())
	{
		length = 0;
	}
	return length;
}

/**
 * Generates a random acronym of a specific length.
 */
string generateAcronym(size_t length, const vector<vector<wordKeySpace>> &byLength, const vector<BigInt> &byLengthKeySpace)
{
	const BigInt ONE(1);

	wordKeySpace searchValue;

	if (byLength[length].size() == 0)
	{
		return "";
	}

	// A random value 1 to byLengthKeySpace[length]
	searchValue.keySpaceSum = byLengthKeySpace[length].randomModSelf();
	searchValue.keySpaceSum.add(ONE);

	// Binary search for the smallest keySpaceSum that's at least as large as searchValue.keySpaceSum
	return lower_bound(byLength[length].begin(), byLength[length].end(), searchValue)->word;
}

/**
 * Generates a random passphrase from an acronym.
 */
string generatePassphrase(const string &acronym, const vector<vector<string>> &byLetter)
{
	string passphrase;

	for (size_t i = 0, size = acronym.size(); i < size; i++)
	{
		// Get next letter from acronym
		size_t letter = (acronym[i] | 32) - 97;
		if (letter > 25)
		{
			cerr << "Error: Invalid letter... memory corruption?" << endl;
			exit(1);
		}

		// Generate random word that starts with letter
		uint32_t randomWord;
		Csprng::getInt(randomWord, 0, (uint32_t) (byLetter[letter].size() - 1));
		if (passphrase.size())
		{
			passphrase += "-";
			passphrase += byLetter[letter][randomWord];
		}
		else
		{
			passphrase = byLetter[letter][randomWord];
		}
	}

	return passphrase;
}

int main(int argc, char *argv[])
{
	vector<vector<wordKeySpace>> byLength;
	vector<vector<string>>       byLetter;
	vector<BigInt>               byLengthKeySpace;

	// Usage
	if (argc != 2)
	{
		cerr << "Usage: \"" << argv[0] << "\" file-name" << endl;
		return 1;
	}

	// Read word list
	uint32_t numWords = readWords(argv[1], byLength, byLetter);
	if (numWords == 0)
	{
		return 1;
	}
	cout << "Words: " << numWords << endl << endl;

	// Print by first letter words
	for (int i = 0; i < 26; i++)
	{
		cout << (char) ('a' + i) << ": " << byLetter[i].size() << endl;
	}
	cout << endl;

	// Set key spaces
	setByLengthKeySpaces(byLength, byLengthKeySpace, byLetter);

	// Print by length key spaces
	uint32_t maxEntropy = 0;
	for (int i = 0; i < byLength.size(); i++)
	{
		double value = byLengthKeySpace[i].get();
		if (value != 0.0)
		{
			value = log2(value);
			uint32_t value32 = (uint32_t) floor(value);
			if (maxEntropy < value32)
			{
				maxEntropy = value32;
			}
			cout << i << " (" << byLength[i].size() << " words): 2^" << value << " (0x";
			byLengthKeySpace[i].print();
			cout << ")" << endl;
		}
	}
	cout << endl;

	while (1)
	{
		uint32_t entropy;

		cout << "Passphrase entropy bits (max " << maxEntropy << "): ";
		cin >> entropy;
		if (cin.eof() || cin.fail())
		{
			break;
		}

		// Get the minimum acronym length that meets the entropy requirement
		size_t length = getAcronymLength(entropy, byLengthKeySpace);
		if (length == 0)
		{
			cout << "Error: Passphrase entropy bits too high" << endl;
			continue;
		}

		// Get passphrase acronym
		string acronym = generateAcronym(length, byLength, byLengthKeySpace);

		// Get passphrase
		string passphrase = generatePassphrase(acronym, byLetter);

		// Print key space and acronym passphrase
		cout << endl;
		cout << "Acronym length:     " << length << endl;
		cout << "Key space:          2^" << log2(byLengthKeySpace[length].get()) << endl;
		cout << "Acronym/passphrase: " << acronym << ": " << passphrase << endl << endl;
	}

	return 0;
}
