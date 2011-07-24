/*
 * (C) Copyright 2010 by Tekkaman Ninja.
 * Author: Tekkaman Ninja <tekkamanninja@gmail.com>
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
 
#if defined(CONFIG_VIDEO_S3C64X0)
 
#include <video_fb.h>
#include "videomodes.h"
#include <asm/arch/regs-fb-v4.h>

#include <asm/arch/s3c6400.h>

/*
 * Export Graphic Device
 */
GraphicDevice smi;
 
#define VIDEO_MEM_SIZE  0x200000        /*NEC 4.3 inches: 480x272x16bit = 0x3FC00 bytes */

//CPU:     S3C6400@667MHz
//         Fclk = 667MHz, Hclk = 133MHz, Pclk = 66MHz (ASYNC Mode) 
 
extern void board_video_init(GraphicDevice *pGD);
 
/*******************************************************************************
 *
 * Init video chip with common Linux graphic modes (lilo)
 */

void *video_hw_init (void)
{
    s3c64xx_fb * const fb = s3c64xx_get_base_fb();
    GraphicDevice *pGD = (GraphicDevice *)&smi;
    int videomode;
    unsigned long t1, hsynch, vsynch;	
    char *penv;
    int tmp, i, bits_per_pixel;
    struct ctfb_res_modes *res_mode;
    struct ctfb_res_modes var_mode;

    int clkval;

//    unsigned char videoout;
 
    /* Search for video chip */
    printf("Video: ");

    tmp = 0;
 
         videomode = CONFIG_SYS_DEFAULT_VIDEO_MODE;
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
                         printf ("no VESA Mode found, switching to mode 0x%x ", CONFIG_SYS_DEFAULT_VIDEO_MODE);
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
#if 0
         /* statically configure settings for debug*/
        pGD->winSizeX = pGD->plnSizeX = 480;
         pGD->winSizeY = pGD->plnSizeY = 272;
         pGD->gdfBytesPP = 2;
         pGD->gdfIndex = GDF_16BIT_565RGB;
#endif 
         pGD->frameAdrs = LCD_VIDEO_ADDR; //in config file :include/configs/mini6410.h
         pGD->memSize = VIDEO_MEM_SIZE;
         /* Clear video memory */
         memset((void *)pGD->frameAdrs, 0x00, pGD->memSize);
 
         board_video_init(pGD);		//in board init file :board/samsung/mini6410/mini6410.c  for gpio etc.

	t1 = res_mode->pixclock;
        t1 /= 1000;
        t1 = 1000000000L / t1;
	clkval = (CONFIG_SYS_VIDEO_VCLOCK_HZ / t1) - 1; 


	fb->VIDCON0 = ( VIDCON0_VIDOUT_RGB 	| VIDCON0_PNRMODE_RGB | 
			VIDCON0_CLKVALUP 	| VIDCON0_CLKVAL_F(clkval)| 
			VIDCON0_CLKDIR		| VIDCON0_CLKSEL_HCLK   );

	fb->VIDCON1  = ( VIDCON1_INV_VSYNC | VIDCON1_INV_HSYNC);

	fb->VIDTCON0 = ( VIDTCON0_VBPD(res_mode->upper_margin) | 
			 VIDTCON0_VFPD(res_mode->lower_margin) | 
			 VIDTCON0_VSPW(res_mode->vsync_len)	);
	fb->VIDTCON1 = ( VIDTCON1_HBPD(res_mode->left_margin) | 
			 VIDTCON1_HFPD(res_mode->right_margin) |
			 VIDTCON1_HSPW(res_mode->hsync_len));
 	fb->VIDTCON2 = (VIDTCON2_LINEVAL(pGD->winSizeY - 1) | VIDTCON2_HOZVAL(pGD->winSizeX - 1));

#if defined(LCD_VIDEO_BACKGROUND)   
	fb->WINCON0  = (WINCON0_BPPMODE_16BPP_565 | WINCONx_ENWIN | WINCONx_HAWSWP);
	fb->VIDOSD0A  = (VIDOSDxA_TOPLEFT_X(0) | VIDOSDxA_TOPLEFT_Y(0));
	fb->VIDOSD0B  = (VIDOSDxB_BOTRIGHT_X(pGD->winSizeX - 1) | VIDOSDxB_BOTRIGHT_Y(pGD->winSizeY - 1));
	fb->VIDOSD0C  = (pGD->winSizeY * pGD->winSizeX);
#endif
	fb->WINCON1  = (WINCON1_BPPMODE_16BPP_565 | WINCONx_ENWIN | WINCONx_HAWSWP);
	fb->VIDOSD1A  = (VIDOSDxA_TOPLEFT_X(0) | VIDOSDxA_TOPLEFT_Y(0));
	fb->VIDOSD1B  = (VIDOSDxB_BOTRIGHT_X(pGD->winSizeX - 1) | VIDOSDxB_BOTRIGHT_Y(pGD->winSizeY - 1));
	fb->VIDOSD1C  = (VIDISD14C_ALPHA0_R(LCD_VIDEO_BACKGROUND_ALPHA) |
			 VIDISD14C_ALPHA0_G(LCD_VIDEO_BACKGROUND_ALPHA) | 
			 VIDISD14C_ALPHA0_B(LCD_VIDEO_BACKGROUND_ALPHA) );
	fb->VIDOSD1D  = (pGD->winSizeY * pGD->winSizeX);



#if defined(LCD_VIDEO_BACKGROUND)  
	 /* config  Display framebuffer addr for background*/
         fb-> VIDW00ADD0B0 = LCD_VIDEO_BACKGROUND_ADDR;
          /* This marks the end of the frame buffer. */
         fb-> VIDW00ADD1B0 = (fb->VIDW00ADD0B0 &0xffffff) + (pGD->winSizeX+0) * pGD->winSizeY * 2;
         fb->  VIDW00ADD2= ((pGD->winSizeX * 2) & 0x1fff);
#endif


	 /* config  Display framebuffer addr for console*/
         fb-> VIDW01ADD0B0 = pGD->frameAdrs;
          /* This marks the end of the frame buffer. */
         fb-> VIDW01ADD1B0 = (fb->VIDW01ADD0B0 &0xffffff) + (pGD->winSizeX+0) * pGD->winSizeY * 2;
         fb->  VIDW01ADD2= ((pGD->winSizeX * 2) & 0x1fff);

         /* Enable  Display  */
         fb-> VIDCON0 |= (VIDCON0_ENVID | VIDCON0_ENVID_F);   /* ENVID = 1     ENVID_F = 1*/ 

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
#endif /* CONFIG_VIDEO_S3C64X0 */
