// Systems Software Fall 2021
// Group 5 - Nicholas Gray, Abraham Hernandez, Prathik Ramesh, Ashley Voglewede

/*
	This is the lex.c file for the UCF Fall 2021 Systems Software Project.
	For HW2, you must implement the function lexeme *lexanalyzer(char *input).
	You may add as many constants, global variables, and support functions
	as you desire.

	If you choose to alter the printing functions or delete list or lex_index,
	you MUST make a note of that in you readme file, otherwise you will lose
	5 points.
*/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "compiler.h"
#define MAX_NUMBER_TOKENS 500
#define MAX_IDENT_LEN 11
#define MAX_NUMBER_LEN 5

lexeme *list;
int lex_index;

void printlexerror(int type);
void printtokens();
token_type isReserved(char *token);
token_type parseSpecialSymbols(char *buffer);
int isSpecialCharacter(char c);

lexeme *lexanalyzer(char *input)
{
	int i, code_len, in_comment, num_len, iden_len, possible_number, possible_word, possible_special_symbol, buffer_index;
	char buffer[12];
	in_comment = num_len = iden_len = possible_number = possible_word = buffer_index = possible_special_symbol = 0;

	// Initialize lexeme array
	list = (lexeme *)calloc(MAX_NUMBER_TOKENS, sizeof(lexeme));
	lex_index = 0;
	code_len = strlen(input);

	// Main lexing logic. 
	for (i = 0; i <= code_len; i++)
	{
		// Ignore if in a comment, until we hit a newline.
		if (in_comment)
		{
			if (input[i] == '\n')
			{
				in_comment = 0;
			}

			continue;
		}

		// Detected Comments
		if (input[i] == '/' && i < code_len && input[i + 1] == '/')
		{
			in_comment = 1;
			continue;
		}

		// Check for > and < symbols. This block is executed once we
		// know the possible special symbol cannot be >= or <=.
		if (possible_special_symbol && !isSpecialCharacter(input[i]))
		{
			buffer[buffer_index] = '\0';
			buffer_index = 0;

			lexeme current;
			token_type symbol_type = parseSpecialSymbols(buffer);
			if (symbol_type && (symbol_type == gtrsym || symbol_type == lsssym))
			{
				current.type = symbol_type;
				list[lex_index++] = current;
			}
			else
			{
				printlexerror(1);
				return NULL;
			}

			buffer[0] = '\0';
			possible_special_symbol = 0;
		}

		// Add either reserved word or identifier lexeme to list
		if (possible_word && !(isalpha(input[i]) || isdigit(input[i])))
		{
			buffer[buffer_index] = '\0';
			buffer_index = 0;

			lexeme current;
			token_type reservedValue = isReserved(buffer);
			if (reservedValue)
			{
				current.type = reservedValue;
				list[lex_index++] = current;
			}
			else
			{
				current.type = identsym;
				strcpy(current.name, buffer);
				list[lex_index++] = current;
			}

			buffer[0] = '\0';
			possible_word = 0;
			iden_len = 0;
		}

		// Add possible number to lexeme list.
		if (possible_number && !isdigit(input[i]))
		{
			// If a token began with a number, it cannot contain a letter.
			if (isalpha(input[i]))
			{
				printlexerror(2);
				return NULL;
			}

			buffer[buffer_index] = '\0';
			buffer_index = 0;

			lexeme current;
			current.type = numbersym;
			current.value = atoi(buffer);
			list[lex_index++] = current;

			buffer[0] = '\0';
			possible_number = 0;
			num_len = 0;
		}

		// Ignore whitespace.
		if (isspace(input[i]) || iscntrl(input[i]))
		{
			continue;
		}

		if (isSpecialCharacter(input[i]))
		{
			buffer[buffer_index++] = input[i];
			buffer[buffer_index] = '\0';
			possible_special_symbol = 1;

			token_type symbol_type = parseSpecialSymbols(buffer);

			// Tokens starting with a > or < symbol could lead to the >= and <=
			// symbols, so we move on to check the next character in these 2 cases.
			// Else, add the special symbol to the lexeme list.
			if (symbol_type && symbol_type != gtrsym && symbol_type != lsssym)
			{
				lexeme current;
				current.type = symbol_type;
				list[lex_index++] = current;
				buffer_index = 0;
				buffer[0] = '\0';
				possible_special_symbol = 0;
			}
		}

		// Mark the start of a possible number, if needed. 
		// Increment the possible number's length or the possible identifier's length.
		if (isdigit(input[i]))
		{
			buffer[buffer_index++] = input[i];
			if (!possible_word && !possible_special_symbol)
			{
				possible_number = 1;
			}
			else if (possible_word)
			{
				iden_len++;
			}

			if (possible_number)
			{
				num_len++;
			}

			if (num_len > MAX_NUMBER_LEN)
			{
				printlexerror(3);
				return NULL;
			}

			if (iden_len > MAX_IDENT_LEN)
			{
				printlexerror(4);
				return NULL;
			}
		}

		// Mark that the current lexeme is possibly an identiifer
		// and increment its length. 
		if (isalpha(input[i]))
		{
			if (!possible_number && !possible_special_symbol)
			{
				possible_word = 1;
				buffer[buffer_index++] = input[i];
				iden_len++;

				if (iden_len > MAX_IDENT_LEN)
				{
					printlexerror(4);
					return NULL;
				}
			}
		}
	}

	printtokens();
	return list;
}

