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

#define MAX_DATA_LENGTH		4096
#define MAX_LINES_NUM		300
#define DEFAULT_ADDRESS		100
#define MAX_LINE_LENGTH		80
#define MAX_LABEL_LENGTH	31
#define MEMORY_WORD_LENGTH	12
#define DECIMAL 10
#define MAX_REGISTER_DIGIT	7
#define MAX_LABELS_NUM		MAX_LINES_NUM


/* TRUE and FALSE */
typedef unsigned int bool;

/* Command struct */
typedef struct
{
    char *name;
    unsigned int opcode : 4;
    int numOfParams;
    unsigned int funct : 4;
} Command;


/* Labels Struct */
typedef struct
{
	int address;
	char name[MAX_LABEL_LENGTH];
	bool isExtern;
	bool isData;
	int line_num;
} Label;

/* Directive Struct */
typedef struct 
{
	char *name;
	void (*parseFunc)();
} Directive;



/* Operands */
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
    int lineNum;				/* Line in the file */
    int address;				/* Address of the first word in the line */
    char *originalString;		/* Original pointer, allocated by malloc */
    char *lineStr;				/* String in the line*/
    bool isError;				/* Error status */
    Label *label;			    /* Pointer to the lines label in labelArr */
    char *commandStr;			/* The string of the Command or Directive */

    /* Command line */
    const Command *cmd;			/* A pointer to the Command in commandArray */
    Operand op1;			/* The 1st operand */
    Operand op2;			/* The 2nd operand */
} Line;

/* ARE Types */
typedef enum { EXTERNAL = 1, RELOCATABLE = 2, ABSOLUTE = 4 } areType;

/* Memory Word Struct */
typedef struct
{
    unsigned int are;
    unsigned int value;
} MemoryWord;


/* Functions */
int getCommand(char *cmdName);
Label *getLabel(char *labelName);
void trimLeftString(char **ptStr);
char *getToken(char *str, char **endOfTok);
bool isOneWord(char *str);
bool isWhiteSpace(char *str);
void trimString(char **ptStr);
bool isDirective(char *cmd);
bool isLegalStrParam(char **strParam, int lineNum);
bool isExistingEntryLabel(char *labelName);
bool isRegister(char *str, int *value);
bool isEmpty(Line *line);
bool isLegalLabel(char *label, int lineNum, bool printErrors);
bool isExistingLabel(char *label);
char *getFirstOperand(char *line, char **endOfOp, bool *foundComma);
bool isLegalNum(char *numStr, int numOfBits, int lineNum, int *value);

/* main methods */
void printError(int lineNum, const char *format, ...);

/* Transition methods */
int firstTransitionRead(FILE *file, Line *linesArr, int *lines, int *IC, int *DC);
int secondTransitionRead(int *memoryArr, int* areArr, Line *linesArr, int lineNum, int IC, int DC);

#endif

