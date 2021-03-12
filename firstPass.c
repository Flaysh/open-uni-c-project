/*
C Project 2021A - Assembler
By:
Itay Flaysher - 318378395
Maxim Voloshin - 327032991

firstPass.c - parse the given assembler file and save data from it in structures, also find errors if needed.
*/

#include "header.h"
#include <ctype.h>
#include <stdlib.h>

/* ****** Directives List ****** */
void parseDataDirective(Line *line, int *IC, int *DC);
void parseStringDirective(Line *line, int *IC, int *DC);
void parseExternDirective(Line *line);
void parseEntryDirective(Line *line);

const Directive directiveArray[] =
{	/* Name | Parsing Function */
	{ "data",   parseDataDirective } ,
	{ "string", parseStringDirective } ,
	{ "extern", parseExternDirective },
	{ "entry",  parseEntryDirective },
    { NULL } /* end of the array */
};

/* ******Commands List ****** */
const Command commandArray[] =
{	/* Name | Opcode | paramsCount | funct  */
	{ "mov", 0, 2 ,0} ,
	{ "cmp", 1, 2 ,0} ,
	{ "add", 2, 2 ,10} ,
	{ "sub", 2, 2,11 } ,
	{ "lea", 4, 2 ,0} ,
	{ "clr", 5, 1,10 } ,
	{ "not", 5, 1 ,11} ,
	{ "inc", 5, 1 ,12} ,
	{ "dec", 5, 1,13 } ,
	{ "jmp", 9, 1,10 } ,
	{ "bne", 9, 1 ,11} ,
	{ "red", 12, 1,0} ,
	{ "prn", 13, 1 ,0} ,
	{ "jsr", 9, 1,12 } ,
	{ "rts", 14, 0 ,0} ,
	{ "stop", 15, 0 ,0} ,
    { NULL } /* end of the array */
}; 

extern Label labelsArray[MAX_LABELS_NUM];
extern int labelsCount;
extern Line *entryLines[MAX_LABELS_NUM];
extern int entryLabelCount;
extern int dataArray[MAX_DATA_LENGTH];

/* Adds the given label to the labelArr and increases labelNum. Returns a pointer to the label in the array. */
Label *addLabelToArray(Label label, Line *line)
{
	/* Check if label is legal */
	if (!isLegalLabel(line->lineStr, line->lineNum, TRUE))
	{
		/* Illegal label name */
		line->isError = TRUE;
		return NULL;
	}

	/* Check if label already exists */
	if (isExistingLabel(line->lineStr))
	{
		printError(line->lineNum, "Label already exists.");
		line->isError = TRUE;
		return NULL;
	}

	/* Add the name to the label */
	strcpy(label.name, line->lineStr);

	/* Add the label to labelsArray and to the Line */
	if (labelsCount < MAX_LABELS_NUM)
	{
        labelsArray[labelsCount] = label;
		return &labelsArray[labelsCount++];
	}

    /* check if theres too many labels */
    printError(line->lineNum, "Too many labels: you cant do more then %d.", MAX_LABELS_NUM, TRUE);
	line->isError = TRUE;
	return NULL;
}

