/*
*********************************
Assembler Project - Mmn 14 2020A
FILENAME: main.c
FILE INFORMATION : This file will manage the process of the assembler, it will call the first and second read methods.
afterwards, it will create the output files: file.ob,file.ent.file.ext.
BY: Gal Nagli
DATE: MARCH 06 2020
*********************************
*/

/* ======== Includes ======== */
#include "header.h"
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

/* ====== Global Data Structures ====== */
/* Labels Array */
Label labelsArray[MAX_LABELS_NUM];
int labelsCount = 0;
/* Entry Lines */
Line *entryLines[MAX_LABELS_NUM]; /**/
int entryLabelCount = 0;
/* Data Array */
int dataArray[MAX_DATA_LENGTH];

/* ====== Methods ====== */
/* getNumDecimalLength function will return decimal number length, which will be useful for the print function */
int getNumDecimalLength(int num)
{
	int l = !num;
	while(num)
	{
		l++;
		num /= DECIMAL;
	}
	return l;
}

/* printError function prints an error with the line number. */
void printError(int lineNum, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	printf("ERROR: At line %d: ", lineNum);
	vprintf(format, args);
	printf("\n");
	va_end(args);
}

/* fprintfDest function prints the dest value as decimal, with at least 4 digits. */
void fprintfDest(FILE *file, int num)
{
	/* Add zeros first, to make the length 4 digits. */
	int length;
	length = getNumDecimalLength(num);
	if(length == 3)
		fprintf(file, "0");
	fprintf(file, "%d", num);
}

void fprintfAre(FILE *file, int num)
{
   if (num == 4)
       fprintf(file, " A");
   else
   {
       if (num == 2)
           fprintf(file, " R");
       else
       {
           if (num == 1)
               fprintf(file, " E");
           else
               fprintf(file, " A");
       }
   }
}

/* fprintfICDC function prints the IC and DC value as decimal */

void fprintfICDC(FILE *file, int num)
{
	fprintf(file, "\t%d", num);
}

/* fprintfEnt function prints the entry value as decimal */

void fprintfEnt(FILE *file, int num)
{
	fprintf(file, "%d", num);
}
/* fprintfData function print's the data as octal representation, with 5 digits. */

void fprintfData(FILE *file, int num)
{
    uint16_t x = num;
    fprintf(file,"%03X",x);
}


/* fprintfExt function prints the Ext value as decimal */

void fprintfExt(FILE *file, int num)
{
	int length;
	length = getNumDecimalLength(num);
	if(length == 3)
		fprintf(file,"0");
	fprintf(file, "%d", num);
}
/* openFile function creates a file (for writing) from a given name and ending, and returns a pointer to it. */
FILE *openFile(char *name, char *ending, const char *mode)
{
	FILE *file;
	char *mallocStr = (char *)malloc(strlen(name) + strlen(ending) + 1), *fileName = mallocStr;
	sprintf(fileName, "%s%s", name, ending);

	file = fopen(fileName, mode);
	free(mallocStr);

	return file;
}

/* createObjectFile function creates the .obj file*/
void createObjectFile(char *name, int IC, int DC, int *memoryArr,int* areArray)
{
	int i;
	FILE *file;
	file = openFile(name, ".ob", "w");

	/* Print IC and DC */
	fprintfICDC(file, IC);
	fprintf(file, "\t\t");
	fprintfICDC(file, DC);

	/* Print all of memoryArr */
	for (i = 0; i < IC + DC; i++)
	{
		fprintf(file, "\n");
		fprintfDest(file, DEFAULT_ADDRESS + i); /* adding the 100 to the IC print */
		fprintf(file, "\t\t");
		fprintfData(file, memoryArr[i]);
		fprintfAre(file,areArray[i]);
	}
	fclose(file);
}

/* createEntriesFile function creates the .ent file, which contains the addresses for the .entry labels */
void createEntriesFile(char *name)
{
	int i;
	FILE *file;

	/* Don't create the entries file if there aren't entry lines */
	if (!entryLabelCount)
	{
		return;
	}

	file = openFile(name, ".ent", "w");

	for (i = 0; i < entryLabelCount; i++)
	{
		fprintf(file, "%s\t\t", entryLines[i]->lineStr);
		fprintfEnt(file, getLabel(entryLines[i]->lineStr)->address);

		if (i != entryLabelCount - 1)
		{
			fprintf(file, "\n");
		}
	}
	fclose(file);
}

