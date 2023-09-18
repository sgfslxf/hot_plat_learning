#include "User.h"
#include "bmp.h"
UI ui;

#define page2_menu_num_max 8

typedef enum  // 用于页面3
{
    temp_mode_menu_num,  // 温控模式
    backflow_menu_num,  // 回流焊参数
    constant_temperature_menu_num,  // 恒温时间参数
    iot_menu_num,  // iot参数
    oled_light_menu_num,  // 屏幕亮度参数
    use_menu_num,  // 使用说明页面参数
    about_menu_num,  // 关于页面参数
    error_temp_fix_num  // 温度校准页面
} page2_menu_num_type;

int8_t page_num = 0;  // 页面记录

/*菜单页定义*/
int8_t page2_menu_num = 0;
int8_t page2_move_tmp = 0;
int8_t page2_move_flg = 0;

const char page2_menu0[] = {"温控模式"};
const char page2_menu1[] = {"回流参数"};
const char page2_menu2[] = {"恒温参数"};
const char page2_menu3[] = {"物联网"};
const char page2_menu4[] = {"屏幕亮度"};
const char page2_menu5[] = {"使用说明"};
const char page2_menu6[] = {"关于"};
const char page2_menu7[] = {"温度校准"};

const char *page2_str_ptr[] = {page2_menu0, page2_menu1, page2_menu2, page2_menu3, page2_menu4, page2_menu5, page2_menu6, page2_menu7};
const char *page2_bmp_ptr[] = {page2_bmp0, page2_bmp1, page2_bmp2, page2_bmp3, page2_bmp4, page2_bmp5, page2_bmp6, page2_bmp7};
/************************/

/*设置页定义*/
int8_t page3_switch_flg = 0;

const char menu0_option0[] = {"回流焊模式"};
const char menu0_option1[] = {"恒温模式"};

const char menu1_option0[] = {"活性区"};
const char menu1_option1[] = {"回流区"};
const char menu1_option_s[] = {"秒"};
const char menu1_option_t[] = {"摄"};
int8_t temp_mode0_option = 0; //回流曲线设置项 0：活性区温度 1：活性区时间 2：回流区温度 3：回流区时间
short curve_temp_buf[] = {    // 0：保温区温度110-200 1：保温区时长60-120 2：回流区温度220-上限  3：回流区时长 30-90
    130, 60, 240, 30};

const char menu2_option0[] = {"分钟"};

const char menu3_option0_0[] = {"单击开启"};
const char menu3_option0_1[] = {"单击关闭"};
const char menu3_option1_0[] = {"单击配置网络密匙"};
const char menu3_option1_1[] = {"请连接至热点"};
const char menu3_option1_2[] = {"QF_HP"};

const char menu5_option0_0[] = {"只要微笑就好了"};

const char menu6_option0_0[] = {"汎用恒温加熱台"};
const char menu6_option0_1[] = {"量產機"};
const char menu6_option0_2[] = {"01"};

const char menu7_option0_0[] = {"实测最高"};
const char menu7_option0_1[] = {"校准温度"};
const char menu7_option0_2[] = {":   `"};
const char menu7_option0_3[] = {"校准结束后将"};
const char menu7_option0_4[] = {"自动返回"};

/************************/



/// @brief 放在loop循环函数中 用于根据各种标志位进行画面移动
void UI::run_task()
{
    wake_sleep_page();  // 根据wake_sleep_change_flg决定苏醒屏幕
    if (!oled_sleep_flg)  // 屏幕未休眠
    {
        switch (page_num)  // 根据标志 确定每个页面的移动
        {
            case 1:
                if(temp_move_flg)  // 界面温度显示动画
                {
                    temp_move();
                    temp_move_flg = 0;
                }
                if(temp_mode_flg)  // 加热模式显示
                {
                    temp_mode_move();
                    temp_mode_flg = 0;
                }
                if(heat_icon_flg)  // 加热动画
                {
                    heat_move();
                    heat_icon_flg = 0;
                }
                if (temp_time_switch_flg == true)  // 小图标温度和时间切换
                {
                    temp_time_switch();
                    temp_time_switch_flg = false;
                }
                if(show_warning_flg)  // 就是回流焊模式 主界面无法调节温度提示
                {
                    show_warning();
                    show_warning_flg = 0;
                }
                break;
            case 2:
                page2_move();
                break;
            case 3:
                if(page3_switch_flg)
                {
                    page3_switch();
                    page3_switch_flg = 0;
                } 
                if(blinker_config_flg)
                {
                    blinker_config();
                    blinker_config_flg = 0;
                }
                error_temp_fix_page_move();
                break;
        }

        if (page2_move_flg)  // 如果还是在页面2 编码器值未到0 就还要移动页面
            return;
        else
        {
            page_switch(switch_buf);  // 第一次进入系统 切换page_num = 1

            if (!oled_flg)  // oled_flg = 0不进行下面的
                return;
            else
            {
                oled_flg = 0;
                oled.clr();

                show_page(0, 0, page_num);
                write_oled_light();  // 更新亮度

                oled.refresh();
            }
        }
    }
    else  // 如果屏幕为睡眠状态   
    {
        return;
    }
}


