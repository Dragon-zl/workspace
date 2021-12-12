#include "bubblesort.h"

/*  冒泡排序    */
void  bubblesort(int a[]  , int len)
{
    for(int i = 0 ; i < len - 1 ; i++)
    {
        for(int j = 0 ; j < len - i - 1 ; j++)
        {
            if( a[j] > a[j+1] )
            {
                a[j]   = a[j] ^ a[j+1];
                a[j+1] = a[j] ^ a[j+1];
                a[j]   = a[j] ^ a[j+1]; 
            }
        }
    }
}