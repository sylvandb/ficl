rel 3.01 -- 2001

Major contribs by Larry Hastings (larry@hastings.org)
- FILE wordset
- ficlEvaluate wrapper for ficlExec
- ficlInitSystemEx makes it possible to bind selectable properties to VMs at create time
- softcore.py (python version)

Environment contains ficl-version (double)
?number handles trailing decimal point per DOUBLE wordset spec

Fixed broken .env (thanks to Leonid Rosin for spotting this goof)
Fixed broken floating point words that depended on evaluation order of stack pops.
env-constant
env-2constant
dictHashSummary is now commented out unless FICL_WANT_FLOAT (thanks to Leonid Rosin again)

Thanks to David McNab for pointing out that .( should be IMMEDIATE. Now it is.

rel 3.00a -- July 2001

- Fixed broken oo.fr by commenting out vcall stuff using FICL_WANT_VCALL. 
  Vcall is still broken.


rel 3.00 -- June 2001

- Added pSys parameter to most ficlXXXX functions - multiple system support
  dictLookupLoc renamed to ficlLookupLoc after addition of pSys param
  ficlInibtSystem returns a FICL_SYSTEM*
  ficlTermSystem
  ficlNewVM
  ficlLookup
  ficlGetDict
  ficlGetEnv
  ficlSetEnv
  ficlSetEnvD
  ficlGetLoc
  ficlBuild

- Fixed off-by-one bug in ficlParsePrefix
- Ficl parse-steps now work correctly - mods to interpret()
- Made tools.c:isAFiclWord more selective
- Tweaked makefiles and code to make gcc happy under linux
- Vetted all instances of LVALUEtoCELL to make sure they're working on CELL sized operands 
  (for 64 bit compatibility)
- Doc updates

rel 2.06 -- May 2001 (feast or famine around here)

- Debugger changes:
  New debugger command "x" to execute the rest of the command line as ficl
  New debugger command "l" lists the source of the innermost word being debugged
  If you attempt to debug a primitive, it gets executed rather than doing nothing
  r.s displays the stack contents symbolically
- Debugger now runs correctly under ficlwin.
- SEE listing enhanced for use with the debugger
- Added Guy Carver's changes to oo.fr for VTABLE support
- float.c words f> and >f to move floats to and from the param stack, analogous to >r and r>
- LOOKUP - Surrogate precompiled parse step for ficlParseWord (this step is hard 
  coded in INTERPRET)
- License text at top of source files changed from LGPL to BSD by request
- Win32 console version now handles exceptions more gracefully rather than crashing - uses win32
  structured exception handling.
- Fixed BASE bug from 2.05 (was returning the value rather than the address) 
- Fixed ALLOT bug - feeds address units to dictCheck, which expects Cells. Changed dictCheck
  to expect AU. 
- Float stack display word renamed to f.s from .f to be consistent with r.s and .s


rel 2.05 -- April 2001

This is a transitional release, but it adds a lot of new features. 
Ficl 3.0 will change the API to allow multiple concurrent FICL_SYSTEMs. 
This involves the addition of a single parameter to several functions 
(the parameter is a pointer to a FICL_SYSTEM).

* Thanks to everyone who contributed fixes and features for this release! Especially
* Guy Carver, Orjan Gustafson, Larry Hastings, Daniel Sobral, and Reuben Thomas.

- HTML documentation extensively revised
- Simple source debugger -- see tools.c
- The text interpreter is now extensible - this is accomplished through the use
  of ficlAddParseStep. FICL_MAX_PARSE_STEPS limits the number of parse steps
  (default: 8). You can write a precompiled parse step (see ficlParseNumber) and
  append it to the chain, or you can write one in ficl and use ADD-PARSE-STEP 
  to append it. Default parse steps are initialized in ficlInitSystem. You can list
  the parse steps with parse-order ( -- ).
- There is now a FICL_SYSTEM structure. This is a transitional release - version 3.0
  will alter several API prototypes to take this as a parameter, allowing multiple
  systems per process (and therefore multiple dictionaries). For those who use ficl
  under a virtual memory O/S like Linux or Win NT, you can just create multiple ficl
  processes (not threads) instead and save youself the wait.
- Fixes for improved command line operation in testmain.c (Larry Hastings)
- Numerous extensions to OO facility, including a new allot methods, ability
  to catch method invocations (thanks to Daniel Sobral again)
- Incorporated Alpha (64 bit) patches contributed by Daniel Sobral and the freeBSD team
  Ficl is now 64 bit friendly! UNS32 is now FICL_UNS.
- Split SEARCH and SEARCH EXT words from words.c to search.c
- ABORT" now complies with the ANS (-2 THROWs)
- 2LOCALS defined in jhlocal syntax now lose the "2:" in their names. See ficl_loc.html
- Floating point support contributed by Guy Carver (Enable FICL_WANT_FLOAT in sysdep.h).
- Win32 vtable model for objects (Guy Carver)
- Win32 dll load/call suport (Larry Hastings)
- Prefix support (Larry Hastings) (prefix.c prefix.fr FICL_EXTENDED_PREFIX) makes it 
  easy to extend the parser to recignize prefixes like 0x and act on them. Use show-prefixes
  to see what's defined.
- Cleaned up initialization sequence so that it's all in ficlInitSystem

Ficl words
- ABORT" and REFILL fixed (thanks to Daniel Sobral)
- ANS CORE EXT words: 2r@ 2r> 2>r 
- Numerous ANS FLOAT and FLOAT EXT words (Larry Carver) -- see float.c
- ANS DOUBLE words: 2variable
- .S now displays all stack entries on one line, like a stack comment
- wid-get-name   ( -- c-adr u )
  given a wid, returns the address and count of its name. If no name, count is 0
- wid-set-name   ( c-addr -- )
  set optional wid name pointer to the \0 terminated string address specified.
- ficl-named-wordlist  ( -- wid ) "name"
  creates a ficl-wordlist and names it
- last-word  ( -- xt ) 
  returns the xt of the word being defined or most recently defined.
- q@ and q! operate on quadbyte quantities for 64 bit friendliness
- add-parse-step   ( xt -- )
  Allows the parser to be extended. To create a parse step, define a word that
  consumes a counted string from the stack and returns (minimally) a flag. Once installed in the
  parse chain, this word will be called when the previous steps in the chain have failed to
  parse the current token. Upon entry, the token's address and count will be on the stack.
  If the parse step succeeds in parsing the token, it should apply whatever semantics the token
  requires, then push FICL_TRUE on the stack. If it fails, the step should push FICL_FALSE.
  To install the parse step, use add-parse-step passing it the xt of the new parse step. 
  Add-parse-step may fail silently if the parse list is full. You can confirm success using
  parse-order ( -- ). 
- (parse-step)   ( c-addr u -- ??? flag )
  Runtime support for precompiled parse steps (see ficl.c: AddPrecompiledParseStep)
- env-constant ( u -- ) "name"
- env-2constant ( ud -- ) "name"
  set environment values from Ficl. Use .env ( -- ) to view defined symbols 
  in the environment, or environment? (CORE) to find their values.

softcore.fr words
- ORDER now lists wordlists by name
- ficl-named-wordlist
- brand-wordlist

New OO stuff
- Double width locals - prefix a local name with "2:" and it is automatically
  created as a double cell local. Handy for objects. Example:
  : method  { 2:this -- } this --> do-nothing ;
- Class methods ALLOT and ALLOT-ARRAY
- METHOD  define method names globally
- my=> early bind a method call to "this" class
- my=[ ] early bind a string of method calls to "this" class and obj members
- c-> late bind method invocation with CATCH
- metaclass method RESUME-CLASS and instance word SUSPEND-CLASS to create
  mutually referring classes. Example in string.fr
- early binding words are now in the instance-vars wordlist, 
  not visible unless defining a class.
- string.fr enhanced for dynamic allocation and resize of string contents


rel 2.04 -- May 2000
ficlwin:
- Catches exceptions thrown by VM in ficlThread (0 @ for example) rather than
  passing them off to the OS.

ficl bugs vanquished
- Fixed leading delimiter bugs in s" ." .( and ( (reported by Reuben Thomas)
- Makefile tabs restored (thanks to Michael Somos)
- ABORT" now throws -2 per the DPANS (thanks to Daniel Sobral for sharp eyes again)
- ficlExec does not print the prompt string unless (source-id == 0)
- Various fixes from the FreeBSD team 

ficl enhancements
- Words.c: modified ficlCatch to use vmExecute and vmInnerLoop (request of Daniel Sobral)
- Added vmPop and vmPush functions (by request of Lars Krueger ) in vm.c 
  These are shortcuts to the param stack. (Use LVALUEtoCELL to get things into CELL form)
- Added function vmGetStringEx with a flag to specify whether or not to
  skip lead delimiters
- Added non-std word: number?
- Added CORE EXT word AGAIN (by request of Reuben Thomas)
- Added double cell local (2local) support
- Augmented Johns Hopkins local syntax so that locals whose names begin
  with char 2 are treated as 2locals (OK - it's goofy, but handy for OOP)
- C-string class revised and enhanced - now dynamically sized
- C-hashstring class derived from c-string computes hashcode too.

rel 2.03 -- April 1999

ficlwin:
- Edit paste works more sensibly if there's already text on the 
  line being appended to...
- File Menu: recent file list and Open now load files.
- Text ouput function is now faster through use of string 
  caching. Cache flushes at the end of each line and each
  time ficlExec returns.
- Edit/paste now behaves more reasonably for text. File/open
  loads the specified file.
- Registry entries specify dictionary and stack sizes. See
  HKEY_CURRENT_USER/Software/CodeLab/ficlwin/Settings

testmain:
- Added CLOCK ( -- u) , wrapper for the ANSI C clock() function.
  Returns the number of clock ticks elapsed since process start.
- MSEC renamed to MS (in line with the ANS)
- Added CLOCKS/SEC ( -- u) , wrapper for ANSI C CLOCKS_PER_SEC
  constant
- Changed gets() in testmain to fgets() to appease the security gods.


Data structures are now 64 bit friendly.

oo.fr: Added alloc and alloc-array methods of METACLASS to
allocate objects and arrays of objects from the heap. Free method
of OBJECT frees the storage. (requires MEMORY wordset)

Added CORE EXT word WITHIN
Added DOUBLE word DNEGATE

Added ficlSetStackSize to specify param and return stack sizes. See ficl.h

Added ficlExecXT in ficl.c/h - executes a FICL_WORD given its address.

Added Michael Gauland's ficlLongMul and ficlLongDiv and support 
routines to math64.c and .h. These routines are coded in C, and are
compiled only if PORTABLE_LONGMULDIV == 1 (default is 0).

Added definition of ficlRealloc to sysdep.c (needed for memory
allocation wordset). If your target OS supports realloc(),
you'll probably want to redefine ficlRealloc in those terms.
The default version does ficlFree followed by ficlMalloc.

[Thanks to Daniel Sobral of FreeBSD for suggesting or implementing 
the next six changes!]
- Added CATCH and THROW (EXCEPTION word set) 
- Added MEMORY allocation word set. Requires ficlRealloc
- EVALUATE respects count parameter, and also passes exceptional
  return conditions back out to the calling instance of ficlExec.
- VM_QUIT clears locals dictionary in ficlExec()
- ficlExec pushes ip and executes interpret at the right times so that
  nested calls to ficlExec behave the way you'd expect them to.
- Control word match check upgraded. Control structure mismatches
  are now errors, not warnings, since the check accepts all 
  syntactally legal constructs.

Added vmInnerLoop to vm.h. This function/macro factors the inner 
interpreter out of ficlExec so it can be used in other places. 
Function/macro behavior is conditioned on INLINE_INNER_LOOP
in sysdep.h. Default: 1 unless _DEBUG is set. In part, this is because
VC++ 5 goes apoplectic when trying to compile it as a function. See 
comments in vm.c

Bug fix in isNumber(): used to treat chars between 'Z' and 'a'
as valid in base 10... (harmless, but weird) (Ficl Finger of Fate
award to Phil Martel for this one ;-)  )

softcore.pl now removes comments, spaces at the start and
  end of lines. As a result:
  sizeof (softWords) == 7663 bytes (used to be 20000)
  and consumes 11384 bytes of dictionary when compiled
  (so it's cheaper to store as text than code, for the 
  memory-conscious)

Deleted 3Com license paste-o in this file (oops)

rel 2.02 -- 17 October 1998

Changed ficlExec so that the search order really does get reset
on an ERREXIT as advertised.

marker   ( "name" -- )
forget   ( "name" -- )
forget-wid  ( wid -- )

SOURCE-ID is now equal to the (<>0) file id when loading a file 
(Win32 only), and -1 when doing EVALUATE. This means that 
REFILL now works correctly when loading a file...
Win32 LOAD command (oops) now complies with the FILE wordset
specification of FILE-INCLUDE (REFILL returns FALSE at EOF)

ficl-wordlist   ( nBins -- wid )  
    Creates a hashed wordlist with the number of bins specified.
    Best hash performance if nBins is prime!
ficl-vocabulary   ( nBins "name" -- )
    Uses ficl-wordlist to make a vocabulary with the given name
    and number of hash bins

:NONAME (bug fix) no longer pushes control marker for colon and
    exec token in wrong order.
WORDS ignores :noname (anonymous) definitions 

dictUnsmudge no longer links anonymous definitions into the hash

HIDE   ( -- wid-was )
new wordlist called HIDDEN and word HIDE for keeping execution
factors from cluttering the default namespace any worse than it 
already is... HIDE sets HIDDEN as the compile wordlist and pushes 
it onto the search order. When finished compiling execution factors,
a call to SET-CURRENT restores the previous compile wordlist. When
finished compiling words that use the execution factors, use PREVIOUS
to restore the prior search order.

Added (my current understanding of) the Johns Hopkins local syntax
in file softwords/jhlocal.fr. This is in the default version of softcore.c
instead of the previous {{ }} local syntax. That syntax is still available
in softwords/ficllocal.fr if you want it instead. Ficl's implementation
of the Johns Hopkins local syntax:
    { a b c | d -- e f }
      ^^^^^   ^    ^^ this is a comment
      |||||   \ this local is cleared initially
      \\\\\ these come off the stack in the correct order

A, b, and c are initialized off the stack in right to left order
(c gets the top of stack). D is initialized to zero. E and f are
treated as comments. The | and -- delimiters are optional. If they
appear, they must appear once only, and in the order shown.


OOP vocabulary - no longer in the search order at startup.
No longer default compile voc at startup

oo.fr 

Revised to make more extensive use of early binding for speed.

META (constant) pushes the address of METACLASS. This word is
    not immediate. Makes it easier to deal with early binding of
    class methods.

object::init now uses metaclass::get-size explicitly rather
    than object::size.

classes.fr

Added c-ptr base class for all pointer classes. derived 
    c-cellPtr, c-bytePtr, and c-wordPtr from c-ptr. These
    classes model pointers to raw scalar types.


rel 2.01
18 sep 98 -- (local) changed so that it does not leave anything 
on the stack after it runs (previously left a marker after the 
first local, consumed it after the last local). Marker is now
a static of (local).

Added {{ -- }} local syntax with variable reordering




