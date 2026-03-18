/*
SDLPoP, a port/conversion of the DOS game Prince of Persia.
Copyright (C) 2013-2025  Dávid Nagy

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

The authors of this program may be contacted at https://forum.princed.org
*/

#ifndef COMMON_H
#define COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef _MSC_VER // unistd.h does not exist in the Windows SDK.
#include <unistd.h>
#else
#ifndef _UNISTD_H
#define _UNISTD_H    1
#define F_OK    0       /* Test for existence.  */
#define access _access
#endif
#include <malloc.h>
#define alloca _alloca
#endif

// S_ISREG and S_ISDIR may not be defined under MSVC
#ifndef S_ISREG
#define S_ISREG(mode)  (((mode) & S_IFMT) == S_IFREG)
#endif
#ifndef S_ISDIR
#define S_ISDIR(mode)  (((mode) & S_IFMT) == S_IFDIR)
#endif

#include "config.h"
#include "types.h"
#include "proto.h"
#include "data.h"

/* Safe unaligned memory access for RISC platforms */
#if defined(_M_ALPHA) || defined(_M_MRX000) || defined(_M_PPC) || defined(__PSP__) || !defined(_M_IX86)
#define UNALIGNED_OK 0
#else
#define UNALIGNED_OK 1
#endif

#if !UNALIGNED_OK
#define READ_U16(p) ((word)((byte*)(p))[0] | ((word)((byte*)(p))[1] << 8))
#define READ_S16(p) ((short)READ_U16(p))
#define READ_U32(p) ((dword)((byte*)(p))[0] | ((dword)((byte*)(p))[1]<<8) | ((dword)((byte*)(p))[2]<<16) | ((dword)((byte*)(p))[3]<<24))
#define WRITE_U16(p,v) do { ((byte*)(p))[0]=(byte)(v); ((byte*)(p))[1]=(byte)((v)>>8); } while(0)
#define WRITE_S16(p,v) WRITE_U16(p,v)

#undef SDL_SwapLE16
#define SDL_SwapLE16(x) (x)
#undef SDL_SwapLE32
#define SDL_SwapLE32(x) (x)
#undef SDL_SwapBE16
#define SDL_SwapBE16(x) ( (((x)>>8)&0xFF) | (((x)&0xFF)<<8) )
#undef SDL_SwapBE32
#define SDL_SwapBE32(x) ( (((x)>>24)&0xFF) | (((x)>>8)&0xFF00) | (((x)&0xFF00)<<8) | (((x)&0xFF)<<24) )
#else
#define READ_U16(p) (*(word*)(p))
#define READ_S16(p) (*(short*)(p))
#define READ_U32(p) (*(dword*)(p))
#define WRITE_U16(p,v) (*(word*)(p) = (word)(v))
#define WRITE_S16(p,v) (*(short*)(p) = (short)(v))
#endif

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef ABS
#define ABS(x) ((x)<0?-(x):(x))
#endif

#ifdef _MSC_VER
#define snprintf _snprintf
#define snprintf_check _snprintf
#else
#define snprintf_check(dst, size, ...)	do {			\
		int __len;					\
		__len = snprintf(dst, size, __VA_ARGS__);	\
		if (__len < 0 || __len >= (int)size) {		\
			fprintf(stderr, "%s: buffer truncation detected!\n", __func__);\
			quit(2);				\
		}						\
	} while (0)
#endif

#ifdef __cplusplus
}
#endif

#endif
