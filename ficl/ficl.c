/*******************************************************************
** f i c l . c
** Forth Inspired Command Language - external interface
** Author: John Sadler (john_sadler@alum.mit.edu)
** Created: 19 July 1997
** $Id$
*******************************************************************/
/*
** This is an ANS Forth interpreter written in C.
** Ficl uses Forth syntax for its commands, but turns the Forth 
** model on its head in other respects.
** Ficl provides facilities for interoperating
** with programs written in C: C functions can be exported to Ficl,
** and Ficl commands can be executed via a C calling interface. The
** interpreter is re-entrant, so it can be used in multiple instances
** in a multitasking system. Unlike Forth, Ficl's outer interpreter
** expects a text block as input, and returns to the caller after each
** text block, so the data pump is somewhere in external code in the 
** style of TCL.
**
** Code is written in ANSI C for portability. 
*/
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
#include <string.h>
#include "ficl.h"


/*
** System statics
** The system builds a global dictionary during its start
** sequence. This is shared by all interpreter instances.
** Therefore only one instance can update the dictionary
** at a time. The system imports a locking function that
** you can override in order to control update access to
** the dictionary. The function is stubbed out by default,
** but you can insert one: #define FICL_MULTITHREAD 1
** and supply your own version of ficlLockDictionary.
*/
static FICL_SYSTEM *pSys = NULL;

static int defaultStack = FICL_DEFAULT_STACK;
static int defaultDict  = FICL_DEFAULT_DICT;


/**************************************************************************
                        f i c l I n i t S y s t e m
** Binds a global dictionary to the interpreter system. 
** You specify the address and size of the allocated area.
** After that, ficl manages it.
** First step is to set up the static pointers to the area.
** Then write the "precompiled" portion of the dictionary in.
** The dictionary needs to be at least large enough to hold the
** precompiled part. Try 1K cells minimum. Use "words" to find
** out how much of the dictionary is used at any time.
**************************************************************************/
void ficlInitSystem(int nDictCells)
{
    pSys = ficlMalloc(sizeof (FICL_SYSTEM));
    assert(pSys);

    memset(pSys, 0, sizeof (FICL_SYSTEM));

    if (nDictCells <= 0)
        nDictCells = defaultDict;

    pSys->dp = dictCreateHashed((unsigned)nDictCells, HASHSIZE);
    pSys->dp->pForthWords->name = "forth-wordlist";

    pSys->envp = dictCreate((unsigned)FICL_DEFAULT_ENV);
    pSys->envp->pForthWords->name = "environment";

#if FICL_WANT_LOCALS
    /*
    ** The locals dictionary is only searched while compiling,
    ** but this is where speed is most important. On the other
    ** hand, the dictionary gets emptied after each use of locals
    ** The need to balance search speed with the cost of the empty
    ** operation led me to select a single-threaded list...
    */
    pSys->localp = dictCreate((unsigned)FICL_MAX_LOCALS * CELLS_PER_WORD);
#endif

    /*
    ** Establish the parse order. Note that prefixes precede numbers -
    ** this allows constructs like "0b101010" which would parse as a
    ** valid hex value otherwise.
    */
    ficlCompilePrefix(pSys);
    ficlAddPrecompiledParseStep(pSys, "number?", ficlParseNumber);

    /*
    ** Build the precompiled dictionary and load softwords. We need a temporary
    ** VM to do this - ficlNewVM links one to the head of the system VM list.
    ** ficlCompilePlatform (defined in win32.c, for example) adds platform specific words.
    */
    ficlCompileCore(pSys);
#if FICL_WANT_FLOAT
    ficlCompileFloat(pSys);
#endif

#if FICL_PLATFORM_EXTEND
    ficlCompilePlatform(pSys);
#endif

    /*
    ** Now we can create a VM to compile the softwords. Note that the VM initialization
    ** code needs to be able to find "interpret" in the dictionary in order to 
    ** succeed, so as presently constructed ficlCompileCore has to finish before
    ** a VM can be created successfully.
    */
    ficlNewVM();
    ficlCompileSoftCore(pSys);
    ficlFreeVM(pSys->vmList);


    return;
}