/// @brief 回流焊模式 第一页面旋转编码器修改参数警告
void UI::show_warning()
{
    for (int8_t i = 32; i >= 0; i--)
    {
        oled.clr();
        show_page(0, i, 4);
        show_page(0, i - 32, 1);
        oled.refresh();
        delay(1);
    }
    delay(1000);
    for (int8_t i = 0; i < 32; i++)
    {
        oled.clr();
        show_page(0, i, 4);
        show_page(0, i - 32, 1);
        oled.refresh();
        delay(1);
    }
}


/// @brief 第一页面按键关联函数
/// @param ec_type 按键类型
/// @param ec_value 按键值
void UI::page1_key(ec11_task_result_type ec_type, int16_t ec_value) //主界面按键对应功能
{
    if (ec_type == ec11_task_is_key)
    {
        switch (ec_value)
        {
        case sw_click:
            if (pwm.power)
            {
                pwm.end();
            }
            else
            {
                pwm.begin();
            }
            break;
        case sw_long:
            switch_buf = sure;
            page_switch_flg = 1;
            ec11.speed_up(false);
            break;
        case sw_double:
            pwm.fan();
            break;
        default:
            break;
        }
    }
    else
    {
        if (pwm.temp_mode)
        {
            pwm.temp_buf += ec_value;

            if (pwm.temp_buf > adc.hotbed_max_temp)
                pwm.temp_buf = adc.hotbed_max_temp;
            else if (pwm.temp_buf < 40)
                pwm.temp_buf = 40;
            eeprom.write_flg = 1;
            if (show_temp_mode != show_set_temp)
            {
                show_temp_mode = show_set_temp;
                temp_move_flg = 1;
            }
        }
        else
        {
            show_warning_flg = 1;
        }
    }
}


/// @brief 第二页面按键关联函数
/// @param ec_type 按键类型
/// @param ec_value 按键值
void UI::page2_key(ec11_task_result_type ec_type, int16_t ec_value) //界面2按键对应功能
{
    if (ec_type == ec11_task_is_key)
    {
        switch (ec_value)
        {
            case sw_long:  // 长按进入第三级页面
                switch_buf = sure;
                page_switch_flg = 1;
                if (page2_menu_num == constant_temperature_menu_num || page2_menu_num == oled_light_menu_num || page2_menu_num == backflow_menu_num)
                    ec11.speed_up(true);  // 恒温时间里开启编码器旋转加速
                ec11.double_click(false);  // 关闭双击
                break;
            case sw_double:  // 双击退出第二页面回到第一页面
                switch_buf = back;
                page_switch_flg = 1;
                ec11.speed_up(true);
                break;
            default:
                break;
        }
    }
    else  // 编码器旋转 切换第二级页面的子页面
    {
        if (page2_move_tmp < 0 && ec_value == 1)
            page2_move_tmp = -1;
        else if (page2_move_tmp > 0 && ec_value == -1)
            page2_move_tmp = 1;

        page2_move_tmp += ec_value;
        page2_move_flg = 1;
    }
}


