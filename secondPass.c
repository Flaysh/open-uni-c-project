#define _CRT_SECURE_NO_WARNINGS
/*
*********************************
Assembler Project - Mmn 14 2020A
FILENAME: secondTransition.c
FILE INFORMATION : This file is used in order to get the data structures from the first read, and convert the information into bits.
BY: Gal Nagli
DATE: MARCH 06 2020
*********************************
*/
/* *****Includes ***** */
#include "header.h"
#include <stdlib.h>
/* ***** Externs ***** */
/* Use the commands list from firstTransition.c */
extern const Command commandArray[];

/* Use the data from firstTransition.c */
extern Label labelsArray[MAX_LABELS_NUM];
extern int labelsCount;
extern Line *entryLines[MAX_LABELS_NUM];
extern int entryLabelCount;
extern int dataArray[MAX_DATA_LENGTH];

/* ***** Methods ***** */

/* updateDataLabelsAddress function updates the addresses of all the data labels in labelsArray. */
void updateDataLabelsAddress(int IC)
{
	int i;

	/* Search in the array for label with isData flag */
	for (i = 0; i < labelsCount; i++)
	{
		if (labelsArray[i].isData)
		{
			/* Increase the address */
			labelsArray[i].address += IC;
		}
	}
}

/* countIllegalEntries function returns if there is an illegal entry line in entryLines. */
int countIllegalEntries()
{
	int i, ret = 0;
	Label *label;

	for (i = 0; i < entryLabelCount; i++)
	{
		label = getLabel(entryLines[i]->lineStr);
		if (label)
		{
			if (label->isExtern)
			{
				printError(entryLines[i]->lineNum, "The parameter for .entry can't be an external label.");
				ret++;
			}
		}
		else
		{
			printError(entryLines[i]->lineNum, "No such label as \"%s\".", entryLines[i]->lineStr);
			ret++;
		}
	}

	return ret;
}



/* updateLabelOpAddress function check's if the op is a label, this method is updating the value of the it to be the address of the label. */
/* Returns FALSE if there is an error, or TRUE otherwise. */
bool updateLabelOpAddress(Operand *op, int lineNum)
{
	if (op->type == LABEL || op->type == RELATIVE_LABEL) {
        Label *label = getLabel(op->str);
        /* Check if op.str is a real label name */
        if (label == NULL) {
            /* Print errors (legal name is illegal or not exists yet) */
            if (isLegalLabel(op->str, lineNum, TRUE)) {
                printError(lineNum, "No such label as \"%s\"", op->str);
            }
            return FALSE;
        }
        op->value = label->address;
    }
return TRUE;
}

/* getNumFromMemoryWord function returns the int value of a memory word. */
int getNumFromMemoryWord(MemoryWord memory)
{
    /* Create an int of "MEMORY_WORD_LENGTH" times '1', and all the rest are '0' */
    unsigned int mask = ~0;
    mask >>= (sizeof(int) * BYTE_SIZE - MEMORY_WORD_LENGTH);

    /* The mask makes sure we only use the first "MEMORY_WORD_LENGTH" bits */
    return mask & ((memory.value));

}

/* getOpTypeId function returns the id of the addressing method of the operand */
int getOpTypeId(Operand op)
{
	/* Check if the operand have legal type */
	if (op.type != INVALID)
	{
		/* NUMBER = 0, LABEL = 1,RELATIVE_LABEL = 2 REGISTER = 3 */
		return (int)op.type;
	}
	return -1;
}

/* getCmdMemoryWord function returns a memory word which represents the Command in a line. */
MemoryWord getCmdMemoryWord(Line line)
{
	MemoryWord memory = {0 };

	if(getOpTypeId(line.op2) == -1)
    {
	    line.op2.type = 0;
    }

    if(getOpTypeId(line.op1) == -1)
    {
        line.op1.type = 0;
    }


	int mask = 0;
    mask >>= (sizeof(int) * BYTE_SIZE - MEMORY_WORD_LENGTH);

	mask += line.cmd->opcode;
	mask<<= 4;
	mask+= line.cmd->funct;
	mask<<= 2;
	mask+=getOpTypeId(line.op1);
	mask<<=2;
	mask+= getOpTypeId(line.op2);
	memory.value  = mask;


	return memory;
}

