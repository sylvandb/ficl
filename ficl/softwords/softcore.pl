#! \perl\bin\perl.exe
# Convert forth source files to a giant C string

$now = localtime;

print <<EOF
/*******************************************************************
** s o f t c o r e . c
** Forth Inspired Command Language - 
** Words from CORE set written in FICL
** Author: John Sadler (john_sadler\@alum.mit.edu)
** Created: 27 December 1997
** Last update: $now
*******************************************************************/
/*
** DO NOT EDIT THIS FILE -- it is generated by softwords/softcore.pl
** Make changes to the .fr files in ficl/softwords instead.
** This file contains definitions that are compiled into the
** system dictionary by the first virtual machine to be created.
** Created automagically by ficl/softwords/softcore.pl 
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


#include "ficl.h"

static char softWords[] = 
#if FICL_WANT_SOFTWORDS
EOF
;

$commenting = 0;

while (<>) {
    s"\n$"";            # remove EOL
    s/\"/\\\"/g;        # escape quotes

    #
    # emit lines beginnning with "\ **" as C comments
    #
    if (/^\\\s\*\*/)  {	
        s"^\\ "";
        if ($commenting == 0) {
            print "/*\n";
        }
        $commenting = 1;
        print "$_\n";
        next;
    }

    if ($commenting == 1) {
        print "*/\n";
    }

    $commenting = 0;

    #
    # ignore empty lines and lines containing
    # only empty comments
    #
    next if /^\s*\\\s*$/;
    next if /^\s*$/;

    #
	# pass commented preprocessor directives
    # == lines starting with "\ #"
    # (supports single line directives only)
    #
    if (/^\\\s#/)  {
        s"^\\ "";
        print "$_\n";
        next;
    }

    next if /^\s*\\ /;  # toss all other \ comment lines
    s"\\\s+.*$"" ;      # lop off trailing \ comments
    s"\s+\(\s.*?\)""g;  # delete ( ) comments
    s"^\s+"";           # remove leading spaces
    s"\s+$"";           # remove trailing spaces

    #
    # emit whatever's left as quoted string fragments
    #
#    $out = "    \"" . $_ . " \\n\"";
     $out = "    \"" . $_ . " \"";
    print "$out\n";
}

print <<EOF
#endif /* WANT_SOFTWORDS */
    "quit ";


void ficlCompileSoftCore(FICL_SYSTEM *pSys)
{
    FICL_VM *pVM = pSys->vmList;
    int ret = sizeof (softWords);
	assert(pVM);

    ret = ficlExec(pVM, softWords);
    if (ret == VM_ERREXIT)
        assert(FALSE);
    return;
}


EOF
;

