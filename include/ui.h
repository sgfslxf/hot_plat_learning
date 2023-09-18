#pragma once

#include "User.h"
#include "EC11.h"

#define show_disable 0  // 不显示
#define show_set_temp 1  // 显示设置温度
#define show_now_temp 2  // 显示adc温度
#define show_set_light 3  // 显示设置亮度
#define show_temp_mode1_time 4  // 设置恒温时间

#define back 0
#define sure 1

void ui_key_callb(ec11_task_result_type ec_type, int16_t ec_value);
extern short curve_temp_buf[4];

class UI
{

public:
    void run_task();
    bool page_switch(uint8_t mode);
    void page1_key(ec11_task_result_type ec_type, int16_t ec_value);
    void page2_key(ec11_task_result_type ec_type, int16_t ec_value);
    void page3_key(ec11_task_result_type ec_type, int16_t ec_value);
    bool oled_display_set();
public:
    uint8_t show_temp_mode = show_now_temp;
    uint8_t temp_move_flg = 0; //温度显示切换标志
    uint8_t temp_mode_flg = 0; //加热模式切换标志
    uint8_t heat_icon_flg = 0; //加热开关切换标志
    bool page_switch_flg = true;  //界面切换标志
    int8_t switch_buf = sure;
    bool temp_time_switch_flg = false;  // 第一页面数字显示切换
    bool oled_sleep_flg = false;
    int16_t oled_light = 0;
    bool wake_sleep_change_flg = 0;
private:
    void show_temp(int8_t x, int8_t y, int8_t xx, int8_t yy);
    void show_page(short x, short y, uint8_t page);
    void temp_move();
    void temp_mode_move();
    void heat_move();
    void page2_move();
    void page3_switch();
    void blinker_config();
    void write_oled_light();
    void show_curve(int8_t y, int8_t data_y);
    void temp_time_switch();
    void wake_sleep_page();
    void error_temp_fix_page_move();
    void show_warning();
private:
    uint8_t set_temp_x = 92;
    uint8 show_warning_flg = 0;
    int8_t miot_option_buf = 0;
    bool blinker_config_flg = 0;
    bool write_oled_flg = 0;
    uint8_t error_temp_fix_page_buf = 0;
    uint8_t error_temp_fix_page_move_buf = 0;
};

extern UI ui;
extern uint8_t show_temp_mode;

