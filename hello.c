#include <stdio.h>
#include <stdlib.h>


int main(){
    char a = 0;
    printf("\033[40;37m\nhello world....\n\033[41;37m\nhello world....\n\033[42;37m\nhello world....\n\033[43;37m\nhello world....\n\033[44;37m\nhello world....\n\033[45;37m\nhello world....\n\033[46;37m\nhello world....\n\033[47;30m\nhello world....\n");
    a=getc(stdin);
    return 0;
}
