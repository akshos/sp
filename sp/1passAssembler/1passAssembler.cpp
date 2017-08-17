#include <iostream>
#include <fstream>
#include <iomanip>
#include <string.h>

#define MAX_TEXT_RECORD_SIZE 12

using namespace std;

struct SYMTAB
{
	char symbol[30];
	int value;
	bool defined;
	int ref[10], refCount;
}symtab[100];

struct OPTAB
{
	char opcode[30];
	int machine_code;
}optab[100];

int optab_count = 0, symtab_count = 0, start_address = 0;
ifstream input( "input" );
ofstream output( "output" );

void generateOptab() //load optab contents from file
{
        ifstream fin( "optab" );
        char opcode[30];
        int machinecode;
        fin >> opcode >> hex >> machinecode;
        while( !fin.eof() )
        {
                strcpy( optab[optab_count].opcode, opcode );
                optab[optab_count].machine_code = machinecode;
                optab_count++;
                fin >> opcode >> hex >> machinecode;
        }      
}

int searchOptab( char opcode[] ) //retuns the index of the opcode in optab if found else returns -1
{
	for( int i = 0; i < optab_count; i++ )
	{
		if( strcmp( optab[i].opcode, opcode ) == 0 )
			return i;
	}
	return -1;
}

int searchSymtab( char symbol[] ) //returns the index of the symtol in symtab if found else retuns -1
{
	for( int i = 0; i < symtab_count; i++ )
	{
		if( strcmp( symtab[i].symbol, symbol ) == 0 ) 
			return i;
	}
	return -1;
}

void writeOperand( int operand, int position = -1 ) //writes the address of the operand in the address field of instruction
{
	if( position != -1 )
		output.seekp( position );
	output << setw(4) << setfill('0') << operand << " " ;
}

void insertSymtab( char symbol[], int value, int index, bool defined = true )
{
	int pos, putPointer, i = 0;
	if( index == -1 ) //symbol is not present in the table
	{
		strcpy( symtab[symtab_count].symbol, symbol );
		symtab[symtab_count].value = value;
		symtab[symtab_count].defined = defined;
		symtab[symtab_count].refCount = 0;
		if( defined == false ) //symbol was not defined
		{
			pos = symtab[symtab_count].refCount++; //add the referencing location to the list
			symtab[symtab_count].ref[ pos ] = output.tellp();
		}
		symtab_count++;
		return;
	}
	if( defined == false ) //symbol is present but not defined
	{
		pos = symtab[index].refCount++; //add the referencing location to the list
		symtab[index].ref[ pos ] = output.tellp();
		return;
	}
	if( symtab[index].defined == true ) //symbol was already previously defined 
	{
		cout << "ERROR : duplicate symbol : " << symbol << endl; 
		return;
	}
	symtab[index].value = value; //symbol was previously encountered but is defined now
	symtab[index].defined = defined;
	pos = symtab[index].refCount;
	putPointer = output.tellp();
	i = 0;
	while( i < pos )
	{
		writeOperand( value, symtab[index].ref[i] );
		i++;
	}
	output.seekp(putPointer);
}

int byte_length( char operand[] ) //returns length of operand in bytes
{
        int n = 0, i = 0;
        char ch;
        if( operand[n++] == 'X' )
        {
                if( operand[n++] == '\'' )
                {
                        while( operand[n] != '\'' )
                        {
                                operand[i++] = operand[n++];
                        }
                        operand[i] = '\0';
                        return( i/2 );
                }
                else
                {
                        cout << "\n\nERROR\n\n";
                        return 0;
                }
        }
        return strlen( operand );
}

int initializeTextRecord( int startAddress )
{
	int pos;
	output << "T " << setw(6) << setfill('0') << startAddress << " ";
	pos = output.tellp();
	output << setw(2) << setfill('0') << 0 << " " ;
	return pos;
}

