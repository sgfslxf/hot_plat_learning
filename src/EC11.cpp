#include "EC11.h"

Skey ec11;
static Ticker key_tic;

static int8_t io_sw = -1;
static int8_t io_sa = -1;
static int8_t io_sb = -1;

static uint16_t click_time = 10000;
static uint16_t lease_time = 10000;
static uint8_t key_scan_buf = 0;

static uint8_t speed_up_feel_num = 0;
static uint16_t speed_up_count = 0;

void key_timer();
IRAM_ATTR void io_sa_sb_int();
IRAM_ATTR void io_sw_int();

// 开始工作
void Skey::begin(uint8_t sw, uint8_t sa, uint8_t sb ,void (*func)(ec11_task_result_type task_type, int16_t rusult_value))
{
    io_sw = sw;
    io_sa = sa;
    io_sb = sb;
    if (io_sw != -1)
        pinMode(io_sw, INPUT_PULLUP);
    if (io_sa != -1)
        pinMode(io_sa, INPUT_PULLUP);
    if (io_sb != -1)
        pinMode(io_sb, INPUT_PULLUP);

    attch_p = func;
    work_flg = 1;
    click_time = 10000;
    lease_time = 10000;
    key_scan_buf = 0;

    int_work();
    key_tic.attach_ms(1, key_timer);
}

#if 0
// 不用了
// void Skey::key_mode_read()
// {
//     if (key_scan_buf < 20)
//     {
//         if (speed_up_flg)
//             speed_up_feel_num = speed_up_feel_count;
//         ec11.ec_buf = -1 - speed_up_count;
//         _take_type_ec_sw = ec11_task_is_encode;
//         task();
//     }
//     else
//     {
//         if (speed_up_flg)
//             speed_up_feel_num = speed_up_feel_count;
//         ec11.ec_buf = 1 + speed_up_count;
//         _take_type_ec_sw = ec11_task_is_encode;
//         task();
//     }
//     key_scan_buf = 0;
// }
#endif

// 左转和右转读取
void Skey::key_mode_read(bool state)
{
    if (state == 0)
    {
        // 如果启用加速，每次读取状态时初始化speed_up_feel_num，
        // 并使用上一次的speed_up_count进行加速
        if (speed_up_flg)
            speed_up_feel_num = speed_up_feel_count;
        ec11.ec_buf = -1 - speed_up_count;
        _take_type_ec_sw = ec11_task_is_encode;
        task();
    }
    
    if(state == 1)
    {
        // 如果启用加速，每次读取状态时初始化speed_up_feel_num，
        // 并使用上一次的speed_up_count进行加速
        if (speed_up_flg)
            speed_up_feel_num = speed_up_feel_count;
        ec11.ec_buf = 1 + speed_up_count;
        _take_type_ec_sw = ec11_task_is_encode;
        task();
    }
}

// 解除绑定
void Skey::detach()
{
    key_tic.detach();
    work_flg = 0;
    int_close();
    attch_p = NULL;
}

// 和屏幕关联起来
void Skey::task()
{
    if (attch_p == NULL)
        return;
    if (_take_type_ec_sw == ec11_task_is_key)
    {        
        attch_p(_take_type_ec_sw, sw_buf);
        Serial.printf("click state: %d\n", sw_buf);
    }    
    else
        attch_p(_take_type_ec_sw, ec_buf);

    sw_buf = sw_clr;
    ec_buf = 0;

}

// ecll引脚中断开启
void Skey::int_work()
{
    if (work_flg)
    {
        attachInterrupt(io_sw, io_sw_int, CHANGE);
        attachInterrupt(io_sa, io_sa_sb_int, CHANGE);
        attachInterrupt(io_sb, io_sa_sb_int, CHANGE);
    }
}

// 关闭引脚中断
void Skey::int_close()
{
    detachInterrupt(io_sw);
    detachInterrupt(io_sa);
    detachInterrupt(io_sb);
}

// 设置加速flag
void Skey::speed_up(bool val)
{
    speed_up_flg = val;
}

// 设置最大加速值
void Skey::speed_up_max(uint16_t val)
{
    speed_up_max_num = val;
}

// 设置加速间隔
void Skey::speed_up_feel(uint8_t val)
{
    speed_up_feel_count = val;
}

// 用于开启和关闭双击
void Skey::double_click(bool val)
{
    if (val)
        double_click_count = key_double_out;
    else
        double_click_count = 1;
}

