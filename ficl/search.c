/*******************************************************************
** s e a r c h . c
** Forth Inspired Command Language
** ANS Forth SEARCH and SEARCH-EXT word-set written in C
** Author: John Sadler (john_sadler@alum.mit.edu)
** Created: 6 June 2000
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

#include <string.h>
#include "ficl.h"
#include "math64.h"

/**************************************************************************
                        d e f i n i t i o n s
** SEARCH ( -- )
** Make the compilation word list the same as the first word list in the
** search order. Specifies that the names of subsequent definitions will
** be placed in the compilation word list. Subsequent changes in the search
** order will not affect the compilation word list. 
**************************************************************************/
static void definitions(FICL_VM *pVM)
{
    FICL_DICT *pDict = ficlGetDict();

    assert(pDict);
    if (pDict->nLists < 1)
    {
        vmThrowErr(pVM, "DEFINITIONS error - empty search order");
    }

    pDict->pCompile = pDict->pSearch[pDict->nLists-1];
    return;
}


/**************************************************************************
                        f o r t h - w o r d l i s t
** SEARCH ( -- wid )
** Return wid, the identifier of the word list that includes all standard
** words provided by the implementation. This word list is initially the
** compilation word list and is part of the initial search order. 
**************************************************************************/
static void forthWordlist(FICL_VM *pVM)
{
    FICL_HASH *pHash = ficlGetDict()->pForthWords;
    stackPushPtr(pVM->pStack, pHash);
    return;
}


/**************************************************************************
                        g e t - c u r r e n t
** SEARCH ( -- wid )
** Return wid, the identifier of the compilation word list. 
**************************************************************************/
static void getCurrent(FICL_VM *pVM)
{
    ficlLockDictionary(TRUE);
    stackPushPtr(pVM->pStack, ficlGetDict()->pCompile);
    ficlLockDictionary(FALSE);
    return;
}


/**************************************************************************
                        g e t - o r d e r
** SEARCH ( -- widn ... wid1 n )
** Returns the number of word lists n in the search order and the word list
** identifiers widn ... wid1 identifying these word lists. wid1 identifies
** the word list that is searched first, and widn the word list that is
** searched last. The search order is unaffected.
**************************************************************************/
static void getOrder(FICL_VM *pVM)
{
    FICL_DICT *pDict = ficlGetDict();
    int nLists = pDict->nLists;
    int i;

    ficlLockDictionary(TRUE);
    for (i = 0; i < nLists; i++)
    {
        stackPushPtr(pVM->pStack, pDict->pSearch[i]);
    }

    stackPushUNS(pVM->pStack, nLists);
    ficlLockDictionary(FALSE);
    return;
}


/**************************************************************************
                        s e a r c h - w o r d l i s t
** SEARCH ( c-addr u wid -- 0 | xt 1 | xt -1 )
** Find the definition identified by the string c-addr u in the word list
** identified by wid. If the definition is not found, return zero. If the
** definition is found, return its execution token xt and one (1) if the
** definition is immediate, minus-one (-1) otherwise. 
**************************************************************************/
static void searchWordlist(FICL_VM *pVM)
{
    STRINGINFO si;
    UNS16 hashCode;
    FICL_WORD *pFW;
    FICL_HASH *pHash = stackPopPtr(pVM->pStack);

    si.count         = (FICL_COUNT)stackPopUNS(pVM->pStack);
    si.cp            = stackPopPtr(pVM->pStack);
    hashCode         = hashHashCode(si);

    ficlLockDictionary(TRUE);
    pFW = hashLookup(pHash, si, hashCode);
    ficlLockDictionary(FALSE);

    if (pFW)
    {
        stackPushPtr(pVM->pStack, pFW);
        stackPushINT(pVM->pStack, (wordIsImmediate(pFW) ? 1 : -1));
    }
    else
    {
        stackPushUNS(pVM->pStack, 0);
    }

    return;
}


/**************************************************************************
                        s e t - c u r r e n t
** SEARCH ( wid -- )
** Set the compilation word list to the word list identified by wid. 
**************************************************************************/
static void setCurrent(FICL_VM *pVM)
{
    FICL_HASH *pHash = stackPopPtr(pVM->pStack);
    FICL_DICT *pDict = ficlGetDict();
    ficlLockDictionary(TRUE);
    pDict->pCompile = pHash;
    ficlLockDictionary(FALSE);
    return;
}


/**************************************************************************
                        s e t - o r d e r
** SEARCH ( widn ... wid1 n -- )
** Set the search order to the word lists identified by widn ... wid1.
** Subsequently, word list wid1 will be searched first, and word list
** widn searched last. If n is zero, empty the search order. If n is minus
** one, set the search order to the implementation-defined minimum
** search order. The minimum search order shall include the words
** FORTH-WORDLIST and SET-ORDER. A system shall allow n to
** be at least eight.
**************************************************************************/
static void setOrder(FICL_VM *pVM)
{
    int i;
    int nLists = stackPopINT(pVM->pStack);
    FICL_DICT *dp = ficlGetDict();

    if (nLists > FICL_DEFAULT_VOCS)
    {
        vmThrowErr(pVM, "set-order error: list would be too large");
    }

    ficlLockDictionary(TRUE);

    if (nLists >= 0)
    {
        dp->nLists = nLists;
        for (i = nLists-1; i >= 0; --i)
        {
            dp->pSearch[i] = stackPopPtr(pVM->pStack);
        }
    }
    else
    {
        dictResetSearchOrder(dp);
    }

    ficlLockDictionary(FALSE);
    return;
}


