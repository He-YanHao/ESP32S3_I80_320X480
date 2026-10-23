/*
* Copyright 2026 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "events_init.h"
#include "widgets_init.h"
#include "custom.h"



void setup_scr_screen(lv_ui *ui)
{
    //Write codes screen
    ui->screen = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen, 320, 420);
    lv_obj_set_scrollbar_mode(ui->screen, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_slider_1
    ui->screen_slider_1 = lv_slider_create(ui->screen);
    lv_slider_set_range(ui->screen_slider_1, 0, 100);
    lv_slider_set_mode(ui->screen_slider_1, LV_SLIDER_MODE_NORMAL);
    lv_slider_set_value(ui->screen_slider_1, 50, LV_ANIM_OFF);
    lv_obj_set_pos(ui->screen_slider_1, 27, 386);
    lv_obj_set_size(ui->screen_slider_1, 268, 24);

    //Write style for screen_slider_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_slider_1, 60, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_slider_1, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_slider_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_slider_1, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(ui->screen_slider_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_slider_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style for screen_slider_1, Part: LV_PART_INDICATOR, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_slider_1, 255, LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_slider_1, lv_color_hex(0x2195f6), LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_slider_1, LV_GRAD_DIR_NONE, LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_slider_1, 8, LV_PART_INDICATOR|LV_STATE_DEFAULT);

    //Write style for screen_slider_1, Part: LV_PART_KNOB, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_slider_1, 255, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_slider_1, lv_color_hex(0x2195f6), LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_slider_1, LV_GRAD_DIR_NONE, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_slider_1, 8, LV_PART_KNOB|LV_STATE_DEFAULT);

    //Write codes screen_slider_2
    ui->screen_slider_2 = lv_slider_create(ui->screen);
    lv_slider_set_range(ui->screen_slider_2, 0, 100);
    lv_slider_set_mode(ui->screen_slider_2, LV_SLIDER_MODE_NORMAL);
    lv_slider_set_value(ui->screen_slider_2, 50, LV_ANIM_OFF);
    lv_obj_set_pos(ui->screen_slider_2, 27, 12);
    lv_obj_set_size(ui->screen_slider_2, 268, 24);

    //Write style for screen_slider_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_slider_2, 60, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_slider_2, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_slider_2, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_slider_2, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(ui->screen_slider_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_slider_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style for screen_slider_2, Part: LV_PART_INDICATOR, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_slider_2, 255, LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_slider_2, lv_color_hex(0x2195f6), LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_slider_2, LV_GRAD_DIR_NONE, LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_slider_2, 8, LV_PART_INDICATOR|LV_STATE_DEFAULT);

    //Write style for screen_slider_2, Part: LV_PART_KNOB, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_slider_2, 255, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_slider_2, lv_color_hex(0x2195f6), LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_slider_2, LV_GRAD_DIR_NONE, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_slider_2, 8, LV_PART_KNOB|LV_STATE_DEFAULT);

    //Write codes screen_slider_3
    ui->screen_slider_3 = lv_slider_create(ui->screen);
    lv_slider_set_range(ui->screen_slider_3, 0, 100);
    lv_slider_set_mode(ui->screen_slider_3, LV_SLIDER_MODE_NORMAL);
    lv_slider_set_value(ui->screen_slider_3, 50, LV_ANIM_OFF);
    lv_obj_set_pos(ui->screen_slider_3, 15, 49);
    lv_obj_set_size(ui->screen_slider_3, 25, 327);

    //Write style for screen_slider_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_slider_3, 60, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_slider_3, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_slider_3, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_slider_3, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(ui->screen_slider_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_slider_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style for screen_slider_3, Part: LV_PART_INDICATOR, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_slider_3, 255, LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_slider_3, lv_color_hex(0x2195f6), LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_slider_3, LV_GRAD_DIR_NONE, LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_slider_3, 8, LV_PART_INDICATOR|LV_STATE_DEFAULT);

    //Write style for screen_slider_3, Part: LV_PART_KNOB, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_slider_3, 255, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_slider_3, lv_color_hex(0x2195f6), LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_slider_3, LV_GRAD_DIR_NONE, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_slider_3, 8, LV_PART_KNOB|LV_STATE_DEFAULT);

    //Write codes screen_slider_4
    ui->screen_slider_4 = lv_slider_create(ui->screen);
    lv_slider_set_range(ui->screen_slider_4, 0, 100);
    lv_slider_set_mode(ui->screen_slider_4, LV_SLIDER_MODE_NORMAL);
    lv_slider_set_value(ui->screen_slider_4, 50, LV_ANIM_OFF);
    lv_obj_set_pos(ui->screen_slider_4, 275, 46);
    lv_obj_set_size(ui->screen_slider_4, 26, 327);

    //Write style for screen_slider_4, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_slider_4, 60, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_slider_4, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_slider_4, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_slider_4, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(ui->screen_slider_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_slider_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style for screen_slider_4, Part: LV_PART_INDICATOR, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_slider_4, 255, LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_slider_4, lv_color_hex(0x2195f6), LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_slider_4, LV_GRAD_DIR_NONE, LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_slider_4, 8, LV_PART_INDICATOR|LV_STATE_DEFAULT);

    //Write style for screen_slider_4, Part: LV_PART_KNOB, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_slider_4, 255, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_slider_4, lv_color_hex(0x2195f6), LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_slider_4, LV_GRAD_DIR_NONE, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_slider_4, 8, LV_PART_KNOB|LV_STATE_DEFAULT);

    //The custom code of screen.


    //Update current screen layout.
    lv_obj_update_layout(ui->screen);

}
