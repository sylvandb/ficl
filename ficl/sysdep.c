/*******************************************************************
** s y s d e p . c
** Forth Inspired Command Language
** Author: John Sadler (john_sadler@alum.mit.edu)
** Created: 16 Oct 1997
** Implementations of FICL external interface functions... 
**
** (simple) port to Linux, Skip Carter 26 March 1998
** $Id$
*******************************************************************/
/*
** Get the latest Ficl release at http://ficl.sourceforge.net
**
** L I C E N S E  and  D I S C L A I M E R
** 
** Ficl is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
** 
** The ficl software code is provided on an "as is"  basis without
** warranty of any kind, including, without limitation, the implied
** warranties of merchantability and fitness for a particular purpose
** and their equivalents under the laws of any jurisdiction.  
** See the GNU Lesser General Public License for more details.
** 
** To view the GNU Lesser General Public License, visit this URL:
** http://www.fsf.org/copyleft/lesser.html
** 
** Any third party may reproduce, distribute, or modify the ficl
** software code or any derivative  works thereof without any 
** compensation or license, provided that the author information
** and this license text are retained in the source code files.
** 
** I am interested in hearing from anyone who uses ficl. If you have
** a problem, a success story, a defect, an enhancement request, or
** if you would like to contribute to the ficl release (yay!), please
** send me email at the address above. 
*/

#include <stdlib.h>
#include <stdio.h>

#include "ficl.h"

/*
*******************  FreeBSD  P O R T   B E G I N S   H E R E ******************** Michael Smith
*/
#if defined (FREEBSD_ALPHA)

#if PORTABLE_LONGMULDIV == 0
DPUNS ficlLongMul(FICL_UNS x, FICL_UNS y)
{
    DPUNS q;
    u_int64_t qx;

    qx = (u_int64_t)x * (u_int64_t) y;

    q.hi = (u_int32_t)( qx >> 32 );
    q.lo = (u_int32_t)( qx & 0xFFFFFFFFL);

    return q;
}

UNSQR ficlLongDiv(DPUNS q, FICL_UNS y)
{
    UNSQR result;
    u_int64_t qx, qh;

    qh = q.hi;
    qx = (qh << 32) | q.lo;

    result.quot = qx / y;
    result.rem  = qx % y;

    return result;
}
#endif

void  ficlTextOut(FICL_VM *pVM, char *msg, int fNewline)
{
    IGNORE(pVM);

    while(*msg != 0)
    putchar(*(msg++));
    if (fNewline)
    putchar('\n');

   return;
}

void *ficlMalloc (size_t size)
{
    return malloc(size);
}

void *ficlRealloc (void *p, size_t size)
{
    return realloc(p, size);
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
*******************  P C / W I N 3 2   P O R T   B E G I N S   H E R E ***********************
*/
#elif defined (_M_IX86)

#if PORTABLE_LONGMULDIV == 0
DPUNS ficlLongMul(FICL_UNS x, FICL_UNS y)
{
    DPUNS q;

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

UNSQR ficlLongDiv(DPUNS q, FICL_UNS y)
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

#endif

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


void *ficlRealloc(void *p, size_t size)
{
    return realloc(p, size);
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

#if PORTABLE_LONGMULDIV == 0
DPUNS ficlLongMul(FICL_UNS x, FICL_UNS y)
{
    DPUNS q;
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

UNSQR ficlLongDiv(DPUNS q, FICL_UNS y)
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

#endif

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


void *ficlRealloc(void *p, size_t size)
{
    void *pv = malloc(size);
    if (p)
    {
        memcpy(pv, p, size)
        free(p);
    }

    return pv;
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

#if defined(linux) || defined(riscos)

#if PORTABLE_LONGMULDIV == 0

#ifdef riscos
typedef unsigned long long __u64;
typedef unsigned long __u32;
#endif

DPUNS ficlLongMul(FICL_UNS x, FICL_UNS y)
{
    DPUNS q;
    __u64 qx;

    qx = (__u64)x * (__u64) y;

    q.hi = (__u32)( qx >> 32 );
    q.lo = (__u32)( qx & 0xFFFFFFFFL);

    return q;
}

UNSQR ficlLongDiv(DPUNS q, FICL_UNS y)
{
    UNSQR result;
    __u64 qx, qh;

    qh = q.hi;
    qx = (qh << 32) | q.lo;

    result.quot = qx / y;
    result.rem  = qx % y;

    return result;
}

#endif

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

void *ficlRealloc(void *p, size_t size)
{
    return realloc(p, size);
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


