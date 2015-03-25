#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	int stackSize=10000;
	int memSize=1000000;
	int i;
	for (i=1; i<argc; i++) {
		if (!strncmp(argv[i], "-memsize", 8))
			memSize=atoi(argv[i] + 8);
		else if (!strncmp(argv[i], "-stacksize", 10))
			stackSize=atoi(argv[i] + 10);
		else
			goto usage;
	}
	int c;
	int label=0;
	int *stack=malloc(stackSize*sizeof(int));
	int stackp=0;
	memset(stack, 0, stackSize*sizeof(int));
	printf(
		"#include <stdio.h>\n"
		"int main(void) {\n"
		"static unsigned char mem[%d];\n"
		"int ptr=0;\n"
		, memSize
		);
	while ((c=getchar())!=EOF) {
		switch (c) {
		int poplbl;
		case '>':
			printf("++ptr;\n");
			break;
		case '<':
			printf("--ptr;\n");
			break;
		case '+':
			printf("++mem[ptr];\n");
			break;
		case '-':
			printf("--mem[ptr];\n");
			break;
		case '.':
			printf("putchar(mem[ptr]);\n");
			break;
		case ',':
			printf("mem[ptr]=getchar();\n");
			break;
		case '[':
			printf("o%d: if (!mem[ptr]) goto c%d;\n", label, label);
			stack[stackp++]=label++;
			break;
		case ']':
			poplbl=stack[--stackp];
			printf("c%d: if (mem[ptr]) goto o%d;\n", poplbl, poplbl);
			break;
		}
	}
	printf(
		"return 0;\n"
		"}\n"
		);
	return 0;
usage:
	printf("usage: %s [-memsize M] [-stacksize S] < input > output\n", argv[0]);
	return -1;
}
