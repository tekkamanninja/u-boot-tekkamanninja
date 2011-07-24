/*
 * (C) Copyright 2006 by OpenMoko, Inc.
 * Author: Harald Welte <laforge@openmoko.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
 
#if defined(CONFIG_VIDEO_S3C2410)
 
#include <video_fb.h>
#include "videomodes.h"
#include <asm/arch/s3c24x0_cpu.h>
#include <asm/io.h>
/*
 * Export Graphic Device
 */
GraphicDevice smi;
 
#define VIDEO_MEM_SIZE  0x200000        /* 240x320x16bit = 0x25800 bytes */
 
extern void board_video_init(GraphicDevice *pGD);
 
/*******************************************************************************
 *
 * Init video chip with common Linux graphic modes (lilo)
 */
void *video_hw_init (void)
{
    struct s3c24x0_lcd * const lcd = s3c24x0_get_base_lcd();
    GraphicDevice *pGD = (GraphicDevice *)&smi;
    int videomode;
    unsigned long t1, hsynch, vsynch;
    char *penv;
    int tmp, i, bits_per_pixel;
    struct ctfb_res_modes *res_mode;
    struct ctfb_res_modes var_mode;
//    unsigned char videoout;
 
    /* Search for video chip */
    printf("Video: ");

    tmp = 0;
 
         videomode = CFG_SYS_DEFAULT_VIDEO_MODE;
         /* get video mode via environment */
         if ((penv = getenv ("videomode")) != NULL) {
                 /* deceide if it is a string */
                 if (penv[0] <= '9') {
                         videomode = (int) simple_strtoul (penv, NULL, 16);
                         tmp = 1;
                 }
         } else {
                 tmp = 1;
         }
         if (tmp) {
                 /* parameter are vesa modes */
                 /* search params */
                 for (i = 0; i < VESA_MODES_COUNT; i++) {
                         if (vesa_modes[i].vesanr == videomode)
                                 break;
                 }
                 if (i == VESA_MODES_COUNT) {
                         printf ("no VESA Mode found, switching to mode 0x%x ", CFG_SYS_DEFAULT_VIDEO_MODE);
                         i = 0;
                }
                 res_mode =
                         (struct ctfb_res_modes *) &res_mode_init[vesa_modes[i].
                                                                  resindex];
                 bits_per_pixel = vesa_modes[i].bits_per_pixel;
         } else {
 
                res_mode = (struct ctfb_res_modes *) &var_mode;
                 bits_per_pixel = video_get_params (res_mode, penv);
         }
 
         /* calculate hsynch and vsynch freq (info only) */
         t1 = (res_mode->left_margin + res_mode->xres +
               res_mode->right_margin + res_mode->hsync_len) / 8;
         t1 *= 8;
         t1 *= res_mode->pixclock;
         t1 /= 1000;
         hsynch = 1000000000L / t1;
         t1 *=
                 (res_mode->upper_margin + res_mode->yres +
                  res_mode->lower_margin + res_mode->vsync_len);
         t1 /= 1000;
         vsynch = 1000000000L / t1;
 
         /* fill in Graphic device struct */
         sprintf (pGD->modeIdent, "%dx%dx%d %ldkHz %ldHz", res_mode->xres,
                  res_mode->yres, bits_per_pixel, (hsynch / 1000),
                  (vsynch / 1000));
         printf ("%s\n", pGD->modeIdent);
         pGD->winSizeX = res_mode->xres;
         pGD->winSizeY = res_mode->yres;
         pGD->plnSizeX = res_mode->xres;
         pGD->plnSizeY = res_mode->yres;
             
         switch (bits_per_pixel) {
         case 8:
                 pGD->gdfBytesPP = 1;
                 pGD->gdfIndex = GDF__8BIT_INDEX;
                 break;
         case 15:
                 pGD->gdfBytesPP = 2;
                 pGD->gdfIndex = GDF_15BIT_555RGB;
                 break;
         case 16:
                 pGD->gdfBytesPP = 2;
                 pGD->gdfIndex = GDF_16BIT_565RGB;
                 break;
         case 24:
                 pGD->gdfBytesPP = 3;
                 pGD->gdfIndex = GDF_24BIT_888RGB;
                 break;
         }
 
         /* statically configure settings */
        pGD->winSizeX = pGD->plnSizeX = 240;
         pGD->winSizeY = pGD->plnSizeY = 320;
         pGD->gdfBytesPP = 2;
         pGD->gdfIndex = GDF_16BIT_565RGB;
 
         pGD->frameAdrs = LCD_VIDEO_ADDR;
         pGD->memSize = VIDEO_MEM_SIZE;
 
         board_video_init(pGD);

	writel((pGD->frameAdrs >> 1), &lcd->LCDSADDR1); 
 
         /* This marks the end of the frame buffer. */
	writel((((readl(&lcd->LCDSADDR1))&0x1fffff) + (pGD->winSizeX+0) * pGD->winSizeY), &lcd->LCDSADDR2); 
	writel((pGD->winSizeX & 0x7ff), &lcd->LCDSADDR3); 
 
         /* Clear video memory */
         memset((void *)pGD->frameAdrs, 0, pGD->memSize);
 
         /* Enable  Display  */
	writel((readl(&lcd->LCDCON1) | 0x01), & lcd->LCDCON1); /* ENVID = 1 */
 
         return ((void*)&smi);
 }
 
 void
 video_set_lut (unsigned int index,      /* color number */
                unsigned char r, /* red */
                unsigned char g, /* green */
                unsigned char b  /* blue */
		)
 {
 }
 
 #endif /* CONFIG_VIDEO_S3C2410 */