/// @brief 第三页面按键关联函数
/// @param ec_type 按键类型
/// @param ec_value 按键值
void UI::page3_key(ec11_task_result_type ec_type, int16_t ec_value) //界面3按键对应功能
{
    if (ec_type == ec11_task_is_key)
    {
        if (ec_value == sw_click) //单击
        {
            if (page2_menu_num == backflow_menu_num && !page3_switch_flg)
            {
                page3_switch_flg = 1;  // 第三页面切换标志
                temp_mode0_option++;  // 三个回流参数切换
                if (temp_mode0_option == 4) // 循环
                    temp_mode0_option = 0;
            }

            if (page2_menu_num == iot_menu_num  && !page3_switch_flg)
            {
                if (miot_option_buf)  // 在单机配网页面
                {
                    blinker_config_flg = 1;
                }
                else  // 在单击开启关闭页面
                {
                    page3_switch_flg = 1;
                    miot.miot_able = !miot.miot_able;
                }
            }
            
            if (page2_menu_num == error_temp_fix_num)  // 温度校准页面
            {
                switch (error_temp_fix_page_buf)
                {
                    case 0:
                        error_temp_fix_page_buf = 1;
                        break;
                    case 1:
                        error_temp_fix_page_buf = 0;
                        break;
                    case 2:
                        if (pwm.power == 0 && adc.now_temp < 150)
                        {
                            error_temp_fix_page_buf = 3;
                            error_temp_fix_page_move_buf = 2;
                        }
                        break;
                    default:
                        error_temp_fix_page_buf = 2;
                        error_temp_fix_page_move_buf = 2;
                        break;
                }
            }
        }
        else  // 第二级页面关闭了双击，此处为长按退出
        {
            if (wifima.wifima_flg)  // 
            {
                wifima.back_flg = 1;
            }
            else
            {
                if (error_temp_fix_page_buf == 1)
                    error_temp_fix_page_buf = 0;
                if (error_temp_fix_page_buf == 3)
                    error_temp_fix_page_buf = 2;

                switch_buf = back;
                page_switch_flg = 1;
                ec11.speed_up(false);
                if (page2_menu_num < 6 || page2_menu_num == error_temp_fix_num)
                    eeprom.write_flg = 1;
                ec11.double_click(true);

                if (pwm.power && page2_menu_num == temp_mode_menu_num)
                {
                    pwm.end();
                }
                if (page2_menu_num == iot_menu_num)
                {
                    if (miot.miot_able && !miot.open_flg)
                    {
                        eeprom.write(miot_able_add, 1);
                        ESP.reset();
                    }
                }
            }
        }
    }
    else
    {
        switch (page2_menu_num)
        {
            case temp_mode_menu_num: //温控模式
                if (!page3_switch_flg)
                    pwm.temp_mode = !pwm.temp_mode;
                break;

            case backflow_menu_num: //回流参数
                if (!page3_switch_flg)
                {
                    switch (temp_mode0_option)
                    {
                    case 0:
                        curve_temp_buf[0] += ec_value;
                        if (curve_temp_buf[0] < 110)
                            curve_temp_buf[0] = 110;
                        else if (curve_temp_buf[0] > 200)
                            curve_temp_buf[0] = 200;
                        break;

                    case 1:
                        curve_temp_buf[1] += ec_value;
                        if (curve_temp_buf[1] < 60)
                            curve_temp_buf[1] = 60;
                        else if (curve_temp_buf[1] > 120)
                            curve_temp_buf[1] = 120;
                        break;

                    case 2:
                        curve_temp_buf[2] += ec_value;
                        if (curve_temp_buf[2] < 220)
                            curve_temp_buf[2] = 220;
                        else if (curve_temp_buf[2] > adc.hotbed_max_temp)
                            curve_temp_buf[2] = adc.hotbed_max_temp;
                        break;

                    case 3:
                        curve_temp_buf[3] += ec_value;
                        if (curve_temp_buf[3] < 30)
                            curve_temp_buf[3] = 30;
                        else if (curve_temp_buf[3] > 90)
                            curve_temp_buf[3] = 90;
                        break;
                    }
                }
                break;

            case constant_temperature_menu_num: //恒温参数
                pwm.constant_temp_time += ec_value;
                if (pwm.constant_temp_time < 0)
                    pwm.constant_temp_time = 0;
                else if (pwm.constant_temp_time > 520)
                    pwm.constant_temp_time = 520;
                break;

            case iot_menu_num: // iot
                if (!page3_switch_flg)
                    miot_option_buf = !miot_option_buf;
                break;

            case oled_light_menu_num: // 屏幕亮度
                oled_light += ec_value;
                if (oled_light < 0)
                    oled_light = 0;
                else if (oled_light > 255)
                    oled_light = 255;
                write_oled_flg = 1;
                break;
            case error_temp_fix_num: //温度校准
                switch (error_temp_fix_page_buf)
                {
                case 0:
                    if (ec_value > 0)
                    {
                        error_temp_fix_page_buf = 2;
                        error_temp_fix_page_move_buf = 1;
                    }
                    break;
                case 1:
                {
                    adc.hotbed_max_temp += ec_value;
                    if (adc.hotbed_max_temp < 240)
                        adc.hotbed_max_temp = 240;
                    else if (adc.hotbed_max_temp > 270)
                        adc.hotbed_max_temp = 270;
                }
                break;
                case 2:
                    if (ec_value < 0)
                    {
                        error_temp_fix_page_buf = 0;
                        error_temp_fix_page_move_buf = 1;
                    }
                    break;
                default:
                    break;
                }
            default:
                break;
        }
        if (page2_menu_num == temp_mode_menu_num || page2_menu_num == iot_menu_num)
            page3_switch_flg = ec_value;
    }
}


