#include "Tick_IRQ.h"

Ticker ticker25ms;
Ticker ticker100_ms;
Ticker ticker50_ms;
Ticker ticker1s;

uint8_t oled_flg = 0;

int16_t temp_time_buf = 0;
int16_t min_count = 0;
uint8_t oled_sleep_t = 0;  // 屏幕关闭计时


// 温度pwm控制
void s1_tic()
{
	adc.get_temp_task();  // 更新now_temp
	pwm.temp_set();
	int8_t tmp;

	if (adc.now_temp == 38 && !pwm.power)  // 如果温度到最低了且没有加热
	{
		oled_sleep_t++;  // 屏幕进入睡眠状态计时开始
		if (oled_sleep_t == oled_display_sleep_time && !ui.oled_sleep_flg)
		{  //时间到达，并且睡眠表示为0
			ui.wake_sleep_change_flg = 1;  // 屏幕苏醒睡眠转换标志
		}
	}
	else
	{
		oled_sleep_t = 0;
	}

	min_count++;
	if (pwm.temp_reached_flg == true)
	{
		if (min_count == 60)
		{
			min_count = 0;
			temp_time_buf--;
		}
	}
	if (pwm.reflow_working_state == 1)
	{
		tmp = (curve_temp_buf[0] - 39) * 10 / 25;
		if (tmp == 0)
			tmp = 1;
		pwm.percent = (adc.now_temp - 38) * 10 / tmp;
		if (pwm.reflow_temp_tmp < (curve_temp_buf[0] - 1))
		{
			pwm.reflow_temp_tmp += 3;
			if (pwm.reflow_temp_tmp > curve_temp_buf[0])
				pwm.reflow_temp_tmp = curve_temp_buf[0];
		}
		else
		{

			if (adc.now_temp >= pwm.reflow_temp_tmp)
			{
				pwm.reflow_working_state = 2;
				min_count = 0;
			}
		}
	}
	else if (pwm.reflow_working_state == 2)
	{
		tmp = curve_temp_buf[1] * 10 / 25;
		if (tmp == 0)
			tmp = 1;
		pwm.percent = 25 + min_count * 10 / tmp;
		if (min_count == curve_temp_buf[1])
		{
			pwm.reflow_working_state = 3;
			pwm.reflow_temp_tmp = curve_temp_buf[2];
		}
	}
	else if (pwm.reflow_working_state == 3)
	{
		tmp = (217 - curve_temp_buf[0]) * 10 / 25;
		if (tmp == 0)
			tmp = 1;
		pwm.percent = 50 + (adc.now_temp - curve_temp_buf[0]) * 10 / tmp;
		if (adc.now_temp > 215)
		{
			pwm.reflow_working_state = 4;
			min_count = 0;
		}
	}
	else if (pwm.reflow_working_state == 4)
	{
		tmp = curve_temp_buf[3] * 10 / 25;
		if (tmp == 0)
			tmp = 1;
		pwm.percent = 75 + min_count * 10 / tmp;
		if (min_count == (curve_temp_buf[3] - 10))
		{
			pwm.reflow_temp_tmp = 0;
		}
		else if (min_count == curve_temp_buf[3])
		{
			pwm.reflow_working_state = 0;
			pwm.percent = 100;
			pwm.end();
		}
	}
}


/// @brief 25ms 更新一次
void ms25_tic()
{
	oled_flg = 1;
}

// 50毫秒采集一次温度
void ms50_tic()
{
	adc.get();
}

void ms100_tic()
{
	if (eeprom.write_t < 21)
	{
		eeprom.write_t++;
		if (eeprom.write_t == 20)
		{
			if (eeprom.write_flg)
				eeprom.write_flg = 2;

			if (ui.show_temp_mode == show_set_temp)
			{
				ui.show_temp_mode = show_now_temp;
				ui.temp_move_flg = 1;
			}
		}
	}
}

void Ticker_init()
{
	ticker25ms.attach_ms(25, ms25_tic);  // screen
	ticker100_ms.attach_ms(100, ms100_tic);  // 
	ticker50_ms.attach_ms(50, ms50_tic);

	ticker1s.attach(1, s1_tic);
}
