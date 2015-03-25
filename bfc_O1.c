#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void flushMem(int *mem) {
	if (*mem) {
		printf("mem[ptr]+=%d;\n", *mem);
		*mem=0;
	}
}

static void flushPtr(int *ptr) {
	if (*ptr) {
		printf("ptr+=%d;\n", *ptr);
		*ptr=0;
	}
}

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
	int ptr=0, mem=0;
	while ((c=getchar())!=EOF) {
		switch (c) {
		int poplbl;
		case '>':
			flushMem(&mem);
			++ptr;
			break;
		case '<':
			flushMem(&mem);
			--ptr;
			break;
		case '+':
			flushPtr(&ptr);
			++mem;
			break;
		case '-':
			flushPtr(&ptr);
			--mem;
			break;
		case '.':
			if (mem)
				printf("putchar(mem[ptr] + %d);\n", mem);
			else if (ptr)
				printf("putchar(mem[ptr + %d]);\n", ptr);
			else
				printf("putchar(mem[ptr];\n");
			break;
		case ',':
			if (mem)
				mem=0;
			if (ptr)
				printf("mem[ptr + %d]=getchar();\n", ptr);
			else
				printf("mem[ptr]=getchar();\n");
			break;
		case '[':
			flushPtr(&ptr);
			flushMem(&mem);
			printf("o%d: if (!mem[ptr]) goto c%d;\n", label, label);
			stack[stackp++]=label++;
			break;
		case ']':
			flushPtr(&ptr);
			flushMem(&mem);
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
