/*
C Project 2021A - Assembler
By:
Itay Flaysher - 318378395
Maxim Voloshin - 327032991

utils.c - contains all the helpers functions.
*/

#include "header.h"
#include <ctype.h>
#include <stdlib.h>

extern const Command commandArray[];
extern Label labelsArray[];
extern int labelsCount;
extern Line *entryLines[MAX_LABELS_NUM];
extern int entryLabelCount;

/* If a certain label exists in the array It will return a pointer to the certain label, otherwise it will return NULL */
Label *getLabel(char *labelName)
{
	int i = 0;

	if (labelName)
	{
		for (i = 0; i < labelsCount; i++)
		{
			if (strcmp(labelName, labelsArray[i].name) == 0)
			{
				return &labelsArray[i];
			}
		}
	}
	return NULL;
}

/* If Command exists in the array, It will return the Command id, if not it will return -1*/
int getCommand(char *cmdName)
{
	int i = 0;

	while (commandArray[i].name)
	{
		if (strcmp(cmdName, commandArray[i].name) == 0)
		{
			return i;
		}

		i++;
	}
	return -1;
}

/* Trim space from the beginning of the string */
void trimLeftString(char **ptStr)
{
	/* Return if it's NULL */
	if (!ptStr)
	{
		return;
	}

	/* Get ptStr to the start of the actual text */
	while (isspace(**ptStr))
	{
		++*ptStr;
	}
}

/* Delete all the spaces from the edges of the string ptStr is pointing to */
void trimString(char **ptStr)
{
	char *endofstring;

	/* Return if it's NULL or empty string */
	if (!ptStr || **ptStr == '\0')
	{
		return;
	}

    trimLeftString(ptStr);

	/* endofstring is pointing to the last char in str, before '\0' */
	endofstring = *ptStr + strlen(*ptStr) - 1;

	/* Remove spces from the end */
	while (isspace(*endofstring) && endofstring != *ptStr)
	{
		*endofstring-- = '\0';
	}
}

/* Returns a pointer to the start of the first token. And makes *endOfTok to point at the last char after the token. */
char *getToken(char *str, char **endOfTok)

{
	char *tokStart = str;
	char *tokEnd = NULL;

	/* Trim the start */
    trimLeftString(&tokStart);

	/* Find the end of the first word */
	tokEnd = tokStart;
	while (*tokEnd != '\0' && !isspace(*tokEnd))
	{
		tokEnd++;
	}

	/* Add \0 at the end if needed */
	if (*tokEnd != '\0')
	{
		*tokEnd = '\0';
		tokEnd++;
	}

	/* Make *endOfTok (if it's not NULL) to point at the last char after the token */
	if (endOfTok)
	{
		*endOfTok = tokEnd;
	}
	return tokStart;
}

/* Returns if string contains only one word. */
bool isOneWord(char *str)
{
    trimLeftString(&str);/* Skip the spaces at the start */
	while (!isspace(*str) && *str) /* Skip the text at the middle */	
 	{
	 str++;
	}					
	/* Return if it's the end of the text or not. */
	return isWhiteSpace(str);
}

/* Returns if string contains only white chars. */
bool isWhiteSpace(char *str)
{
	while (*str)
	{
		if (!isspace(*str++))
		{
			return FALSE;
		}
	}
	return TRUE;
}

/* Returns if labelStr is a legal label name.*/
bool isLegalLabel(char *labelStr, int lineNum, bool printErrors)
{
	int labelLength = strlen(labelStr), i;

	/* Check if the label is at the correct eligable length */
	if (strlen(labelStr) > MAX_LABEL_LENGTH)
	{
		if (printErrors) printError(lineNum, "Label is too long. Max label length is %d.", MAX_LABEL_LENGTH);
		return FALSE;
	}

	/* Check if the label isn't an empty string */
	if (*labelStr == '\0')
	{
		if (printErrors) printError(lineNum, "Label is empty.");
		return FALSE;
	}

	/* Check if the 1st char of the label is a letter. */
	if (isspace(*labelStr))
	{
		if (printErrors) printError(lineNum, "Label must start at the beginning of the line.");
		return FALSE;
	}

	/* Check if the label consists numbers only. */
	for (i = 1; i < labelLength; i++)
	{
		if (!isalnum(labelStr[i]))
		{
			if (printErrors) printError(lineNum, "\"%s\" is illegal label - use numbers and letters only.", labelStr);
			return FALSE;
		}
	}

	/* Check if the label 1st char is a letter. */
	if (!isalpha(*labelStr))
	{
		if (printErrors) printError(lineNum, "\"%s\" is illegal label - first char must be a letter.", labelStr);
		return FALSE;
	}

	/* Check if it's not a name of a register */
	if (isRegister(labelStr, NULL)) /* NULL since we don't have to save the register number */
	{
		if (printErrors) printError(lineNum, "\"%s\" is illegal label - don't use a name of a register.", labelStr);
		return FALSE;
	}
	


	/* Check if it's not a name of a Command */
	if (getCommand(labelStr) != -1)
	{
		if (printErrors) printError(lineNum, "\"%s\" is illegal label - don't use a name of Command.", labelStr);
		return FALSE;
	}

	return TRUE;
}

