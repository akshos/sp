#include <iostream>
#include <string.h>

using namespace std;

char stack[40];
int top = -1;

void push( char item )
{
    stack[++top] = item;
}

char pop()
{
    char item = stack[top];
    top--;
    return item;
}

int stackEmpty()
{
    return top;
}

int isOperand( char op )
{
	if( (op <= 'z' && op >= 'a') || (op <= 'Z' && op >= 'A') )
		return 1;
	return 0;  
}

int precedance( char op )
{
	switch( op )
	{
		case '+':
		case '-': return 1;
		case '*':
		case '/': return 2;
		case '^': return 3;
	}
	return -1;
}

void to_postfix( char exp[], char post[] )
{
	char stack[100], op;
	int top = -1, n = 0, i = 0;
	
	while( exp[i] != '\0' )
	{
		op = exp[i];
		if( isOperand( op ) )
		{
			post[n++] = op;
		}
		else if( op == '(' )
		{
			stack[++top] = op;
		}
		else if( op == ')' )
		{
			while( stack[top] != '(' && top != -1 )
			{
				post[n++] = stack[top--];
			}
			top--;
		}
		else
		{
			while( precedance(op) <= precedance( stack[top] ) && top != -1 )
			{
				post[n++] = stack[top--];
			}
			stack[++top] = op;
		}
		
		i++;
	}
	
	while( top != -1 )
		post[n++] = stack[top--];
	
	post[n] = '\0';
}

void evaluate( char postfix[30] )
{
    int n = 0, i = 1;
    char operand1, operand2, op;
    while( postfix[n] != '\0' )
    {
        if( isOperand( postfix[n] ) )
            push( postfix[n] );
        else
        {
            operand1 = pop();
            operand2 = pop();
            op = postfix[n];
			if( operand1 < 'a'  && operand2 < 'a' ) //extra condition 
				cout << "t" << i << " = t" << (int)operand2 << " " << op << " t" << (int)operand1 << endl;
            else if( operand1 < 'a' )
                cout << "t" << i << " = " << operand2 << " " << op << " t" << (int)operand1 << endl;
            else if( operand2 < 'a' )
                cout << "t" << i << " = t" << (int)operand2 << " " << op << " " << operand1 << endl;
            else
                cout << "t" << i << " = " << operand2 << " " << op << " " << operand1 << endl;
            push( (char)i );
            i++;
        }
        n++;
    }
}

int main()
{
    char expression[30], postfix[30];
    cout << "Enter an expression : ";
    cin >> expression;
    to_postfix( expression, postfix );
    cout << "\nPostfix : " << postfix;
    cout << endl;
    evaluate( postfix );
    return 0;
}


/*
Enter an expression : (a+b)/c*d

Postfix : ab+c/d*
t1 = a + b
t2 = t1 / c
t3 = t2 * d
*/