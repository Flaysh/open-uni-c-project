#define _CRT_SECURE_NO_WARNINGS
/*
C Project 2021A - Assembler
Served by:
Itay Flaysher - 318378395
Maxim Voloshin - 327032991

secondPass.c - works with data and memory and rechecks for errors.
*/

#include "header.h"
#include <stdlib.h>

/* data from the first pass */
extern Line *entryLines[MAX_LABELS_NUM];
extern int entryLabelCount;
extern int dataArray[DATA_MAX_LENGTH];
extern const Command commandArray[];
extern Label labelsArray[MAX_LABELS_NUM];
extern int labelsCount;


/* ***** Methods ***** */


/* returns if there is an illegal entry line in entryLines. */
int countWrongEntries()
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
/* update the addresses in LabelsArray. */
void updateDataLabelsAddress(int IC)
{
    int i;
    /* Search in the array for label with isData flag */
    for (i = 0; i < labelsCount; i++)
    {
        if (labelsArray[i].isData)
        {
            /* Increase the label_address */
            labelsArray[i].label_address += IC;
        }
    }
}



/* check if the operand is a label. then update the value to the label_address. If error accrued - return false.*/
bool changeLabelOpAddress(Operand *op, int lineNum)
{
	if (op->type == LABEL || op->type == RELATIVE_LABEL) {
        Label *label = getLabel(op->str);
        /* Check if op.str is a real label label_name */
        if (label == NULL) {
            /* Print errors (legal label_name is illegal or not exists yet) */
            if (isLegalLabel(op->str, lineNum, TRUE)) {
                printError(lineNum, "No such label as \"%s\"", op->str);
            }
            return FALSE;
        }
        op->value = label->label_address;
    }
return TRUE;
}

/* return the value of the memory word */
int getMemoryWordNum(MemoryWord memory)
{
    /* Create an int of "MEMORY_WORD_LEN" times '1', and all the rest are '0' */
    unsigned int mask = ~0;
    mask >>= (sizeof(int) * BYTE_SIZE - MEMORY_WORD_LEN);

    /* The mask makes sure we only use the first "MEMORY_WORD_LEN" bits */
    return mask & ((memory.value));

}

/* returns the id of the method and the op */
int getOpTypeId(Operand op)
{
	/* Check if the operand have legal type */
	if (op.type != INVALID)
	{
		return (int)op.type;
	}
	return -1;
}

/* returns a memory word for a command. */
MemoryWord getCmdMemoryWord(Line line)
{
	MemoryWord memory = {0 };
    int mask = 0;

    if(getOpTypeId(line.op2) == -1)
    {
	    line.op2.type = 0;
    }

    if(getOpTypeId(line.op1) == -1)
    {
        line.op1.type = 0;
    }

    mask >>= (sizeof(int) * BYTE_SIZE - MEMORY_WORD_LEN);
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

/* Adds the data from data array to the end of memory array. */
void addDataToMemory(int *memoryArr ,int *memoryCounter, int DC)
{
    int i;
    /* "MEMORY_WORD_LEN" X '1', and all the rest are '0' */
    unsigned int temp = ~0;
    temp >>= (sizeof(int) * BYTE_SIZE - MEMORY_WORD_LEN);

    /* add the integers from data array to the memory array */
    for (i = 0; i < DC; i++)
    {
        if (*memoryCounter < DATA_MAX_LENGTH)
        {
            memoryArr[(*memoryCounter)++] = temp & dataArray[i];
        }
        else
        {
            /* No space left */
            return;
        }
    }
}

/* Returns a memory word which represents the operand (assuming it's a valid operand). */
MemoryWord getMemoryWordOp(Operand op, bool isDest, int Relative_label_firstcmd)
{
	MemoryWord memory = {0 };

	if (op.type == NUMBER)
    {
	    memory.value = atoi(op.str);
    }

	if (op.type == REGISTER)
	{
        char *registr;
        int i;
		int j = 0;
		int mask = 1;
        memory.are = (areType)ABSOLUTE; /* Registers are absolute */
        ++op.str;
        registr = op.str;
        i = atoi(registr);


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
            if(isExistingEntryLabel(op.str))
            {
                memory.are = (areType)RELOCATABLE;
            }
        }
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

/*  Adds the value of a memWord to the array and increases the counter*/
void addWordToMemory(int *memoryArr, int* areArray, int *memoryCounter, MemoryWord memory)
{
    /* check if the array is full */
	if (*memoryCounter < DATA_MAX_LENGTH)
	{
        /* add the word and increase the counter */
		areArray[(*memoryCounter)] = memory.are;
		memoryArr[(*memoryCounter)++] = getMemoryWordNum(memory);
	}
}

/* Adds a line to the memory array and add to counter*/
bool addLineToMemory(int *memoryArr, int *areArray, int *memoryCounter, Line *line)
{
	bool isError = FALSE;
    int RelativeLabelFirstCommand;
    /* check if the line is error or if it's not a Command line */
	if (!line->isError && line->cmd != NULL)
	{
        /* Update the operands */
		if (!changeLabelOpAddress(&line->op1, line->lineNum) || !changeLabelOpAddress(&line->op2, line->lineNum))
		{
			line->isError = TRUE;
            isError = TRUE;
		}

		/* Add the command word to the memory */
		addWordToMemory(memoryArr,areArray, memoryCounter, getCmdMemoryWord(*line));
        RelativeLabelFirstCommand = *memoryCounter + 101 ;
			/* Check if there is a source operand in this line */
			if (line->op1.type != INVALID)
			{
				/* Add the op1 word to the memory */
				line->op1.address = STARTING_ADDRESS + *memoryCounter;
				addWordToMemory(memoryArr, areArray, memoryCounter,
                                getMemoryWordOp(line->op1, FALSE, RelativeLabelFirstCommand));
			}

        /*Check for dest operand in line */
			if (line->op2.type != INVALID)
			{
                /* add the second operand to the memory */
				line->op2.address = STARTING_ADDRESS + *memoryCounter;
				addWordToMemory(memoryArr, areArray, memoryCounter,
                                getMemoryWordOp(line->op2, TRUE, RelativeLabelFirstCommand));
			}
		}
	return !isError;
}




/* reads the data from the file. */
/* converts all the lines into the memory. */
int secondPassRead(int *memoryArr, int* areArr, Line *linesArr, int lineNum, int IC, int DC)
{
	int errorsCount = 0, memoryCount = 0, i;

    /* Update data labels */
	updateDataLabelsAddress(IC);

    /* Check for wrong inputs*/
	errorsCount += countWrongEntries();

    /* moves the data from lines array to the memoryArr*/
	for (i = 0; i < lineNum; i++)
	{
		if (!addLineToMemory(memoryArr, areArr, &memoryCount, &linesArr[i]))
		{
            /* ERROR FOUND */
			errorsCount++;
		}
	}

    /* move data from dataArray to the end of memoryArr */
	addDataToMemory(memoryArr, &memoryCount, DC);
	return errorsCount;
}
