#ifndef _PWM_H_
#define _PWM_H_

#include <Arduino.h>
#include <Ticker.h>
#include "ADC.h"
#include "Tick_IRQ.h"

#define PWM_IO 14
#define FAN_IO 15


#define kp 4.5
#define ki 1.0
#define ki_high 3.0 
#define kd 30.0


#define OFF 0
#define ON 1

#define Reflow_Soldering 0
#define Constant_Temp 1





class PWM{
public:
    PWM();
    void begin();
    void end();
    //caculate pwm
    void temp_set();
    // control fan
    void fan();
    
    friend void pwm_irq();

    bool power = OFF;
    bool sleep_flg = OFF;
    int16_t temp_buf = 0;//设置温度
    uint8_t temp_mode = 0; //0 回流 1：恒温
    int16_t constant_temp_time = 0;//恒温时间
    uint8_t percent = 0;

    //temp_reached_flg唯一修改的地方就在这里，也就是由temp_time_switch_flg控制
    bool temp_reached_flg = false;
    uint8_t reflow_working_state = 0;
    // int16_t backflow_temp_buf = 0;
    int16_t reflow_temp_tmp = 0;

private:
    bool fan_state = OFF;
    int16_t need_set_temp = 50;
    uint8_t high_time = 0;  // High Level Duration
    int16_t pwm_buf = 0;
    float pwm_buf_f = 0;
};

extern PWM pwm;
#endif