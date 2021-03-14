/*
C Project 2021A - Assembler
By:
Itay Flaysher - 318378395
Maxim Voloshin - 327032991

assembler.h - header file for the project, includes, defines and declarations.
*/

#ifndef ASSEMBLER_H
	#define ASSEMBLER_H
#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Defines */
#define FALSE				0
#define TRUE				1
#define BYTE_SIZE			8
#define INFINITE_LOOP		for(;;)

#define MAX_LABELS_NUM		MAXIMUM_LINES

#define MEMORY_WORD_LEN	12
#define MAXIMUM_LINES		300
#define DATA_MAX_LENGTH		4096
#define STARTING_ADDRESS		100
#define LINE_MAX_LEN		80
#define LAST_REGISTER	7
#define LABEL_MAX_LEN	31
#define DECIMAL_TEN 10



/* TRUE  FALSE */
typedef unsigned int bool;

/* Labels Struct */
typedef struct
{
	int label_address;
	char label_name[LABEL_MAX_LEN];
	bool isExtern;
	bool isData;
	int line_num;
} Label;

/* Command struct */
typedef struct
{
    char *name;
    unsigned int opcode : 4;
    int numOfParams;
    unsigned int funct : 4;
} Command;

/* Directive Struct */
typedef struct 
{
	char *name;
	void (*parseFunc)();
} Directive;



/* Operand types */
typedef enum { NUMBER = 0, LABEL = 1, RELATIVE_LABEL = 2, REGISTER = 3, INVALID = -1 } opType;

/*Operand struct*/
typedef struct
{
    int value;
    char *str;
    opType type;
    int address;
    int line_num;
} Operand;

/* Line Struct */
typedef struct
{
    int lineNum;
    int address;
    char *originalString;
    char *lineStr;
    bool isError;
    Label *label;
    char *commandStr;

    /* Command line */
    const Command *cmd;
    Operand op1;
    Operand op2;
} Line;

/* ARE Types */
typedef enum { EXTERNAL = 1, RELOCATABLE = 2, ABSOLUTE = 4 } areType;

/* Memory Word Struct */
typedef struct
{
    unsigned int are;
    unsigned int value;
} MemoryWord;


/* FUNCTION DECLARATIONS*/
bool isDirective(char *cmd);
bool isLegalStringParam(char **strParam, int lineNum);
bool isExistingEntryLabel(char *labelName);
bool isRegister(char *str, int *value);
bool isEmpty(Line *line);
bool isOneWord(char *str);
bool checkWhiteSpace(char *str);
bool isLegalLabel(char *label, int lineNum, bool printErrors);
bool isExistingLabel(char *label);
bool isLegalNumber(char *numStr, int numOfBits, int lineNum, int *value);
void trimLeftString(char **ptStr);
char *getToken(char *str, char **endOfTok);
void trimString(char **ptStr);
int getCommand(char *cmdName);
Label *getLabel(char *labelName);
char *getFirstOperand(char *line, char **endOfOp, bool *foundComma);

void printError(int lineNum, const char *format, ...);

int firstPassRead(FILE *file, Line *linesArr, int *lines, int *IC, int *DC);
int secondPassRead(int *memoryArr, int* areArr, Line *linesArr, int lineNum, int IC, int DC);

#endif

