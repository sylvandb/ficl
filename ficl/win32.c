/* 
 * win32.c
 * submitted to Ficl by Larry Hastings, larry@hastings.org
 * Additional Win32 words by Guy Carver
 *
 * adds calling arbitrary DLL function calls from inside Ficl.
 *
 * note that Microsoft's own header files won't compile without
 * "language extensions" (anonymous structs/unions) turned on.
 * and even with that, it still gives a warning in rpcasync.h
 * for something that compiles clean in C++.  I turned it off.
 *
 */
#pragma warning(disable : 4115)
#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <direct.h>

#include "ficl.h"

static void loadLibrary(FICL_VM *pVM) /* ( address length --  hmodule ) */
{
    int length = stackPopINT(pVM->pStack);
    void *address = (void *)stackPopPtr(pVM->pStack);

    char *buf = (char *)_alloca(length + 1);
    memcpy(buf, address, length);
    buf[length] = 0;

    stackPushINT(pVM->pStack, (int)LoadLibrary(buf));
}

static void getProcAddress(FICL_VM *pVM) /* ( address length hmodule -- ) */
{
    HMODULE hModule = (HMODULE)stackPopINT(pVM->pStack);
    int length = stackPopINT(pVM->pStack);
    void *address = (void *)stackPopPtr(pVM->pStack);

    char *buf = (char *)_alloca(length + 1);
    memcpy(buf, address, length);
    buf[length] = 0;

    stackPushINT(pVM->pStack, (int)GetProcAddress(hModule, buf));
}


static void freeLibrary(FICL_VM *pVM) /* ( hmodule -- ) */
{
    HMODULE hModule = (HMODULE)stackPopINT(pVM->pStack);
    FreeLibrary(hModule);
}


static void uAddrToCString(FICL_VM *pVM) /* ( address length -- c-string ) */
{
    int length = stackPopINT(pVM->pStack);
    void *address = (void *)stackPopPtr(pVM->pStack);

    char *buf = (char *)malloc(length + 1);
    memcpy(buf, address, length);
    buf[length] = 0;
    stackPushPtr(pVM->pStack, buf);
    return;
}


static void callNativeFunction(FICL_VM *pVM) /* ( ... argcount fnaddress popstack -- returnvalue ) */
{
    int popstack = stackPopINT(pVM->pStack);
    int fnaddress = stackPopINT(pVM->pStack);
    int argcount = stackPopINT(pVM->pStack);
    int returnvalue;

    int i;
    for (i = 0; i < argcount; i++)
    {
        int nextarg = stackPopINT(pVM->pStack);
        __asm
        {
            mov eax, nextarg
            push eax
        }
    }


    __asm
    {
        call fnaddress
        mov returnvalue, eax
    }

    /*
     * if popstack is nonzero,
     * the arguments are popped off the stack after calling
     */
    if (popstack)
    {
        argcount *= 4;
        __asm add esp, argcount
    }
    stackPushINT(pVM->pStack, returnvalue);
    return;
}


/**************************************************************************
                        v c a l l
** Call a class method. (Contributed by Guy Carver)
** FORTH:   (params inst paramcnt vtableindex -- res )
** INFO:    paramcnt has msb set if return value is desired.
**************************************************************************/
static void VCall(FICL_VM *pVM)
{
    int ind,p,paramCnt;
    void *instance;
    int I;
    
#if FICL_ROBUST > 1
    vmCheckStack(pVM,3,1);
#endif

    ind = POPINT() * 4;
    paramCnt = POPINT();
    instance = POPPTR();                        //Get instance of class.

    __asm push ecx                              //Save ecx.
    __asm push esp                              //Save stack.

    I = paramCnt & 0xFF;                        //Strip off any flags.

#if FICL_ROBUST > 1
    vmCheckStack(pVM,I,0);
#endif

    while(I--)                                  //Loop for parameter count.
    {
        p = POPINT();
        __asm
        {
            mov eax,p
            push eax                            //Push on stack.
        }
    }
    __asm
    {
        mov ecx,instance                        //Set ecx to instance.
        mov eax,[ecx]                           //Get method pointer.
        add eax,ind
        call [eax]                              //Call method.
        mov p,eax                               //Put result in p.
        pop esp
        pop ecx                                 //Restore ecx and esp.
    }
    if (paramCnt & 0x80000000)                  //If supposed to return a result.
        PUSHINT(p);                             //Store result.
}


