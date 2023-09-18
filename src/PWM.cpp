#include "PWM.h"

Ticker pwm_tic;
PWM pwm;
uint8_t count = 0;

int16_t ek[3] = {0, 0, 0};

PWM::PWM()
{
    // temperature pwn control
    pinMode(PWM_IO, OUTPUT);
    digitalWrite(PWM_IO, LOW);
    // fan control
    pinMode(FAN_IO, OUTPUT);
    digitalWrite(FAN_IO, LOW);
}


// 根据high_time高电平占空比，4ms定时器pwm控制温度
void pwm_irq()
{
    count++;
    // Serial.printf("count =%d\n", count);
    // Serial.printf("!count && pwm.high_time: %d\n", !count && pwm.high_time);
    // when count is 0, set pwn_io to high
    if (!count && pwm.high_time)
        digitalWrite(PWM_IO, HIGH);
    // when count large than high_time, set pwn_io to low
    if (count > pwm.high_time)
        digitalWrite(PWM_IO, LOW);
}

// pwm开始函数
void PWM::begin()
{
    if (temp_mode == Constant_Temp)  // 恒温焊
        temp_time_buf = constant_temp_time;
    else  // 回流焊
    {
        if (adc.now_temp > curve_temp_buf[0])
            return;
        reflow_working_state = 1;
        reflow_temp_tmp = adc.now_temp;
        percent = 0;
    }
    pinMode(PWM_IO, OUTPUT);  // 输出模式
    digitalWrite(PWM_IO, LOW);  // 低电平（不加热）
    pwm_tic.attach_ms(4, pwm_irq);  // 绑定计时器
    power = 1;  // 开启加热

    ui.heat_icon_flg = 1;  // 切换加热图标
    oled_sleep_t = 0;  // 关闭屏幕计时清零
}

//pwm停止函数
void PWM::end()
{
    pwm_tic.detach();  // 解绑计时器
    high_time = 0;  // 高电平占空比清零
    digitalWrite(PWM_IO, LOW);  // 低电平不加热
    power = 0;  // 关闭加热

    reflow_working_state = 0;  // 关闭回流焊状态
    if (temp_mode == Constant_Temp && temp_reached_flg)  // 应该是恒温时间到了切换温度时间标志
        ui.temp_time_switch_flg = true;  // 温度时间标志切换
    ui.heat_icon_flg = 1;  // 切换加热图标
    oled_sleep_t = 0;  // 关闭屏幕计时清零
}

void PWM::temp_set()
{
    // 加热开关没开 不设置高电平占空比
    if (!pwm.power)
    {
        high_time = 0;
        return;
    }

    // 如果是恒温模式
    if (temp_mode == Constant_Temp)
    {
        // 写入设置温度
        need_set_temp = temp_buf;
        // 如果温度到了 更改温度时间切换标志位
        if (adc.now_temp >= temp_buf && temp_reached_flg == false)
        {
            if (ui.temp_time_switch_flg == false)
                ui.temp_time_switch_flg = true;  //开始恒温倒计时，并翻转temp_reached_flg
        }
        if (temp_reached_flg == true && temp_time_buf == 0)
        {
            end();
            return;
        }
    }
    // 如果是回流模式
    else
        need_set_temp = reflow_temp_tmp;

    ////////////////////////////////////////////

    ek[2] = ek[1];
    ek[1] = ek[0];
    ek[0] = need_set_temp - adc.now_temp;

    if (adc.now_temp == 38)
    {
        pwm_buf = need_set_temp;
    }
    else
    {
        pwm_buf_f += ek[0];

        if (adc.now_temp < 180)
        {
            if (pwm_buf_f > 20)
                pwm_buf_f = 20;
            else if (pwm_buf_f < -20)
                pwm_buf_f = -20;
        }
        else
        {
            if (pwm_buf_f > 50)
                pwm_buf_f = 50;
            else if (pwm_buf_f < -50)
                pwm_buf_f = -50;
        }

        // caculate pwm
        if (adc.now_temp < 200)
            pwm_buf = kp * (ek[0] + ki * pwm_buf_f + kd * (ek[0] - ek[1]));
        else
            pwm_buf = kp * (ek[0] + ki_high * pwm_buf_f + kd * (ek[0] - ek[1]));

        // scale pwm
        if (ek[0] < 10 && ek[0] > 3)
        {
            if (adc.now_temp < 200)
                pwm_buf /= 3;
        }
        else if (ek[0] < 4)
        {
            if (adc.now_temp < 200)
            {
                pwm_buf /= 8;
            }
            else
                pwm_buf /= 3;

            if (ek[0] > -1)
                pwm_buf += need_set_temp / 10;
        }
        if (ek[0] < 0 && pwm_buf > 0)
        {
            pwm_buf = 0;
        }
    }

    // Avoid going out of scope
    if (pwm_buf < 0)
    {
        pwm_buf = 0;
    }
    if (pwm_buf > 255)
    {
        pwm_buf = 255;
    }

    high_time = pwm_buf;
    // high_time = 255;
    // Serial.println(high_time);
}

void PWM::fan()
{
    fan_state = !fan_state;
    digitalWrite(FAN_IO, fan_state);
}