/**************************************************************************
                        f i c l A d d P a r s e S t e p
** Appends a parse step function to the end of the parse list (see 
** FICL_PARSE_STEP notes in ficl.h for details). Returns 0 if successful,
** nonzero if there's no more room in the list.
**************************************************************************/
int ficlAddParseStep(FICL_SYSTEM *pSys, FICL_WORD *pFW)
{
    int i;
    for (i = 0; i < FICL_MAX_PARSE_STEPS; i++)
    {
        if (pSys->parseList[i] == NULL)
        {
            pSys->parseList[i] = pFW;
            return 0;
        }
    }

    return 1;
}


/*
** Compile a word into the dictionary that invokes the specified FICL_PARSE_STEP
** function. It is up to the user (as usual in Forth) to make sure the stack 
** preconditions are valid (there needs to be a counted string on top of the stack)
** before using the resulting word.
*/
void ficlAddPrecompiledParseStep(FICL_SYSTEM *pSys, char *name, FICL_PARSE_STEP pStep)
{
    FICL_DICT *dp = pSys->dp;
    FICL_WORD *pFW = dictAppendWord(dp, name, parseStepParen, FW_DEFAULT);
    dictAppendCell(dp, LVALUEtoCELL(pStep));
    ficlAddParseStep(pSys, pFW);
}


/*
** This word lists the parse steps in order
*/
void ficlListParseSteps(FICL_VM *pVM)
{
    int i;
    FICL_SYSTEM *pSys = pVM->pSys;
    assert(pSys);

    vmTextOut(pVM, "Parse steps:", 1);
    vmTextOut(pVM, "lookup", 1);

    for (i = 0; i < FICL_MAX_PARSE_STEPS; i++)
    {
        if (pSys->parseList[i] != NULL)
        {
            vmTextOut(pVM, pSys->parseList[i]->name, 1);
        }
        else break;
    }
    return;
}


/**************************************************************************
                        f i c l N e w V M
** Create a new virtual machine and link it into the system list
** of VMs for later cleanup by ficlTermSystem.
**************************************************************************/
FICL_VM *ficlNewVM(void)
{
    FICL_VM *pVM = vmCreate(NULL, defaultStack, defaultStack);
    pVM->link = pSys->vmList;
    pVM->pSys = pSys;

    pSys->vmList = pVM;
    return pVM;
}


/**************************************************************************
                        f i c l F r e e V M
** Removes the VM in question from the system VM list and deletes the
** memory allocated to it. This is an optional call, since ficlTermSystem
** will do this cleanup for you. This function is handy if you're going to
** do a lot of dynamic creation of VMs.
**************************************************************************/
void ficlFreeVM(FICL_VM *pVM)
{
    FICL_VM *pList = pSys->vmList;

    assert(pVM != 0);

    if (pSys->vmList == pVM)
    {
        pSys->vmList = pSys->vmList->link;
    }
    else for (; pList != NULL; pList = pList->link)
    {
        if (pList->link == pVM)
        {
            pList->link = pVM->link;
            break;
        }
    }

    if (pList)
        vmDelete(pVM);
    return;
}


/**************************************************************************
                        f i c l B u i l d
** Builds a word into the dictionary.
** Preconditions: system must be initialized, and there must
** be enough space for the new word's header! Operation is
** controlled by ficlLockDictionary, so any initialization
** required by your version of the function (if you overrode
** it) must be complete at this point.
** Parameters:
** name  -- duh, the name of the word
** code  -- code to execute when the word is invoked - must take a single param
**          pointer to a FICL_VM
** flags -- 0 or more of F_IMMEDIATE, F_COMPILE, use bitwise OR!
** 
**************************************************************************/
int ficlBuild(char *name, FICL_CODE code, char flags)
{
    int err = ficlLockDictionary(TRUE);
    if (err) return err;

    assert(dictCellsAvail(pSys->dp) > sizeof (FICL_WORD) / sizeof (CELL));
    dictAppendWord(pSys->dp, name, code, flags);

    ficlLockDictionary(FALSE);
    return 0;
}


/**************************************************************************
                        f i c l E x e c
** Evaluates a block of input text in the context of the
** specified interpreter. Emits any requested output to the
** interpreter's output function.
**
** Contains the "inner interpreter" code in a tight loop
**
** Returns one of the VM_XXXX codes defined in ficl.h:
** VM_OUTOFTEXT is the normal exit condition
** VM_ERREXIT means that the interp encountered a syntax error
**      and the vm has been reset to recover (some or all
**      of the text block got ignored
** VM_USEREXIT means that the user executed the "bye" command
**      to shut down the interpreter. This would be a good
**      time to delete the vm, etc -- or you can ignore this
**      signal.
**************************************************************************/
int ficlExec(FICL_VM *pVM, char *pText)
{
    return ficlExecC(pVM, pText, -1);
}