/// @brief 第三级页面：物联网中 单击设置网络密钥
void UI::blinker_config()
{
    int8_t y;
    y = -1;
    for (;;)
    {
        if (y == -33)
            break;
        oled.clr();
        show_page(0, y, 3);
        oled.chinese(16, y + 32, menu3_option1_1, 16, 1, 0);
        oled.str(44, y + 48, menu3_option1_2, 16, 1, 0);
        oled.refresh();
        y--;
        yield();
    }
    Serial.println("Start ap config");
    wifima.startConfigPortal("QF_HP");
    Serial.println("End ap config");

    y = 1;
    for (;;)
    {
        if (y == 32)
            break;
        oled.clr();
        show_page(0, y - 32, 3);
        oled.chinese(16, y, menu3_option1_1, 16, 1, 0);
        oled.str(44, y + 16, menu3_option1_2, 16, 1, 0);
        oled.refresh();
        y++;
        yield();
    }
}


/// @brief 在睡眠模式将屏幕切换标志设为1，当按键按下时使用
/// @return 返回屏幕状态 0 为清醒，1为睡眠
bool UI::oled_display_set()
{
    oled_sleep_t = 0;  // 清空睡眠计时

    if (!ui.oled_sleep_flg)  // 屏幕为开启（睡眠为0）
    {
        return 0;
    }
    else // 屏幕为关闭（睡眠为1），则切换屏幕状态，并清零睡眠计时
    {
        ui.wake_sleep_change_flg = 1;
        return 1;
    }
}


/// @brief 和ec11编码器关联的回调函数，用于控制ui界面
/// @param ec_type 按键类型
/// @param ec_value 按键值
void ui_key_callb(ec11_task_result_type ec_type, int16_t ec_value) //按键事件中断处理
{
    if (ui.oled_display_set()) return;

    switch (page_num)
    {
        case 1:
            ui.page1_key(ec_type, ec_value);
            break;
        case 2:
            ui.page2_key(ec_type, ec_value);
            break;
        case 3:
            ui.page3_key(ec_type, ec_value);
            break;
    }
    eeprom.write_t = 0;
}


/// @brief 写屏幕亮度
void UI::write_oled_light()
{
    if (write_oled_flg)
    {
        write_oled_flg = 0;
        oled.light(oled_light);
    }
}


/// @brief 根据mode值，更新page_num值，并显示更新后的页面
/// @param mode mode：sure进入下一页，mode：back返回上一页
/// @return mode值错误返回0，否则返回1
bool UI::page_switch(uint8_t mode)
{
    if (!page_switch_flg)
        return 0;
    page_switch_flg = false;  // 屏幕转换标志清零

    int8_t next_page;  // 根据mode确定下一页屏幕编号
    int8_t show_y = 0;  // 当前显示屏幕左上角y值
    int8_t next_y = 0;  // 下一张屏幕左上角y值

    if (mode == back)
    {
        next_page = page_num - 1;
        next_y = -32;
    }
    else if (mode == sure)
    {
        next_page = page_num + 1;
        next_y = 32;
    }
    else
        return 0;

    while(1)
    {
        oled.clr();
        show_page(0, show_y, page_num);
        show_page(0, next_y, next_page);
        oled.refresh();

        if (mode == back)  // 如果屏幕后退，向下滚动
        {
            show_y++;
            next_y++;
        }
        else  // 如果屏幕前进，向上滚动
        {
            show_y--;
            next_y--;
        }
        if (show_y == 33 || show_y == -33)
            break;
        yield();  // 暂时让出CPU的控制权
    }

    page_num = next_page;  // 更新page_num

    return 1;
}


