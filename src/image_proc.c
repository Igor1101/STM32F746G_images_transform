#include <stdbool.h>
#include <math.h>
#include "stm32f7xx_hal.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_sdram.h"
#include "sdram_alloc.h"
#include "stlogo.h"
#include "car.h"
#include "serial.h"
#include"fft.h"



void image_proc(void)
{
    BSP_LCD_Init();
    BSP_LCD_LayerDefaultInit(LTDC_ACTIVE_LAYER, LCD_FB_START_ADDRESS); 
    BSP_LCD_SelectLayer(LTDC_ACTIVE_LAYER); 
    BSP_LCD_DisplayOn();
    BSP_LCD_Clear(LCD_COLOR_DARKBLUE); 
    BSP_LCD_DrawBitmap(0,0, car_bmp);
    HAL_Delay(1000); 
    BSP_SDRAM_Init();
    //ct_sdram_ct_sdram_malloc(256*256*3)
    uint8_t*im = car_bmp;
    size_t im_size = im_width(im)*im_height(im)*im_bit_pixel(im)/8;
    pr_debugln("im w %i h %i", im_width(im),im_height(im));
    size_t grey_size = im_width(im) * im_height(im);
    size_t bigger = (im_width(im)>im_height(im))?im_width(im):im_height(im);
    size_t fft_tmpsz = bigger * bigger*sizeof(complex);
    uint8_t*alloc_car_grey = ct_sdram_malloc(im_size / 3);
    RGB888_to_grey(im_start(im), alloc_car_grey, grey_size);
    complex*alloc_car_complex = ct_sdram_malloc(grey_size*sizeof(complex));
    to_complex(alloc_car_grey, alloc_car_complex, grey_size);
    uint8_t*alloc_car_tmp = ct_sdram_malloc(grey_size*sizeof(complex));
    uint8_t*alloc_car_tmp1 = ct_sdram_malloc(grey_size*sizeof(complex));
    fft2d(alloc_car_complex, im_width(im), im_height(im), alloc_car_tmp, alloc_car_tmp1);
    abs_complex(alloc_car_complex, alloc_car_grey, grey_size);
    grey_to_RGB888(alloc_car_grey, im_start(im), grey_size);
    BSP_LCD_DrawBitmap(0,0,car_bmp);
    HAL_Delay(3000);
    log_complex(alloc_car_complex, alloc_car_grey, grey_size);
    //fftshift(alloc_car_grey, im_width(im), im_height(im));
    grey_to_RGB888(alloc_car_grey, im_start(im), grey_size);
    BSP_LCD_DrawBitmap(0,0, car_bmp);
    // main loop
    /*
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW); 
    BSP_LCD_FillRect(0,0,800,480); 
    BSP_LCD_SetTextColor(LCD_COLOR_LIGHTBLUE); 
    BSP_LCD_DisplayStringAt(0,50, ( uint8_t *) "test font" , LEFT_MODE); 
    static char str[40];
    sprintf (str, "%5lu" ,BSP_LCD_GetXSize());
  BSP_LCD_DisplayStringAt(0,100, (

uint8_t

*)str,
LEFT_MODE

);
 

sprintf

(str,
"%5lu"

,BSP_LCD_GetYSize());
  BSP_LCD_DisplayStringAt(0,150, (

uint8_t

*)str,
LEFT_MODE

);
  BSP_LCD_SetTextColor(LCD_COLOR_DARKCYAN);

  BSP_LCD_DrawLine(0,200,800,310);*/
}
