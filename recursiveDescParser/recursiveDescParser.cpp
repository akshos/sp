/*
Author : Akshay Venugopal 207
Program Name : Recursive Descent Parser
*/

#include <iostream>
#include <string.h>

/*
E  -> TE'
E' -> +TE'/ESP
T  -> FT'
T' -> *FT'/ESP
F  -> (E)/id
*/

using namespace std;

int ret = 0, flag = 0, pos = 0;
char str[30], inputSymbol;

void forward()
{
	if( str[pos] != '\0' )
		pos += 1;
	inputSymbol = str[pos];
}

int isID( char ch )
{
	if( ch >= 'a' && ch <= 'z' )
		return 1;
	return 0;
}

void E();
void EPrime();
void T();
void TPrime();
void F();

void E()
{
	T();
	EPrime();
}

void EPrime()
{
	if( inputSymbol == '+' )
	{
		forward();
		T();
		EPrime();
	}
}

void T()
{
	F();
	TPrime();
}

void TPrime()
{
	if( inputSymbol == '*' )
	{
		forward();
		F();
		TPrime();
	} 
}

void F()
{
	if( inputSymbol == '(' ) 
	{
		forward();
		E();
		if( inputSymbol == ')' )
			forward();
		else
			flag = -1;
	}
	else if( isID( inputSymbol ) )
		forward();
	else
		flag = -1;
}
	
int main()
{
	cout << "Enter an expression : ";
	cin >> str;
	inputSymbol = str[0];
	E();
	if( flag == 0 && ( pos == strlen(str) ) )
		cout << "Accepted" << endl;
	else
		cout << "Not Accepted" << endl;
	return 0;
}

//--------------------END OF PROGRAM-------------//


/*

Enter an expression : a+b
Accepted

Enter an expression : a-b
Not Accepted

Enter an expression : (a+b
Not Accepted

Enter an expression : (a+b)*(c+d)
Accepted

*/