// Return 0 for invalid symbols.
// For valid symbols, return the correct token value.
token_type isReserved(char *token)
{
	if (strcmp(token, "const") == 0)
	{
		return constsym;
	}
	else if (strcmp(token, "var") == 0)
	{
		return varsym;
	}
	else if (strcmp(token, "procedure") == 0)
	{
		return procsym;
	}
	else if (strcmp(token, "call") == 0)
	{
		return callsym;
	}
	else if (strcmp(token, "if") == 0)
	{
		return ifsym;
	}
	else if (strcmp(token, "then") == 0)
	{
		return thensym;
	}
	else if (strcmp(token, "else") == 0)
	{
		return elsesym;
	}
	else if (strcmp(token, "while") == 0)
	{
		return whilesym;
	}
	else if (strcmp(token, "do") == 0)
	{
		return dosym;
	}
	else if (strcmp(token, "begin") == 0)
	{
		return beginsym;
	}
	else if (strcmp(token, "end") == 0)
	{
		return endsym;
	}
	else if (strcmp(token, "read") == 0)
	{
		return readsym;
	}
	else if (strcmp(token, "write") == 0)
	{
		return writesym;
	}
	else if (strcmp(token, "odd") == 0)
	{
		return oddsym;
	}

	return 0;
}

// Identify special characters, to be lexed separately.
int isSpecialCharacter(char c)
{
	switch (c)
	{
	case '=':
	case '!':
	case '<':
	case '>':
	case '%':
	case '*':
	case '/':
	case '+':
	case '-':
	case '(':
	case ')':
	case ',':
	case '.':
	case ';':
	case ':':
		return 1;
	default:
		return 0;
	}
}

// Return 0 for invalid symbols.
// For valid symbols, return the correct token value.
token_type parseSpecialSymbols(char *buffer)
{
	if (strcmp(buffer, "==") == 0)
	{
		return eqlsym;
	}
	else if (strcmp(buffer, "!=") == 0)
	{
		return neqsym;
	}
	else if (strcmp(buffer, "<") == 0)
	{
		return lsssym;
	}
	else if (strcmp(buffer, "<=") == 0)
	{
		return leqsym;
	}
	else if (strcmp(buffer, ">") == 0)
	{
		return gtrsym;
	}
	else if (strcmp(buffer, ">=") == 0)
	{
		return geqsym;
	}
	else if (strcmp(buffer, "%") == 0)
	{
		return modsym;
	}
	else if (strcmp(buffer, "*") == 0)
	{
		return multsym;
	}
	else if (strcmp(buffer, "/") == 0)
	{
		return divsym;
	}
	else if (strcmp(buffer, "+") == 0)
	{
		return addsym;
	}
	else if (strcmp(buffer, "-") == 0)
	{
		return subsym;
	}
	else if (strcmp(buffer, "(") == 0)
	{
		return lparensym;
	}
	else if (strcmp(buffer, ")") == 0)
	{
		return rparensym;
	}
	else if (strcmp(buffer, ",") == 0)
	{
		return commasym;
	}
	else if (strcmp(buffer, ".") == 0)
	{
		return periodsym;
	}
	else if (strcmp(buffer, ";") == 0)
	{
		return semicolonsym;
	}
	else if (strcmp(buffer, ":=") == 0)
	{
		return assignsym;
	}
	else
	{
		return 0;
	}
}