/* isExistingLabel function returns if the label exists. */
bool isExistingLabel(char *label)
{
	if (getLabel(label))
	{
		return TRUE;
	}

	return FALSE;
}

/* iReturn if the label is already in the entry lines array. */
bool isExistingEntryLabel(char *labelName)
{
	int i = 0;

	if (labelName)
	{
		for (i = 0; i < entryLabelCount; i++)
		{
			if (strcmp(labelName, entryLines[i]->lineStr) == 0)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

/* Returns if str is a register name, and update value to be the register value. */
bool isRegister(char *str, int *value)
{
	if (str[0] == 'r'  && str[1] >= '0' && str[1] - '0' <= MAX_REGISTER_DIGIT && str[2] == '\0') 
	{
		/* Update value if it's not NULL */
		if (value)
		{
			*value = str[1] - '0'; /* -'0' To get the actual number the char represents */
		}
		return TRUE;
	}

	return FALSE;
}

/* Returns a bool, represent whether 'line' is a comment or not. */
/* If the first char is ';' but it's not at the start of the line, it returns true and update line->isError to be TRUE. */
bool isEmpty(Line *line)
{
	char *startOfText = line->lineStr; /* We don't want to change line->lineStr */

	if (*line->lineStr == ';')
	{
		/* Comment */
		return TRUE;
	}

    trimLeftString(&startOfText);
	if (*startOfText == '\0')
	{
		/* Empty line */
		return TRUE;
	}
	if (*startOfText == ';')
	{
		/* Illegal comment - ';' isn't at the start of the line */
		printError(line->lineNum, "Comments need to start with ';' at the beginning of the line.");
		line->isError = TRUE;
		return TRUE;
	}

	/* Not empty or comment */
	return FALSE;
}

/* Returns a pointer to the start of the first operand in 'line' and change the end of it to '\0'. */
/* Also makes *endOfOp (if it's not NULL) point at the next char after the operand. */
char *getFirstOperand(char *line, char **endOfOp, bool *foundComma)
{
	if (!isWhiteSpace(line))
	{
		/* Find the first comma */
		char *end = strchr(line, ',');
		if (end)
		{
			*foundComma = TRUE;
			*end = '\0';
			end++;
		}
		else
		{
			*foundComma = FALSE;
		}

		/* Set endOfOp (if it's not NULL) to point at the next char after the operand */
		if (endOfOp)
		{
			if (end)
			{
				*endOfOp = end;
			}
			else
			{
				*endOfOp = strchr(line, '\0');
			}
		}
	}

    trimString(&line);
	return line;
}

/* Returns if the cmd is a Directive. */
bool isDirective(char *cmd)
{
	return (*cmd == '.') ? TRUE : FALSE;
}

/* Returns if the strParam is a legal string param (enclosed in quotes), and remove the quotes. */
bool isLegalStringParam(char **strParam, int lineNum)
{
	/* check if the string param is enclosed in quotes */
	if ((*strParam)[0] == '"' && (*strParam)[strlen(*strParam) - 1] == '"')
	{
		/* remove the quotes */
		(*strParam)[strlen(*strParam) - 1] = '\0';
		++*strParam;
		return TRUE;
	}

	if (**strParam == '\0')
	{
		printError(lineNum, "No parameter.");
	}
	else
	{
		printError(lineNum, "The parameter for .string must be inside a quotes.");
	}
	return FALSE;
}

/* Returns if the num is a legal number param, and save it's value in *value. */
bool isLegalNumber(char *numStr, int numOfBits, int lineNum, int *value)
{
	char *endOfNum;
	/* maxNum is the max number you can represent with (MAX_LABEL_LENGTH - 1) bits 
	 (-1 for the negative/positive bit) */
	int maxNum = (1 << (numOfBits - 1)) - 1;
	if (isWhiteSpace(numStr))
	{
		printError(lineNum, "Empty parameter.");
		return FALSE;
	}

	*value = strtol(numStr, &endOfNum, 0);


	/* Check if endOfNum is at the end of the string */
	if (*endOfNum)
	{
		printError(lineNum, "\"%s\" is not a valid number.", numStr);
		return FALSE;
	}

    /* Check if the number is small enough to fit into 1 memory word */
	if (*value > maxNum || *value < -maxNum)
	{
		printError(lineNum, "\"%s\" is too %s, must be between %d and %d.", numStr, (*value > 0) ? "big" : "small", -maxNum, maxNum);
		return FALSE;
	}
	return TRUE;
}
