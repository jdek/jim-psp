/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2004 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Sam Lantinga
    slouken@libsdl.org
*/

/* PSP port contributed by Marcus R. Brown <mrbrown@ocgnet.org>. */

#ifdef SAVE_RCSID
static char rcsid =
 "@(#) $Id: SDL_diskaudio.c,v 1.5 2004/01/04 16:49:13 slouken Exp $";
#endif

/* Output raw audio data to a file. */

#include <stdio.h>

#include "SDL_audio.h"
#include "SDL_error.h"
#include "SDL_audiomem.h"
#include "SDL_audio_c.h"
#include "SDL_timer.h"
#include "SDL_audiodev_c.h"
#include "SDL_pspaudio.h"

/* The tag name used by PSP audio */
#define PSPAUD_DRIVER_NAME         "psp"

/* Audio driver functions */
static int PSPAUD_OpenAudio(_THIS, SDL_AudioSpec *spec);
static void PSPAUD_WaitAudio(_THIS);
static void PSPAUD_PlayAudio(_THIS);
static Uint8 *PSPAUD_GetAudioBuf(_THIS);
static void PSPAUD_CloseAudio(_THIS);

/* Audio driver bootstrap functions */
static int PSPAUD_Available(void)
{
	return 1;
}

static void PSPAUD_DeleteDevice(SDL_AudioDevice *device)
{
	free(device->hidden);
	free(device);
}

static SDL_AudioDevice *PSPAUD_CreateDevice(int devindex)
{
	SDL_AudioDevice *this;
    const char *envr;

	/* Initialize all variables that we clean on shutdown */
	this = (SDL_AudioDevice *)malloc(sizeof(SDL_AudioDevice));
	if ( this ) {
		memset(this, 0, (sizeof *this));
		this->hidden = (struct SDL_PrivateAudioData *)
				malloc((sizeof *this->hidden));
	}
	if ( (this == NULL) || (this->hidden == NULL) ) {
		SDL_OutOfMemory();
		if ( this ) {
			free(this);
		}
		return(0);
	}
	memset(this->hidden, 0, (sizeof *this->hidden));

	/* Set the function pointers */
	this->OpenAudio = PSPAUD_OpenAudio;
	this->WaitAudio = PSPAUD_WaitAudio;
	this->PlayAudio = PSPAUD_PlayAudio;
	this->GetAudioBuf = PSPAUD_GetAudioBuf;
	this->CloseAudio = PSPAUD_CloseAudio;

	this->free = PSPAUD_DeleteDevice;

	return this;
}

AudioBootStrap PSPAUD_bootstrap = {
	PSPAUD_DRIVER_NAME, "PSP audio driver",
	PSPAUD_Available, PSPAUD_CreateDevice
};

/* This function waits until it is possible to write a full sound buffer */
static void PSPAUD_WaitAudio(_THIS)
{
	/* Do nothing. */
}

static void PSPAUD_PlayAudio(_THIS)
{
	/* Do nothing. */
}

static Uint8 *PSPAUD_GetAudioBuf(_THIS)
{
	return NULL;
}

static void PSPAUD_CloseAudio(_THIS)
{
	/* Do nothing. */
}

static int PSPAUD_OpenAudio(_THIS, SDL_AudioSpec *spec)
{
	return -1;
}

/* vim: ts=4 sw=4
 */