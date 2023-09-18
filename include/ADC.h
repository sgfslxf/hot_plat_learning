#ifndef _ADC_H_
#define _ADC_H_

#define switch_io 16
#define channel_low_temp 0
#define channel_high_temp 1

#include <Arduino.h>
#include "pwm.h"

void adc_max_temp_auto_feed();



#define temp_error_value 0
#define temp_max_value 250
#define tmep_error_mode 1 //是否开启温度误差校准程序，给1将显示实际温度，校准是设置为1，校准完后改回0

/*  误差手动校准步骤：
    1）将上面的参数tmep_error_mode设置为1，编译固件烧录后
    2）将加热台在保温条件好的比较暖和的室温下开启恒温模式加热（恒温时长设置20分钟以上），
        设置多少温度不用管，校准模式下会全速加热
    3）十分钟后看显示的温度是多少，将此温度值填写到temp_error_value后面替换数值
    4）如果自己有比较精确的测温设备可以测试加热10分钟后的当前最大温度值，
        可以将temp_max_value后面的值替换为最大温度值更好，没设备的就默认250就好
    5）将tmep_error_mode改回0，重新编译固件烧录即可



*/

class ADC
{

public:
    ADC();

    void get_temp_task();
    void get();
    void set_channel(bool channel);
    bool adc_max_temp_auto();
    uint16_t now_temp = 0;
    uint16_t now_temp_high = 0;
    bool adc_get_temp_flg = 0;
    uint16_t hotbed_max_temp = 0;
    uint16_t adc_max_temp = 0;
    bool adc_max_temp_auto_flg = 1;

private:
    void get_voltage();
    uint16_t adc_buf[8];
    uint16_t adc_buf_high[8];
    uint16_t vol_low = 0;
    uint16_t vol_high = 0;
    int8_t adc_error = 0;

    bool adc_mode_state = channel_low_temp;
};

extern ADC adc;

#endif