int ficlExecC(FICL_VM *pVM, char *pText, FICL_INT size)
{
    FICL_WORD **pInterp =  pSys->pInterp;
    FICL_DICT *dp = pSys->dp;

    int        except;
    jmp_buf    vmState;
    jmp_buf   *oldState;
    TIB        saveTib;

    if (!pInterp[0])
    {
        pInterp[0] = ficlLookup("interpret");
        pInterp[1] = ficlLookup("(branch)");
        pInterp[2] = (FICL_WORD *)(void *)(-2);
    }
    
    assert(pInterp[0]);
    assert(pVM);

    if (size < 0)
        size = strlen(pText);

    vmPushTib(pVM, pText, size, &saveTib);

    /*
    ** Save and restore VM's jmp_buf to enable nested calls to ficlExec 
    */
    oldState = pVM->pState;
    pVM->pState = &vmState; /* This has to come before the setjmp! */
    except = setjmp(vmState);

    switch (except)
    {
    case 0:
        if (pVM->fRestart)
        {
            pVM->runningWord->code(pVM);
            pVM->fRestart = 0;
        }
        else
        {   /* set VM up to interpret text */
            vmPushIP(pVM, &pInterp[0]);
        }

        vmInnerLoop(pVM);
        break;

    case VM_RESTART:
        pVM->fRestart = 1;
        except = VM_OUTOFTEXT;
        break;

    case VM_OUTOFTEXT:
        vmPopIP(pVM);
        if ((pVM->state != COMPILE) && (pVM->sourceID.i == 0))
            ficlTextOut(pVM, FICL_PROMPT, 0);
        break;

    case VM_USEREXIT:
    case VM_INNEREXIT:
    case VM_BREAK:
        break;

    case VM_QUIT:
        if (pVM->state == COMPILE)
        {
            dictAbortDefinition(dp);
#if FICL_WANT_LOCALS
            dictEmpty(pSys->localp, pSys->localp->pForthWords->size);
#endif
        }
        vmQuit(pVM);
        break;

    case VM_ERREXIT:
    case VM_ABORT:
    case VM_ABORTQ:
    default:    /* user defined exit code?? */
        if (pVM->state == COMPILE)
        {
            dictAbortDefinition(dp);
#if FICL_WANT_LOCALS
            dictEmpty(pSys->localp, pSys->localp->pForthWords->size);
#endif
        }
        dictResetSearchOrder(dp);
        vmReset(pVM);
        break;
   }

    pVM->pState    = oldState;
    vmPopTib(pVM, &saveTib);
    return (except);
}


/**************************************************************************
                        f i c l E x e c X T
** Given a pointer to a FICL_WORD, push an inner interpreter and
** execute the word to completion. This is in contrast with vmExecute,
** which does not guarantee that the word will have completed when
** the function returns (ie in the case of colon definitions, which
** need an inner interpreter to finish)
**
** Returns one of the VM_XXXX exception codes listed in ficl.h. Normal
** exit condition is VM_INNEREXIT, ficl's private signal to exit the
** inner loop under normal circumstances. If another code is thrown to
** exit the loop, this function will re-throw it if it's nested under
** itself or ficlExec.
**
** NOTE: this function is intended so that C code can execute ficlWords
** given their address in the dictionary (xt).
**************************************************************************/
int ficlExecXT(FICL_VM *pVM, FICL_WORD *pWord)
{
    static FICL_WORD *pQuit = NULL;
    int        except;
    jmp_buf    vmState;
    jmp_buf   *oldState;
    FICL_WORD *oldRunningWord;

    if (!pQuit)
        pQuit = ficlLookup("exit-inner");

    assert(pVM);
    assert(pQuit);
    
    /* 
    ** Save the runningword so that RESTART behaves correctly
    ** over nested calls.
    */
    oldRunningWord = pVM->runningWord;
    /*
    ** Save and restore VM's jmp_buf to enable nested calls
    */
    oldState = pVM->pState;
    pVM->pState = &vmState; /* This has to come before the setjmp! */
    except = setjmp(vmState);

    if (except)
        vmPopIP(pVM);
    else
        vmPushIP(pVM, &pQuit);

    switch (except)
    {
    case 0:
        vmExecute(pVM, pWord);
        vmInnerLoop(pVM);
        break;

    case VM_INNEREXIT:
    case VM_BREAK:
        break;

    case VM_RESTART:
    case VM_OUTOFTEXT:
    case VM_USEREXIT:
    case VM_QUIT:
    case VM_ERREXIT:
    case VM_ABORT:
    case VM_ABORTQ:
    default:    /* user defined exit code?? */
        if (oldState)
        {
            pVM->pState = oldState;
            vmThrow(pVM, except);
        }
        break;
    }

    pVM->pState    = oldState;
    pVM->runningWord = oldRunningWord;
    return (except);
}