/// @brief 屏幕显示，一共三级页面，判断进入
/// @param x 整个页面左上角x坐标值0-128 为128的时候就相当于整个不显示了
/// @param y 整个页面左上角y坐标值0-32 为32的时候也是相当于整个不显示了
/// @param page 第几级页面，共三级
void UI::show_page(short x, short y, uint8_t page)
{
    uint8_t mode_tmp = ui.show_temp_mode;
    switch (page)
    {
        case 1:
            // 第一页面显示两个数字，一个是ADC温度用的图库数字显示，
            // 一个是小的数字显示设置温度或者恒温时间
            if (show_temp_mode == show_now_temp)  // 显示ADC温度
                show_temp(x, y, 93, y + 18);  // 显示小的设置或时间温度
            else  // 显示设置温度 show_set_temp
                show_temp(x, y, 0, 0);  // 不显示小的设置或时间温度
            oled.xy_set(0, 0, 128, 4);

            // 第一页面显示控温模式
            if (pwm.temp_mode == Reflow_Soldering)
                oled.chinese(69, y, "回流", 16, 1, 0);
            else
                oled.chinese(69, y, "恒温", 16, 1, 0);

            // 第一页面显示加热图标
            if (pwm.power)
                oled.BMP(95, y + 2, 32, 28, heating, 1);
           
            break;
        case 2:
            // 第二页面显示 显示功能图标和功能名称
            oled.xy_set(0, 0, 128, 4);
            oled.chinese(64, y + 8, page2_str_ptr[page2_menu_num], 16, 1, 0);
            oled.BMP(y, page2_bmp_ptr[page2_menu_num]);
            break;
        case 3:
            // 根据第二页面确定第三页面
            switch (page2_menu_num)
            {
                case temp_mode_menu_num: //模式设置 恒温还是回流
                    if (pwm.temp_mode)
                        oled.chinese(32, y + 8, menu0_option1, 16, 1, 0);
                    else
                        oled.chinese(24, y + 8, menu0_option0, 16, 1, 0);
                    break;

                case backflow_menu_num: //回流曲线
                    if (page3_switch_flg)  // 居然能绘制回流焊曲线
                    {
                        show_curve(0, y);
                    }
                    else
                    {
                        show_curve(y, y);
                    }
                    break;

                case constant_temperature_menu_num: //恒温时长
                    ui.show_temp_mode = show_temp_mode1_time;  // 设置显示模式为恒温时间调节
                    show_temp(12, y, 0, 0);
                    oled.chinese(84, y + 16, menu2_option0, 16, 1, 0);
                    ui.show_temp_mode = mode_tmp;  // 复原为初始的
                    break;

                case iot_menu_num: //物联网
                    if (miot_option_buf)  // "单击配置网络密匙"
                    {
                        oled.chinese(0, y + 8, menu3_option1_0, 16, 1, 0);
                    }
                    else
                    {
                        if (miot.miot_able) // 单击关闭
                            oled.chinese(32, y + 8, menu3_option0_1, 16, 1, 0);
                        else  // 单击开启
                            oled.chinese(32, y + 8, menu3_option0_0, 16, 1, 0);
                    }
                    break;

                case oled_light_menu_num: //屏幕亮度
                    ui.show_temp_mode = show_set_light;
                    show_temp(28, y, 0, 0);
                    ui.show_temp_mode = mode_tmp;
                    break;

                case use_menu_num: //使用说明
                    oled.chinese(8, y + 10, menu5_option0_0, 16, 1, 0);
                    // oled.str(64, y + 8, menu5_option0_1, 16, 1, 0);
                    break;

                case about_menu_num: //关于
                    oled.chinese(0, y, menu6_option0_0, 16, 1, 1);
                    oled.chinese(16, y+16, menu6_option0_1, 16, 1, 0);
                    oled.str(72, y+16, menu6_option0_2 , 16, 1, 0);
                    break;

                case error_temp_fix_num: //温度校准
                    oled.chinese(0, y, menu7_option0_0, 16, 1, 1); // 实测最高
                    oled.chinese(0, y + 16, menu7_option0_1, 16, 1, 1);  // 校准温度
                    oled.str(64, y, menu7_option0_2, 16, 1, 0); // 冒号
                    oled.str(64, y + 16, menu7_option0_2, 16, 1, 0);  // 冒号
                    oled.num(72, y, adc.hotbed_max_temp, 3, 16, LEFT, 1);  //热床最大温度
                    oled.num(72, y + 16, adc.adc_max_temp, 3, 16, LEFT, 1);  // 最大测量温度

                    switch (error_temp_fix_page_buf)  // 小圆圈位置状态
                    {
                        case 0:
                            oled.BMP(118, y + 4, circle_kong);  // 空心圆
                            break;
                        case 1:
                            oled.BMP(118, y + 4, circle_shi);  // 实心圆
                            break;
                        default:
                            oled.BMP(118, y + 20, circle_kong);  // 空心圆
                            break;
                    }
                    break;
            }
            break;
        case 4: //显示提示
            oled.chinese(x + 8, y, "回流模式请到菜", 16, 1, 0);
            oled.chinese(x + 16, y + 16, "单内设置参数", 16, 1, 0);
            break;
        default:
            break;
    }
}


/// @brief 根据wake_sleep_change_flg标志位改变屏幕状态，当状态为1则亮屏，为0
void UI::wake_sleep_page()
{
    if (wake_sleep_change_flg)  // 为1表示屏幕切换
    {
        if (oled_sleep_flg)  //表示屏幕为休眠状态 
        {
            oled.display_on();
            for (int8_t i = 32; i > 0; i--)
            {
                oled.clr();
                show_page(0, i, page_num);
                oled.refresh();
                yield();
            }
        }
        else //oled_sleep_flg 为0 表示未休眠
        {
            oled.roll(0, 0, 128, 4, 1, UP, 32);
            oled.display_off();
        }
        oled_sleep_flg = !oled_sleep_flg;  // 当进入一个状态后，翻转屏幕为另一个状态
        wake_sleep_change_flg = 0;  // 关闭屏幕状态转换标志
    }
}


