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
** This file contains definitions that are compiled into the
** system dictionary by the first virtual machine to be created.
** Created automagically by ficl/softwords/softcore.pl 
*/


#include "ficl.h"

static char softWords[] = 
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
    "quit ";


void ficlCompileSoftCore(FICL_VM *pVM)
{
    int ret = sizeof (softWords);
    ret = ficlExec(pVM, softWords);
    if (ret == VM_ERREXIT)
        assert(FALSE);
    return;
}


EOF
;