/**************************************************************************
                        f i c l L o o k u p
** Look in the system dictionary for a match to the given name. If
** found, return the address of the corresponding FICL_WORD. Otherwise
** return NULL.
**************************************************************************/
FICL_WORD *ficlLookup(char *name)
{
    STRINGINFO si;
    SI_PSZ(si, name);
    return dictLookup(pSys->dp, si);
}


/**************************************************************************
                        f i c l G e t D i c t
** Returns the address of the system dictionary
**************************************************************************/
FICL_DICT *ficlGetDict(void)
{
    return pSys->dp;
}


/**************************************************************************
                        f i c l G e t E n v
** Returns the address of the system environment space
**************************************************************************/
FICL_DICT *ficlGetEnv(void)
{
    return pSys->envp;
}


/**************************************************************************
                        f i c l S e t E n v
** Create an environment variable with a one-CELL payload. ficlSetEnvD
** makes one with a two-CELL payload.
**************************************************************************/
void ficlSetEnv(char *name, FICL_UNS value)
{
    STRINGINFO si;
    FICL_WORD *pFW;
    FICL_DICT *envp = pSys->envp;

    SI_PSZ(si, name);
    pFW = dictLookup(envp, si);

    if (pFW == NULL)
    {
        dictAppendWord(envp, name, constantParen, FW_DEFAULT);
        dictAppendCell(envp, LVALUEtoCELL(value));
    }
    else
    {
        pFW->param[0] = LVALUEtoCELL(value);
    }

    return;
}

void ficlSetEnvD(char *name, FICL_UNS hi, FICL_UNS lo)
{
    FICL_WORD *pFW;
    STRINGINFO si;
    FICL_DICT *envp = pSys->envp;
    SI_PSZ(si, name);
    pFW = dictLookup(envp, si);

    if (pFW == NULL)
    {
        dictAppendWord(envp, name, twoConstParen, FW_DEFAULT);
        dictAppendCell(envp, LVALUEtoCELL(lo));
        dictAppendCell(envp, LVALUEtoCELL(hi));
    }
    else
    {
        pFW->param[0] = LVALUEtoCELL(lo);
        pFW->param[1] = LVALUEtoCELL(hi);
    }

    return;
}


/**************************************************************************
                        f i c l G e t L o c
** Returns the address of the system locals dictionary. This dict is
** only used during compilation, and is shared by all VMs.
**************************************************************************/
#if FICL_WANT_LOCALS
FICL_DICT *ficlGetLoc(void)
{
    return pSys->localp;
}
#endif



/**************************************************************************
                        f i c l S e t S t a c k S i z e
** Set the stack sizes (return and parameter) to be used for all
** subsequently created VMs. Returns actual stack size to be used.
**************************************************************************/
int ficlSetStackSize(int nStackCells)
{
    if (nStackCells >= FICL_DEFAULT_STACK)
        defaultStack = nStackCells;
    else
        defaultStack = FICL_DEFAULT_STACK;

    return defaultStack;
}


/**************************************************************************
                        f i c l T e r m S y s t e m
** Tear the system down by deleting the dictionaries and all VMs.
** This saves you from having to keep track of all that stuff.
**************************************************************************/
void ficlTermSystem(void)
{
    if (pSys->dp)
        dictDelete(pSys->dp);
    pSys->dp = NULL;

    if (pSys->envp)
        dictDelete(pSys->envp);
    pSys->envp = NULL;

#if FICL_WANT_LOCALS
    if (pSys->localp)
        dictDelete(pSys->localp);
    pSys->localp = NULL;
#endif

    while (pSys->vmList != NULL)
    {
        FICL_VM *pVM = pSys->vmList;
        pSys->vmList = pSys->vmList->link;
        vmDelete(pVM);
    }

    ficlFree(pSys);
    pSys = NULL;
    return;
}


