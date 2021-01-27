#include <stdio.h>
#include <stdint.h>

/**
 * Sort array
 */
void sort(int buf[], int size)
{
    int index = 0;
    int count[3] = {0};

    //Count n Complexity
    for(int i=0; i<size; i++)
    {
        if(buf[i]>=0 && buf[i]<3)
            count[buf[i]]++;
    }

    //Arrange n Complexity
    for(int j=0; j<3; j++)
    {
        for(int i=0; i<count[j]; i++,index++)
        {
            buf[index] = j;
        }
    }
}

void dump(int buf[], int size)
{
    for(int i=0; i<size; i++)
        printf("%02d ", buf[i]);
    printf("\n");
}

int main()
{
    int foo[30]= {2,2,1,0,1,0,1,1,0,2,2,1,1,1,2,0,0,
        1,1,0, 1, 0, 1, 2, 0, 0, 0, 1, 1, 2 };
    
    dump(foo, 30);
    sort(foo, 30);
    dump(foo, 30);
}