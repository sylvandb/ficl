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




========================================================================
       MICROSOFT FOUNDATION CLASS LIBRARY : ficlwin
========================================================================


AppWizard has created this ficlwin application for you.  This application
not only demonstrates the basics of using the Microsoft Foundation classes
but is also a starting point for writing your application.

This file contains a summary of what you will find in each of the files that
make up your ficlwin application.

ficlwin.h
    This is the main header file for the application.  It includes other
    project specific headers (including Resource.h) and declares the
    CFiclwinApp application class.

ficlwin.cpp
    This is the main application source file that contains the application
    class CFiclwinApp.

ficlwin.rc
    This is a listing of all of the Microsoft Windows resources that the
    program uses.  It includes the icons, bitmaps, and cursors that are stored
    in the RES subdirectory.  This file can be directly edited in Microsoft
	Developer Studio.

res\ficlwin.ico
    This is an icon file, which is used as the application's icon.  This
    icon is included by the main resource file ficlwin.rc.

res\ficlwin.rc2
    This file contains resources that are not edited by Microsoft 
	Developer Studio.  You should place all resources not
	editable by the resource editor in this file.

ficlwin.clw
    This file contains information used by ClassWizard to edit existing
    classes or add new classes.  ClassWizard also uses this file to store
    information needed to create and edit message maps and dialog data
    maps and to create prototype member functions.

/////////////////////////////////////////////////////////////////////////////

For the main frame window:

MainFrm.h, MainFrm.cpp
    These files contain the frame class CMainFrame, which is derived from
    CFrameWnd and controls all SDI frame features.

res\Toolbar.bmp
    This bitmap file is used to create tiled images for the toolbar.
    The initial toolbar and status bar are constructed in the
    CMainFrame class.  Edit this toolbar bitmap along with the
    array in MainFrm.cpp to add more toolbar buttons.

/////////////////////////////////////////////////////////////////////////////

AppWizard creates one document type and one view:

ficlwinDoc.h, ficlwinDoc.cpp - the document
    These files contain your CFiclwinDoc class.  Edit these files to
    add your special document data and to implement file saving and loading
    (via CFiclwinDoc::Serialize).

ficlwinView.h, ficlwinView.cpp - the view of the document
    These files contain your CFiclwinView class.
    CFiclwinView objects are used to view CFiclwinDoc objects.



/////////////////////////////////////////////////////////////////////////////
Other standard files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named ficlwin.pch and a precompiled types file named StdAfx.obj.

Resource.h
    This is the standard header file, which defines new resource IDs.
    Microsoft Developer Studio reads and updates this file.

/////////////////////////////////////////////////////////////////////////////
Other notes:

AppWizard uses "TODO:" to indicate parts of the source code you
should add to or customize.

If your application uses MFC in a shared DLL, and your application is 
in a language other than the operating system's current language, you
will need to copy the corresponding localized resources MFC40XXX.DLL
from the Microsoft Visual C++ CD-ROM onto the system or system32 directory,
and rename it to be MFCLOC.DLL.  ("XXX" stands for the language abbreviation.
For example, MFC40DEU.DLL contains resources translated to German.)  If you
don't do this, some of the UI elements of your application will remain in the
language of the operating system.

/////////////////////////////////////////////////////////////////////////////

YOU SHOULD CAREFULLY READ THE FOLLOWING TERMS AND CONDITIONS BEFORE USING THIS PRODUCT.
IT CONTAINS SOFTWARE, THE USE OF WHICH IS LICENSED BY PALM COMPUTING, INC., A SUBSIDIARY 
OF 3COM CORPORATION (COLLECTIVELY, "3COM"), TO ITS CUSTOMERS FOR THEIR USE ONLY AS SET 
FORTH BELOW.  IF YOU DO NOT AGREE TO THE TERMS AND CONDITIONS OF THIS AGREEMENT,  
DO NOT USE THE SOFTWARE.  USING ANY PART OF THE SOFTWARE INDICATES THAT YOU ACCEPT THESE 
TERMS.

LICENSE:  3Com grants you a nonexclusive license to use the accompanying software program(s)
(the "Software") subject to the terms and restrictions set forth in this License Agreement.  
You are not permitted to lease or rent (except under separate mutually agreeable terms set 
forth in writing), distribute or sublicense the Software or to use the Software in a 
time-sharing arrangement or in any other unauthorized manner.  Further, no license is granted 
to you in the human readable code of the Software (source code).  Except as provided below, 
this License Agreement does not grant you any rights to patents, copyrights, trade secrets, 
trademarks, or any other rights in respect to the Software.

The Software is licensed to be used on any personal computer and/or any 3Com product, provided 
that the Software is used only in connection with 3Com products.  With respect to the Desktop 
Software, you may reproduce and provide one (1) copy of such Software for each personal computer 
or 3Com product on which such Software is used as permitted hereunder.  With respect to the 
Device Software, you may use such Software only on one (1) 3Com product.  Otherwise, the Software 
and supporting documentation may be copied only as essential for backup or archive purposes in 
support of your use of the Software as permitted hereunder.  You must reproduce and include all 
copyright notices and any other proprietary rights notices appearing on the Software on any 
copies that you make.  

NO Assignment; No Reverse Engineering:  You may transfer the Software and this License 
Agreement to another party if the other party agrees in writing to accept the terms and 
conditions of this License Agreement.  If you transfer the Software, you must at the same 
time either transfer all copies of the Software as well as  the supporting documentation 
to the same party or destroy any such materials not transferred.  Except as set forth 
above, you may not transfer or assign the Software or your rights under this License Agreement.  