// 得到私有成员变量key_result
ec11_key_result_type Skey::get_key_result()
{
    return sw_buf;
}

// 得到私有成员变量encode_value
int16_t Skey::get_encode_value()
{
    return ec_buf;
}

// 用一系列计时用来判断按键类型
void key_timer()
{
    // 用于消抖和记录到state, 进行了优化
    if (ec11.state_count < 5)
    {
        ec11.state_count++;
        if (ec11.state_count == shake_t)
        {
            uint8_t read_io_sw = digitalRead(io_sw);
            if(ec11.sw_state_buf != read_io_sw)
                ec11.sw_state_buf = read_io_sw;
            ec11.state = ec11.sw_state_buf;
            Serial.printf("io_sw: %d\n", ec11.sw_state_buf);
        }
    }

    // 记录第一次按下时间
    if (!ec11.state && click_time < 10000)
    {
        click_time++;
        // 如果按下时间等于400ms则为长按，否则继续判断
        if (click_time == key_time_out)
        {
            ec11.sw_buf = sw_long;
            ec11._take_type_ec_sw = ec11_task_is_key;
            ec11.task();
        }
    }

    // 记录第一次按下后，第一次松开时间
    if (lease_time < 10000)
    {
        lease_time++;
        // 第一次按下时间小于400ms，且第一次松开时间等于400ms为单机
        if (lease_time == ec11.double_click_count)
        {
            lease_time = 10000;
            ec11.sw_buf = sw_click;
            ec11._take_type_ec_sw = ec11_task_is_key;
            ec11.task();
        }
    }

    // 判断是否松开
    if (ec11.state)
    {
        // 每次按下且不为长按才会进来，进来之后都会清零
        if (click_time > 5 && click_time < key_time_out)
        {
            // 如果是第一次按下，且松开时间未更新，就开始计时松开时间
            if (ec11.sw_buf == sw_clr && lease_time > key_double_out)
            {
                lease_time = 0; 
            }
            //第二次短按进入，且第一次松开时间小于400ms，判断为长按
            else if (lease_time < key_double_out)
            {
                lease_time = 10000;
                ec11.sw_buf = sw_double;
                ec11._take_type_ec_sw = ec11_task_is_key;
                ec11.task();
            }
        }
        click_time = 0;
    }

    if (speed_up_feel_num)
    {
        speed_up_feel_num--;
        if (speed_up_count < (ec11.speed_up_max_num - 1))
            speed_up_count++;
    }
    else
    {
        speed_up_count = 0;
    }
}

// 这样就可以将函数移动到IRAM中，从而提高中断响应速度。并结合key_timer函数进行了消抖处理
IRAM_ATTR void io_sw_int()
{
    ec11.sw_state_buf = digitalRead(io_sw);
    ec11.state_count = 0;
}

// 按我的逻辑的改版
IRAM_ATTR void io_sa_sb_int()
{   
    // 左右转状态检测 左转tmp是1 0 2 3 右转tmp是 2 0 1 3
    static uint8_t state1 = 0, state2 = 1, state3 = 0;
    uint8_t tmp = digitalRead(io_sb) << 1;
    tmp |= digitalRead(io_sa);

    // Serial.printf("tmp: %x\n", tmp);
    
    if((tmp == 1 || tmp == 2) && state1 == 0 && state2 == 1 && state3 == 0)
    {
        state1 = tmp;
    }
    if(tmp == 0 && state1 != 0 && state2 == 1 && state3 == 0)
    {
        state2 = 0;
    }
    if((tmp == 1 || tmp == 2) && state1 != 0 && state2 == 0 && state3 == 0)
    {
        state3 = tmp;
    }
    if(state1 == 1 && state2 == 0 && state3 == 2)
    {
        Serial.printf("left\n");
        ec11.key_mode_read(0);
        state1 = state3 = 0;
        state2 = 1;
    }
    if(state1 == 2 && state2 == 0 && state3 == 1)
    {
        Serial.printf("right\n");
        ec11.key_mode_read(1);
        state1 = state3 = 0;
        state2 = 1;
    }

    // 当出现错误顺序时，肯定会有tmp = 3，此时清零重新开始
    if(tmp == 3)
    {
        state1 = state3 = 0;
        state2 = 1;
    }
    // Serial.printf("state1: %d, state2: %d, state3: %d\n", state1, state2, state3);
}