/// @brief 第三页面切换显示
void UI::page3_switch()
{
    int8_t y;

    if (page3_switch_flg < 0)
    {
        oled.roll(0, 0, 128, 4, 2, DOWN, 16);
        y = -32;
    }
    else
    {
        if (page2_menu_num == backflow_menu_num)
            oled.roll(72, 0, 48, 4, 1, UP, 32);
        else
            oled.roll(0, 0, 128, 4, 2, UP, 16);
        y = 32;
    }

    while(1)
    {
        if (y == 0)
            break;

        oled.clr();
        show_page(0, y, 3);
        oled.refresh();

        if (y < 0)
            y++;
        else
            y--;
        yield();
    }
}


/// @brief 页面2 的移动显示动画
void UI::page2_move()
{

    if (!page2_move_flg)
        return;
    int8_t num_tmp;
    int8_t now_y = 0;
    int8_t next_y;

    if (page2_move_tmp < 0)  // 向下移动第二级页面
    {
        num_tmp = page2_menu_num - 1;
        next_y = -32;
    }
    else  // 向上移动第二级页面
    {
        num_tmp = page2_menu_num + 1;
        next_y = 32;
    }

    // 限制num_tmp在（0，7）
    if (num_tmp < 0)
        num_tmp = page2_menu_num_max - 1;
    else if (num_tmp == page2_menu_num_max)
        num_tmp = 0;

    while(1)
    {
        if (page2_move_tmp == 0)
        {
            if (next_y < 0)
                page2_move_tmp = -1;
            else
                page2_move_tmp = 1;
        }
        now_y -= page2_move_tmp;
        next_y -= page2_move_tmp;
        if (now_y < -32 || now_y > 32)
            break;
        oled.clr();
        oled.chinese(64, now_y + 8, page2_str_ptr[page2_menu_num], 16, 1, 0);
        oled.BMP(now_y, page2_bmp_ptr[page2_menu_num]);

        oled.chinese(64, next_y + 8, page2_str_ptr[num_tmp], 16, 1, 0);
        oled.BMP(next_y, page2_bmp_ptr[num_tmp]);

        oled.refresh();
        yield();
    }

    // 编码器左右旋转有加速，要循环到0才停
    if (page2_move_tmp < 0)
    {
        page2_move_tmp++;
    }
    else
    {
        page2_move_tmp--;
    }
    // Serial.println(page2_move_tmp);
    if (page2_move_tmp == 0)
        page2_move_flg = 0;
    page2_menu_num = num_tmp;
}


/// @brief 加热图标显示动画
void UI::heat_move()
{
    int8_t y;
    if (pwm.power)
        y = 32;
    else
        y = 2;
    oled.xy_set(0, 0, 128, 4);
    for (;;)
    {
        if (pwm.power)
            y--;
        else
            y++;
        oled.choose_clr(101, 0, 26, 2);
        oled.choose_clr(95, 2, 32, 2);
        oled.BMP(95, y, 32, 28, heating, 1);
        oled.choose_refresh(95, 0, 32, 4);
        if (y == 2 || y == 32)
            return;
        yield();
    }
}


/// @brief 加热模式显示与切换
void UI::temp_mode_move()
{
    int8_t y = 0;
    oled.xy_set(0, 0, 128, 2);
    for (y = 0; y >= -16; y--)
    {
        oled.choose_clr(69, 0, 32, 2);
        if (pwm.temp_mode == Reflow_Soldering)
        {
            oled.chinese(69, y + 16, "回流", 16, 1, 0);
            oled.chinese(69, y, "恒温", 16, 1, 0);
        }
        else
        {
            oled.chinese(69, y, "回流", 16, 1, 0);
            oled.chinese(69, y + 16, "恒温", 16, 1, 0);
        }
        oled.choose_refresh(69, 0, 32, 2);
        delay(8);
    }
}

extern Ticker adc_max_temp_tic;


