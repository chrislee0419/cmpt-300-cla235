//
//	Name: 		Chris Lee
//	Student #: 	301238906
//	SFU ID: 	cla235
//
//	Course: 	CMPT 300 D100
//	Instructor: 	Brian Booth
//	TA: 		Scott Kristjanson
//


#ifndef _DECRYPT_H_
#define _DECRYPT_H_

int decrypt(char* encrypted_string);
void stepOne(char* string);
void stepTwo(char* string, unsigned long long* numbers, unsigned int* table);
void stepThree(unsigned long long* numbers, int pos);
void stepFour(char* string, unsigned long long* numbers, unsigned int* table, int pos);
unsigned long long modularExp(unsigned long long base, unsigned long long power, unsigned long long mod);
void generateCipherTable(unsigned int* table);
void generateTextTable(unsigned int* table);

#endif 

