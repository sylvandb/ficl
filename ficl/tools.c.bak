/*******************************************************************
** t o o l s . c
** Forth Inspired Command Language - programming tools
** Author: John Sadler (john_sadler@alum.mit.edu)
** Created: 20 June 2000
** $Id$
*******************************************************************/
/*
** NOTES:
** SEE needs information about the addresses of functions that
** are the CFAs of colon definitions, constants, variables, DOES>
** words, and so on. It gets this information from a table and supporting
** functions in words.c.
** colonParen doDoes createParen variableParen userParen constantParen
**
** Step and break debugger for Ficl
** The debugger requies real implementations of KEY and ACCEPT
** because it bypasses the normal VM return mechanism for getting 
** text??
** debug  ( xt -- )   Start debugging an xt
** Set a breakpoint
** Specify breakpoint default action
*/

#include <stdlib.h>
#include <stdio.h>          /* sprintf */
#include <string.h>
#include <ctype.h>
#include "ficl.h"


/*
** nBREAKPOINTS sizes the breakpoint array. One breakpoint (bp 0) is reserved
** for the STEP command. The rest are user programmable. 
*/
#define nBREAKPOINTS 10

/*
** BREAKPOINT record.
** origXT - if NULL, this breakpoint is unused. Otherwise it stores the xt 
** that the breakpoint overwrote. This is restored to the dictionary when the
** BP executes or gets cleared
*/
typedef struct breakpoint
{
	FICL_WORD *origXT;
	FICL_WORD *onBreak;
} BREAKPOINT;

static BREAKPOINT bpTable[nBREAKPOINTS];
static FICL_WORD *pBreak = NULL;

/**************************************************************************
                        i n i t T o o l s
** Initializes static variables of this file, including:
** 1. The lookup table of control structure XTs used by isDebuggable and
** see
** 2. The breakpoint table
** This routine MUST execute AFTER the core dictionary is successfully
** built, and AFTER debug words are inserted into the dictionary, but
** BEFORE any use of the debugger.
**************************************************************************/
static void initTools(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();
	&pVM;

    return;	
}



/**************************************************************************
                        s e e 
** TOOLS ( "<spaces>name" -- )
** Display a human-readable representation of the named word's definition.
** The source of the representation (object-code decompilation, source
** block, etc.) and the particular form of the display is implementation
** defined. 
** NOTE: these funcs come late in the file because they reference all
** of the word-builder funcs without declaring them again. Call me lazy.
**************************************************************************/
/*
** isAFiclWord
** Vet a candidate pointer carefully to make sure
** it's not some chunk o' inline data...
** It has to have a name, and it has to look
** like it's in the dictionary address range.
** NOTE: this excludes :noname words!
*/
static int isAFiclWord(FICL_WORD *pFW)
{
    FICL_DICT *pd  = ficlGetDict();

    if (!dictIncludes(pd, pFW))
       return 0;

    if (!dictIncludes(pd, pFW->name))
        return 0;

    return ((pFW->nName > 0) && (pFW->name[pFW->nName] == '\0'));
}

/*
** seeColon (for proctologists only)
** Walks a colon definition, decompiling
** on the fly. Knows about primitive control structures.
*/
static void seeColon(FICL_VM *pVM, CELL *pc)
{
    static FICL_WORD *pSemiParen = NULL;

	if (!pSemiParen)
		pSemiParen = ficlLookup("(;)");
	assert(pSemiParen);

    for (; pc->p != pSemiParen; pc++)
    {
        FICL_WORD *pFW = (FICL_WORD *)(pc->p);

        if (isAFiclWord(pFW))
        {
			WORDKIND kind = ficlWordClassify(pFW);
			CELL c;

			switch (kind)
			{
			case LITERAL:
				c = *++pc;
				if (isAFiclWord(c.p))
				{
					FICL_WORD *pLit = (FICL_WORD *)c.p;
					sprintf(pVM->pad, "    literal %.*s (%#lx)", 
						pLit->nName, pLit->name, c.u);
				}
				else
					sprintf(pVM->pad, "    literal %ld (%#lx)", c.i, c.u);
				break;
            case STRINGLIT:
				{
					FICL_STRING *sp = (FICL_STRING *)(void *)++pc;
					pc = (CELL *)alignPtr(sp->text + sp->count + 1) - 1;
					sprintf(pVM->pad, "    s\" %.*s\"", sp->count, sp->text);
				}
				break;
			case IF:
				c = *++pc;
				if (c.i > 0)
					sprintf(pVM->pad, "    if / while (branch rel %ld)", c.i);
				else
					sprintf(pVM->pad, "    until (branch rel %ld)", c.i);
				break;
			case BRANCH:
				c = *++pc;
				if (c.i > 0)
					sprintf(pVM->pad, "    else (branch rel %ld)", c.i);
				else
					sprintf(pVM->pad, "    repeat (branch rel %ld)", c.i);
				break;

			case QDO:
				c = *++pc;
				sprintf(pVM->pad, "    ?do (leave abs %#lx)", c.u);
				break;
			case DO:
				c = *++pc;
				sprintf(pVM->pad, "    do (leave abs %#lx)", c.u);
				break;
            case LOOP:
				c = *++pc;
				sprintf(pVM->pad, "    loop (branch rel %#ld)", c.i);
				break;
			case PLOOP:
				c = *++pc;
				sprintf(pVM->pad, "    +loop (branch rel %#ld)", c.i);
				break;
			default:
                sprintf(pVM->pad, "    %.*s", pFW->nName, pFW->name);
				break;
            }
 
            vmTextOut(pVM, pVM->pad, 1);
        }
        else /* probably not a word - punt and print value */
        {
            sprintf(pVM->pad, "    %ld (%#lx)", pc->i, pc->u);
            vmTextOut(pVM, pVM->pad, 1);
        }
    }

    vmTextOut(pVM, ";", 1);
}

