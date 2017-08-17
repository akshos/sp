/*
Author : Akshay Venugopal
Program Name : Lexical Analyzer
*/

#include <iostream>
#include <fstream>
#include <ctype.h>
#include <string.h>

#define KEYWORD_COUNT 18
#define SKIP -1 //to skip a line
#define KEY 0 //keyword
#define ID 1 //identifier
#define RELOP 2 //relational operator
#define ARTH 3 //arithmetic operator
#define SYM 4 //symbol
#define NUM 5 //number
#define LIT_STR 6 //string literal
#define LIT_CHR 7 //character literal 
#define LOG 8 //logical operator
#define ASSIGN 9 //assignment operator
#define BIN 10 //binary operator

using namespace std;

char keywords[KEYWORD_COUNT][20] = { "int", "char", "double", "float", "bool", "return",
			"using", "namespace", "if", "else", "do", "while", "for", "switch", "case", "default", "continue", "break" };		
char tokenType[][20] = { "KEY", "ID", "RELOP", "ARTH", "SYM", "NUM", "LIT_STR", "LIT_CHR", "LOG", "ASSIGN", "BIN" };
char token[100], inputSymbol;
bool eof = false;
ifstream input;

int openFile()
{
	input.open( "input" );
	if( !input )
	{
		cout << "Failed to open input file..." << endl;
		return 0;
	}
	return 1;
}

void forward() //read next input character and store in inputSymbol
{
	input.get( inputSymbol );		
	if( input.eof() )
	{
		inputSymbol = '\0';
		eof = true;
	}
}

int isKeyword( char lexeme[] )
{
	for( int i = 0; i < KEYWORD_COUNT; i++ )
	{
		if( strcmp( keywords[i], lexeme ) == 0 )
			return 1;
	}
	return 0;
}

int isDelim( char ch )
{
	switch( ch )
	{
		case ' ' : 
		case '\t' :
		case '\r' :
		case '\n' : return 1;
	}
	return 0;
}

void removeDelims() //returns the first character after delimiters
{
	while( isDelim( inputSymbol ) )
		forward();
}

int isSymbol( char ch )
{
	switch( ch )
	{
		case '{' :
		case '}' :
		case '.' :
		case ';' :
		case '[' :
		case ']' :
		case '(' :
		case ')' :
		case ',' :
		case ':' :
		case '\\' : return 1;
	}
	return 0;
}

int isArithmetic( char op )
{
	switch( op )
	{
		case '+' :
		case '-' :
		case '*' :
		case '/' :
		case '%' :
			return 1;
	}
	return 0;
}

int isRelop( char op )
{
	switch( op )
	{
		case '=' :
		case '>' :
		case '<' :
			return 1;
	}
	return 0;
}

int isLogical( char op )
{
	switch( op )
	{
		case '&' :
		case '|' :
		case '!' : return 1;
	}
	return 0;
}

int skipLine()
{
	while( inputSymbol != '\n' )
		forward();
	return SKIP;
}		

int checkKeyword()
{
	int n = 0;
	while( isalnum( inputSymbol ) || inputSymbol == '_' )
	{
		token[n++] = inputSymbol;
		forward();
	}
	token[n] = '\0';
	if( isKeyword( token ) )
		return KEY;
	return ID;
}

int number()
{
	int n = 0;
	while( isdigit( inputSymbol ) )
	{
		token[n++] = inputSymbol;
		forward();
	}
	token[n] = '\0';
	return NUM;
}

int stringLiteral()
{
	forward(); //skip the first quote
	int n = 0;
	while( inputSymbol != '"' )
	{
		token[n++] = inputSymbol;
		forward();
	}
	forward(); //skip the last quote
	token[n] = '\0';
	return LIT_STR;
}

int charLiteral()
{
	forward(); //skip the first quote
	token[0] = inputSymbol;
	forward(); //last quote
	forward(); //skip the last quote
	token[1] = '\0';
	return LIT_CHR;
}

int symbol()
{
	token[0] = inputSymbol;
	forward(); //next character
	token[1] = '\0';
	return SYM;
}

