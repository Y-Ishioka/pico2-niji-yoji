/*
 * Copyright (C) 2022-2024 by Y.Ishioka
 */


#include <string.h>
#include <stdlib.h>

#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "FreeRTOS.h"
#include "task.h"

#include "app_param.h"
#include "yoji_data.c"
#include "eki_data.c"
#include "niji_data.c"


// Which core to run on if configNUMBER_OF_CORES==1
#ifndef RUN_FREE_RTOS_ON_CORE
#define RUN_FREE_RTOS_ON_CORE 0
#endif

// Priorities of our threads - higher numbers are higher priority
#define MAIN_TASK_PRIORITY      ( tskIDLE_PRIORITY + 4UL )
#define WORKER_TASK_PRIORITY    ( tskIDLE_PRIORITY + 2UL )


void  pico_gpio_led_set( int on );
unsigned int  pico_dev_read_adc0( void );
unsigned int  pico_dev_read_adc1( void );
unsigned int  pico_dev_read_adc4( void );
int  pico_dev_adc_init( void );
int  pico_dev_read_dip( void );
int  pico_dev_dip_init( void );
int  pico_gpio_select_btn( void );
int  pico_gpio_restart_btn( void );
int  pico_dev_btn_init( void );
int  pico_dev_chk_spi_miso( void );

unsigned char  *read_fontx2_a( unsigned int code );
unsigned char  *read_fontx2_k( unsigned int code );
int hub75_test_init( void );
int hub75_test_loop( void );

int  fat_test_init( void );
int  fat_test_read( char *filename, char *buff, int bsize );


extern  const unsigned char  niji_data[];

unsigned char  kigou_quest[2] = { 0x81, 0x48 };
unsigned char  kigou_kotae[2] = { 0x93, 0x9a };

