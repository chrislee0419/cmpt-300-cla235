//
//  Name:       Chris Lee
//  Student #:  301238906
//  SFU ID:     cla235
//
//  Course:     CMPT 300 D100
//  Instructor:     Brian Booth
//  TA:         Scott Kristjanson
//


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "decrypt.h"
#include "memwatch.h"

main (int argc, char **argv) {
    // string = contains an encrypted tweet and eventually, the
    //          resulting decrypted message      
    char* string = malloc(sizeof(char) * 200);

    FILE* input = fopen(argv[1], "r");
    if (input == NULL) {
        printf("ERROR: input file \"%s\" does not exist. Terminating.\n", argv[1]);
        free(string);
        return;
    }
    FILE* output = fopen(argv[2], "w");

    while (!feof(input)) {
        memset(string, 0, sizeof(char) * 200);
        fgets(string, 200, input);

        int res = decrypt(string);
        if (res == 1) {printf("ERROR: variable \"code\" failed to malloc. Terminating.\n"); break;}
        else if (res == 2) {printf("ERROR: variable \"table\" failed to malloc. Terminating.\n"); break;}

        fputs(string, output);
        fputc('\n', output);
    }

    fclose(input);
    fclose(output);
    free(string);
} 

