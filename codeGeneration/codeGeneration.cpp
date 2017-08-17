#include <iostream>
#include <string.h>
#include <fstream>

using namespace std;

ifstream input( "intermediate" );
char res[5], operand1[5], operand2[5], op, eq;

void readLine()
{
    input >> res >> eq >> operand1 >> op >> operand2;
}

void generate()
{
    int flag = 0;
    readLine();
    while( !input.eof() )
    {
        if( operand1[0] == 't' )
            cout << "MOV DX, " << operand2 << endl;
        else if( operand2[0] == 't' )
            cout << "MOV DX, " << operand1 << endl;
        else
        {
            cout << "MOV AX, " << operand1 << endl;
            cout << "MOV DX, " << operand2 << endl;
            flag = 1; //extra condition
        }
        switch( op )
        {
            case '+' : cout << "ADD AX, DX"; break;
            case '-' : cout << "SUB AX, DX"; break;
            case '*' : cout << "MUL DX"; break;
            case '/' : cout << "DIV DX"; break;
        }
        cout << endl;
        if( flag == 1 ) //extra condition
        {
            cout << "MOV " << res << ", AX" << endl;
            flag = 0;
        }   
        readLine();
    }
}


int main()
{
    generate();
    cout << "MOV res, AX" << endl;
    return 0;
}


/*
Input :

t1 = a + b
t2 = t1 / c
t3 = t2 * d

Output :

MOV AX, a
MOV DX, b
ADD AX, DX
MOV DX, c
DIV DX
MOV res, AX

*/