//**************************************************************
//Load forth file.
//ENTER:    pVM = Pointer to forth virtual machine.
//FORTH: ( -<FileName>- )
//**************************************************************
static void ForthLoad(FICL_VM *pVM)
{
    char cp[256];
    char fileName[256];
    FILE *fp;
    int result = 0;
    CELL id;
    int nLine = 0;
    FICL_STRING *pFileName = (FICL_STRING *)fileName;

    vmGetString(pVM,pFileName, ' ');

    if (pFileName->count <= 0)
    {
        vmTextOut(pVM,"Type fload filename", 1);
        return;
    }

    fp = fopen(pFileName->text, "r");
    if (fp)
    {
        id = pVM->sourceID;
        pVM->sourceID.p = (void *)fp;           //Set input source id.

        while (fgets(cp,256,fp))                //Read line.
        {
            int len = strlen(cp) - 1;           //Get length.

            nLine++;                            //Inc line count.
            if (len > 0)                        //if length.
            {
                cp[len] = 0;                    //Make sure null terminated.
                result = ficlExec(pVM,cp);      //Execute line.
                if ((result == VM_ERREXIT)      //If exit.
                    || (result == VM_USEREXIT)
                    || (result == VM_QUIT))
                {
                    pVM->sourceID = id;
                    fclose(fp);
                    vmThrowErr(pVM, "Error loading file <%s> line %d", pFileName->text, nLine);
                    break;
                }
            }
        }
        pVM->sourceID.i = -1;
        ficlExec(pVM,"");                       //Empty line to flush any pending refills.
        pVM->sourceID = id;                     //Reset source ID.
        fclose(fp);
        if (result == VM_USEREXIT)              //If user exit.
            vmThrow(pVM,VM_USEREXIT);           //Resend user exit code.
    }
    else
    {
        vmTextOut(pVM,"Unable to open file: ", 0);
        vmTextOut(pVM, pFileName->text,1);
    }
}

//********************************************************************************
//
//********************************************************************************
static STRINGINFO parseFileName(FICL_VM *pVM)
{
    STRINGINFO si;
    char *pSrc = vmGetInBuf(pVM);
    si.cp = pSrc;                               /* mark start of text */
    while ((*pSrc != ' ') && (*pSrc != 0) && (*pSrc != '\n'))
    {
        if (*(pSrc++) == '\\')                  /* find next delimiter or end */
            si.cp = pSrc;
    }
    si.count = pSrc - si.cp;                    /* set length of result */
    return(si);
}

//********************************************************************************
//check for included file and load if not loaded.
//********************************************************************************
static void include(FICL_VM *pVM)
{
    STRINGINFO si;
    FICL_WORD *pFW;
    FICL_DICT *dp  = vmGetDict(pVM);
    FICL_CODE pCreateParen = ficlLookup(pVM->pSys, "(create)")->code;

    si = parseFileName(pVM);

    if (si.count)
    {
        pFW = dictLookup(dp, si);
        if (!pFW)                               //Forget word.
        {
            dictAppendWord2(dp, si, pCreateParen, FW_DEFAULT);
            dictAllotCells(dp, 1);
            ForthLoad(pVM);
        }
    }
}

//********************************************************************************
//check for included file and kill it if its included to reload.
//********************************************************************************
static void reinclude(FICL_VM *pVM)
{
    STRINGINFO si;
    FICL_WORD *pFW;
    FICL_DICT *dp  = vmGetDict(pVM);
    FICL_CODE pCreateParen = ficlLookup(pVM->pSys, "(create)")->code;

    si = parseFileName(pVM);

    if (si.count)
    {
        pFW = dictLookup(dp, si);
        if (pFW)                                //Forget word.
        {
            hashForget(dp->pCompile,pFW->name);
            dp->here = PTRtoCELL (pFW->name);
        }

        dictAppendWord2(dp, si, pCreateParen, FW_DEFAULT);
        dictAllotCells(dp, 1);
        ForthLoad(pVM);
    }
}


/**************************************************************************
                        f i c l C o m p i l e P l a t f o r m
** Build Win32 platform extensions into the system dictionary
**************************************************************************/
void ficlCompilePlatform(FICL_SYSTEM *pSys)
{
    FICL_DICT *dp = pSys->dp;
    assert (dp);

    dictAppendWord(dp, "loadlibrary",    loadLibrary,    FW_DEFAULT);
    dictAppendWord(dp, "getprocaddress", getProcAddress, FW_DEFAULT);
    dictAppendWord(dp, "freelibrary",    freeLibrary,    FW_DEFAULT);
    dictAppendWord(dp, "uaddr->cstring", uAddrToCString, FW_DEFAULT);
    dictAppendWord(dp, "callnativefunction", 
                                         callNativeFunction,
                                                         FW_DEFAULT);
    dictAppendWord(dp, "vcall",          VCall,          FW_DEFAULT);
    dictAppendWord(dp, "include",        include,        FW_DEFAULT);
    dictAppendWord(dp, "reinclude",      reinclude,      FW_DEFAULT);

    return;
}

