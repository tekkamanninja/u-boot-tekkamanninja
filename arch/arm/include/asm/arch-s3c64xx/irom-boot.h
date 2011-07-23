#ifndef __IROM_BOOT_H__
#define __IROM_BOOT_H__

#include <config.h>

/* TCM function for bl2 load */
#if defined(CONFIG_S3C6400)
#define CopyMMCtoMem(a,b,c,d,e)	(((int(*)(uint, ushort, uint *, uint, int))(*((uint *)(TCM_BASE + 0x8))))(a,b,c,d,e))
#elif defined(CONFIG_S3C6410) || defined(CONFIG_S3C6430)
#define CopyMMCtoMem(a,b,c,d,e)	(((int(*)(int, uint, ushort, uint *, int))(*((uint *)(TCM_BASE + 0x8))))(a,b,c,d,e))
#endif


#endif

