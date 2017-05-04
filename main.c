#include <stdio.h>
#include <stdlib.h>

#define PSTACK_SIZE 1024
#define RSTACK_SIZE PSTACK_SIZE

typedef long long mword;

void* np; /* points to next mword */
void* ip; /* on calling interpreter, points to start of thread */

typedef void interp_t( void );
interp_t (*func);

/** STACK *******************************************************************/

mword pStack[PSTACK_SIZE];
mword* psp = pStack;
mword rStack[RSTACK_SIZE];
mword* rsp = rStack;

#define psPop() (*(--psp))
#define psPush( x ) (*(psp++) = (mword)x)
#define rsPop() (*(--rsp))
#define rsPush( x ) (*(rsp++) = (mword)x)

/** WORD DEFINITION *********************************************************/

void DoCol( void );

typedef enum fFlags_s {
	F_NONE = 0,
	F_IMMED = 0x80,
	F_HIDDEN = 0x40
} fFlags_t;

#define LATEST NULL

typedef struct fword_s {
	void* last;
	fFlags_t flags;
	char name[32];
	void* thread[];
} fword_t;

fword_t* latest;

#define FCODEWORD( name, fname, flags ) \
void f_##name ( void ); \
fword_t name = { \
	LATEST, \
	flags, \
	fname, \
	{ \
		f_##name, \
		NULL \
	} \
}; \
void f_##name ( void )

#define FTHREADWORD( name, fname, flags ) \
fword_t name = { \
	LATEST, \
	flags, \
	fname, \
	{ \
		DoCol, \

#define ENDTHREAD \
		NULL \
	} \
};

#define FVAR( name, fname ) \
mword v_##name; \
FCODEWORD( name, fname ) { \
	psPush( &v_##name ); \
} 

#define FCONST( name, fname, val ) \
FCODEWORD( name, fname, F_NONE ) { \
 	psPush( val ); \
}

/** CODEWORDS ***************************************************************/

FCODEWORD( Nop, "NOP", F_NONE ) {
	;
}
#undef LATEST
#define LATEST &Nop

FCODEWORD( Emit, "EMIT", F_NONE ) {
	putchar( (char)psPop() );
}
#undef LATEST
#define LATEST &Emit

FCODEWORD( Drop, "DROP", F_NONE ) {
	--psp;
}
#undef LATEST
#define LATEST &Drop

FCODEWORD( Swap, "SWAP", F_NONE ) {
	mword a, b;

	a = psPop();
	b = psPop();
	psPush( a );
	psPush( b );
}
#undef LATEST
#define LATEST &Swap

FCODEWORD( Dup, "DUP", F_NONE ) {
	int a;

	a = psPop();
	psPush( a );
	psPush( a );
}
#undef LATEST
#define LATEST &Dup

FCODEWORD( Test, "TEST", F_NONE ) {
	printf( "Test\n" );
	psPush( '\n' );
	psPush( 'a' );
	psPush( 'b' );
	psPush( 'c' );
}
#undef LATEST
#define LATEST &Test

/** THREADWORDS *************************************************************/

FTHREADWORD( test2, "TEST2", F_NONE )
	Test.thread,
	Emit.thread,
	Emit.thread,
	Emit.thread,
	Emit.thread,
ENDTHREAD

FTHREADWORD( coldboot, "COLDBOOT", F_NONE )
	test2.thread,
ENDTHREAD

void DoCol( void ) {
	printf( "DoCol\n" );
	rsPush( (mword)np );

	np = ip + (sizeof(void*));

	do {
		ip = *((void**)np);
		np = np + sizeof( void* );

		if ( ip ) {
            func = **((interp_t**)ip);
			func();
		}
	} while ( ip );

	np = (void*)rsPop();
	printf( "DoCol done\n" );
}

/** Entry into FORTH proper *************************************************/

void* boot[] = {
	DoCol,
	test2.thread,
	NULL
};

int main( void ) {
	rsPush( 0 );
	np = coldboot.thread;

	ip = np;
	np = np + sizeof( void* );
	func = **((interp_t**)ip);
	func();
}
