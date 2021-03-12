/*
C Project 2021A - Assembler
Served by:
Itay Flaysher - 318378395
Maxim Voloshin - 327032991

main.c - this is the main file that contains the main function and send the program to the first and second pass
and manage the printing.
*/

#include "header.h"
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

Label labelsArray[MAX_LABELS_NUM];
int labelsCount = 0;
Line *entryLines[MAX_LABELS_NUM];
int entryLabelCount = 0;
int dataArray[MAX_DATA_LENGTH];


/* ====== Methods ====== */
/* Return decimal number length, which will be useful for the print function */
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

/* Prints the dest value as decimal, with at least 4 digits. */
void printDestToFile(FILE *file, int num)
{
	/* Add zeros first, to make the length 4 digits. */
	int length;
	length = getNumDecimalLength(num);
	if(length == 3)
		fprintf(file, "0");
	fprintf(file, "%d", num);
}

/* print A,R,E types to the ob file.*/
void printAREToFile(FILE *file, int num)
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

/* Prints the IC and DC value as decimal */
void printICDCToFile(FILE *file, int num)
{
	fprintf(file, "\t%d", num);
}

/* Prints the entry value as decimal */
void printEntryToFile(FILE *file, int num)
{
	fprintf(file, "%d", num);
}

/* print data to the file */
void printDataToFile(FILE *file, int num)
{
    uint16_t x = num;
    fprintf(file,"%03X",x);
}

/* Prints the Extern value as decimal */
void printExternToFile(FILE *file, int num)
{
	int length;
	length = getNumDecimalLength(num);
	if(length == 3)
		fprintf(file,"0");
	fprintf(file, "%d", num);
}

/* Creates a file from a given name and ending, and returns a pointer to it. */
FILE *openFile(char *name, char *ending, const char *mode)
{
	FILE *file;
	char *mallocStr = (char *)malloc(strlen(name) + strlen(ending) + 1), *fileName = mallocStr;
	sprintf(fileName, "%s%s", name, ending);

	file = fopen(fileName, mode);
	free(mallocStr);

	return file;
}

/* Creates the .obj file*/
void createObjectFile(char *name, int IC, int DC, int *memoryArr,int* areArray)
{
	int i;
	FILE *file;
	file = openFile(name, ".ob", "w");

	/* Print IC and DC */
	printICDCToFile(file, IC);
	fprintf(file, "\t\t");
	printICDCToFile(file, DC);

	/* Print all of memoryArr */
	for (i = 0; i < IC + DC; i++)
	{
		fprintf(file, "\n");
		printDestToFile(file, DEFAULT_ADDRESS + i); /* adding the 100 to the IC print */
		fprintf(file, "\t\t");
		printDataToFile(file, memoryArr[i]);
		printAREToFile(file, areArray[i]);
	}
	fclose(file);
}

/* Creates the .ent file */
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
		printEntryToFile(file, getLabel(entryLines[i]->lineStr)->address);

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
				printExternToFile(file, linesArr[i].op1.address);
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
				printExternToFile(file, linesArr[i].op2.address);
				firstPrint = FALSE;
			}
		}
	}

	if (file)
	{
		fclose(file);
	}
}

/* Resets all the globals and free all the memory allocations. */
void clearAllData(Line *linesArr, int linesFound, int dataCount)
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

/* Parses a file, and creates the output files. */
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
		printf("Success! Output files Created for \"%s.as\".\n", fileName);
	}
	else
	{
		/* print the number of errors. */
		printf("%d error%s found in \"%s.as\".\n", numOfErrors, (numOfErrors > 1) ? "s were" : " was", fileName);
	}
	/* Free all malloc pointers, and reset the globals. */
	clearAllData(linesArr, linesFound, IC + DC);

	/* Close File */
	fclose(file);
}

/* Main function. Calls the "parsefile" method for each file name in argv. */
int main(int argc, char *argv[])
{
	int i;
    if (argc < 2)
	{
		printf("ERROR: No file names.\n");
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