/* getOpMemoryWord function returns a memory word which represents the operand (assuming it's a valid operand). */
MemoryWord getOpMemoryWord(Operand op, bool isDest, int Relative_label_firstcmd)
{
	MemoryWord memory = {0 };

	if (op.type == NUMBER)
    {
	    memory.value = atoi(op.str);
    }

	if (op.type == REGISTER)
	{
		memory.are = (areType)ABSOLUTE; /* Registers are absolute */
        char *registr;
        ++op.str;
        registr = op.str;
		int i = atoi(registr);
		int j = 0;
		int mask = 1;

		while(j<i)
        {
		    mask <<=1;
		    j++;
        }

		memory.value = mask;
	}
	else if (op.type == RELATIVE_LABEL)
	{
        Label *label = getLabel(op.str);

        /* Set are */
        if ( label && label->isExtern)
        {
            memory.are = (areType)EXTERNAL;
        }
        else
        {
            /* DVIR REPAIR */

            if(isExistingEntryLabel(op.str))
            {
                memory.are = (areType)RELOCATABLE;
            }
        }

        /* check repair */
        memory.value = op.value - Relative_label_firstcmd + 1;

	}
	else if(isExistingLabel(op.str))
	{
		Label *label = getLabel(op.str);

		/* Set are */	
		if (op.type == LABEL && label && label->isExtern)
		{
			memory.are = (areType)EXTERNAL;
		}
		else
		{
			memory.are = (op.type == NUMBER) ? (areType)ABSOLUTE : (areType)RELOCATABLE;
		}

		/* check repair */
		memory.value = op.value;
	}

	return memory;
}

/* addWordToMemory function adds the value of memory word to the memoryArr, and increase the memory counter. */
void addWordToMemory(int *memoryArr, int* areArray, int *memoryCounter, MemoryWord memory)
{
	/* Check if memoryArr isn't full yet */
	if (*memoryCounter < MAX_DATA_LENGTH)
	{
		/* Add the memory word and increase memoryCounter */
		areArray[(*memoryCounter)] = memory.are;
		memoryArr[(*memoryCounter)++] = getNumFromMemoryWord(memory);
	}
}

/* addLineToMemory function adds a whole line into the memoryArr, and increase the memory counter. */
bool addLineToMemory(int *memoryArr, int *areArray, int *memoryCounter, Line *line)
{
	bool foundError = FALSE;
    int Relative_label_firstcmd;
	/* Don't do anything if the line is error or if it's not a Command line */
	if (!line->isError && line->cmd != NULL)
	{
		/* Update the label operands value */
		if (!updateLabelOpAddress(&line->op1, line->lineNum) || !updateLabelOpAddress(&line->op2, line->lineNum))
		{
			line->isError = TRUE;
			foundError = TRUE;
		}

		/* Add the Command word to the memory */
		addWordToMemory(memoryArr,areArray, memoryCounter, getCmdMemoryWord(*line));
        Relative_label_firstcmd = *memoryCounter + 100 + 1 ;
			/* Check if there is a source operand in this line */
			if (line->op1.type != INVALID)
			{
				/* Add the op1 word to the memory */
				line->op1.address = DEFAULT_ADDRESS + *memoryCounter;
				addWordToMemory(memoryArr,areArray, memoryCounter, getOpMemoryWord(line->op1, FALSE,Relative_label_firstcmd));
				/* ^^ The FALSE param means it's not the 2nd op */
			}

			/*Check if there is a destination operand in this line */
			if (line->op2.type != INVALID)
			{
				/* Add the op2 word to the memory */
				line->op2.address = DEFAULT_ADDRESS + *memoryCounter;
				addWordToMemory(memoryArr,areArray, memoryCounter, getOpMemoryWord(line->op2, TRUE,Relative_label_firstcmd));
				/* ^^ The TRUE param means it's the 2nd op */
			}
		}


	return !foundError;
}

/* addDataToMemory function adds the data from dataArray to the end of memoryArr. */
void addDataToMemory(int *memoryArr ,int *memoryCounter, int DC)
{
	int i;
	/* Create an int of "MEMORY_WORD_LENGTH" times '1', and all the rest are '0' */
	unsigned int mask = ~0;
	mask >>= (sizeof(int) * BYTE_SIZE - MEMORY_WORD_LENGTH);

	/* Add each int from dataArray to the end of memoryArr */
	for (i = 0; i < DC; i++)
	{
		if (*memoryCounter < MAX_DATA_LENGTH)
		{
			/* The mask makes sure we only use the first "MEMORY_WORD_LENGTH" bits */
			memoryArr[(*memoryCounter)++] = mask & dataArray[i];
		}
		else
		{
			/* No more space in memoryArr */
			return;
		}
	}
}

/* secondTransitionRead function reads the data from the file for the second time. */
/* It converts all the lines into the memory. */
int secondTransitionRead(int *memoryArr, int* areArr, Line *linesArr, int lineNum, int IC, int DC)
{
	int errorsFound = 0, memoryCounter = 0, i;

	/* Update the data labels */
	updateDataLabelsAddress(IC);

	/* Check if there are illegal entries */
	errorsFound += countIllegalEntries();

	/* Add each line in linesArr to the memoryArr */
	for (i = 0; i < lineNum; i++)
	{
		if (!addLineToMemory(memoryArr,areArr, &memoryCounter, &linesArr[i]))
		{
			/* An error was found while adding the line to the memory */
			errorsFound++;
		}
	}

	/* Add the data from dataArray to the end of memoryArr */
	addDataToMemory(memoryArr, &memoryCounter, DC);
	return errorsFound;
}
