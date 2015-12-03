//
//	Name: 		Chris Lee
//	Student #: 	301238906
//	SFU ID: 	cla235
//
//	Course: 	CMPT 300 D100
//	Instructor: 	Brian Booth
//	TA: 		Scott Kristjanson
//


#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "decrypt.h"
#include "memwatch.h"

int decrypt(char* encrypted_string) {
	// Preparing "code", used to store cipher/plain text numbers
	unsigned long long* code = malloc(sizeof(unsigned long long) * 35);
	if (code == NULL) return 1;
	memset(code, 0, sizeof(unsigned long long) * 35);

	int pos = 199, i;
	for (i = 0; i < 200; i++) {
		if (encrypted_string[i] == '\0' || encrypted_string[i] == '\n') {
			pos = i;
			break;
		}
	}

	// Preparing "table", used to store cipher table
	unsigned int* table = malloc(sizeof(unsigned int) * 128);
	if (table == NULL) return 2;
	generateCipherTable(table);

	// Begin decrypting algorithm
	stepOne(encrypted_string);
	stepTwo(encrypted_string, code, table);
	stepThree(code, pos);

	// Changing cipher table to plain text table
	generateTextTable(table);
	memset(encrypted_string, 0, sizeof(encrypted_string));

	stepFour(encrypted_string, code, table, pos);
	// End of decrypting algorithm

	// Freeing variables initialized to the heap in this function
	free(code);
	free(table);
	return 0;
} 


//
//	MAIN ALGORITHM CODE
//

//	stepOne:
//	Takes the encrypted string and removes the characters in positions
//	that are divisible by 8.
//
//	string =	char array containing the encrypted string
void stepOne(char* string) {
	// Initializing copy, stores the
	// string minus the positions divisible by 8
	char copy[200];
	memset(copy, 0, sizeof(copy));
	int i = 0, j = 0;

	// Loop runs until a NULL or newline character is found
	while (!(string[i] == '\0' || string[i] == '\n')) {

		// If i+1 is divisible by 8, don't copy this character
		// and move on to the next character
		if (i != 0 && (i+1)%8 == 0) i++;

		// Otherwise, copy the ith character in "string" 
		// into the jth position in "copy"
		// Iterate i and j afterwards
		else {
			copy[j] = string[i];
			i++; j++;
		}
	}

	// Copying contents of "copy" into string
	memcpy(string, copy, sizeof(copy));
}

//	stepTwo:
//	Takes the manipulated encrypted string from stepOne and sections it
//	into groups of 6. Takes each character and converts it to a number
//	using the cipher table. Each number from a group of 6 is multiplied by
//	a corresponding power of 41 and added to get one large number.
//
//	string =	char array containing the encrypted string
//	numbers =	ull array containing resulting number from each group of six
//	table = 	unsigned int array containing the cipher table
void stepTwo(char* string, unsigned long long* numbers, unsigned int* table) {
	// i = position in "string"
	// j = counts to six (used to group characters)
	// k = position in "numbers"
	int i = 0, j, k = 0;

	while (string[i] != '\0') {
		numbers[k] = 0;
		// Reads sequence of 6 character in "string"
		for (j = 0; j<6; j++) {
			// If the next char is NULL, we are finished
			if (string[i] == '\0') break;

			// Otherwise, use the cipher table to get the value of 
			// the char, multiply it with 41^(5-j) and add it into
			// position k of the "numbers" array
			numbers[k] += 	(unsigned long long)table[(int)string[i]] * 
							(unsigned long long)pow(41, 5-j);
			i++;
		}

		k++;
	}
}

//	stepThree:
//	Takes each cipher number (C) that came from stepTwo and puts it in
//	the equation M = C^d mod n. Stores the resulting number (M) into
//	the number array that held the original cipher number at the same
//	position.
//
//	numbers =	ull array containing the cipher numbers obtained in
//				stepTwo
void stepThree(unsigned long long* numbers, int pos) {
	unsigned long long d = 1921821779, n = 4294434817;
	int i;

	for(i = 0; i < pos/6; i++) {
		numbers[i] = modularExp(numbers[i], d, n);
	}
}

//	stepFour:
//	Takes each plain-text number (M) from stepThree and reverses the
//	process done in stepTwo. Each number corresponds to a group of six
//	characters in the decrypted string
//
//	string =	char array that will capture the decrypted string
//	numbers =	ull array containing the plain-text numbers from stepThree
//	table =		unsigned int array containing the plain-text table
void stepFour(char* string, unsigned long long* numbers, unsigned int* table, int pos) {
	// i = position in "string"
	// j = counts to six (used to ungroup characters)
	// k = position in "numbers"
	int i = 5, j, k;

	for (k = 0; k < pos/6; k++) {
		for (j = 0; j<6; j++) {
			if (i-j < pos) string[i-j] = (char)table[(int)(numbers[k] % 41)];
			else string[i-j] = '\0';
			numbers[k] /= 41;
		}

		i += 6;
	}
}

//
//	END OF MAIN ALGORITHM CODE
//


//	modularExp:
//	Performs modular exponentiation using exponentiation by squaring.
//	Follows the form "result = (base)^(power) mod (mod)".
//	Returns unsigned long long.
unsigned long long modularExp(unsigned long long base, unsigned long long power, unsigned long long mod) {
	unsigned long long result = 1;
	base = base % mod;
	while (power > 0) {
		if (power % 2 == 1)
			result = (result * base) % mod;
		power /= 2;
		base = (base * base) % mod;
	}

	return result;
}

//	generateCipherTable:
//	Returns an unsigned int array containing the cipher table that
//	converts characters into the corresponding cipher number.
//	To use, put the char you want to decipher into the index.
void generateCipherTable(unsigned int* table) {
	int i;
	memset(table, 0, sizeof(unsigned int) * 128);
	// Characters not in the cipher table have a value of 0

	table[(int)' '] = 0;
	table[(int)'#'] = 27;		
	table[(int)'.'] = 28;			
	table[(int)','] = 29;			
	table[39] = 30;			// apostrophe		
	table[(int)'!'] = 31;			
	table[(int)'?'] = 32;			
	table[(int)'('] = 33;
	table[(int)')'] = 34;
	table[(int)'-'] = 35;
	table[(int)':'] = 36;
	table[(int)'$'] = 37;
	table[(int)'/'] = 38;
	table[(int)'&'] = 39;
	table[92] = 40;			// backslash

	// 'a' to 'z'
	for (i = (int)'a'; i <= (int)'z'; i++) table[i] = i - (int)'a' + 1;
}

//	generateTextTable:
//	Returns an unsigned int array containing the plain-text table that
//	converts numbers into the corresponding plain-text character.
//	To use, put the number you want to decipher into the index.
//	Convert the output from type 'unsigned int' to 'char'.
void generateTextTable(unsigned int* table) {
	int i;
	memset(table, 0, sizeof(unsigned int) * 128);

	table[0] = (unsigned int)' ';
	table[27] = (unsigned int)'#';
	table[28] = (unsigned int)'.';
	table[29] = (unsigned int)',';
	table[30] = (unsigned int)39;
	table[31] = (unsigned int)'!';
	table[32] = (unsigned int)'?';
	table[33] = (unsigned int)'(';
	table[34] = (unsigned int)')';
	table[35] = (unsigned int)'-';
	table[36] = (unsigned int)':';

	// 'a' to 'z'
	for (i = 0; i < 26; i++) table[i+1] = i + (int)'a';
}