/*
** Here's the outer part of the decompiler. It's 
** just a big nested conditional that checks the
** CFA of the word to decompile for each kind of
** known word-builder code, and tries to do 
** something appropriate. If the CFA is not recognized,
** just indicate that it is a primitive.
*/
static void seeXT(FICL_VM *pVM)
{
    FICL_WORD *pFW;
	WORDKIND kind;

    pFW = (FICL_WORD *)stackPopPtr(pVM->pStack);
	kind = ficlWordClassify(pFW);

	switch (kind)
	{
	case COLON:
        sprintf(pVM->pad, ": %.*s", pFW->nName, pFW->name);
        vmTextOut(pVM, pVM->pad, 1);
        seeColon(pVM, pFW->param);
		break;

	case DOES:
        vmTextOut(pVM, "does>", 1);
        seeColon(pVM, (CELL *)pFW->param->p);
 		break;

	case CREATE:
        vmTextOut(pVM, "create", 1);
		break;

    case VARIABLE:
        sprintf(pVM->pad, "variable = %ld (%#lx)", pFW->param->i, pFW->param->u);
        vmTextOut(pVM, pVM->pad, 1);
		break;

	case USER:
        sprintf(pVM->pad, "user variable %ld (%#lx)", pFW->param->i, pFW->param->u);
        vmTextOut(pVM, pVM->pad, 1);
		break;

	case CONSTANT:
        sprintf(pVM->pad, "constant = %ld (%#lx)", pFW->param->i, pFW->param->u);
        vmTextOut(pVM, pVM->pad, 1);

	default:
        vmTextOut(pVM, "primitive", 1);
		break;
    }

    if (pFW->flags & FW_IMMEDIATE)
    {
        vmTextOut(pVM, "immediate", 1);
    }

    if (pFW->flags & FW_COMPILE)
    {
        vmTextOut(pVM, "compile-only", 1);
    }

    return;
}


static void see(FICL_VM *pVM)
{
	ficlTick(pVM);
	seeXT(pVM);
	return;
}


/**************************************************************************
                        f i c l D e b u g
** debug  ( xt -- )
** Given an xt of a colon definition or a word defined by DOES>, set the
** VM up to debug the word: push IP, set the xt as the next thing to execute,
** set a breakpoint at its first instruction, and run to the breakpoint.
**************************************************************************/
void ficlDebug(FICL_VM *pVM)
{
	int ret;
	FICL_WORD *xt = stackPopPtr(pVM->pStack);
    assert(pBreak);

	if (ficlWordIsDebuggable(xt))
	{
		stackPushPtr(pVM, xt);
		seeXT(pVM);
		/*
		** Set a breakpoint at the first instruction and run the word
		*/
        
	    ret = ficlExecXT(pVM, xt);
	}
	else
	{
		ficlTextOut(pVM, "primitive - cannot debug", 1);
	}

    return;
}



/**************************************************************************
                        d e b u g - b r e a k
** FICL
** Throws a breakpoint exception - used by DEBUG to step and break.
**************************************************************************/
void debugBreak(FICL_VM *pVM)
{
	vmThrow(pVM, VM_BREAK);
	return;
}


/**************************************************************************
                        b y e
** TOOLS
** Signal the system to shut down - this causes ficlExec to return
** VM_USEREXIT. The rest is up to you.
**************************************************************************/

static void bye(FICL_VM *pVM)
{
    vmThrow(pVM, VM_USEREXIT);
    return;
}


/**************************************************************************
                        d i s p l a y S t a c k
** TOOLS 
** Display the parameter stack (code for ".s")
**************************************************************************/

static void displayStack(FICL_VM *pVM)
{
    int d = stackDepth(pVM->pStack);
    int i;
    CELL *pCell;

    vmCheckStack(pVM, 0, 0);

    if (d == 0)
        vmTextOut(pVM, "(Stack Empty) ", 0);
    else
    {
        pCell = pVM->pStack->base;
        for (i = 0; i < d; i++)
        {
            vmTextOut(pVM, ltoa((*pCell++).i, pVM->pad, pVM->base), 0);
            vmTextOut(pVM, " ", 0);
        }
    }
}


