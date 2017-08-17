#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <fstream>

#define LHS_COUNT 10
#define PROD_COUNT 20
#define LESS 1
#define GREATER 2
#define EQUAL 3

using namespace std;

struct LHS
{
    char ch;
    char prod[PROD_COUNT][10];
    char leading[20], trailing[20];
    int prodCount, leadingCount, trailingCount;
}lhs[LHS_COUNT];

char terminals[10], relations[5] = { ' ', '<', '>', '=', '\0' };
int terminalCount = 0, lhsCount = 0;

void initialize()
{
    for( int i = 0; i < LHS_COUNT; i++ )
    {
        lhs[i].ch = '\0';
        for( int j = 0; j < PROD_COUNT; j++ )
            strcpy( lhs[i].prod[j], "\0" );
        strcpy( lhs[i].leading, "\0" );
        strcpy( lhs[i].trailing, "\0" );
        lhs[i].prodCount = lhs[i].leadingCount = lhs[i].trailingCount = 0;
    }
}

void clearMatrix( int mat[][10], int row, int col )
{
    for( int i = 0; i < row; i++ )
        for( int j = 0; j < col; j++ )
            mat[i][j] = 0;
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

void addTerminals( char str[] )
{
    for( int i = 0; str[i] != '\0'; i++ )
    {
        if( isTerminal( str[i] ) )
            terminals[ terminalCount++ ] = str[i];
    }
    terminals[ terminalCount ] = '\0';
}

int terminalIndex( char ch )
{
    for( int i = 0; i < terminalCount; i++ )
    {
        if( terminals[i] == ch )
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
        addTerminals( production );
    }
    terminals[terminalCount++] = '$';
    terminals[terminalCount] = '\0';
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
        addTerminals( production );
    }
    terminals[terminalCount++] = '$';
    terminals[terminalCount] = '\0';
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

char *leading( int n )
{
    char *str, c1, c2, c3; int k;
    for( int j = 0; j < lhs[n].prodCount; j++ )
    {
        k = 0;
        c1 = lhs[n].prod[j][k++];
        c2 = lhs[n].prod[j][k++];
        c3 = lhs[n].prod[j][k++];
        if( isTerminal( c1 ) ) // A -> aB
        {
            lhs[n].leading[ lhs[n].leadingCount++ ] = c1;
        }
        else if( isTerminal(c2) && isNonTerminal(c3) ) // A -> BaC
        {
            lhs[n].leading[ lhs[n].leadingCount++ ] = c2;
        }
        else if( isNonTerminal(c1) && c2 == '\0' ) // A -> B
        {
            str = leading( findLhs( c1 ) );
            strcat( lhs[n].leading, str );
            lhs[n].leadingCount = strlen( lhs[n].leading );
        }
    }
    return lhs[n].leading;
}

char *trailing( int n )
{
    char *str, c1, c2, c3; int k;
    for( int j = 0; j < lhs[n].prodCount; j++ )
    {
        k = strlen( lhs[n].prod[j] ) - 1;
        c1 = c2 = c3 = '\0';
        c1 = lhs[n].prod[j][k--];
        if( k >= 0 )
            c2 = lhs[n].prod[j][k--];
        if( k >= 0 )
            c3 = lhs[n].prod[j][k--];
        if( isTerminal( c1 ) )
        {
            lhs[n].trailing[ lhs[n].trailingCount++ ] = c1;
        }
        else if( isTerminal( c2 ) && isNonTerminal( c3 ) )
        {
            lhs[n].trailing[ lhs[n].trailingCount++ ] = c2;
        }
        else if( isNonTerminal( c1 ) && c2 == '\0' )
        {
            str = trailing( findLhs( c1 ) );
            strcat( lhs[n].trailing, str );
            lhs[n].trailingCount = strlen( lhs[n].trailing );
        }
    }
    return lhs[n].trailing;
}

void createTable()
{
    int matrix[10][10], row, col, p , q, k, pos;
    char c1, c2, c3;
    row = col = terminalCount;
    clearMatrix( matrix, row, col );
    p = terminalIndex( '$' );
    for( int i = 0; i < lhs[0].leadingCount; i++ )
        matrix[p][ terminalIndex( lhs[0].leading[i] ) ] = LESS;
    for( int i = 0; i < lhs[0].trailingCount; i++ )
        matrix[ terminalIndex( lhs[0].trailing[i] ) ][p] = GREATER;
    for( int i = 0; i < lhsCount; i++ )
    {
        for( int j = 0; j < lhs[i].prodCount; j++ )
        {
            k = 0;
            do
            {
                c1 = lhs[i].prod[j][k];
                c2 = lhs[i].prod[j][k+1];
                c3 = lhs[i].prod[j][k+2];
                if( isTerminal(c1) && isNonTerminal(c2) && isTerminal(c3) ) // A -> aBb
                    matrix[ terminalIndex(c1) ][ terminalIndex(c3) ] = EQUAL;
                if( isNonTerminal(c1) && isTerminal(c2) )  // A -> Ba
                {
                    p = terminalIndex( c2 );
                    pos = findLhs(c1);
                    for( q = 0; q < lhs[pos].trailingCount; q++ )
                        matrix[ terminalIndex( lhs[pos].trailing[q] ) ][p] = GREATER;
                }
                if( isTerminal(c1) && isNonTerminal(c2) ) //A -> aB
                {
                    p = terminalIndex(c1);
                    pos = findLhs(c2);
                    for( q = 0; q < lhs[pos].leadingCount; q++ )
                    {
                        matrix[p][ terminalIndex( lhs[pos].leading[q] ) ] = LESS;
                    }
                }
                k++;
            }while( c2 != '\0' );
        }
    }
    for( int i = 0; i < terminalCount; i++ )
        cout << " \t" << terminals[i];
    cout << "\n\n";
    for( int i = 0; i < row; i++ )
    {
        cout << terminals[i] << "\t";
        for( int j = 0; j < col; j++ )
        {
            cout << relations[ matrix[i][j] ] << "\t";
        }
        cout << "\n\n";
    }
}

int main()
{
    char *temp;
    initialize();
    readGrammarFile();
    printGrammar();
    for( int i = 0; i < lhsCount; i++ )
    {
        if( lhs[i].leadingCount == 0 )
            temp = leading( i );
        if( lhs[i].trailingCount == 0 )
            temp = trailing( i );
    }
    cout << "\nOperator Precedance Table \n\n";
    createTable();
    return 0;
}


/*
The inupt grammar is :
E -> E+T / T
T -> T*F / F
F -> (E) / i

Operator Precedance Table

        +       *       (       )       i       $

+       >       <       <       >       <       >

*       >       >       <       >       <       >

(       <       <       <       =       <

)       >       >               >               >

i       >       >               >               >

$       <       <       <               <
*/