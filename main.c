#include <stdio.h>
#include <stdlib.h>

#define PSTACK_SIZE 1024
#define RSTACK_SIZE PSTACK_SIZE

#define F_TRUE -1
#define F_FALSE 0

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
void f_##name ( void ); 				\
fword_t name = { 						\
	LATEST, 							\
	flags, 								\
	fname, 								\
	{ 									\
		&f_##name, 						\
		NULL 							\
	} 									\
}; 										\
void f_##name ( void )					\

#define NEXT 	ip = *(void**)np;			\
                np = np + sizeof( void* ); 	\
				return	 					\

FCODEWORD( EXIT, "EXIT", F_NONE ) {
	np = (void*)rsPop();
	NEXT;
}

#define FTHREADWORD( name, fname, flags ) 	\
fword_t name = { 							\
	LATEST, 								\
	flags, 									\
	fname, 									\
	{ 										\
		&DoCol, 							\

#define ENDTHREAD 							\
		EXIT.thread							\
	} 										\
};											\

#define FVAR( name, fname ) 			\
mword v_##name; 						\
FCODEWORD( name, fname ) { 				\
	psPush( &v_##name ); 				\
} 										\

#define FCONST( name, fname, val ) 			\
FCODEWORD( name, fname, F_NONE ) { 			\
 	psPush( val ); 							\
} 											\

/** DOCOL *******************************************************************/

void DoCol( void ) {
	printf( "DoCol\n" );
	rsPush( (mword)np );

	np = ip + sizeof( void* );

	NEXT;
}

/** CODEWORDS ***************************************************************/

FCODEWORD( Nop, "NOP", F_NONE ) {
	NEXT;
}
#undef LATEST
#define LATEST &Nop

FCODEWORD( Term, "TERM", F_NONE ) {
	exit( 0 );
}
#undef LATEST
#define LATEST &Nop


FCODEWORD( Emit, "EMIT", F_NONE ) {
	putchar( (char)psPop() );
    NEXT;
}
#undef LATEST
#define LATEST &Emit

FCODEWORD( Drop, "DROP", F_NONE ) {
	--psp;
    NEXT;
}
#undef LATEST
#define LATEST &Drop

FCODEWORD( Swap, "SWAP", F_NONE ) {
	mword a = psPop(), b = psPop();

	psPush( a );
	psPush( b );

    NEXT;
}
#undef LATEST
#define LATEST &Swap

FCODEWORD( Dup, "DUP", F_NONE ) {
	mword a = psPop();

	psPush( a );
	psPush( a );

	NEXT;
}
#undef LATEST
#define LATEST &Dup

FCODEWORD( Over, "OVER", F_NONE ) {
	mword a = psPop(), b = psPop();

	psPush( b );
	psPush( a );
	psPush( b );

	NEXT;
}
#undef LATEST
#define LATEST &Over

FCODEWORD( Rot, "ROT", F_NONE ) {
	mword a = psPop(), b = psPop(), c = psPop();
	psPush( b );
	psPush( a );
	psPush( c );
	NEXT;
}
#undef LATEST
#define LATEST &Rot

FCODEWORD( Nrot, "-ROT", F_NONE ) {
	mword a = psPop(), b = psPop(), c = psPop();
	psPush( a );
	psPush( c );
	psPush( b );
	NEXT;
}
#undef LATEST
#define LATEST &Nrot

FCODEWORD( TwoDrop, "2DROP", F_NONE ) {
	psp -= 2;
	NEXT;
}
#undef LATEST
#define LATEST &TwoDrop

FCODEWORD( TwoDup, "2DUP", F_NONE ) {
	mword a = psPop(), b = psPop();
	psPush( b );
	psPush( a );
	psPush( b );
	psPush( a ); 
	NEXT;
}
#undef LATEST
#define LATEST &TwoDup

FCODEWORD( Qdup, "?DUP", F_NONE ) {
	mword a = psPop();
	psPush( a );
	if ( a != 0 )
		psPush( a );
	NEXT;
}
#undef LATEST
#define LATEST &Qdup

FCODEWORD( Incr, "1+", F_NONE ) {
	mword a = psPop();
	psPush( a + 1 );
	NEXT;
}
#undef LATEST
#define LATEST &Incr

FCODEWORD( Decr, "1-", F_NONE ) {
	mword a = psPop();
	psPush( a - 1 );
	NEXT;
}
#undef LATEST
#define LATEST &Decr

FCODEWORD( FourIncr, "4+", F_NONE ) {
	mword a = psPop();
	psPush( a + 4 );
	NEXT;
}
#undef LATEST
#define LATEST &FourIncr

FCODEWORD( FourDecr, "4-", F_NONE ) {
	mword a = psPop();
	psPush( a - 4 );
	NEXT;
}
#undef LATEST
#define LATEST &FourDecr

FCODEWORD( Add, "+", F_NONE ) {
	mword a = psPop(), b = psPop();
	psPush( a + b );
	NEXT;
}
#undef LATEST
#define LATEST &Add

FCODEWORD( Sub, "-", F_NONE ) {
	mword a = psPop(), b = psPop();
	psPush( b - a );
	NEXT;
}
#undef LATEST
#define LATEST &Sub

FCODEWORD( DivMod, "/MOD", F_NONE ) {
	mword a = psPop(), b = psPop();
	psPush( a % b );
	psPush( a / b );
	NEXT;
}
#undef LATEST
#define LATEST &DivMod

FCODEWORD( Equ, "=", F_NONE ) {
	mword a = psPop(), b = psPop();
	if ( a == b )
		psPush( F_TRUE );
	else
		psPush( F_FALSE );
	NEXT;
}
#undef LATEST
#define LATEST &Equ

FCODEWORD( Test, "TEST", F_NONE ) {
	printf( "Test\n" );
	psPush( '\n' );
	psPush( 'a' );
	psPush( 'b' );
	psPush( 'c' );
	NEXT;
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
	Term.thread,
ENDTHREAD

/** Entry into FORTH proper *************************************************/

int main( void ) {
	np = 0;

    ip = coldboot.thread;
    func = **((interp_t**)ip);

    np = np + sizeof( void* );

	while ( 1 ) {
		func();
		func = **((interp_t**)ip);
	}

}
