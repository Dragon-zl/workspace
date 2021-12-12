#include <stdio.h>
#include "bubblesort.h"
int main()
{
    int array[5] = { 5 , 4 , 3 , 2 , 1};
    int len = sizeof(array)/sizeof(int);
    bubblesort( array , len );
    for( int i = 0 ; i < len ; i++ )
    {
        printf("%d " , array[i]);
    }
    printf("\n");
    return 0;
}