/*******************************************************************
** s y s d e p . c
** Forth Inspired Command Language
** Author: John Sadler (john_sadler@alum.mit.edu)
** Created: 16 Oct 1997
** Implementations of FICL external interface functions... 
**
** (simple) port to Linux, Skip Carter 26 March 1998
**
*******************************************************************/

#include <stdlib.h>
#include <stdio.h>

#include "ficl.h"
 
/*
*******************  P C / W I N 3 2   P O R T   B E G I N S   H E R E ***********************
*/
#if defined (_M_IX86)

UNS64 ficlLongMul(UNS32 x, UNS32 y)
{
    UNS64 q;

    __asm
    {
        mov eax,x
        mov edx,y
        mul edx
        mov q.hi,edx
        mov q.lo,eax
    }

    return q;
}

UNSQR ficlLongDiv(UNS64 q, UNS32 y)
{
    UNSQR result;

    __asm
    {
        mov eax,q.lo
        mov edx,q.hi
        div y
        mov result.quot,eax
        mov result.rem,edx
    }

    return result;
}

#if !defined (_WINDOWS)

void ficlTextOut(FICL_VM *pVM, char *msg, int fNewline)
{
    IGNORE(pVM);

    if (fNewline)
        puts(msg);
    else
        fputs(msg, stdout);

   return;
}

#endif

void *ficlMalloc (size_t size)
{
    return malloc(size);
}

void  ficlFree   (void *p)
{
    free(p);
}

/*
** Stub function for dictionary access control - does nothing
** by default, user can redefine to guarantee exclusive dict
** access to a single thread for updates. All dict update code
** is guaranteed to be bracketed as follows:
** ficlLockDictionary(TRUE);
** <code that updates dictionary>
** ficlLockDictionary(FALSE);
**
** Returns zero if successful, nonzero if unable to acquire lock
** befor timeout (optional - could also block forever)
*/
#if FICL_MULTITHREAD
int ficlLockDictionary(short fLock)
{
	IGNORE(fLock);
	return 0;
}
#endif /* FICL_MULTITHREAD */

/*
*******************  6 8 K  C P U 3 2  P O R T   B E G I N S   H E R E ********************
*/
#elif defined (MOTO_CPU32)

UNS64 ficlLongMul(UNS32 x, UNS32 y)
{
    UNS64 q;
    IGNORE(q);    /* suppress goofy compiler warnings */
    IGNORE(x);
    IGNORE(y);

#pragma ASM
    move.l (S_x,a6),d1
    mulu.l (S_y,a6),d0:d1
    move.l d1,(S_q+4,a6)
    move.l d0,(S_q+0,a6)
#pragma END_ASM

    return q;
}

UNSQR ficlLongDiv(UNS64 q, UNS32 y)
{
    UNSQR result;
    IGNORE(result); /* suppress goofy compiler warnings */
    IGNORE(q);
    IGNORE(y);

#pragma ASM
    move.l (S_q+0,a6),d0    ; hi 32 --> d0
    move.l (S_q+4,a6),d1    ; lo 32 --> d1
    divu.l (S_y,a6),d0:d1   ; d0 <-- rem, d1 <-- quot
    move.l d1,(S_result+0,a6)
    move.l d0,(S_result+4,a6)
#pragma END_ASM

    return result;
}

void  ficlTextOut(FICL_VM *pVM, char *msg, int fNewline)
{
   return;
}

void *ficlMalloc (size_t size)
{
}

void  ficlFree   (void *p)
{
}

/*
** Stub function for dictionary access control - does nothing
** by default, user can redefine to guarantee exclusive dict
** access to a single thread for updates. All dict update code
** is guaranteed to be bracketed as follows:
** ficlLockDictionary(TRUE);
** <code that updates dictionary>
** ficlLockDictionary(FALSE);
**
** Returns zero if successful, nonzero if unable to acquire lock
** befor timeout (optional - could also block forever)
*/
#if FICL_MULTITHREAD
int ficlLockDictionary(short fLock)
{
	IGNORE(fLock);
	return 0;
}
#endif /* FICL_MULTITHREAD */

#endif /* MOTO_CPU32 */

/*
*******************  Linux  P O R T   B E G I N S   H E R E ******************** Skip Carter, March 1998
*/

#ifdef linux

UNS64 ficlLongMul(UNS32 x, UNS32 y)
{
    UNS64 q;
    __u64 qx;

    qx = (__u64)x * (__u64) y;

    q.hi = (__u32)( qx >> 32 );
    q.lo = (__u32)( qx & 0xFFFFFFFFL);

    return q;
}

UNSQR ficlLongDiv(UNS64 q, UNS32 y)
{
    UNSQR result;
    __u64 qx, qh;

    qh = q.hi;
    qx = (qh << 32) | q.lo;

    result.quot = qx / y;
    result.rem  = qx % y;

    return result;
}

void  ficlTextOut(FICL_VM *pVM, char *msg, int fNewline)
{
    IGNORE(pVM);

    if (fNewline)
        puts(msg);
    else
        fputs(msg, stdout);

   return;
}

void *ficlMalloc (size_t size)
{
    return malloc(size);
}

void  ficlFree   (void *p)
{
    free(p);
}

/*
** Stub function for dictionary access control - does nothing
** by default, user can redefine to guarantee exclusive dict
** access to a single thread for updates. All dict update code
** is guaranteed to be bracketed as follows:
** ficlLockDictionary(TRUE);
** <code that updates dictionary>
** ficlLockDictionary(FALSE);
**
** Returns zero if successful, nonzero if unable to acquire lock
** befor timeout (optional - could also block forever)
*/
#if FICL_MULTITHREAD
int ficlLockDictionary(short fLock)
{
	IGNORE(fLock);
	return 0;
}
#endif /* FICL_MULTITHREAD */

#endif /* linux */


