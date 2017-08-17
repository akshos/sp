#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <fstream>

#define LHS_COUNT 10
#define PROD_COUNT 20

using namespace std;

struct LHS
{
    char ch;
    char prod[PROD_COUNT][10];
    char first[20], follow[20];
    int prodCount, firstCount, followCount;
}lhs[LHS_COUNT];

int lhsCount = 0;

void initialize()
{
    for( int i = 0; i < LHS_COUNT; i++ )
    {
        lhs[i].ch = '\0';
        strcpy( lhs[i].first, "\0" );
        strcpy( lhs[i].follow, "\0" );
        lhs[i].prodCount = lhs[i].firstCount = lhs[i].followCount = 0;
    }
} 

int findLhs( char ch )
{
    for( int i = 0; i < lhsCount; i++ )
    {
        if( lhs[i].ch == ch )
            return i;
    }
    return -1;
}

void addProduction( char ch, char prod[] )
{
    int i;
    for( i = 0; i < lhsCount; i++ )
    {
        if( lhs[i].ch == ch )
            break;
    }
    lhs[i].ch = ch;
    strcpy( lhs[i].prod[lhs[i].prodCount], prod );
    lhs[i].prodCount++;
    if( i == lhsCount )
        lhsCount++;
}

void readGrammar()
{
    int n;
    char ch, arrow[4], production[10];
    cout << "Enter the number of productions : ";
    cin >> n;
    cout << "Enter the productions : \n";
    for( int i = 0; i < n; i++ )
    {
        cin >> ch >> arrow >> production;
        addProduction( ch, production );
    }
}

void readGrammarFile()
{
    int n;
    char ch, arrow[4], production[10];
    ifstream input( "input" );
    input >> n;
    for( int i = 0; i < n; i++ )
    {
        input >> ch >> arrow >> production;
        addProduction( ch, production );
    }
}

void printGrammar()
{
    cout << "\nThe inupt grammar is : " << endl;
    for( int i = 0; i < lhsCount; i++ )
    {
        cout << lhs[i].ch << " -> ";
        cout << lhs[i].prod[0];
        for( int j = 1; j < lhs[i].prodCount; j++ )
            cout << " / " << lhs[i].prod[j];
        cout << endl;
    }
}

int searchEpsilon( int n )
{
     if( lhs[n].first[ lhs[n].firstCount -1 ] == '0' )
        return 1;
    return 0;
}

void removeEpsilonFirst( int n )
{
    lhs[n].first[ lhs[n].firstCount - 1 ] = '\0';
    lhs[n].firstCount--;
}

int removeEpsilonFollow( int n )
{
    if( lhs[n].follow[ lhs[n].followCount - 1 ] == '0' )
    {
        lhs[n].follow[ lhs[n].followCount - 1 ] = '\0';
        lhs[n].followCount--;
        return 1;
    }
    return 0;
}

int isTerminal( char ch )
{
    if( (ch >= 'a' && ch <= 'z') || (ch >= '!' && ch <= '/') )
        return 1;
    return 0;
}

int isNonTerminal( char ch )
{
    if( ch >= 'A' && ch <= 'Z' )
        return 1;
    return 0;
}