/* createExternFile function creates the .ext file, which contains the addresses for the extern labels operands */
void createExternFile(char *name, Line *linesArr, int linesFound)
{
	int i;
	Label *label;
	bool firstPrint = TRUE; /* This bool meant to prevent the creation of the file if there aren't any externs */
	FILE *file = NULL;

	for (i = 0; i < linesFound; i++)
	{
		/* Check if the 1st operand is extern label, and print it. */
		if (linesArr[i].cmd && linesArr[i].cmd->numOfParams >= 2 && linesArr[i].op1.type == LABEL)
		{
			label = getLabel(linesArr[i].op1.str);
			if (label && label->isExtern)
			{
				if (firstPrint)
				{
					/* Create the file only if there is at least 1 extern */
					file = openFile(name, ".ext", "w");
				}
				else
				{
					fprintf(file, "\n");
				}
				fprintf(file, "%s\t\t", label->name);
				fprintfExt(file, linesArr[i].op1.address);
				firstPrint = FALSE;
			}
		}

		/* Check if the 2nd operand is extern label, and print it. */
		if (linesArr[i].cmd && linesArr[i].cmd->numOfParams >= 1 && linesArr[i].op2.type == LABEL)
		{
			label = getLabel(linesArr[i].op2.str);
			if (label && label->isExtern)
			{
				if (firstPrint)
				{
					/* Create the file only if there is at least 1 extern */
					file = openFile(name, ".ext", "w");
				}
				else
				{
					fprintf(file, "\n");
				}

				fprintf(file, "%s\t\t", label->name);
				fprintfExt(file, linesArr[i].op2.address);
				firstPrint = FALSE;
			}
		}
	}

	if (file)
	{
		fclose(file);
	}
}

/* clearData function resets all the globals and free all the malloc blocks. */
void clearData(Line *linesArr, int linesFound, int dataCount)
{
	int i;

	/* --- Reset Globals --- */

	/* Reset global labels */
	for (i = 0; i < labelsCount; i++)
	{
        labelsArray[i].address = 0;
        labelsArray[i].isData = 0;
        labelsArray[i].isExtern = 0;
	}
    labelsCount = 0;

	/* Reset global entry lines */
	for (i = 0; i < entryLabelCount; i++)
	{
        entryLines[i] = NULL;
	}
    entryLabelCount = 0;

	/* Reset global data */
	for (i = 0; i < dataCount; i++)
	{
        dataArray[i] = 0;
	}

	/* Free malloc blocks */
	for (i = 0; i < linesFound; i++)
	{
		free(linesArr[i].originalString);
	}
}

/* parseFile function parses a file, and creates the output files. */
void parseFile(char *fileName)
{
 	FILE *file = openFile("ps", ".as", "r");
	Line linesArr[MAX_LINES_NUM];
	int memoryArr[MAX_DATA_LENGTH] = {0 }, IC = 0, DC = 0, numOfErrors = 0, linesFound = 0;
    int areArr [9000] = { 0 };

	/* Open File */
	if (file == NULL)
	{
		printf("ERROR: Can't open the file \"%s.as\".\n", fileName);
		return;
	}
	printf("\"%s.as\" opened Successfully.\n", fileName);

	/* First Read */
	numOfErrors += firstPassRead(file, linesArr, &linesFound, &IC, &DC);
	/* Second Read */
	numOfErrors += secondPassRead(memoryArr, areArr, linesArr, linesFound, IC, DC);

	/* Create Output Files */
	if (numOfErrors == 0)
	{
		/* Create all the output files */
		createObjectFile(fileName, IC, DC, memoryArr,areArr);
		createExternFile(fileName, linesArr, linesFound); 
		createEntriesFile(fileName);
		printf("[Info] Created output files for the file \"%s.as\".\n", fileName);
	}
	else
	{
		/* print the number of errors. */
		printf("[Info] A total number of %d error%s found in \"%s.as\".\n", numOfErrors, (numOfErrors > 1) ? "s were" : " was", fileName);
	}
	/* Free all malloc pointers, and reset the globals. */
	clearData(linesArr, linesFound, IC + DC);

	/* Close File */
	fclose(file);
}

/* Main function. Calls the "parsefile" method for each file name in argv. */
int main(int argc, char *argv[])
{
	int i;
    if (argc < 2)
	{
		printf("ERROR: No file names were given.\n");
		return 1;
	}
	argc = 2;
	for (i = 1; i < argc; i++)
	{
        parseFile(argv[i]);
		printf("\n");
	}
	return 0;
}
