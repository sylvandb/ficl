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
** Copyright (c) 1997-2001 John Sadler (john_sadler\@alum.mit.edu)
** All rights reserved.
**
** Get the latest Ficl release at http://ficl.sourceforge.net
**
** I am interested in hearing from anyone who uses ficl. If you have
** a problem, a success story, a defect, an enhancement request, or
** if you would like to contribute to the ficl release, please send
** contact me by email at the address above.
**
** L I C E N S E  and  D I S C L A I M E R
** 
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
** OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
** SUCH DAMAGE.
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

