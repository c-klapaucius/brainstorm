#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef unsigned char u8;
typedef signed char s8;

// S00vvvvv : ptr +31/-32
// S01vvvvv : mem +31/-32
// 01000000 : out
// 01000001 : in
// 11XXXXXX XXXXXXXX : callz lookup
// 01000002 : retnz lookup

static u8 *code;
static int pc=0;

static void flushPtr(int *pdptr) {
	int dptr=*pdptr;
	*pdptr=0;
	while (dptr>=31) {
		code[pc++]=0x1f;
		dptr-=31;
	}
	while (dptr<=-32) {
		code[pc++]=0x80;
		dptr+=32;
	}
	if (dptr)
		code[pc++]=(dptr&0x1f)|(dptr<0 ? 0x80 : 0x00);
}

static void flushMem(int *pdmem) {
	int dmem=*pdmem;
	*pdmem=0;
	while (dmem>=31) {
		code[pc++]=0x3f;
		dmem-=31;
	}
	while (dmem<=-32) {
		code[pc++]=0xa0;
		dmem+=32;
	}
	if (dmem)
		code[pc++]=(dmem&0x1f)|(dmem<0 ? 0xa0 : 0x20);
}

#define FLUSHPTR()\
	do {\
		if (dptr)\
			flushPtr(&dptr);\
	} while (0)

#define FLUSHMEM()\
	do {\
		if (dmem)\
			flushMem(&dmem);\
	} while (0)

int main(int argc, char *argv[])
{
	int stackSize=10000;
	int memSize=1000000;
	const char *fname=NULL;
	int i;
	for (i=1; i<argc; i++) {
		if (!strncmp(argv[i], "-memsize", 8))
			memSize=atoi(argv[i] + 8);
		else if (!strncmp(argv[i], "-stacksize", 10))
			stackSize=atoi(argv[i] + 10);
		else if (!fname)
			fname=argv[i];
		else
			goto usage;
	}
	if (!fname)
		goto usage;
	FILE *f=fopen(fname, "rb");
	if (!f) {
		printf("error opening file\n");
		goto error_exit;
	}
	fseek(f, 0, SEEK_END);
	int size=ftell(f);
	if (size<1 || fseek(f, SEEK_SET, 0)) {
		printf("could not read file (or file empty)\n");
		goto error_exit;
	}

	int c;
	unsigned lookup=0;
	int *stack=malloc(stackSize*sizeof(int));
	int *lookupT=malloc(stackSize*sizeof(int));
	u8 *mem=malloc(memSize);
	memset(mem, 0, memSize);
	code=malloc(size*2);
	int stackp=0;
	int dptr=0, pci=0, dmem=0;
	while ((c=fgetc(f))!=EOF) {
		switch (c) {
		case '>':
			FLUSHMEM();
			++dptr;
			break;
		case '<':
			FLUSHMEM();
			--dptr;
			break;
		case '+':
			FLUSHPTR();
			++dmem;
			break;
		case '-':
			FLUSHPTR();
			--dmem;
			break;
		case '.':
			FLUSHMEM();
			FLUSHPTR();
			code[pc++]=0x40;
			break;
		case ',':
			FLUSHMEM();
			FLUSHPTR();
			code[pc++]=0x41;
			break;
		case '[':
			FLUSHMEM();
			FLUSHPTR();
			code[pc++]=0xc0u|(lookup>>8);
			code[pc++]=lookup;
			stack[stackp++]=lookup++;
			break;
		case ']':
			FLUSHMEM();
			FLUSHPTR();
			code[pc++]=0x42;
			lookupT[stack[--stackp]]=pc;
			break;
		}
	}
	int ip=0;
	stackp=-1;
	int ptr=0;
	unsigned long long insns=0;
	while (ip<pc) {
		++insns;
		if (code[ip]==0x42) {
			if (mem[ptr]) {
				ip=stack[stackp];
			} else {
				ip++;
				--stackp;
			}
		} else if ((code[ip]&0xc0)==0xc0) {
			if (mem[ptr]) {
				ip+=2;
				stack[++stackp]=ip;
			} else {
				lookup=(code[ip++]<<8)&0x3f00;
				lookup|=code[ip++];
				ip=lookupT[lookup];
			}
		} else if (!(code[ip]&0x60)) {
			s8 dptr=code[ip++];
			if (dptr&0x80)
				dptr|=0x60;
			ptr+=dptr;
		} else if ((code[ip]&0x60)==0x20) {
			s8 dmem=code[ip++]&0x9f;
			if (dmem&0x80)
				dmem|=0x60;
			mem[ptr]+=dmem;
		} else if (code[ip++]==0x40)
			putchar(mem[ptr]);
		else {
			mem[ptr]=getchar();
			++ip;
		}
	}
	printf(">>> %llu\n", insns);
	return 0;
usage:
	printf("usage: %s [-memsize M] [-stacksize S] <file>\n", argv[0]);
error_exit:
	return -1;
}