/// @brief 温度校准页面切换显示
void UI::error_temp_fix_page_move()
{
    if (error_temp_fix_page_move_buf == 0)
        return;

    int8_t tmp;

    if (error_temp_fix_page_move_buf == 1)
    {
        if (error_temp_fix_page_buf)
            tmp = 4;
        else
            tmp = 20;
        for (uint8_t x = 1; x < 17; x++)
        {
            oled.choose_clr(118, 0, 9, 4);
            oled.BMP(118, tmp, circle_kong);
            oled.choose_refresh(118, 0, 9, 4);
            if (error_temp_fix_page_buf)
                tmp++;
            else
                tmp--;
            delay(10);
        }
    }
    else
    {

        if (error_temp_fix_page_buf == 2)
            tmp = -32;
        else
            tmp = 0;
        for (uint8_t x = 0; x < 33; x++)
        {
            oled.clr();
            show_page(0, tmp, 3);
            oled.chinese(0, tmp + 32, menu7_option0_3, 16, 1, 1);
            oled.chinese(0, tmp + 48, menu7_option0_4, 16, 1, 1);
            oled.BMP(95, tmp + 34, 32, 28, heating, 1);
            oled.num(64, tmp + 48, adc.now_temp, 3, 16, LEFT, 1);
            oled.refresh();
            if (error_temp_fix_page_buf == 2)
                tmp++;
            else
                tmp--;
            yield();
        }
        if (error_temp_fix_page_buf == 3)
        {
            adc.adc_max_temp_auto_flg = 0;
            digitalWrite(PWM_IO, HIGH);
            adc_max_temp_tic.attach(20, adc_max_temp_auto_feed);
            error_temp_fix_page_buf = 4;
            while (error_temp_fix_page_buf == 4)
            {
                if (adc.adc_max_temp_auto())
                {
                    digitalWrite(PWM_IO, LOW);
                    break;
                }
                oled.choose_clr(64, 2, 24, 2);
                oled.num(64, 16, adc.now_temp, 3, 16, LEFT, 1);
                oled.refresh();
                delay(1000);
            }
            adc_max_temp_tic.detach();
            error_temp_fix_page_buf = 2;
            adc.adc_max_temp_auto_flg = 1;
            digitalWrite(PWM_IO, LOW);
            tmp = -32;
            for (uint8_t x = 0; x < 33; x++)
            {
                oled.clr();
                show_page(0, tmp, 3);
                oled.chinese(0, tmp + 32, menu7_option0_3, 16, 1, 1);
                oled.chinese(0, tmp + 48, menu7_option0_4, 16, 1, 1);
                oled.BMP(95, tmp + 34, 32, 28, heating, 1);
                oled.num(64, tmp + 48, adc.now_temp, 3, 16, LEFT, 1);
                oled.refresh();
                tmp++;
                yield();
            }
        }
    }
    error_temp_fix_page_move_buf = 0;
}


/// @brief 
/* 温度移动 一种是在设置温度模式，先上移大图标 显示温度，再移动大图标 设置温度从（68，32）位置到（0，0），小图标从（93，18）到（69，18）
    一种是在显示温度模式，先移动大图标设置温度温度从（0，0）位置到（68，32），小图标从（69，18）到（93，18），再上移大图标 显示温度
*/
void UI::temp_move()
{
    int8_t temp_x;
    uint8_t temp_y;
    uint8_t small_x;
    if (show_temp_mode == show_now_temp)
    {
        temp_x = 0;
    }
    else
    {
        oled.roll(0, 0, 68, 4, 2, UP, 16);  // 先上移大图标 显示温度
        temp_x = 68;
    }

    for (;;)
    {
        if (show_temp_mode == show_now_temp)
        {
            temp_x += 4;
        }
        else if (show_temp_mode == show_set_temp)
        {
            temp_x -= 4;
        }

        temp_y = temp_x * 32 / 68;  // 按x和字符比例确定y
        small_x = temp_x * 100 / 283 + 69;  // 小数字

        oled.choose_clr(0, 0, 68, 4);
        oled.choose_clr(68, 2, 24, 2);

        show_temp(temp_x, temp_y, small_x, 18);

        oled.choose_refresh(0, 0, 68, 4);
        oled.choose_refresh(68, 2, 24, 2);
        if (temp_x >= 68 || temp_x <= 0)
            break;
        yield();
    }

    // 再上移大图标 显示温度
    if (show_temp_mode == show_now_temp)
    {
        for (temp_y = 32; temp_y > 0; temp_y -= 2)
        {
            oled.choose_clr(0, 0, 68, 4);
            show_temp(0, temp_y, 0, 18);
            oled.choose_refresh(0, 0, 68, 4);
            yield();
        }
    }
}


/// @brief 第一页面显示两个数字，一个是ADC温度用的图库数字显示，
/// 一个是小的数字显示设置温度或者恒温时间
/// @param x 大数字图标 左上角的位置x 0-128
/// @param y 大数字图标 左上角的位置y 0-32
/// @param xx 小图标左上角的0-128范围位置xx
/// @param yy 小图标左上角的0-32范围位置yy 
void UI::show_temp(int8_t x, int8_t y, int8_t xx, int8_t yy)
{
    uint8_t dat_buf[3];
    uint16_t tmp;
    if (show_temp_mode == show_now_temp)
    {
        tmp = adc.now_temp;
        if (adc.now_temp <= 38)  // 如果温度小于38显示小于号
            oled.BMP(x + 2, y, less);
    }
    else if (show_temp_mode == show_set_temp)
    {
        tmp = pwm.temp_buf;
    }
    else if (show_temp_mode == show_set_light)
    {
        tmp = oled_light;
    }
    else if (show_temp_mode == show_temp_mode1_time)
    {
        tmp = pwm.constant_temp_time;
    }
    else
        return;

    oled.xy_set(68, 0, 128, 4);
    if (pwm.temp_mode)
    {
        if (pwm.temp_reached_flg)  // 达到设置温度显示恒温时间
            oled.num(xx, yy, temp_time_buf, 3, 16, RIGHT, 1);
        else  // 没到指定温度，就显示设置温度
            oled.num(xx, yy, pwm.temp_buf, 3, 16, RIGHT, 1);
    }
    else  // 回流模式
        oled.num(xx, yy, pwm.percent, 3, 16, RIGHT, 1);

    // 这便是用图片形式显示左边大的数字温度
    dat_buf[0] = tmp / 100 % 10;  // 百位
    dat_buf[1] = tmp / 10 % 10;   // 十位
    dat_buf[2] = tmp % 10;        // 个位
    if (show_temp_mode < 3)
        oled.xy_set(0, 0, 68, 4);
    else
        oled.xy_set(0, 0, 128, 4);
    if (dat_buf[0])  // 百位为零不显示
        oled.BMP(x, y, 20, 32, number[dat_buf[0]], 1);
    if (dat_buf[0] || dat_buf[1])  //百位和十位有一个不为零都显示
        oled.BMP(x + 24, y, 20, 32, number[dat_buf[1]], 1);
    oled.BMP(x + 48, y, 20, 32, number[dat_buf[2]], 1);  // 显示个位
}


