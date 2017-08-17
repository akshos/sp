#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <fstream>

using namespace std;
	
struct NAMTAB
{
	char name[30];
	int start, end;
}namtab[50];

//DEFTAB is a file and ARGTAB is a string array

fstream deftab( "deftab", ios::in|ios::out|ios::trunc );
ifstream input( "input" );
ofstream output( "output" );

char argtab[20][30];
char line[100], label[30], opcode[30], operand[30];
bool expanding;
int namtab_count = 0, argtab_count = 0;

int searchNamtab( char name[] ) //return the index of the name in namtab or -1 if not found
{
	for( int i = 0; i < namtab_count; i++ )
	{
		if( strcmp( namtab[i].name, name ) == 0 )
			return i;
	}
	return -1;
}

int searchArgtab( char arg[] ) //returns the index of arg in argtab else -1 if not found
{ 
	for( int i = 0; i < argtab_count ; i++ )
	{
		if( strcmp( argtab[i] , arg ) == 0 )
			return i;
	}
	return -1;
}

void makeArgtab( char operand[] )
{
	int i, n;
	char arg[20];
	for( i = 0; operand[i] != '\0'; i++ )
	{
		for( n = 0; operand[i] != ',' && operand[i] != '\0' && operand[i] != '\r'; n++, i++ )
			arg[n] = operand[i];
		arg[n] = '\0';
		strcpy( argtab[argtab_count], arg );
		argtab_count++;
	}
}

void writeExpanded() //write to expanded file
{
	output << label << "\t" << opcode << "\t" << operand << endl;
}

void insertNamtab( char name[], int start, int end ) //insert a name to namtab with start and end positions in deftab
{
	strcpy( namtab[namtab_count].name, name );
	namtab[namtab_count].start = start;
	namtab[namtab_count].end = end;
	namtab_count++;
}

int delim( char ch ) //check if ch is a delimitter character
{
	switch( ch )
	{
		case ' ' :
		case '\n' :
		case '\r' :
		case '\t' : return 1;
	}
	return 0;
}

void define();
void expand( int );
void getLine();

void extract( char token[], int &pos ) //extract a token from line
{
	int i = 0;
	while( delim( line[pos] ) && line[pos] != '\0' )
		pos++;
	token[i++] = line[pos++];
	while( !delim( line[pos] ) && line[pos] != '\0' )
		token[i++] = line[pos++];
	token[i] = '\0';
}

void getTokens() //extract label opcode and operand from line and check for no label
{
	int pos = 0;
	extract( label, pos );
	extract( opcode, pos );
	extract( operand, pos );
	if( operand[0] == '\0' ) // there was no label in input line
	{
		strcpy( operand, opcode );
		strcpy( opcode, label );
		strcpy( label, "\0" );
	}
}

void insertArguments()
{
	char ch;
	int pos; 
	if( operand[0] == '$' )
	{
		ch = operand[1];
		pos = (int)( (int)ch - (int)'0' );
		strcpy( operand, argtab[pos] );
	}
}


void processLine()
{
	int index = searchNamtab( opcode ); //search for opcode in namtab
	if( index != -1 ) //if the input line is macro invocation
		expand( index );
	else if( strcmp( opcode, "MACRO" ) == 0 ) //if the input line is macro definition
		define();
	else //normal input line
		writeExpanded(); //copy to expanded without processing
		
}

void define()
{
	char macroName[30], pos[5];
	int level = 1, start, end, index;
	strcpy( macroName, label ); //store the macro name
	start = deftab.tellp(); //get the starting point of the macro in deftab
	argtab_count = 0; 
	deftab << label << "\t" << opcode << "\t" << operand << endl; //store macro definition line in deftab
	argtab_count = 0; //reset the argument table
	makeArgtab( operand ); //split operand into arguments
	while( level > 0 )
	{
		getLine(); //get the next line
		getTokens();
		index = searchArgtab( operand );
		if( index != -1 ) //insert positional notation for parameters
		{
			strcpy( operand, "$" );
			pos[0] = (char)( (int)'0' + index );
			pos[1] = '\0';
			strcat( operand, pos );
		}
		if( strcmp( opcode, "MACRO" ) == 0 ) //for multi level macros
		{
			level = level + 1;
		}
		else if( strcmp( opcode, "MEND" ) == 0 )
		{
			level = level - 1;
		}
		if( level == 0 )
			break;
		deftab << label << "\t" << opcode << "\t" << operand << endl; //write line to deftab
	}
	end = deftab.tellp(); //get the end position of the macro in deftab
	insertNamtab( macroName, start, end ); //insert thr macro name and start and end positions in namtab
	//cout << "Define end " << opcode << "\t" << operand << endl;
}

void expand( int index )
{
	//return;
	int start, end, cur;
	start = namtab[index].start;
	end = namtab[index].end;
	cur = deftab.tellp();
	deftab.seekg( start );
	//cout << "\nExpanding " << opcode << "\t" << operand << endl;
	expanding = true;
	argtab_count = 0; //reset the arument table
	makeArgtab( operand ); //set up arguments from macro invocation in ARGTAB
	output << "# " ;
	writeExpanded();
	getLine(); //get first line of macro definition from deftab
	while( deftab.tellg() < end )
	{
		getLine();
		processLine();
	}
	deftab.seekp( cur );
	expanding = false;
	return;
}

void getLine()
{
	if( expanding == true )
	{
		deftab.getline( line, 100, '\n' );
		getTokens(); //separate the label opcocde and operand
		insertArguments();
	}
	else
	{
		input.getline( line, 100, '\n' );
		getTokens(); //separate the label opcocde and operand
	}
}

void macroProcessor()
{
	expanding = false;
	while( strcmp( opcode, "END" ) != 0 )
	{
		getLine();
		processLine();
	}
}

int main()
{
	//test2();
	expanding = false;
	macroProcessor();
	deftab.close();
	input.close();
	output.close();
	return 0;
}

