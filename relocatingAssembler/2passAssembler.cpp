#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdlib.h>
#include <string.h>

#define MAX_TEXT_RECORD_SIZE 12

using namespace std;

struct OPTAB
{
        char opcode[30];
        int machinecode;
}optab[100];

struct SYMTAB
{
        char symbol[30];
        int value;
}symtab[100];

int optab_count = 0, symtab_count = 0, program_length = 0;
int modification_address[100], modification_count = 0;

void generateOptab() //load optab contents from file
{
        ifstream fin( "optab" );
        char opcode[30];
        int machinecode;
        fin >> opcode >> hex >> machinecode;
        while( !fin.eof() )
        {
                strcpy( optab[optab_count].opcode, opcode );
                optab[optab_count].machinecode = machinecode;
                optab_count++;
                fin >> opcode >> hex >> machinecode;
        }      
}

int searchSymtab( char symbol[] ) //returns the index of the symbol entry in symtab
{
        for( int i = 0 ; i < symtab_count; i++ )
        {
                if( strcmp( symtab[i].symbol, symbol ) == 0 )
                {
                        return i;
                }
        }
        return (-1);
}

int searchOptab( char opcode[] ) //returns the index of the opcode in the optab
{
        for( int i = 0; i < optab_count; i++ )
        {
                if( strcmp( optab[i].opcode, opcode ) == 0 )
                {
                        return i;
                }
        }
        return -1;
}