int arithmetic()
{
	int n = 0, type = ARTH;
	token[n++] = inputSymbol;
	forward();
	if( inputSymbol == '=' )
	{
		token[n++] = inputSymbol;
		forward();
	}
	if( inputSymbol == '/' ) //if pattern is // then it is comment
		type = skipLine();
	token[n] = '\0';
	return type;
}

int relop()
{
	int n = 0, type = RELOP;
	token[n++] = inputSymbol;
	forward();
	if( isRelop( inputSymbol ) )
	{
		token[n++] = inputSymbol;
		forward();
		if( ( token[0] == '<' && token[1] == '<' ) || ( token[0] == '>' && token[1] == '>' ) )
			type = BIN;
	}
	else if( token[0] == '=' )
		type = ASSIGN;
	token[n] = '\0';
	return type;
}

int logical()
{
	int n = 0, type = LOG;
	token[n++] = inputSymbol;
	forward();
	if( isLogical( inputSymbol ) ) //if pattern is && or ||
	{
		token[n++] = inputSymbol;
		forward();
	}
	else if( token[0] == '&' || token[0] == '|' ) //if it is a single & or | then binary operator
		type = BIN;
	token[n] = '\0';
	return type;
}

int getToken()
{
	int type;
	if( inputSymbol == '#' ) //pre-processor directive : skip line
		type = skipLine();
	else if( isalpha( inputSymbol ) || inputSymbol == '_' ) //keyword or identifier 
		type = checkKeyword();
	else if( isdigit( inputSymbol ) ) //number
		type = number();
	else if( inputSymbol == '"' ) //string literal
		type = stringLiteral();
	else if( inputSymbol == '\'' ) //character literal
		type = charLiteral();
	else if( isSymbol( inputSymbol ) ) //symbol
		type = symbol();
	else if( isArithmetic( inputSymbol ) ) //arithmetic operator or comment, if comment skip line
		type = arithmetic();
	else if( isRelop( inputSymbol ) ) //relational operator or assignment operator or binary operator
		type = relop();
	else if( isLogical( inputSymbol ) ) //logical operator or binary operator
		type = logical();
	return type;
}

void analyze()
{
	int count = 0, type;
	while( eof == false )
	{
		removeDelims(); //skip any delimiters
		if( inputSymbol == '\0' ) 
			return;
		type = getToken();
		if( type == SKIP )
			continue;
		cout << "< " << token << " , " << tokenType[type] << " > ";
		if( count == 5 )
		{
			cout << endl;
			count = 0;
		}
		count ++;
	}
	cout << endl;
}

int main()
{
	if( openFile() )
	{	
		forward(); //get the first input character
		analyze(); //start the analysis
	}
	return 0;
}

//--------------------------END OF PROGRAM-------------------------//


/*
Input :
#include<iostream>

using namespace std;

int main()
{
	char str[25];
	int a, b, sum;
	cout << "\n Enter a string :";
	cin >> str;
	cout << "\n String is "<< str;
	cout << "Enter two numbers : ";
	cin >> a >> b;
	sum = a + b;
	cout << "The sum is : " << sum;
	retutn 0;
}

Output :

< using , KEY > < namespace , KEY > < std , ID > < ; , SYM > < int , KEY > < main , ID >
< ( , SYM > < ) , SYM > < { , SYM > < char , KEY > < str , ID >
< [ , SYM > < 25 , NUM > < ] , SYM > < ; , SYM > < int , KEY >
< a , ID > < , , SYM > < b , ID > < , , SYM > < sum , ID >
< ; , SYM > < cout , ID > < << , BIN > < \n Enter a string : , LIT_STR > < ; , SYM >
< cin , ID > < >> , BIN > < str , ID > < ; , SYM > < cout , ID >
< << , BIN > < \n String is  , LIT_STR > < << , BIN > < str , ID > < ; , SYM >
< cout , ID > < << , BIN > < Enter two numbers :  , LIT_STR > < ; , SYM > < cin , ID >
< >> , BIN > < a , ID > < >> , BIN > < b , ID > < ; , SYM >
< sum , ID > < = , ASSIGN > < a , ID > < + , ARTH > < b , ID >
< ; , SYM > < cout , ID > < << , BIN > < The sum is :  , LIT_STR > < << , BIN >
< sum , ID > < ; , SYM > < retutn , ID > < 0 , NUM > < ; , SYM >
< } , SYM >
*/