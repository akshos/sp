#include <iostream>
#include <fstream>
#include <iomanip>
#include <string.h>

#define MAX_TEXT_RECORDS 5

using namespace std;

struct node
{
	int position;
	struct node *next;
};

struct SYMTAB
{
	char symbol[30];
	int value;
	bool defined;
	struct node *ref = NULL;
}symtab[100];

struct OPTAB
{
	char opcode[30];
	int machine_code[30];
}optab[100];

int optab_count = 0, symtab_count = 0, start_address = 0;
ifstream input( "input" );
ofstream output( "output" );

void generateOptab()
{
	ifstream optab_file( "optab" );
	while( optab_file )
	{
		optab_file >> optab[optab_count].opcode >> optab[optab_count].machine_code;
		optab_count++;
	}
	optab_file.close();
} 

int searchOptab( char opcode[] )
{
	for( int i = 0; i < optab_count; i++ )
	{
		if( strcmp( optab[i].opcode, opcode ) == 0 )
			return i;
	}
	return -1;
}

int searchSymtab( char symbol[] )
{
	for( int i = 0; i < symtab_count; i++ )
	{
		if( strcmp( symtab[i].symbol, symbol ) == 0 ) 
			return i;
	}
	return -1;
}

void writeOperand( int operand, int position = -1 )
{
	if( position != -1 )
		output.seekp( position );
	output << setw(6) << setfill('0') << operand << " " ;
}

void insertSymtab( char symbol[], int value, int index, bool defined = true )
{
	if( index == -1 ) //symbol is not present in the table
	{
		strcpy( symtab[symtab_count].symbol, symbol );
		symtab[symtab_count].value = value;
		symtab[symtab_count].defined = defined;
		if( defined == false ) //symbol was not defined
		{
			symtab[symtab_count].ref = new node;
			symtab[symtab_count].ref->position = output.tellp(); //store the location where the symbol was referenced
			symtab[symtab_count].ref->next = NULL;
		}
		symtab_count++;
		return;
	}
	if( defined == false ) //symbol is present but not defined
	{
		node *ptr = symtab[index].ref;
		while( ptr->next != NULL ) //add the referencing location to the list
			ptr = ptr->next;
		ptr->next = new node;
		ptr = ptr->next;
		ptr->position = output.tellp();
		ptr->next = NULL;
		return;
	}
	if( symtab[index].defined == true ) //symbol was previously defined
	{
		cout << "ERROR : duplicate symbol : " << symbol << endl; 
		return;
	}
	symtab[index].value = value; //symbol was previously encountered but is defined now
	symtab[index].defined = defined;
	node *ptr = symtab[index].ref;
	if( ptr != NULL )
	{
		int putPointer = output.tellp();
		while( ptr != NULL )
		{
			writeOperand( value, ptr->position );
			ptr = ptr->next;
		}
		output.seekp(putPointer);
	}
}

int initializeTextRecord( int startAddress )
{
	int pos;
	output << "T " << setw(6) << setfill('0') << startAddress << " ";
	pos = output.tellp();
	output << setw(2) << setfill('0') << 0 << " " ;
	return pos;
}

void pass1()
{
	int startAddress = 0, locctr = 0, programLength = 0, textRecordLength= 0, programLengthPos, testRecordSizePos, index;
	int value, textRecordStartAddress = 0;
	char opcode[30], label[30], operand[30];
	input >> label >> opcode; //read first input record
	if( strcmp( opcode, "START" ) == 0 )
	{
		input >> hex >> startAddress;
		locctr = startAddress;
		output << "H " << label << " " << setw(6) << setfill('0') << startAddress << " "; //wrtie header record
		programLengthPos = output.tellp(); //store position of program length in header record
		output << setw(6) << setfill('0') << hex << programLength << endl; //wrtie length of program as 0 in header reocrd
		input >> label >> opcode; //read next input record
	}
	TextRecordSizePos = initializeTextRecord( locctr ); //iniitlize text record
	textRecordStartAddress = locctr; 
	while( strcmp( opcode, "END" ) != 0 )
	{
		if( strcmp( label, "0" ) != 0 ) //if there is a symbol in the label field
		{
			index = searchSymtab( label );
			insertSymtab( label, locctr, index, true );  //Case 1 : label defined
		}
		index = searchOptab( opcode ); //search optab for opcode
		if( index != -1  ) //if opcode found
		{
			input >> operand;
			output << hex << optab[index].machine_code ; //write the machine code
			index = searchSymtab( operand ); //search for the symbol in the operand field
			if( index == -1 || symtab[index].defined == false ) //if the symbol is not defined
				insertSymtab( operand, 0, index, false );
			else //if the symbol is defined
			{
				writeOperand( symtab[index].value );
			}
			locctr += 3;
		}
		else if( strcmp( opcode, "WORD" ) == 0 )
		{
			input >> hex >> value;

	}
		
		
}


int main()
{
	return 0;
	
}
