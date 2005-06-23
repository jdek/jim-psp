/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2003 Marcus R. Brown (mrbrown@0xd6.org)
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
*/

#include <stdio.h>
#include <psptypes.h>
#include <pspkernel.h>

extern void _exit(int code);

__attribute__((weak))
void abort()
{
	printf("Program aborted.\n");

	while (1)
		_exit(1);
}

__attribute__((weak))
void exit(int retval)
{
	while (1)
		_exit(retval);
}