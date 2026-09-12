/*
* Copyright 2026 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "lvgl.h"
#include "gui_guider.h"
#include "widgets_init.h"
#include <stdlib.h>
#include <string.h>


__attribute__((unused)) void kb_event_cb (lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *kb = lv_event_get_target(e);
    if(code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }
}

__attribute__((unused)) void ta_event_cb (lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
#if LV_USE_KEYBOARD || LV_USE_ZH_KEYBOARD
    lv_obj_t *ta = lv_event_get_target(e);
#endif
    lv_obj_t *kb = lv_event_get_user_data(e);
    if (code == LV_EVENT_FOCUSED || code == LV_EVENT_CLICKED)
    {
#if LV_USE_ZH_KEYBOARD != 0
        lv_zh_keyboard_set_textarea(kb, ta);
#endif
#if LV_USE_KEYBOARD != 0
        lv_keyboard_set_textarea(kb, ta);
#endif
        lv_obj_move_foreground(kb);
        lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }
    if (code == LV_EVENT_CANCEL || code == LV_EVENT_DEFOCUSED)
    {

#if LV_USE_ZH_KEYBOARD != 0
        lv_zh_keyboard_set_textarea(kb, ta);
#endif
#if LV_USE_KEYBOARD != 0
        lv_keyboard_set_textarea(kb, ta);
#endif
        lv_obj_move_background(kb);
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }
}

#if LV_USE_ANALOGCLOCK != 0
void clock_count(int *hour, int *min, int *sec)
{
    (*sec)++;
    if(*sec == 60)
    {
        *sec = 0;
        (*min)++;
    }
    if(*min == 60)
    {
        *min = 0;
        if(*hour < 12)
        {
            (*hour)++;
        } else {
            (*hour)++;
            *hour = *hour %12;
        }
    }
}
#endif


const char * screen_animimg_1_imgs[82] = {
#if LV_USE_GUIDER_SIMULATOR
    "C:/home/he/esp/void/guider/import/image/0000.jpg",
    "C:/home/he/esp/void/guider/import/image/0001.jpg",
    "C:/home/he/esp/void/guider/import/image/0002.jpg",
    "C:/home/he/esp/void/guider/import/image/0003.jpg",
    "C:/home/he/esp/void/guider/import/image/0004.jpg",
    "C:/home/he/esp/void/guider/import/image/0005.jpg",
    "C:/home/he/esp/void/guider/import/image/0006.jpg",
    "C:/home/he/esp/void/guider/import/image/0007.jpg",
    "C:/home/he/esp/void/guider/import/image/0008.jpg",
    "C:/home/he/esp/void/guider/import/image/0009.jpg",
    "C:/home/he/esp/void/guider/import/image/0010.jpg",
    "C:/home/he/esp/void/guider/import/image/0011.jpg",
    "C:/home/he/esp/void/guider/import/image/0012.jpg",
    "C:/home/he/esp/void/guider/import/image/0013.jpg",
    "C:/home/he/esp/void/guider/import/image/0014.jpg",
    "C:/home/he/esp/void/guider/import/image/0015.jpg",
    "C:/home/he/esp/void/guider/import/image/0016.jpg",
    "C:/home/he/esp/void/guider/import/image/0017.jpg",
    "C:/home/he/esp/void/guider/import/image/0018.jpg",
    "C:/home/he/esp/void/guider/import/image/0019.jpg",
    "C:/home/he/esp/void/guider/import/image/0020.jpg",
    "C:/home/he/esp/void/guider/import/image/0021.jpg",
    "C:/home/he/esp/void/guider/import/image/0022.jpg",
    "C:/home/he/esp/void/guider/import/image/0023.jpg",
    "C:/home/he/esp/void/guider/import/image/0024.jpg",
    "C:/home/he/esp/void/guider/import/image/0025.jpg",
    "C:/home/he/esp/void/guider/import/image/0026.jpg",
    "C:/home/he/esp/void/guider/import/image/0027.jpg",
    "C:/home/he/esp/void/guider/import/image/0028.jpg",
    "C:/home/he/esp/void/guider/import/image/0029.jpg",
    "C:/home/he/esp/void/guider/import/image/0030.jpg",
    "C:/home/he/esp/void/guider/import/image/0031.jpg",
    "C:/home/he/esp/void/guider/import/image/0032.jpg",
    "C:/home/he/esp/void/guider/import/image/0033.jpg",
    "C:/home/he/esp/void/guider/import/image/0034.jpg",
    "C:/home/he/esp/void/guider/import/image/0035.jpg",
    "C:/home/he/esp/void/guider/import/image/0036.jpg",
    "C:/home/he/esp/void/guider/import/image/0037.jpg",
    "C:/home/he/esp/void/guider/import/image/0038.jpg",
    "C:/home/he/esp/void/guider/import/image/0039.jpg",
    "C:/home/he/esp/void/guider/import/image/0040.jpg",
    "C:/home/he/esp/void/guider/import/image/0041.jpg",
    "C:/home/he/esp/void/guider/import/image/0042.jpg",
    "C:/home/he/esp/void/guider/import/image/0043.jpg",
    "C:/home/he/esp/void/guider/import/image/0044.jpg",
    "C:/home/he/esp/void/guider/import/image/0045.jpg",
    "C:/home/he/esp/void/guider/import/image/0046.jpg",
    "C:/home/he/esp/void/guider/import/image/0047.jpg",
    "C:/home/he/esp/void/guider/import/image/0048.jpg",
    "C:/home/he/esp/void/guider/import/image/0049.jpg",
    "C:/home/he/esp/void/guider/import/image/0050.jpg",
    "C:/home/he/esp/void/guider/import/image/0051.jpg",
    "C:/home/he/esp/void/guider/import/image/0052.jpg",
    "C:/home/he/esp/void/guider/import/image/0053.jpg",
    "C:/home/he/esp/void/guider/import/image/0054.jpg",
    "C:/home/he/esp/void/guider/import/image/0055.jpg",
    "C:/home/he/esp/void/guider/import/image/0056.jpg",
    "C:/home/he/esp/void/guider/import/image/0057.jpg",
    "C:/home/he/esp/void/guider/import/image/0058.jpg",
    "C:/home/he/esp/void/guider/import/image/0059.jpg",
    "C:/home/he/esp/void/guider/import/image/0060.jpg",
    "C:/home/he/esp/void/guider/import/image/0061.jpg",
    "C:/home/he/esp/void/guider/import/image/0062.jpg",
    "C:/home/he/esp/void/guider/import/image/0063.jpg",
    "C:/home/he/esp/void/guider/import/image/0064.jpg",
    "C:/home/he/esp/void/guider/import/image/0065.jpg",
    "C:/home/he/esp/void/guider/import/image/0066.jpg",
    "C:/home/he/esp/void/guider/import/image/0067.jpg",
    "C:/home/he/esp/void/guider/import/image/0068.jpg",
    "C:/home/he/esp/void/guider/import/image/0069.jpg",
    "C:/home/he/esp/void/guider/import/image/0070.jpg",
    "C:/home/he/esp/void/guider/import/image/0071.jpg",
    "C:/home/he/esp/void/guider/import/image/0072.jpg",
    "C:/home/he/esp/void/guider/import/image/0073.jpg",
    "C:/home/he/esp/void/guider/import/image/0074.jpg",
    "C:/home/he/esp/void/guider/import/image/0075.jpg",
    "C:/home/he/esp/void/guider/import/image/0076.jpg",
    "C:/home/he/esp/void/guider/import/image/0077.jpg",
    "C:/home/he/esp/void/guider/import/image/0078.jpg",
    "C:/home/he/esp/void/guider/import/image/0079.jpg",
    "C:/home/he/esp/void/guider/import/image/0080.jpg",
    "C:/home/he/esp/void/guider/import/image/0081.jpg",
#else
    "S:/0000.jpg",
    "S:/0001.jpg",
    "S:/0002.jpg",
    "S:/0003.jpg",
    "S:/0004.jpg",
    "S:/0005.jpg",
    "S:/0006.jpg",
    "S:/0007.jpg",
    "S:/0008.jpg",
    "S:/0009.jpg",
    "S:/0010.jpg",
    "S:/0011.jpg",
    "S:/0012.jpg",
    "S:/0013.jpg",
    "S:/0014.jpg",
    "S:/0015.jpg",
    "S:/0016.jpg",
    "S:/0017.jpg",
    "S:/0018.jpg",
    "S:/0019.jpg",
    "S:/0020.jpg",
    "S:/0021.jpg",
    "S:/0022.jpg",
    "S:/0023.jpg",
    "S:/0024.jpg",
    "S:/0025.jpg",
    "S:/0026.jpg",
    "S:/0027.jpg",
    "S:/0028.jpg",
    "S:/0029.jpg",
    "S:/0030.jpg",
    "S:/0031.jpg",
    "S:/0032.jpg",
    "S:/0033.jpg",
    "S:/0034.jpg",
    "S:/0035.jpg",
    "S:/0036.jpg",
    "S:/0037.jpg",
    "S:/0038.jpg",
    "S:/0039.jpg",
    "S:/0040.jpg",
    "S:/0041.jpg",
    "S:/0042.jpg",
    "S:/0043.jpg",
    "S:/0044.jpg",
    "S:/0045.jpg",
    "S:/0046.jpg",
    "S:/0047.jpg",
    "S:/0048.jpg",
    "S:/0049.jpg",
    "S:/0050.jpg",
    "S:/0051.jpg",
    "S:/0052.jpg",
    "S:/0053.jpg",
    "S:/0054.jpg",
    "S:/0055.jpg",
    "S:/0056.jpg",
    "S:/0057.jpg",
    "S:/0058.jpg",
    "S:/0059.jpg",
    "S:/0060.jpg",
    "S:/0061.jpg",
    "S:/0062.jpg",
    "S:/0063.jpg",
    "S:/0064.jpg",
    "S:/0065.jpg",
    "S:/0066.jpg",
    "S:/0067.jpg",
    "S:/0068.jpg",
    "S:/0069.jpg",
    "S:/0070.jpg",
    "S:/0071.jpg",
    "S:/0072.jpg",
    "S:/0073.jpg",
    "S:/0074.jpg",
    "S:/0075.jpg",
    "S:/0076.jpg",
    "S:/0077.jpg",
    "S:/0078.jpg",
    "S:/0079.jpg",
    "S:/0080.jpg",
    "S:/0081.jpg",
#endif

};