/// @brief 绘制回流焊曲线
/// @param y 
/// @param data_y 
void UI::show_curve(int8_t y, int8_t data_y)
{
    int8_t y_tmp;
    int8_t i;
    int8_t y_buf;
    int8_t x_tmp;
    int8_t x_buf;
    int8_t tmp;
    int8_t last_y;

    y_tmp = -((curve_temp_buf[0] - 40) * 100000 / 671875) + 32;
    y_buf = y_tmp;

    oled.point(11, y_buf + 1 + y, 1);

    for (i = 10; i > 0; i--) //升温曲线1
    {
        oled.point(i, ++y_buf + y, 1);
    }
    x_tmp = curve_temp_buf[1] / 5;
    x_buf = curve_temp_buf[3] / 5;

    for (i = 0; i < x_tmp; i++) //保温曲线
    {
        oled.point(12 + i, y_tmp + y, 1);
    }
    x_tmp += 12;

    y_tmp--;

    oled.point(x_tmp++, y_tmp + y, 1);

    y_buf = -((curve_temp_buf[2] - 40) * 100000 / 671875) + 32;

    for (i = 10; i > 0; i--) //升温曲线2
    {
        oled.point(x_tmp++, y_tmp + y, 1);
        y_tmp--;
        if (y_tmp == y_buf)
            break;
        if (y_tmp < 6)
        {
            x_buf -= 2;
        }
    }

    oled.point(x_tmp++, y_tmp + 1 + y, 1);
    x_buf -= 2;

    for (i = 0; i < x_buf; i++)
    {
        oled.point(x_tmp++, y_tmp + y, 1);
    }
    y_tmp++;
    oled.point(x_tmp++, y_tmp + y, 1);

    for (i = 0; i < 10; i++)
    {
        oled.point(x_tmp++, y_tmp + y, 1);
        y_tmp++;
        if (x_tmp == 72)
            break;
    }

    if (temp_mode0_option < 2)
        oled.chinese(72, data_y, menu1_option0, 16, 1, 0);
    else
        oled.chinese(72, data_y, menu1_option1, 16, 1, 0);

    oled.num(100, data_y + 16, curve_temp_buf[temp_mode0_option], 3, 16, RIGHT, 1);

    if (temp_mode0_option == 0 || temp_mode0_option == 2)
        oled.chinese(100, data_y + 16, menu1_option_t, 16, 1, 0);
    else
        oled.chinese(100, data_y + 16, menu1_option_s, 16, 1, 0);
}


/// @brief 恒温温度或者恒温时间来回切换
void UI::temp_time_switch()
{
    if (pwm.temp_reached_flg == false)  // 如果温度还没达到
        min_count = 0;  // 恒温计时清零

    Serial.println(pwm.temp_reached_flg);
    
    //temp_reached_flg唯一修改的地方就在这里，也就是由temp_time_switch_flg控制
    pwm.temp_reached_flg = !pwm.temp_reached_flg;
    oled.roll(69, 2, 24, 2, 1, UP, 16);  // 小数字 上移2*16=32格

    for (int8_t i = 32; i > 18; i--)  // 从下往上移动温度或者时间
    {
        oled.choose_clr(69, 2, 24, 2);  // 先清空指定区域
        if (pwm.temp_reached_flg)  // 如果温度到了 切换为恒温时间
            oled.num(93, i, temp_time_buf, 3, 16, RIGHT, 1);
        else // 温度没到 切换为恒温温度
            oled.num(93, i, pwm.temp_buf, 3, 16, RIGHT, 1);
        oled.choose_refresh(69, 2, 24, 2);  // 刷新指定区域
        delay(10);
    }
}