char *first( int n )
{
    char ch; int k, epsilon = 0;
    if( lhs[n].firstCount != 0 ) //if first was already found return the first
        return lhs[n].first;
    for( int j = 0; j < lhs[n].prodCount; j++ ) //for each production of the NT
    {
        k = 0;
        ch = lhs[n].prod[j][k++]; //get the first symbol of the production
        if(  ch == '0' ) //if Epsilon production
        {
            lhs[n].first[lhs[n].firstCount++] = ch;
            continue;
        }else if( isTerminal(ch) ) //if first symbol is a T
        {
            lhs[n].first[lhs[n].firstCount++] = ch;
            continue;
        }else //first symbol was a NT
        {
            while( isNonTerminal(ch) ) //if production starts with a NT
            {
                epsilon = 0; //if the last production ended with epsilon
                lhs[n].first[lhs[n].firstCount] = '\0'; //terminate the string for strcat to work
                char *temp = first( findLhs( ch ) ); //include first of NT
                strcat( lhs[n].first, temp );
                lhs[n].firstCount = strlen( lhs[n].first ); //update firstCount
                ch = lhs[n].prod[j][k++]; //read next symbol in the production
                if( searchEpsilon( n ) && ch != '\0' ) //if first of NT had 0
                {
                    removeEpsilonFirst( n );
                    epsilon = 1;
                }else 
                {
                    break; //if there was no 0 then stop
                }
            }
            if( isTerminal(ch) && epsilon == 1 ) //if a T comes after NT with 0 production
                lhs[n].first[lhs[n].firstCount++] = ch;
        }
    }
    lhs[n].first[lhs[n].firstCount] = '\0'; //terminate the first with a \0 for strcat to work
    return lhs[n].first;
}

void addFollow( int n, char *str )
{
    int flag = 0;
    while( *str != '\0' )
    {   
        flag = 0;
        for( int i = 0; i < lhs[n].followCount; i++ )
        {
            if( lhs[n].follow[i] == *str )
                flag = 1;
        }
        if( flag == 0 )
            lhs[n].follow[lhs[n].followCount++] = *str;
        str++;
    }
    lhs[n].follow[lhs[n].followCount] = '\0';
} 

void follow( int n )
{
    char ch, temp[3], *str, next;
    int k, pos, epsilon;
    for( int j = 0; j < lhs[n].prodCount; j++ )
    {
        k = 0;
        do
        {
            epsilon = 0;
            ch = lhs[n].prod[j][k++];
            next = lhs[n].prod[j][k];
            if( isTerminal(ch) )
                continue;
            if( isNonTerminal(ch) )
            {
                pos = findLhs( ch );
                if( isTerminal(next) )
                {
                    temp[0] = next; temp[1] = '\0';
                    addFollow( pos, temp );
                }else if( isNonTerminal(next) )
                {
                    addFollow( pos, lhs[ findLhs(next) ].first );
                    epsilon = removeEpsilonFollow( pos );
                }
                if( next == '\0' || epsilon == 1 )
                {
                    addFollow( pos, lhs[n].follow );
                }
            }
        }while( ch != '\0' );
    }
}

void findFirst()
{
    char *temp;
    for( int i = 0; i < lhsCount; i++ ) //find the first of each NT
    {
        if( lhs[i].firstCount == 0 ) //find first if not already found
            temp = first( i );
    }
}

void findFollow()
{
    lhs[0].follow[ lhs[0].followCount++ ] = '$';
    for( int i = 0; i < lhsCount; i++ )
        follow( i );
    for( int i = 0; i < lhsCount; i++ )
        follow( i );
}

void display()
{
    cout << "\n\nSymbol \t     FIRST \t    FOLLOW\n" << endl;
    for( int i = 0; i < lhsCount; i++ )
    {
        cout << lhs[i].ch << "\t | { ";
        cout << lhs[i].first[0];
        for( int j = 1; lhs[i].first[j] != '\0'; j++ )
            cout <<  " , " << lhs[i].first[j];
        cout << " }\t | { ";
        cout << lhs[i].follow[0];
        for( int j = 1; lhs[i].follow[j] != '\0'; j++ )
            cout <<  " , " << lhs[i].follow[j];
        cout << " } \n\n";
    }
}

int main()
{
    initialize();
    readGrammarFile();
    printGrammar();
    findFirst();
    findFollow();
    display();    
    return 0;
}



/*
The inupt grammar is :
E -> TA
A -> +TA / 0
T -> FB
B -> *FB / 0
F -> (E) / i


Symbol       FIRST          FOLLOW

E        | { ( , i }     | { $ , ) }

A        | { + , 0 }     | { $ , ) }

T        | { ( , i }     | { + , $ , ) }

B        | { * , 0 }     | { + , $ , ) }

F        | { ( , i }     | { * , + , $ , ) }
*/