#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>

int main()
{
	long int a = syscall(353);
    long int b = syscall(354);
         
	printf(“System call 353: %ld\n”, a);
    printf(“System call 354: %ld\n”, b);
         
	return 0;
}