void insertSymtab( char label[], int value ) //insert a symbol name and value in symtab
{
        strcpy( symtab[symtab_count].symbol, label );
        symtab[symtab_count].value = value;
        symtab_count++;
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

/*
NOTE : label field has 0 (numeric zero) for empty label field
*/

int pass1()
{
        ifstream input( "input" );
        ofstream output( "intermediate" );
        char label[30], opcode[30], operand[30];
        int startAddress = 0, locctr = 0, index = 0;
        input >> label >> opcode;; //read first input record
        if( strcmp( opcode, "START" ) == 0 )
        {
                input >> hex >> startAddress;
                locctr = startAddress;
                //write first line to intermediate file
                output <<hex<< locctr << "\t" << label << "\t" << opcode << "\t" <<hex<< startAddress << endl;
                insertSymtab( label, locctr );
                input >> label >> opcode >> operand; //read next input record
        }
        while( strcmp( opcode, "END" ) != 0 )
        {
                if( strcmp( label, "0" ) != 0 ) //if there is s symbol in the label field
                {
                        if( searchSymtab( label ) != -1 ) //serach symtab for symbol
                        {
                                cout << "\nERROR : Duplicate record : " << label ;
                                return 1;
                        }
                        insertSymtab( label, locctr );
                }
                //write line to intermediate file
                output <<hex<< locctr << "\t" << label << "\t" << opcode << "\t";
                //increment locctr with appropriate value
                if( searchOptab( opcode ) != -1 )
                {
                        if( opcode[0] == '+' ) 
                                locctr += 4;
                        else
                                locctr += 3;
                }
                else if( strcmp( opcode, "WORD" ) == 0 )
                        locctr += 3;
                else if( strcmp( opcode, "RESW" ) == 0 )
                        locctr += ( 3 * atoi(operand) );
                else if( strcmp( opcode, "RESB" ) == 0 )
                        locctr += ( atoi(operand) );
                else if( strcmp( opcode, "BYTE" ) == 0 )
                        locctr += byte_length( operand );
                else
                {
                        cout << "\nERROR : Inavlid Opcode : " << opcode ;
                        return 2;
                }
                output << operand << endl;
                input >> label >> opcode >> operand; //read next input record
        } //end record was reached
        output <<hex<< locctr << "\t" << label << "\t" << opcode << "\t" << operand << endl;
        //write last line to imtermediate.
        program_length = locctr - startAddress;
        input.close();
        output.close();
}

void storeSymtab() //stores the contents of the symtab struct array to a file for debug
{
        ofstream sym( "symtab" );
        for( int i = 0; i < symtab_count; i++ )
        {
                sym << symtab[i].symbol << "\t" << hex << symtab[i].value << endl;
        }
}

int initializeTextRecord( ofstream &output, int startAddress ) //starts a new text record
{
        output << "T " << setw(6) << setfill('0') << hex << startAddress << " "; //wrtie T and start address
        int pos = output.tellp(); //get the position for the length field
        output << "00 "; //write a dummy length
        return pos; //return the position of the length field
}

int endTextRecord( ofstream &output, int textRecordLengthPos, int textRecordLength ) //ends a text record
{
        int cur = output.tellp(); //backup the current put pointer position
        output.seekp(textRecordLengthPos); //seek the put pointer to the position of length field of the current text record 
        output << setw(2) << setfill('0') << hex << textRecordLength; //write the correct length value at the position
        output.seekp(cur); //restore the original put pointer position
        output << endl; //end the text record by a new line
        return 0; 
}

void generateModificationRecords( ofstream &output )
{
        for( int i = 0; i < modification_count; i++ )
        {
                output << "M " << setw(6) << setfill('0') << hex << modification_address[i];
                output << " 05" << endl;
        }
} 

void pass2()
{
        ifstream input( "intermediate" );
        ofstream output( "object" );
        char label[30], opcode[30], operand[30];
        int startAddress = 0, locctr, textRecordLength = 0, textRecordLengthPos, index, value;
        int addressFieldSize = 4;
        input >>hex>> locctr >> label >> opcode; //read first input line
        if( strcmp( opcode, "START" ) == 0 ) //if opcode = START
        {
                input >>hex>> startAddress; //read starting address
                //write header record
                output << "H " << label << " " << setw(6) << setfill('0') << hex << startAddress << " ";
                output << setw(6) << setfill('0') << hex << program_length << endl;
        }
        input >>hex>> locctr >> label >> opcode; //read next input line
        textRecordLengthPos = initializeTextRecord( output, locctr ); //initialize first text record
        while( strcmp( opcode, "END" ) != 0 )
        {
                index = searchOptab(opcode); //search optab for opcode
                if( index != -1 ) //if opcode found
                {
                        input >> operand; //read the operand
                        output << setw(2) << setfill('0')<< hex << optab[index].machinecode; //write machine code of opcode
                        index = searchSymtab( operand ); //search symtab for symbol
                        if( opcode[0] == '+' )
                        {
                                addressFieldSize = 6;
                                modification_address[modification_count++] = (locctr+1);
                        } 
                        if( index != -1 ) //if symbol found
                        {
                                output << setw(addressFieldSize) << setfill('0') << hex << symtab[index].value << " ";
                                addressFieldSize = 4;
                        }
                        else //if not found
                        {
                                cout << "\n\nERROR : Undefined Symbol : " << operand << "\n\n";
                                return ;
                        }
                        textRecordLength += 3;
                }
                else if ( strcmp( opcode, "WORD" ) == 0 ) //if opcode = WORD
                {
                        input >> value; //read the operand value as int
                        output << setw(6) << setfill('0') << hex << value << " "; //write the operand value as hex
                        textRecordLength += 3;
                }
                else if( strcmp( opcode, "BYTE" ) == 0 ) //if opcode = BYTE
                {
                        input >> operand; //read the operand as a string
                        output << operand << " " ; //write the operand as string
                        textRecordLength += ( byte_length( operand ) / 2 );
                }
                else if( strcmp( opcode, "RESB" ) == 0 || strcmp( opcode, "RESW" ) == 0 ) //if opcode = RESB or opcode = RESW
                {
                        //endTextRecord() returns 0 which is used to reset the textRecordLength variable to 0 after ending the current text record
                        textRecordLength = endTextRecord( output, textRecordLengthPos, textRecordLength ); //finish the current text record
                        while( strcmp( opcode, "RESB" ) == 0 || strcmp( opcode, "RESW" ) == 0 ) //read any successive RESW or RESB
                        {
                                input >> value;
                                input >> hex >> locctr >> label >> opcode;
                        }
                        if( strcmp( opcode, "END" ) != 0 ) //if END of input is not reached
                                textRecordLengthPos = initializeTextRecord( output, locctr ); //start a new text record  
                        continue;
                }
                input >> hex >> locctr >> label >> opcode; //read next input line
                if( textRecordLength >= MAX_TEXT_RECORD_SIZE || strcmp( opcode, "END" ) == 0 ) //if text record length limit has been reached or END opcode was reached
                {
                        textRecordLength = endTextRecord( output, textRecordLengthPos, textRecordLength ); //finish the current text record
                         //if the END opcode wasnt reached and the next opcode is not RESW or RESB
                        if( strcmp( opcode, "END" )!= 0 && strcmp( opcode, "RESB" ) != 0 && strcmp( opcode, "RESW" ) != 0 ) 
                                textRecordLengthPos = initializeTextRecord( output, locctr ); //start a new text record
                }
        }
        generateModificationRecords( output );
        output << "E " << setw(6) << setfill('0') << hex << startAddress << endl; //write the end record 
}

int main()
{
        generateOptab();
        pass1();
        storeSymtab();
        pass2();
        return 0;
}