unsigned char  kigou_right[ DEF_FONT_HIGH ][DEF_FONT_WIDTH ] = {
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

unsigned char  kigou_down[ DEF_FONT_HIGH ][DEF_FONT_WIDTH ] = {
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

unsigned char  kigou_center[ DEF_FONT_HIGH ][DEF_FONT_WIDTH ] = {
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
};

unsigned char  kigou_ank_null[ DEF_FONT_HIGH ][DEF_FONT_WIDTH/2 ] = {
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 }
};

char  *niji_data_filnename = "niji.dat";


unsigned int disp_color_code[] = {
    0xff0000, 0x00ff00, 0x1010ff, 0xffff00,
    0x00ffff, 0xff00ff, 0xffffff, 0x000000
};

unsigned int  disp_rct_color_code = 0x3f0303;
unsigned int  disp_arw_color_code = 0x107f00;
unsigned int  disp_que_color_code = 0x00ffff;
unsigned int  disp_ans_color_code = 0xffffff;
unsigned int  disp_sec_color_code = 0x0000ff;
unsigned int  disp_rem_color_code = 0x003f3f;

int  yoji_clear_hor_pat[][DEF_YOJI_MOJI] = {
    {  0,  1,  2,  3 },
    {  4,  5,  6,  7 },
    {  8,  9, 10, 11 },
    { 12, 13, 14, 15 }
};

int  yoji_clear_ver_pat[][DEF_YOJI_MOJI] = {
    {  0,  4,  8, 12 },
    {  1,  5,  9, 13 },
    {  2,  6, 10, 14 },
    {  3,  7, 11, 15 }
};


unsigned char  image_buffer[ DEF_IMAGEBUFF ];

unsigned char  yoji_buffer[ 4 ][ DEF_FONT_HIGH ][DEF_FONT_WIDTH ];
int  yoji_list[ DEF_YOJI_LST_MAX ];

unsigned char  niji_data_buffer[ DEF_TEXT_BUFF+1 ];
unsigned char  ans_buffer[ DEF_FONT_HIGH ][ DEF_FONT_WIDTH ];
unsigned char  sec_buffer[2][ DEF_FONT_HIGH ][ DEF_FONT_WIDTH/2 ];
unsigned char  rem_buffer[2][ DEF_FONT_HIGH ][ DEF_FONT_WIDTH/2 ];
unsigned char  tmp_buffer[ DEF_FONT_HIGH ][ DEF_FONT_WIDTH ];

int  char_list_1st[ DEF_NIJI_LST_MAX ];
int  char_list_2nd[ DEF_NIJI_LST_MAX ];

unsigned char  *niji_data_pnt;

int  x_sec_init;
int  x_sec_remain;
int  x_handle_counter;
int  x_wait_flag;
int  x_item_all_cnt;
int  x_item_1st_cnt;
int  x_item_2nd_cnt;

unsigned int  yoji_adc4;
unsigned int  yoji_rand_seed;

int  disp_color;
int  disp_color_bak;
int  yoji_item_num;
int  yoji_item_pos;
int  yoji_item_pos_bef1;
int  yoji_item_pos_bef2;

int  yoji_pos;
int  yoji_dir;
int  yoji_bef_pos;
int  yoji_bef_dir;
unsigned int  yoji_code;
unsigned char  *src_data;

int  mode_niji_yoji;

TaskHandle_t htask1;  /* main task */
TaskHandle_t htask2;  /* yoji or niji task */
TaskHandle_t htask3;  /* check task */
TaskHandle_t htask4;  /* cyclic task */


void delay( int ms )
{
  vTaskDelay( ms );
}


void  set_font( uint8_t *font, uint8_t *buff, int width )
{
    int  i, j, k;
    int  row;
    int  w = (width/8);   /* font width byte */
    uint8_t  pat;

    /* row */
    for( i=0 ; i<DEF_FONT_HIGH ; i++ ) {
        //row = DEF_FONT_WIDTH * i;
        row = width * i;
        /* col */
        for( j=0 ; j<w ; j++ ) {
            pat = 0x80;
            for( k=0 ; k<8 ; k++ ) {
                if( (font[ i * w + j ] & pat) != 0 ) {
                    buff[ row + j*8 + k ] = 1;
                }
                pat >>= 1; /* bit shift */
            }
        }
    }
}


/*********************************************************/
void  make_kanji_bitmap( unsigned char *kanji, unsigned char *out )
{
    unsigned int  code;
    unsigned char  *fontdata;

    code = (unsigned int)(*kanji);
    code = (code<<8) + *(kanji+1);

    fontdata = read_fontx2_k( code );
    set_font( fontdata, (uint8_t *)out, DEF_FONT_WIDTH );
}


void  set_font_to_img_buf_quick( int x, int y, unsigned char *font, unsigned int color )
{
    int  ver, hor;
    unsigned char  *img_base;
    unsigned char  *img_pnt;
    unsigned char  *fnt_pnt;
    unsigned char  pix_r = (unsigned char)(color & 0xff);
    unsigned char  pix_g = (unsigned char)((color >> 8) & 0xff);
    unsigned char  pix_b = (unsigned char)((color >> 16) & 0xff);

    img_base = image_buffer 
               + (DEF_IMG_WIDTH * DEF_IMG_PIXEL) * y
               + DEF_IMG_PIXEL * x;

        fnt_pnt = font;
        for( ver=0 ; ver<DEF_FONT_HIGH ; ver++ ) {
            img_pnt = img_base + (DEF_IMG_WIDTH * DEF_IMG_PIXEL) * ver;

            for( hor=0 ; hor<DEF_FONT_WIDTH ; hor++ ) {
                if( *fnt_pnt ) {
                    *img_pnt     = pix_r;
                    *(img_pnt+1) = pix_g;
                    *(img_pnt+2) = pix_b;
                }
                img_pnt += 3;
                fnt_pnt++;
            }
        }
}


void  set_font_to_img_buf( int x, int y, unsigned char *font, unsigned int color )
{
    int  ver, hor;
    int  grad;
    unsigned char  *img_base;
    unsigned char  *img_pnt;
    unsigned char  *fnt_pnt;
    unsigned char  pix_r = (unsigned char)(color & 0xff);
    unsigned char  pix_g = (unsigned char)((color >> 8) & 0xff);
    unsigned char  pix_b = (unsigned char)((color >> 16) & 0xff);

    img_base = image_buffer 
               + (DEF_IMG_WIDTH * DEF_IMG_PIXEL) * y
               + DEF_IMG_PIXEL * x;

    for( grad=8 ; grad>0 ; grad-- ) {
        fnt_pnt = font;
        for( ver=0 ; ver<DEF_FONT_HIGH ; ver++ ) {
            img_pnt = img_base + (DEF_IMG_WIDTH * DEF_IMG_PIXEL) * ver;

            for( hor=0 ; hor<DEF_FONT_WIDTH ; hor++ ) {
                if( *fnt_pnt ) {
                    *img_pnt     = pix_r>>grad;
                    *(img_pnt+1) = pix_g>>grad;
                    *(img_pnt+2) = pix_b>>grad;
                }
                img_pnt += 3;
                fnt_pnt++;
            }
        }
        vTaskDelay( DEF_TIM_GRAD_DLY );
    }
}


void  clr_img_buf( int x, int y )
{
    int  ver, hor;
    int  grad;
    unsigned char  *img_base;
    unsigned char  *pix_p;

    img_base = image_buffer 
               + (DEF_IMG_WIDTH * DEF_IMG_PIXEL) * y
               + DEF_IMG_PIXEL * x;

    for( grad=8 ; grad>0 ; grad-- ) {
        for( ver=0 ; ver<DEF_FONT_HIGH ; ver++ ) {
            for( hor=0 ; hor<DEF_FONT_WIDTH ; hor++ ) {
                pix_p = img_base + (DEF_IMG_WIDTH * DEF_IMG_PIXEL) * ver + (DEF_IMG_PIXEL) * hor;
                *pix_p     = *pix_p >> 1;
                *(pix_p+1) = *(pix_p+1) >> 1;
                *(pix_p+2) = *(pix_p+2) >> 1;
            }
        }
        vTaskDelay( DEF_TIM_GRAD_DLY );
    }
}


void  make_ascii_bitmap( unsigned char code, unsigned char *out )
{
    unsigned char  *fontdata;

    fontdata = read_fontx2_a( (unsigned int)code );
    set_font( fontdata, (uint8_t *)out, DEF_FONT_WIDTH/2 );
}


void  set_ank_to_img_buf( int x, int y, unsigned char *font, unsigned int color )
{
    int  ver, hor;
    unsigned char  *img_base;
    unsigned char  *img_pnt;
    unsigned char  *fnt_pnt;
    unsigned char  pix_r = (unsigned char)(color & 0xff);
    unsigned char  pix_g = (unsigned char)((color >> 8) & 0xff);
    unsigned char  pix_b = (unsigned char)((color >> 16) & 0xff);

    img_base = image_buffer 
               + (DEF_IMG_WIDTH * DEF_IMG_PIXEL) * y
               + DEF_IMG_PIXEL * x;

    fnt_pnt = font;
    for( ver=0 ; ver<DEF_FONT_HIGH ; ver++ ) {
        img_pnt = img_base + (DEF_IMG_WIDTH * DEF_IMG_PIXEL) * ver;

        for( hor=0 ; hor<DEF_FONT_WIDTH/2 ; hor++ ) {
            if( *fnt_pnt ) {
                *img_pnt     = pix_r;
                *(img_pnt+1) = pix_g;
                *(img_pnt+2) = pix_b;
            }
            img_pnt += 3;
            fnt_pnt++;
        }
    }
}


void  clr_ank_img_buf( int x, int y )
{
    int  ver, hor;
    unsigned char  *img_base;
    unsigned char  *img_pnt;
    unsigned char  pix = 0x00;

    img_base = image_buffer 
               + (DEF_IMG_WIDTH * DEF_IMG_PIXEL) * y
               + DEF_IMG_PIXEL * x;

    for( ver=0 ; ver<DEF_FONT_HIGH ; ver++ ) {
        img_pnt = img_base + (DEF_IMG_WIDTH * DEF_IMG_PIXEL) * ver;

        for( hor=0 ; hor<DEF_FONT_WIDTH/2 ; hor++ ) {
            *img_pnt     = pix;
            *(img_pnt+1) = pix;
            *(img_pnt+2) = pix;
            img_pnt += 3;
        }
    }
}


void view_anser_rect( unsigned int color )
{
    int  ver, hor;
    unsigned char  *img_base;
    unsigned char  *img_pnt1;
    unsigned char  *img_pnt2;
    unsigned char  pix_r = (unsigned char)(color & 0xff);
    unsigned char  pix_g = (unsigned char)((color >> 8) & 0xff);
    unsigned char  pix_b = (unsigned char)((color >> 16) & 0xff);

    img_base = image_buffer 
               + (DEF_IMG_WIDTH * DEF_IMG_PIXEL) * 23
               + DEF_IMG_PIXEL * 23;

    img_pnt1 = img_base + (DEF_IMG_WIDTH * DEF_IMG_PIXEL) * 0;
    img_pnt2 = img_base + (DEF_IMG_WIDTH * DEF_IMG_PIXEL) * 17;
    for( hor=0 ; hor<(16+2) ; hor++ ) {
        *img_pnt1     = pix_r;
        *(img_pnt1+1) = pix_g;
        *(img_pnt1+2) = pix_b;
        *img_pnt2     = pix_r;
        *(img_pnt2+1) = pix_g;
        *(img_pnt2+2) = pix_b;
        img_pnt1 += 3;
        img_pnt2 += 3;
    }

    for( ver=0 ; ver<(16+2) ; ver++ ) {
        img_pnt1 = img_base + (DEF_IMG_WIDTH * DEF_IMG_PIXEL) * ver;
        img_pnt2 = img_base + (DEF_IMG_WIDTH * DEF_IMG_PIXEL) * ver + DEF_IMG_PIXEL * 17;
        *img_pnt1     = pix_r;
        *(img_pnt1+1) = pix_g;
        *(img_pnt1+2) = pix_b;
        *img_pnt2     = pix_r;
        *(img_pnt2+1) = pix_g;
        *(img_pnt2+2) = pix_b;
    }
}


void view_sec_counter( void )
{
    int  num;
    unsigned char  col10, col1;

    num = x_sec_init;

    memset( sec_buffer, 0x00, sizeof(sec_buffer) );
    col10 = (unsigned char)(num / 10);
    col1  = (unsigned char)(num % 10);
    make_ascii_bitmap( col10+'0', (unsigned char *)sec_buffer[0] );
    make_ascii_bitmap( col1+'0',  (unsigned char *)sec_buffer[1] );

    clr_ank_img_buf( DEF_SEC_COL10_X, DEF_SEC_COL10_Y );
    clr_ank_img_buf( DEF_SEC_COL1_X,  DEF_SEC_COL1_Y );
    if( col10 != 0 ) {
        set_ank_to_img_buf( DEF_SEC_COL10_X, DEF_SEC_COL10_Y, (unsigned char *)sec_buffer[0], disp_sec_color_code );
    }
    set_ank_to_img_buf( DEF_SEC_COL1_X, DEF_SEC_COL1_Y, (unsigned char *)sec_buffer[1], disp_sec_color_code );
}


void view_remain_counter( void )
{
    int  num;
    unsigned char  col10, col1;

    num = x_sec_remain;

    memset( rem_buffer, 0x00, sizeof(rem_buffer) );
    col10 = (unsigned char)(num / 10);
    col1  = (unsigned char)(num % 10);
    make_ascii_bitmap( col10+'0', (unsigned char *)rem_buffer[0] );
    make_ascii_bitmap( col1+'0',  (unsigned char *)rem_buffer[1] );

    clr_ank_img_buf( DEF_REM_COL10_X, DEF_REM_COL10_Y );
    clr_ank_img_buf( DEF_REM_COL1_X,  DEF_REM_COL1_Y );
    if( col10 != 0 ) {
        set_ank_to_img_buf( DEF_REM_COL10_X, DEF_REM_COL10_Y, (unsigned char *)rem_buffer[0], disp_rem_color_code );
    }
    set_ank_to_img_buf( DEF_REM_COL1_X, DEF_REM_COL1_Y, (unsigned char *)rem_buffer[1], disp_rem_color_code );
}


int  niji_item_count( void )
{
    int  count = 0;
    unsigned char  *pnt = (unsigned char *)niji_data_pnt;

    while( *pnt != 0x00 ) {
        count++;
        pnt += 6; /* k1k1 k2k2 crlf */
    }

    return  count;
}


unsigned char  *get_random_code( void )
{
    unsigned char  *code;
    int  niji_pnt;
    int  sel_char;

    niji_pnt = rand() % x_item_all_cnt;
    sel_char = rand() & 0x1;  /* 0 or 1 */
    code = (unsigned char *)niji_data_pnt + niji_pnt * 6 + sel_char * 2;

    return  code;
}


/*********************************************************/
void  make_yoji_bitmap( unsigned char  *yoji )
{
    unsigned int  code;
    int  i;
    unsigned char  *fontdata;

	memset( yoji_buffer, 0x00, sizeof(yoji_buffer) );

    for( i=0 ; i<DEF_YOJI_MOJI ; i++ ) {
        code = (unsigned int)(*yoji);
        code = (code<<8) + *(yoji+1);
        fontdata = read_fontx2_k( code );
        set_font( fontdata, (uint8_t *)yoji_buffer[i], DEF_FONT_WIDTH );
        yoji += 2;
    }
}


/*
 * pos  : 0-3
 * dir  : 0:horizontal, 1:vertical
 * color: 0xBBGGRR
 *
 * src-data : yoji_buffer[][DEF_FONT_HIGH][DEF_FONT_WIDTH]
 * dst-data : image_buffer[DEF_IMG_WIDTH*DEF_DISP_HIGH*DEF_IMG_PIXEL]
 *
 * pos=0, dir=0  pos=1, dir=0  pos=2, dir=0  pos=3, dir=0
 *   0 1 2 3       - - - -       - - - -       - - - -
 *   - - - -       4 5 6 7       - - - -       - - - -
 *   - - - -       - - - -       8 9 a b       - - - -
 *   - - - -       - - - -       - - - -       c d e f
 *
 * pos=0, dir=1  pos=1, dir=1  pos=2, dir=1  pos=3, dir=1
 *   0 - - -       - 1 - -       - - 2 -       - - - 3
 *   4 - - -       - 5 - -       - - 6 -       - - - 7
 *   8 - - -       - 9 - -       - - a -       - - - b
 *   c - - -       - d - -       - - e -       - - - f
 *
 */
void  set_yoji_to_image( int pos, int dir, unsigned int color )
{
    int  ver, hor;
    int  moji;
    int inv_dir;
    int  grad;
    unsigned char  *img_base;
    unsigned char  *img_tmp;
    unsigned char  *img_pnt;
    unsigned char  pix_r = (unsigned char)(color & 0xff);
    unsigned char  pix_g = (unsigned char)((color >> 8) & 0xff);
    unsigned char  pix_b = (unsigned char)((color >> 16) & 0xff);

    if( dir == 0 ) {
        inv_dir = 1;
    } else {
        inv_dir = 0;
    }

    img_base = image_buffer 
             + (DEF_IMG_WIDTH * DEF_IMG_PIXEL) * DEF_FONT_HIGH * (pos * inv_dir)
             + (DEF_FONT_WIDTH * DEF_IMG_PIXEL) * (pos * dir);

    for( moji=0 ; moji<DEF_YOJI_MOJI ; moji++ ) {
        img_tmp = img_base
                  + (DEF_IMG_WIDTH * DEF_IMG_PIXEL) * DEF_FONT_HIGH * (moji * dir)
                  + (DEF_FONT_WIDTH * DEF_IMG_PIXEL) * (moji * inv_dir);

      for( grad=8 ; grad>0 ; grad-- ) {
        for( ver=0 ; ver<DEF_FONT_HIGH ; ver++ ) {
            img_pnt = img_tmp + (DEF_IMG_WIDTH * DEF_IMG_PIXEL) * ver;

            for( hor=0 ; hor<DEF_FONT_WIDTH ; hor++ ) {
                if( yoji_buffer[moji][ver][hor] ) {
                    *img_pnt     = pix_r>>grad;
                    *(img_pnt+1) = pix_g>>grad;
                    *(img_pnt+2) = pix_b>>grad;
                }
                img_pnt += 3;
            }
        }
        vTaskDelay( DEF_TIM_GRAD_DLY );
      }
    }
}


/*
 * area:
 *   0  1  2  3
 *   4  5  6  7
 *   8  9 10 11
 *  12 13 14 15
 */
void  clear_image( int area )
{
    int  ver, hor;
    int  grad;
    unsigned char  *area_pnt;
    unsigned char  *pix_p;

    area_pnt = image_buffer 
             + (DEF_IMG_WIDTH * DEF_IMG_PIXEL) * DEF_FONT_HIGH * (area>>2)
             + (DEF_FONT_WIDTH * DEF_IMG_PIXEL) * (area & 0x03);

    for( grad=8 ; grad>0 ; grad-- ) {
        for( ver=0 ; ver<DEF_FONT_HIGH ; ver++ ) {
            for( hor=0 ; hor<DEF_FONT_WIDTH ; hor++ ) {
                pix_p = area_pnt + (DEF_IMG_WIDTH * DEF_IMG_PIXEL) * ver + (DEF_IMG_PIXEL) * hor + 0;
                *pix_p     = *pix_p >> 1;
                *(pix_p+1) = *(pix_p+1) >> 1;
                *(pix_p+2) = *(pix_p+2) >> 1;
            }
        }
        vTaskDelay( DEF_TIM_GRAD_DLY );
    }
}


int  yoji_item_count( void )
{
    int  count = 0;
    unsigned char  *pnt = (unsigned char *)src_data;

    while( *pnt != 0x00 ) {
        count++;
        pnt += 10;
    }

    return  count;
}


int  yoji_comp_list( unsigned int code, int pos )
{
    int  num;
    int  i;
    unsigned char  *pnt = (unsigned char *)src_data;

    for( i=0 ; i<DEF_YOJI_LST_MAX ; i++ ) {
        yoji_list[i] = 0;
    }

    pnt += pos * 2;
    num = 0;
    for( i=0 ; i<yoji_item_num ; i++ ) {
        if( *pnt == ((code >> 8) & 0xff) && *(pnt+1) == (code & 0xff) ) {
            yoji_list[ num++ ] = i;
            if( num >= DEF_YOJI_LST_MAX ) {
                break;
            }
        }
        pnt += 10;
    }

    return  num;
}


void  task_yoji( __unused void *params )
{
    unsigned char  *yoji_table;
    int  num;
    int  i;
    int  match;
    int  pos;
    int  loop;
    int  tmp_yoji_item_pos;

    memset( image_buffer, 0x00, sizeof(image_buffer) );

    yoji_pos = 0;
    yoji_dir = 0;
    yoji_bef_pos = 0;
    yoji_bef_dir = 0;
    yoji_code = 0x0000;
    yoji_item_pos_bef1 = 99999;
    yoji_item_pos_bef2 = 99999;

    if( pico_dev_read_dip() == 0 ) {
        src_data = (unsigned char *)yoji_data;
    } else {
        src_data = (unsigned char *)eki_data;
    }

    yoji_item_num = yoji_item_count();
    yoji_item_pos = rand() % yoji_item_num;

    disp_color = 0;
    disp_color_bak = 0;
    yoji_table = (unsigned char *)src_data + yoji_item_pos * 10;

    /* set yoji */
    make_yoji_bitmap( yoji_table );
    set_yoji_to_image( yoji_pos, yoji_dir, disp_color_code[disp_color] );

    vTaskDelay( DEF_TIM_YOJI_DLY );

    while( true ) {
        disp_color_bak = disp_color;
        disp_color++;
        if( disp_color_code[disp_color] == 0x0000 ) {
            disp_color = 0;
        }

        yoji_bef_pos = yoji_pos;
        yoji_pos = rand() % DEF_YOJI_MOJI;

        /* search yoji */
        for( match=0 ; match <= 1 ; match++ ) {
            for( i=0 ; i<DEF_YOJI_MOJI ; i++ ) {
                yoji_table = (unsigned char *)src_data + yoji_item_pos * 10 + yoji_pos * 2;
                yoji_code = (*yoji_table << 8) + *(yoji_table + 1);

                /* make list and get list-num */
                num = yoji_comp_list( yoji_code, yoji_bef_pos );

                if( num == 0 ) {
                    yoji_pos++;
                    if( yoji_pos >= DEF_YOJI_MOJI ) {
                        yoji_pos = 0;
                    }
                    continue;
                }

                /* duplication check for previous data */
                tmp_yoji_item_pos = yoji_item_pos;
                for( loop=0 ; loop<5 ; loop++ ) {
                    pos = rand() % num;
                    yoji_item_pos = yoji_list[pos];
                    //if( (match > 0 && num == 1) || 
                    if( (match > 0) || 
                        (yoji_item_pos != yoji_item_pos_bef1 && yoji_item_pos != yoji_item_pos_bef2) ) {
                        match = 2;
                        break;
                    }
                }
                if( match == 2 ) {
                    break;
                } else {
                    yoji_item_pos = tmp_yoji_item_pos;
                    yoji_pos++;
                    if( yoji_pos >= DEF_YOJI_MOJI ) {
                        yoji_pos = 0;
                    }
                }
            }
        }

            yoji_bef_dir = yoji_dir;
            if( yoji_dir == 0 ) {
                yoji_dir = 1;
            } else {
                yoji_dir = 0;
            }

            yoji_table = (unsigned char *)src_data + yoji_item_pos * 10;
            /* set yoji */
            make_yoji_bitmap( yoji_table );
            set_yoji_to_image( yoji_pos, yoji_dir, disp_color_code[disp_color] );
            vTaskDelay( DEF_TIM_SET_DLY );

            if( yoji_bef_dir == 0 ) {
                for( i=0 ; i<DEF_YOJI_MOJI ; i++ ) {
                    if( i != yoji_pos ) {
                        clear_image( yoji_clear_hor_pat[yoji_bef_pos][i] );
                    }
                }
            } else {
                for( i=0 ; i<DEF_YOJI_MOJI ; i++ ) {
                    if( i != yoji_pos ) {
                        clear_image( yoji_clear_ver_pat[yoji_bef_pos][i] );
                    }
                }
            }

            yoji_item_pos_bef2 = yoji_item_pos_bef1;
            yoji_item_pos_bef1 = yoji_item_pos;

        vTaskDelay( DEF_TIM_YOJI_DLY );
    }
}


void  task_niji( __unused void *params )
{
    int  color;
    unsigned char  *code_pnt;
    unsigned char  *niji_pnt;
    unsigned char  code1, code2;
    unsigned char  niji_ans[2];
    int  niji_a;
    int  niji_b;
    int  niji_c;
    int  niji_d;
    int  num;
    int  ret;

    ret = pico_dev_chk_spi_miso();
    printf( "pico_dev_chk_spi_miso()=%d\n", ret );
    if( ret != 0 ) {
        printf( "# Memory card interface available.\n" );
    } else {
        printf( "# No memory card interface.\n" );
    }

    niji_data_pnt = (unsigned char *)niji_data;

    if( ret != 0 ) {
        ret = fat_test_init();
        printf( "fat_test_init()=%d\n", ret );
        if( ret == 0 ) {
            printf( "# Memory card initialization successful.\n" );
            ret = fat_test_read( niji_data_filnename, (char *)niji_data_buffer, DEF_TEXT_BUFF );
            printf( "fat_test_read()=%d\n", ret );
            if( ret > 0 ) {
                printf( "# Memory card file read successful.\n" );
                printf( "# File size is %d bytes.\n", ret );
                niji_data_buffer[ ret ] = 0x00;
                niji_data_pnt = niji_data_buffer;
            } else {
                printf( "# Memory card file read failure.\n" );
            }
        } else {
            printf( "# Memory card initialization failure.\n" );
        }
    }

    x_sec_init = 99;
    x_sec_remain = 0;
    x_handle_counter = 0;
    x_wait_flag = 0;
    x_item_all_cnt = niji_item_count();
    printf( "# Number of items : %d\n", x_item_all_cnt );

    memset( image_buffer, 0x00, sizeof(image_buffer) );

    /* disp allow */
    set_font_to_img_buf_quick( 16, 24, (unsigned char *)kigou_right, disp_arw_color_code );
    set_font_to_img_buf_quick( 24, 16, (unsigned char *)kigou_down,  disp_arw_color_code );
    set_font_to_img_buf_quick( 24, 40, (unsigned char *)kigou_down,  disp_arw_color_code );
    set_font_to_img_buf_quick( 40, 24, (unsigned char *)kigou_right, disp_arw_color_code );
    view_anser_rect( disp_color_code[0] );

    view_sec_counter();

    color = 0;
    niji_pnt = (unsigned char  *)niji_data_pnt;

    niji_pnt = niji_pnt + (6 * pico_dev_read_adc1());

    while( true ) {
        do {
            /* get kanji code from niji-zyukugo */
            code_pnt = get_random_code();
            code1 = *code_pnt;
            code2 = *(code_pnt+1);

            niji_pnt = (unsigned char *)niji_data_pnt;
            x_item_1st_cnt = 0;
            x_item_2nd_cnt = 0;
            for( num=0 ; num<x_item_all_cnt ; num++ ) {
                if( (*niji_pnt == code1) && (*(niji_pnt+1) == code2) ) {
                    char_list_1st[x_item_1st_cnt] = num;
                    x_item_1st_cnt++;
                    if( x_item_1st_cnt >= DEF_NIJI_LST_MAX ) {
                        break;
                    }
                } else if( (*(niji_pnt+2) == code1) && (*(niji_pnt+3) == code2) ) {
                    char_list_2nd[x_item_2nd_cnt] = num;
                    x_item_2nd_cnt++;
                    if( x_item_2nd_cnt >= DEF_NIJI_LST_MAX ) {
                        break;
                    }
                }
                niji_pnt += 6;
            }
        } while ( x_item_1st_cnt < 2 || x_item_2nd_cnt < 2 );

        niji_ans[0] = code1;
        niji_ans[1] = code2;
        printf( "# Anser code : 0x%x%x\n", code1, code2 );
        printf( "# 1st items : %d,  2nd items : %d\n", x_item_1st_cnt, x_item_2nd_cnt );

        do {
            int  tmp1, tmp2;
            tmp1 = rand() % x_item_2nd_cnt;
            tmp2 = rand() % x_item_2nd_cnt;
            niji_a = char_list_2nd[ tmp1 ];
            niji_b = char_list_2nd[ tmp2 ];
        } while( niji_a == niji_b );

        do {
            int  tmp1, tmp2;
            tmp1 = rand() % x_item_1st_cnt;
            tmp2 = rand() % x_item_1st_cnt;
            niji_c = char_list_1st[ tmp1 ];
            niji_d = char_list_1st[ tmp2 ];
        } while( niji_c == niji_d );
        printf( "# Question item number : A=%d  B=%d  C=%d  D=%d\n", niji_a, niji_b, niji_c, niji_d );

        memset( yoji_buffer, 0x00, sizeof(yoji_buffer) );

        make_kanji_bitmap( (unsigned char *)(niji_data_pnt + niji_a*6), (unsigned char  *)yoji_buffer[0] );
        make_kanji_bitmap( (unsigned char *)(niji_data_pnt + niji_b*6), (unsigned char  *)yoji_buffer[1] );
        make_kanji_bitmap( (unsigned char *)(niji_data_pnt + niji_c*6+2), (unsigned char  *)yoji_buffer[2] );
        make_kanji_bitmap( (unsigned char *)(niji_data_pnt + niji_d*6+2), (unsigned char  *)yoji_buffer[3] );

        set_font_to_img_buf(  0, 24, (unsigned char *)yoji_buffer[0], disp_color_code[color%6] );
        color++;
        set_font_to_img_buf( 24,  0, (unsigned char *)yoji_buffer[1], disp_color_code[color%6] );
        color++;
        set_font_to_img_buf( 48, 24, (unsigned char *)yoji_buffer[2], disp_color_code[color%6] );
        color++;
        set_font_to_img_buf( 24, 48, (unsigned char *)yoji_buffer[3], disp_color_code[color%6] );
        color++;

        memset( tmp_buffer, 0x00, sizeof(tmp_buffer) );
        make_kanji_bitmap( kigou_quest, (unsigned char  *)tmp_buffer );
        set_font_to_img_buf( 24, 24, (unsigned char *)tmp_buffer, disp_que_color_code );

        x_handle_counter = 50 ; /* cyclic = 20msec */
        x_sec_remain = x_sec_init;
        x_wait_flag = 1;

        /* count down */
        while( x_sec_remain > 0 ) {
            vTaskDelay( DEF_TIM_CHK_DLY );
        }
        view_remain_counter();

        /* wait restart-btn */
        while( x_wait_flag ) {
            vTaskDelay( DEF_TIM_CHK_DLY );
        }

        memset( tmp_buffer, 0x00, sizeof(tmp_buffer) );
        clr_img_buf( 24, 24 );
        make_kanji_bitmap( niji_ans, (unsigned char  *)tmp_buffer );
        set_font_to_img_buf( 24, 24, (unsigned char *)tmp_buffer, disp_ans_color_code );

        vTaskDelay( DEF_TIM_ANS_DISP );

        clr_img_buf(  0, 24 );
        clr_img_buf( 24,  0 );
        clr_img_buf( 48, 24 );
        clr_img_buf( 24, 48 );
        clr_img_buf( 24, 24 );

        clr_ank_img_buf( DEF_REM_COL10_X, DEF_REM_COL10_Y );
        clr_ank_img_buf( DEF_REM_COL1_X,  DEF_REM_COL1_Y );

        vTaskDelay( DEF_TIM_NEXT_DLY );
    }
}


void  task_check( __unused void *params )
{
    int  rand_wait = 0;
    int  mode;
    int  tmp;

    mode = pico_dev_read_dip();
    printf( "# task_check: DIP-SW val : %d\n", mode );

    while( true ) {
        vTaskDelay( 5*100 );
        rand_wait++;
        if( rand_wait >= 120 ) {
            yoji_rand_seed = pico_dev_read_adc1() & 0xffff;
            while( yoji_rand_seed == 0 ) {
                yoji_rand_seed = rand() & 0xffff;
            }
            srand( yoji_rand_seed );
            rand_wait = 0;
        }

        tmp = pico_dev_read_dip();
        if( mode != tmp ) {
            vTaskDelete( htask2 );

            memset( image_buffer, 0x00, sizeof(image_buffer) );
            mode = tmp;
            if( mode == 2 ) {
                mode_niji_yoji = 2;
                xTaskCreate( task_niji, "NijiTask", 1024, NULL, WORKER_TASK_PRIORITY, &htask2);
            } else {
                mode_niji_yoji = 0;
                xTaskCreate( task_yoji, "YojiTask", 1024, NULL, WORKER_TASK_PRIORITY, &htask2);
            }
        }
    }
}


void cyclic_handler( __unused void *params )
{
    unsigned int  vol;
    int  bak_sec;
    unsigned char  col10, col1;
    int  vol_def_tbl[] = { 1, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 80, 99 };

  portTickType  xLastWakeTime;
  const portTickType  xFrequency = 20;

  // Initialise the xLastWakeTime variable with the current time.
  xLastWakeTime = xTaskGetTickCount();

  while( true ) {
    // Wait for the next cycle.
    vTaskDelayUntil( &xLastWakeTime, xFrequency );

    if( mode_niji_yoji != 2 ) {
        continue;
    }

    if( pico_gpio_select_btn() ) {
        x_wait_flag = 0;
    }

    if( x_sec_remain > 0 ) {
        view_remain_counter();
        x_handle_counter--;
        if( x_handle_counter <= 0 ) {
            x_handle_counter = 50;
            x_sec_remain--;
        }
    }

    if( pico_gpio_restart_btn() ) {
        x_sec_remain = 0;
        x_wait_flag = 0;
    }

    bak_sec = x_sec_init;
    vol = pico_dev_read_adc0();
    if(        vol < 0x100 ) {
        x_sec_init = vol_def_tbl[0];
    } else if( vol < 0x200 ) {
        x_sec_init = vol_def_tbl[1];
    } else if( vol < 0x300 ) {
        x_sec_init = vol_def_tbl[2];
    } else if( vol < 0x400 ) {
        x_sec_init = vol_def_tbl[3];
    } else if( vol < 0x500 ) {
        x_sec_init = vol_def_tbl[4];
    } else if( vol < 0x600 ) {
        x_sec_init = vol_def_tbl[5];
    } else if( vol < 0x700 ) {
        x_sec_init = vol_def_tbl[6];
    } else if( vol < 0x800 ) {
        x_sec_init = vol_def_tbl[7];
    } else if( vol < 0x900 ) {
        x_sec_init = vol_def_tbl[8];
    } else if( vol < 0xa00 ) {
        x_sec_init = vol_def_tbl[9];
    } else if( vol < 0xb00 ) {
        x_sec_init = vol_def_tbl[10];
    } else if( vol < 0xc00 ) {
        x_sec_init = vol_def_tbl[11];
    } else if( vol < 0xd00 ) {
        x_sec_init = vol_def_tbl[12];
    } else if( vol < 0xe00 ) {
        x_sec_init = vol_def_tbl[13];
    } else {
        x_sec_init = vol_def_tbl[14];
    }

    if( bak_sec != x_sec_init ) {
        view_sec_counter();
    }
  }
}


void main_task( __unused void *params )
{
    while( 1 ) {
        (void)hub75_test_loop();
        //vTaskDelay( 10 );
        vTaskDelay( 2 );
    }
}


int main( void )
{
    int  tmp;

    stdio_init_all();

    (void)hub75_test_init();
    (void)pico_dev_adc_init();
    (void)pico_dev_btn_init();
    (void)pico_dev_dip_init();

    printf( "# Number of items : %d\n", x_item_all_cnt );
    if( pico_gpio_select_btn() != 0 ) {
        printf( "# MODE button status : Continuous Mode\n" );
    } else {
        printf( "# Mode button status : Stop Mode\n" );
    }

    if( pico_gpio_restart_btn() != 0 ) {
        printf( "# NEXT button status : ON\n" );
    } else {
        printf( "# Mode button status : OFF\n" );
    }

    yoji_rand_seed = pico_dev_read_adc1();
    printf( "# Value of CdS : 0x%x\n", yoji_rand_seed );
    srand( yoji_rand_seed );

    printf( "# MAIN: DIP-SW val : %d\n", pico_dev_read_dip() );

    xTaskCreate( cyclic_handler, "CyclicTask", 512, NULL, MAIN_TASK_PRIORITY, &htask4);

    tmp = pico_dev_read_dip();
    if( tmp == 2 ) {
        mode_niji_yoji = 2;
        xTaskCreate( task_niji, "NijiTask", 512, NULL, WORKER_TASK_PRIORITY, &htask2);
    } else {
        mode_niji_yoji = 0;
        xTaskCreate( task_yoji, "YojiTask", 512, NULL, WORKER_TASK_PRIORITY, &htask2);
    }
    xTaskCreate( task_check, "CheckTask", 512, NULL, WORKER_TASK_PRIORITY, &htask3);

    xTaskCreate( main_task, "MainTask", 512, NULL, MAIN_TASK_PRIORITY, &htask1);
    vTaskStartScheduler();

    return 0;
}