Modification, reverse engineering, reverse compiling, or disassembly of the Software is 
expressly prohibited.  However, if you are a European Community ("EC") resident, information 
necessary to achieve interoperability of the Software with other programs within the meaning 
of the EC Directive on the Legal Protection of Computer Programs is available to you from 
3Com upon written request.

EXPORT RESTRICTIONS:  You agree that you will not export or re-export the Software or 
accompanying documentation (or any copies thereof) or any products utilizing the Software 
or such documentation in violation of any applicable laws or regulations of the United States 
or the country in which you obtained them.

Trade Secrets; Title:  You acknowledge and agree that the structure, sequence and organization 
of the Software are the valuable trade secrets of 3Com and its suppliers.  You agree to hold 
such trade secrets in confidence.  You further acknowledge and agree that ownership of, and 
title to, the Software and all subsequent copies thereof regardless of the form or media are 
held by 3Com and its suppliers.

UNITED STATES Government Legend:  

The Software is commercial in nature and developed solely at private expense.  The Software 
is delivered as "Commercial Computer Software" as defined in DFARS 252.227-7014 (June 1995) 
or as a commercial item as defined in FAR 2.101(a) and as such is provided with only such 
rights as are provided in this License Agreement, which is 3Com's standard commercial license 
for the Software.  Technical data is provided with limited rights only as provided in 
DFAR 252.227-7015 (Nov. 1995) or FAR 52.227-14 (June 1987), whichever is applicable.

TERM AND TERMINATION:  This License Agreement is effective until terminated.  You may 
terminate it at any time by destroying the Software and documentation together with 
all copies and merged portions in any form.  It will also terminate immediately if 
you fail to comply with any term or condition of this License Agreement.  Upon such 
termination you agree to destroy the Software and documentation, together with all copies 
and merged portions in any form.

GOVERNING LAW:  This License Agreement shall be governed by the laws of the State of 
California as such laws are applied to agreements entered into and to be performed 
entirely within California between California residents and by the laws of the United 
States.  You agree that the United Nations Convention on Contracts for the International 
Sale of Goods (1980) is hereby excluded in its entirety from application to this License Agreement.

NO WARRANTY:  THE SOFTWARE AND ALL RELATED DOCUMENTATION ARE PROVIDED ON AN "AS IS" BASIS 
AND ALL RISK IS WITH YOU.  BECAUSE THE SOFTWARE AND DOCUMENTATION ARE PROVIDED TO YOU FREE 
OF CHARGE, 3COM MAKES NO WARRANTIES, TERMS, OR CONDITIONS, EXPRESS, IMPLIED OR STATUTORY, 
AS TO ANY MATTER WHATSOEVER.  IN PARTICULAR, ANY AND ALL WARRANTIES, TERMS OR CONDITIONS 
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT OF THIRD PARTIES 
RIGHTS ARE EXPRESSLY EXCLUDED.  FURTHER, 3COM MAKES NO REPRESENTATIONS, WARRANTIES, TERMS 
OR CONDITIONS THAT THE SOFTWARE AND DOCUMENTATION PROVIDED ARE FREE OF ERRORS OR VIRUSES 
OR THAT THE SOFTWARE AND DOCUMENTATION ARE SUITABLE FOR YOUR INTENDED USE.   

LIMITATION OF LIABILITY:  IN NO EVENT SHALL 3COM OR ITS SUPPLIERS BE LIABLE TO YOU OR ANY 
OTHER PARTY FOR ANY INCIDENTAL, SPECIAL OR CONSEQUENTIAL DAMAGES, LOSS OF DATA OR DATA 
BEING RENDERED INACCURATE, LOSS OF PROFITS OR REVENUE, OR INTERRUPTION OF BUSINESS IN 
ANY WAY ARISING OUT OF OR RELATED TO THE USE OR INABILITY TO USE THE SOFTWARE AND/OR 
DOCUMENTATION, REGARDLESS OF THE FORM OF ACTION, WHETHER IN CONTRACT, TORT 
(INCLUDING NEGLIGENCE), STRICT PRODUCT LIABILITY OR OTHERWISE, EVEN IF ANY REPRESENTATIVE 
OF 3COM OR ITS SUPPLIERS HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.  NOTHING IN 
THIS AGREEMENT SHALL HAVE THE EFFECT OF LIMITING OR EXCLUDING 3COM'S LIABILITY FOR DEATH 
OR PERSONAL INJURY CAUSED BY ITS OWN  NEGLIGENCE.  THIS DISCLAIMER OF LIABILITY FOR DAMAGES 
WILL NOT BE AFFECTED BY ANY FAILURE OF THE SOLE AND EXCLUSIVE REMEDIES HEREUNDER.

DISCLAIMER:   Some countries, states, or provinces do not allow the exclusion or 
limitation of implied warranties or the limitation of incidental or consequential 
damages for certain products supplied to consumers or the limitation of liability 
for personal injury, so the above limitations and exclusions may be limited in their 
application to you.  When the implied warranties are not allowed to be excluded in 
their entirety, they will be limited to the duration of the applicable written 
warranty.  This warranty gives you specific legal rights which may vary depending 
on local law.

SEVERABILITY:  In the event any provision of this License Agreement is found to be 
invalid, illegal or unenforceable, the validity, legality and enforceability of any 
of the remaining provisions shall not in any way be affected or impaired and a valid, 
legal and enforceable provision of similar intent and economic impact shall be 
substituted therefor.

ENTIRE AGREEMENT:  This License Agreement sets forth the entire understanding and 
agreement between you and 3Com, supersedes all prior agreements, whether written or 
oral, with respect to the Software, and may be amended only in a writing signed by 
both parties.  