void printtokens()
{
	int i;
	printf("Lexeme Table:\n");
	printf("lexeme\t\ttoken type\n");
	for (i = 0; i < lex_index; i++)
	{
		switch (list[i].type)
		{
		case oddsym:
			printf("%11s\t%d", "odd", oddsym);
			break;
		case eqlsym:
			printf("%11s\t%d", "==", eqlsym);
			break;
		case neqsym:
			printf("%11s\t%d", "!=", neqsym);
			break;
		case lsssym:
			printf("%11s\t%d", "<", lsssym);
			break;
		case leqsym:
			printf("%11s\t%d", "<=", leqsym);
			break;
		case gtrsym:
			printf("%11s\t%d", ">", gtrsym);
			break;
		case geqsym:
			printf("%11s\t%d", ">=", geqsym);
			break;
		case modsym:
			printf("%11s\t%d", "%", modsym);
			break;
		case multsym:
			printf("%11s\t%d", "*", multsym);
			break;
		case divsym:
			printf("%11s\t%d", "/", divsym);
			break;
		case addsym:
			printf("%11s\t%d", "+", addsym);
			break;
		case subsym:
			printf("%11s\t%d", "-", subsym);
			break;
		case lparensym:
			printf("%11s\t%d", "(", lparensym);
			break;
		case rparensym:
			printf("%11s\t%d", ")", rparensym);
			break;
		case commasym:
			printf("%11s\t%d", ",", commasym);
			break;
		case periodsym:
			printf("%11s\t%d", ".", periodsym);
			break;
		case semicolonsym:
			printf("%11s\t%d", ";", semicolonsym);
			break;
		case assignsym:
			printf("%11s\t%d", ":=", assignsym);
			break;
		case beginsym:
			printf("%11s\t%d", "begin", beginsym);
			break;
		case endsym:
			printf("%11s\t%d", "end", endsym);
			break;
		case ifsym:
			printf("%11s\t%d", "if", ifsym);
			break;
		case thensym:
			printf("%11s\t%d", "then", thensym);
			break;
		case elsesym:
			printf("%11s\t%d", "else", elsesym);
			break;
		case whilesym:
			printf("%11s\t%d", "while", whilesym);
			break;
		case dosym:
			printf("%11s\t%d", "do", dosym);
			break;
		case callsym:
			printf("%11s\t%d", "call", callsym);
			break;
		case writesym:
			printf("%11s\t%d", "write", writesym);
			break;
		case readsym:
			printf("%11s\t%d", "read", readsym);
			break;
		case constsym:
			printf("%11s\t%d", "const", constsym);
			break;
		case varsym:
			printf("%11s\t%d", "var", varsym);
			break;
		case procsym:
			printf("%11s\t%d", "procedure", procsym);
			break;
		case identsym:
			printf("%11s\t%d", list[i].name, identsym);
			break;
		case numbersym:
			printf("%11d\t%d", list[i].value, numbersym);
			break;
		}
		printf("\n");
	}
	printf("\n");
	printf("Token List:\n");
	for (i = 0; i < lex_index; i++)
	{
		if (list[i].type == numbersym)
			printf("%d %d ", numbersym, list[i].value);
		else if (list[i].type == identsym)
			printf("%d %s ", identsym, list[i].name);
		else
			printf("%d ", list[i].type);
	}
	printf("\n");
	list[lex_index++].type = -1;
}

void printlexerror(int type)
{
	if (type == 1)
		printf("Lexical Analyzer Error: Invalid Symbol\n");
	else if (type == 2)
		printf("Lexical Analyzer Error: Invalid Identifier\n");
	else if (type == 3)
		printf("Lexical Analyzer Error: Excessive Number Length\n");
	else if (type == 4)
		printf("Lexical Analyzer Error: Excessive Identifier Length\n");
	else
		printf("Implementation Error: Unrecognized Error Type\n");

	free(list);
	return;
}