/**************************************************************************
                        f o r g e t
** TOOLS EXT  ( "<spaces>name" -- )
** Skip leading space delimiters. Parse name delimited by a space.
** Find name, then delete name from the dictionary along with all
** words added to the dictionary after name. An ambiguous
** condition exists if name cannot be found. 
** 
** If the Search-Order word set is present, FORGET searches the
** compilation word list. An ambiguous condition exists if the
** compilation word list is deleted. 
**************************************************************************/
static void forgetWid(FICL_VM *pVM)
{
    FICL_DICT *pDict = ficlGetDict();
    FICL_HASH *pHash;

    pHash = (FICL_HASH *)stackPopPtr(pVM->pStack);
    hashForget(pHash, pDict->here);

    return;
}


static void forget(FICL_VM *pVM)
{
    void *where;
    FICL_DICT *pDict = ficlGetDict();
    FICL_HASH *pHash = pDict->pCompile;

    ficlTick(pVM);
    where = ((FICL_WORD *)stackPopPtr(pVM->pStack))->name;
    hashForget(pHash, where);
    pDict->here = PTRtoCELL where;

    return;
}


/**************************************************************************
                        l i s t W o r d s
** 
**************************************************************************/
#define nCOLWIDTH 8
static void listWords(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();
    FICL_HASH *pHash = dp->pSearch[dp->nLists - 1];
    FICL_WORD *wp;
    int nChars = 0;
    int len;
    unsigned i;
    int nWords = 0;
    char *cp;
    char *pPad = pVM->pad;

    for (i = 0; i < pHash->size; i++)
    {
        for (wp = pHash->table[i]; wp != NULL; wp = wp->link, nWords++)
        {
            if (wp->nName == 0) /* ignore :noname defs */
                continue;

            cp = wp->name;
            nChars += sprintf(pPad + nChars, "%s", cp);

            if (nChars > 70)
            {
                pPad[nChars] = '\0';
                nChars = 0;
                vmTextOut(pVM, pPad, 1);
            }
            else
            {
                len = nCOLWIDTH - nChars % nCOLWIDTH;
                while (len-- > 0)
                    pPad[nChars++] = ' ';
            }

            if (nChars > 70)
            {
                pPad[nChars] = '\0';
                nChars = 0;
                vmTextOut(pVM, pPad, 1);
            }
        }
    }

    if (nChars > 0)
    {
        pPad[nChars] = '\0';
        nChars = 0;
        vmTextOut(pVM, pPad, 1);
    }

    sprintf(pVM->pad, "Dictionary: %d words, %ld cells used of %u total", 
        nWords, (long) (dp->here - dp->dict), dp->size);
    vmTextOut(pVM, pVM->pad, 1);
    return;
}


/**************************************************************************
                        l i s t E n v
** 
**************************************************************************/
static void listEnv(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetEnv();
    FICL_HASH *pHash = dp->pForthWords;
    FICL_WORD *wp;
    unsigned i;
    int nWords = 0;

    for (i = 0; i < pHash->size; i++)
    {
        for (wp = pHash->table[i]; wp != NULL; wp = wp->link, nWords++)
        {
            vmTextOut(pVM, wp->name, 1);
        }
    }

    sprintf(pVM->pad, "Environment: %d words, %ld cells used of %u total", 
        nWords, (long) (dp->here - dp->dict), dp->size);
    vmTextOut(pVM, pVM->pad, 1);
    return;
}


/**************************************************************************
                        f i c l C o m p i l e T o o l s
** Builds wordset for debugger and TOOLS optional word set
**************************************************************************/

void ficlCompileTools(FICL_DICT *dp)
{
    assert (dp);

    /*
    ** TOOLS and TOOLS EXT
    */
    dictAppendWord(dp, ".s",        displayStack,   FW_DEFAULT);
    dictAppendWord(dp, "bye",       bye,            FW_DEFAULT);
    dictAppendWord(dp, "forget",    forget,         FW_DEFAULT);
    dictAppendWord(dp, "see",       see,            FW_DEFAULT);
    dictAppendWord(dp, "words",     listWords,      FW_DEFAULT);

    /*
    ** Set TOOLS environment query values
    */
    ficlSetEnv("tools",            FICL_TRUE);
    ficlSetEnv("tools-ext",        FICL_FALSE);

    /*
    ** Ficl extras
    */
    dictAppendWord(dp, ".env",      listEnv,        FW_DEFAULT);
    dictAppendWord(dp, "debug",     ficlDebug,      FW_DEFAULT);
	pBreak = 
    dictAppendWord(dp, "debug-break",debugBreak,    FW_DEFAULT);
    dictAppendWord(dp, "forget-wid",forgetWid,      FW_DEFAULT);
    dictAppendWord(dp, "see-xt",    seeXT,          FW_DEFAULT);
    return;
}