/**************************************************************************
                        f i c l - w o r d l i s t
** SEARCH ( -- wid )
** Create a new empty word list, returning its word list identifier wid.
** The new word list may be returned from a pool of preallocated word
** lists or may be dynamically allocated in data space. A system shall
** allow the creation of at least 8 new word lists in addition to any
** provided as part of the system. 
** Notes: 
** 1. ficl creates a new single-list hash in the dictionary and returns
**    its address.
** 2. ficl-wordlist takes an arg off the stack indicating the number of
**    hash entries in the wordlist. Ficl 2.02 and later define WORDLIST as
**    : wordlist 1 ficl-wordlist ;
**************************************************************************/
static void ficlWordlist(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();
    FICL_HASH *pHash;
    FICL_UNS nBuckets;
    
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 1, 1);
#endif
    nBuckets = stackPopUNS(pVM->pStack);
    pHash = dictCreateWordlist(dp, nBuckets);
    stackPushPtr(pVM->pStack, pHash);
    return;
}


/**************************************************************************
                        S E A R C H >
** ficl  ( -- wid )
** Pop wid off the search order. Error if the search order is empty
**************************************************************************/
static void searchPop(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();
    int nLists;

    ficlLockDictionary(TRUE);
    nLists = dp->nLists;
    if (nLists == 0)
    {
        vmThrowErr(pVM, "search> error: empty search order");
    }
    stackPushPtr(pVM->pStack, dp->pSearch[--dp->nLists]);
    ficlLockDictionary(FALSE);
    return;
}


/**************************************************************************
                        > S E A R C H
** ficl  ( wid -- )
** Push wid onto the search order. Error if the search order is full.
**************************************************************************/
static void searchPush(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();

    ficlLockDictionary(TRUE);
    if (dp->nLists > FICL_DEFAULT_VOCS)
    {
        vmThrowErr(pVM, ">search error: search order overflow");
    }
    dp->pSearch[dp->nLists++] = stackPopPtr(pVM->pStack);
    ficlLockDictionary(FALSE);
    return;
}


/**************************************************************************
                        W I D - G E T - N A M E
** ficl  ( wid -- c-addr u )
** Get wid's (optional) name and push onto stack as a counted string
**************************************************************************/
static void widGetName(FICL_VM *pVM)
{
    FICL_HASH *pHash = vmPop(pVM).p;
    char *cp = pHash->name;
    int len = 0;
    
    if (cp)
        len = strlen(cp);

    vmPush(pVM, LVALUEtoCELL(cp));
    vmPush(pVM, LVALUEtoCELL(len));
    return;
}

/**************************************************************************
                        W I D - S E T - N A M E
** ficl  ( wid c-addr -- )
** Set wid's name pointer to the \0 terminated string address supplied
**************************************************************************/
static void widSetName(FICL_VM *pVM)
{
    char *cp = (char *)vmPop(pVM).p;
    FICL_HASH *pHash = vmPop(pVM).p;
    pHash->name = cp;
    return;
}


/**************************************************************************
                        setParentWid
** FICL
** setparentwid   ( parent-wid wid -- )
** Set WID's link field to the parent-wid. search-wordlist will 
** iterate through all the links when finding words in the child wid.
**************************************************************************/
static void setParentWid(FICL_VM *pVM)
{
    FICL_HASH *parent, *child;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 2, 0);
#endif
    child  = (FICL_HASH *)stackPopPtr(pVM->pStack);
    parent = (FICL_HASH *)stackPopPtr(pVM->pStack);

    child->link = parent;
    return;
}


/**************************************************************************
                        f i c l C o m p i l e S e a r c h
** Builds the primitive wordset and the environment-query namespace.
**************************************************************************/

void ficlCompileSearch(FICL_SYSTEM *pSys)
{
    FICL_DICT *dp = pSys->dp;
    assert (dp);

    /*
    ** optional SEARCH-ORDER word set 
    */
    dictAppendWord(dp, ">search",   searchPush,     FW_DEFAULT);
    dictAppendWord(dp, "search>",   searchPop,      FW_DEFAULT);
    dictAppendWord(dp, "definitions",
                                    definitions,    FW_DEFAULT);
    dictAppendWord(dp, "forth-wordlist",  
                                    forthWordlist,  FW_DEFAULT);
    dictAppendWord(dp, "get-current",  
                                    getCurrent,     FW_DEFAULT);
    dictAppendWord(dp, "get-order", getOrder,       FW_DEFAULT);
    dictAppendWord(dp, "search-wordlist",  
                                    searchWordlist, FW_DEFAULT);
    dictAppendWord(dp, "set-current",  
                                    setCurrent,     FW_DEFAULT);
    dictAppendWord(dp, "set-order", setOrder,       FW_DEFAULT);
    dictAppendWord(dp, "ficl-wordlist", 
                                    ficlWordlist,   FW_DEFAULT);

    /*
    ** Set SEARCH environment query values
    */
    ficlSetEnv("search-order",      FICL_TRUE);
    ficlSetEnv("search-order-ext",  FICL_TRUE);
    ficlSetEnv("wordlists",         FICL_DEFAULT_VOCS);

    dictAppendWord(dp, "wid-get-name", widGetName,  FW_DEFAULT);
    dictAppendWord(dp, "wid-set-name", widSetName,  FW_DEFAULT);
    dictAppendWord(dp, "wid-set-super", 
                                    setParentWid,   FW_DEFAULT);
    return;
}