/* Adds the given number to the dataArray and increases DC. Returns if it succeeded. */
bool addNumberToData(int num, int *IC, int *DC, int lineNum)
{
	/* Check if there is enough space in dataArray for the data */
	if (*DC + *IC < MAX_DATA_LENGTH)
	{
        dataArray[(*DC)++] = num;
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

/* Adds the given str to the dataArray and increases DC. Returns if it succeeded. */
bool addStringToData(char *str, int *IC, int *DC, int lineNum)
{
	do
	{
		if (!addNumberToData((int)*str, IC, DC, lineNum))
		{
			return FALSE;
		}
	} while (*str++);

	return TRUE;
}

/* finds the label in line->lineStr and add it to the label list. */
/* Returns a pointer to the next char after the label, or NULL is there isn't a legal label. */
char *findLabel(Line *line, int IC)
{
	char *labelEnd = strchr(line->lineStr, ':');
	Label label = {0};
	label.address = DEFAULT_ADDRESS + IC;

    /* Find the label (or return NULL if there's none) */
	if (!labelEnd)
	{
		return NULL;
	}
	*labelEnd = '\0';

    /* Check if the ':' came after the first word */
	if (!isOneWord(line->lineStr))
	{
		*labelEnd = ':'; /* Fix the change in line->lineStr */
		return NULL;
	}

    /* Check of the label is legal and add it to the labelList */
	line->label = addLabelToArray(label, line);
	return labelEnd + 1; /* +1 to make it point at the next char after the \0 */
}

/* remove the last label in labelArr and updating labelsCount. */
/* removing the label from a entry/extern line. */
void removeLastLabel(int lineNum)
{
	labelsCount--;
    printf("WARNING: At line %d: Assembler ignored label before the Directive.\n", lineNum);
}

/* Parses a .data Directive. */
void parseDataDirective(Line *line, int *IC, int *DC)
{
	char *operandTok = line->lineStr, *endOfOp = line->lineStr;
	int operandValue;
	bool foundComma;

	/* Make the label a data label (is there is one) */
	if (line->label)
	{
		line->label->isData = TRUE;
		line->label->address = DEFAULT_ADDRESS + *DC;
	}

	/* Check if there are params */
	if (isWhiteSpace(line->lineStr))
	{
		/* No parameters */
		printError(line->lineNum, "No parameter.");
		line->isError = TRUE;
		return;
	}

	/* Find all the params and add them to dataArray */
	INFINITE_LOOP
	{
		/* Get next param or break if there isn't */
		if (isWhiteSpace(line->lineStr))
		{
			break;
		}
		operandTok = getFirstOperand(line->lineStr, &endOfOp, &foundComma);

		/* Add the param to dataArray */
		if (isLegalNumber(operandTok, MEMORY_WORD_LENGTH, line->lineNum, &operandValue))
		{
			if (!addNumberToData(operandValue, IC, DC, line->lineNum))
			{
				/* Not enough memory */
				line->isError = TRUE;
				return;
			}
		}
		else
		{
			/* Illegal number */
			line->isError = TRUE;
			return;
		}

		/* Change the line to start after the parameter */
		line->lineStr = endOfOp;
	}

	if (foundComma)
	{
		/* Comma after the last param */
		printError(line->lineNum, "Do not write a comma after the last parameter.");
		line->isError = TRUE;
		return;
	}
}

/* parses a .string Directive. */
void parseStringDirective(Line *line, int *IC, int *DC)
{
	/* Make the label a data label (if there is one) */
	if (line->label)
	{
		line->label->isData = TRUE;
		line->label->address = DEFAULT_ADDRESS + *DC;
	}

    trimString(&line->lineStr);

	if (isLegalStringParam(&line->lineStr, line->lineNum))
	{
		if (!addStringToData(line->lineStr, IC, DC, line->lineNum))
		{
			/* Not enough memory */
			line->isError = TRUE;
			return;
		}
	}
	else
	{
		/* Illegal string */
		line->isError = TRUE;
		return;
	}
}

/* parses a .extern Directive. */
void parseExternDirective(Line *line)
{
	Label label = {0 }, *labelPointer;

	/* If there is a label in the line, remove the it from labelArr */
	if (line->label)
	{
		removeLastLabel(line->lineNum);
	}

    trimString(&line->lineStr);
	labelPointer = addLabelToArray(label, line);

	/* Make the label an extern label */
	if (!line->isError)
	{
		labelPointer->address = 0;
		labelPointer->isExtern = TRUE;
	}
}

/* parses a .entry Directive. */
void parseEntryDirective(Line *line)
{
	/* If there is a label in the line, remove the it from labelArr */
	if (line->label)
	{
		removeLastLabel(line->lineNum);
	}

	/* Add the label to the entry labels list */
    trimString(&line->lineStr);

	if (isLegalLabel(line->lineStr, line->lineNum, TRUE))
	{
		if (isExistingEntryLabel(line->lineStr))
		{
            printError(line->lineNum, "Label is already an entry label.");
			line->isError = TRUE;
		}
		else if (entryLabelCount < MAX_LABELS_NUM)
		{
            entryLines[entryLabelCount++] = line;
		}
	}
}

/* parses the Directive and in a Directive line. */
void parseDirective(Line *line, int *IC, int *DC)
{
	int i = 0;
	while (directiveArray[i].name)
	{
		if (!strcmp(line->commandStr, directiveArray[i].name))
		{
			/* Call the parse function for this type of Directive */
			directiveArray[i].parseFunc(line, IC, DC);
			return;
		}
		i++;
	}
	
	/* line->commandStr isn't a real Directive */
    printError(line->lineNum, "\"%s\" is not directive.", line->commandStr);
	line->isError = TRUE;
}

/* returns if the operands' types are legal (depending on the Command). */
bool isLegalOpTypes(const Command *cmd, Operand op1, Operand op2, int lineNum)
{
    /* Check First Operand */
    if (cmd->opcode == 4 && op1.type != LABEL)
    {
        printError(lineNum, "Source operand for \"%s\" Command must be a label.", cmd->name);
        return FALSE;
    }

    /* Check Second Operand */
    if (op2.type == NUMBER && cmd->opcode != 1 && cmd->opcode != 13)
    {
        printError(lineNum, "Destination operand for \"%s\" Command can't be a number.", cmd->name);
        return FALSE;
    }

    return TRUE;
}

/* parseOpInfo function updates the type and value of operand. */
void parseOpInfo(Operand *operand, int lineNum)
{
	int value = 0;

	/* DVIR REPAIR */
	operand->type = NUMBER;

	if (isWhiteSpace(operand->str))
	{
		printError(lineNum, "Empty parameter.");
		operand->type = INVALID;
		return;
	}

    /* set the operand type */
    if (isLegalLabel(operand->str, lineNum, FALSE)){
        operand->type = LABEL;
    }

	/* Check the type is NUMBER */
	if (*operand->str == '#'){
		operand->str++; /* Remove the '#' */

		/* Check if the number is legal */
		if (isspace(*operand->str))
		{
            printError(lineNum, "White space after '#'.");
			operand->type = INVALID;
		}
		else
		{
			operand->type = isLegalNumber(operand->str, MEMORY_WORD_LENGTH, lineNum, &value) ? NUMBER : INVALID;
		}
	 }
	/* Check if the type is REGISTER */
	else if (isRegister(operand->str, &value))
    {
        operand->type = REGISTER;

    }

	/* Check if the type is LABEL */
	else if (isLegalLabel(operand->str, lineNum, FALSE))
    {
        operand->type = LABEL;

    }
	else if(*operand->str == '%') {
            operand->str++;
            operand->type = RELATIVE_LABEL;
        }

	/* The type is INVALID */
	else
	{
		printError(lineNum, "\"%s\" is an invalid parameter.", operand->str);
		operand->type = INVALID;
		value = -1;
	}
	}

/* parses the operands in a Command line. */
void parseCmdOperands(Line *line, int *IC, int *DC)
{
	char *startOfNextPart = line->lineStr;
	bool foundComma = FALSE;
	int numOfOpsFound = 0;

	/* Reset the op types */
	line->op1.type = INVALID;
	line->op2.type = INVALID;
	/* Get the parameters */
	INFINITE_LOOP
	{

		/* Check if there are still more operands to read */
		if (isWhiteSpace(line->lineStr) || numOfOpsFound > 2)
		{
			/* If there are more than 2 operands it's already illegal */
			break;
		}

		/* If there are 1 ops, make the destination become the source op */
		if (numOfOpsFound == 1)
		{
			line->op1 = line->op2;
			/* Reset op2 */
			line->op2.type = INVALID;
		}

		/* Parse the opernad*/
		line->op2.str = getFirstOperand(line->lineStr, &startOfNextPart, &foundComma);
		parseOpInfo(&line->op2, line->lineNum);

		if (line->op2.type == INVALID)
		{
			line->isError = TRUE;
			return;
		}

		numOfOpsFound++;
        if (*IC + *DC < MAX_DATA_LENGTH)
        {
            *IC = *IC + 1; /* Count the last Command word or operand. */
        }

		line->lineStr = startOfNextPart;
	} /* End of while */

	/* Check if there are enough operands */
	if (numOfOpsFound != line->cmd->numOfParams) 
	{
		/* There are more/less operands than needed */
		if (numOfOpsFound <  line->cmd->numOfParams)
		{
			printError(line->lineNum, "Not enough operands.", line->commandStr);
		}
		else
		{
			printError(line->lineNum, "Too many operands.", line->commandStr);
		}

		line->isError = TRUE;
		return;
	}

	/* Check if there is a comma after the last param */
	if (foundComma)
	{
        printError(line->lineNum, "Comma after the last parameter.");
		line->isError = TRUE;
		return;
	}
	/* Check if the operands' types are legal */
	if (!isLegalOpTypes(line->cmd, line->op1, line->op2, line->lineNum))
	{
		line->isError = TRUE;
		return;
	}
}

/* Parses the Command in a Command line. */
void parseCommand(Line *line, int *IC, int *DC)
{

	int cmdId = getCommand(line->commandStr);

	if (cmdId == -1)
	{
		line->cmd = NULL;
		if (*line->commandStr == '\0')
		{
			/* The Command is empty, but the line isn't empty so it's only a label. */
			printError(line->lineNum, "Can't write a label to an empty line.", line->commandStr);
		}
		else
		{
			/* Illegal Command. */
            printError(line->lineNum, "There's no \"%s\" Command.", line->commandStr);
		}
		line->isError = TRUE;
		return;
	}


	line->cmd = &commandArray[cmdId];
	*IC = *IC + 1;
	parseCmdOperands(line, IC, DC);
}

/* Returns the same string in a different part of the memory by using malloc. */
char *allocString(const char *str) 
{
	char *newString = (char *)malloc(strlen(str) + 1);
	if (newString) 
	{
		strcpy(newString, str); 
	}

	return newString;
}

/* Parses a line, and print errors if needed */
void parseLine(Line *line, char *lineStr, int lineNum, int *IC, int *DC)
{
	char *startOfNextPart = lineStr;

	line->lineNum = lineNum;
	line->address = DEFAULT_ADDRESS + *IC;
	line->originalString = allocString(lineStr);
	line->lineStr = line->originalString;
	line->isError = FALSE;
	line->label = NULL;
	line->commandStr = NULL;
	line->cmd = NULL;

	if (!line->originalString)
	{
        printf("ERROR: Memory exceeded - malloc failed.");
		return;
	}

	/* Check if the line is a comment */
	if (isEmpty(line))
	{	
		return;
	}

	/* Find label and add it to the label list */
	startOfNextPart = findLabel(line, *IC);
	if (line->isError)
	{
		return;
	}
	/* Update the line if startOfNextPart isn't NULL */
	if (startOfNextPart)
	{
		line->lineStr = startOfNextPart;
	}

	/* Find the Command token */
	line->commandStr = getToken(line->lineStr, &startOfNextPart);
	line->lineStr = startOfNextPart;
	/* Parse the Command / Directive */
	if (isDirective(line->commandStr))
	{
		line->commandStr++; /* Remove the '.' from the Command */
		parseDirective(line, IC, DC);
	}
	else
	{
		parseCommand(line, IC, DC);
	}


	if (line->isError)
	{
		return;
	}
}

/* Puts a line from file in buffer.  Continue if the line is shorter than maxLength. */
bool readLine(FILE *file, char *buf, size_t maxLength)
{
	char *endOfLine;

	if (!fgets(buf, maxLength, file))
	{
		return FALSE;
	}

    /* Check if the line is too long */
	endOfLine = strchr(buf, '\n');
	if (endOfLine)
	{
		*endOfLine = '\0';
	}
	else
	{
		char c;
        /* Return FALSE, unless it's the end of the file */
        bool ret = (feof(file)) ? TRUE : FALSE;

		/* Keep reading chars until you reach the end of the line ('\n') or EOF */
		do
		{
			c = fgetc(file);
		} while (c != '\n' && c != EOF);

		return ret;
	}

	return TRUE;
}

/* Reads the file and parses it, Returns how many errors were found */
int firstPassRead(FILE *file, Line *linesArr, int *lines, int *IC, int *DC)
{
	char lineStr[MAX_LINE_LENGTH + 2]; /* +2 for the \n and \0 at the end */
	int errorsFound = 0;
	*lines = 0;

	/* Read lines and parse them */
	while (!feof(file))
	{
		if (readLine(file, lineStr, MAX_LINE_LENGTH + 2))
		{
			/* Check if the file is too lone */
			if (*lines >= MAX_LINES_NUM)
			{
                printf("ERROR: File is too long. Max lines in file is %d.\n", MAX_LINES_NUM);
				return ++errorsFound;
			}

			/* Parse a line */
			parseLine(&linesArr[*lines], lineStr, *lines + 1, IC, DC);

			/* Update errorsFound */
			if (linesArr[*lines].isError)
			{
				errorsFound++;
			}

			/* Check if the number of memory words needed is small enough */
			if (*IC + *DC >= MAX_DATA_LENGTH)
			{
				/* dataArr is full. Stop reading the file. */
                printError(*lines + 1, "Too much data. Max memory words is %d.", MAX_DATA_LENGTH);
                printf("Memory is full. File reading stopped.\n");
				return ++errorsFound;
			}
			++*lines;
		}
		else if (!feof(file))
		{
			/* Line is too long */
            printError(*lines + 1, "Line is too long. Max line length is %d.", MAX_LINE_LENGTH);
			errorsFound++;
			 ++*lines;
		}
	}
	return errorsFound;
}