int endTextRecord( int textRecordLengthPos, int textRecordLength ) //ends a text record and returns 0
{
        int cur = output.tellp(); //backup the current put pointer position
        output.seekp(textRecordLengthPos); //seek the put pointer to the position of length field of the current text record 
        output << setw(2) << setfill('0') << hex << textRecordLength; //write the correct length value at the position
        output.seekp(cur); //restore the original put pointer position
        output << endl; //end the text record by a new line
        return 0; 
}

void pass1()
{
	int startAddress = 0, locctr = 0, programLength = 0, textRecordLength= 0, programLengthPos, textRecordLengthPos, index;
	int value, textRecordStartAddress = 0;
	char opcode[30], label[30], operand[30];
	input >> label >> opcode; //read first input record
	if( strcmp( opcode, "START" ) == 0 )
	{
		input >> hex >> startAddress;
		locctr = startAddress;
		output << "H " << label << " " << setw(6) << setfill('0') << hex << startAddress << " "; //wrtie header record
		programLengthPos = output.tellp(); //store position of program length in header record
		output << setw(6) << setfill('0') << hex << programLength << endl; //wrtie length of program as 0 in header reocrd
		input >> label >> opcode; //read next input line
	}
	textRecordLengthPos = initializeTextRecord( locctr ); //iniitlize text record
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
			{
					insertSymtab( operand, 0, index, false );
					writeOperand( 0 ); //store 0 as operand address
			}
			else //if the symbol is defined
			{
				writeOperand( symtab[index].value );
			}
			textRecordLength += 3; locctr += 3; 
		}
		else if( strcmp( opcode, "WORD" ) == 0 )
		{
			input >> value; //read the operand value as int
			output << setw(6) << setfill('0') << hex << value << " "; //write the operand value as hex
            textRecordLength += 3; locctr += 3;
		}
		else if( strcmp( opcode, "BYTE" ) == 0 )
		{
			 input >> operand; //read the operand as a string
             int len = byte_length( operand );
			 textRecordLength += len; locctr += len;
			 output << operand << " "; //write the operand as string
		}
		else if( strcmp( opcode, "RESB" ) == 0 || strcmp( opcode, "RESW" ) == 0 )
		{
			 //endTextRecord() returns 0 which is used to reset the textRecordLength variable to 0 after ending the current text record
			textRecordLength = endTextRecord( textRecordLengthPos, textRecordLength ); //finish the current text record
            while( strcmp( opcode, "RESB" ) == 0 || strcmp( opcode, "RESW" ) == 0 ) //read any successive RESW or RESB
            {
            	input >> dec >> value; //read the size as a decimal value
				if( strcmp( opcode, "RESB" ) == 0 )
					locctr += value; //if opcode is RESB
				else //if opcode is RESW
					locctr += (3*value); //size of word * no of words
                input >> label >> opcode; 
            }
            if( strcmp( opcode, "END" ) != 0 ) //if END of input is not reached
                textRecordLengthPos = initializeTextRecord( locctr ); //start a new text record 
            continue;
		}
		input >> label >> opcode; //read next input line
        if( textRecordLength >= MAX_TEXT_RECORD_SIZE || strcmp( opcode, "END" ) == 0 ) //if text record length limit has been reached or END opcode was reached
        {
        	textRecordLength = endTextRecord( textRecordLengthPos, textRecordLength ); //finish the current text record
            //if the END opcode wasnt reached and the next opcode is not RESB or RESW
            if( strcmp( opcode, "END" ) != 0 && strcmp( opcode, "RESB" ) != 0 && strcmp( opcode, "RESW" ) != 0  )  
                    textRecordLengthPos = initializeTextRecord( locctr ); //start a new text record
        }
	}
	output << "E " << setw(6) << setfill('0') << hex << startAddress << endl; //write the end record
	//write the correct program length to the header record
	programLength = locctr - startAddress;
	output.seekp( programLengthPos );
	output << setw(6) << setfill('0') << hex << programLength;		
}

int main()
{
	generateOptab();
	pass1();
	return 0;	
}
