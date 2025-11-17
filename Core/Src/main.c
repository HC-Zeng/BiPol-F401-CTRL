/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "crc.h"
#include "dma.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "lcd.h"
#include "flash_store.h"
#include "debug_printf.h"
#include "ui.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#ifdef __GNUC__
PUTCHAR_PROTOTYPE
{
    HAL_UART_Transmit(&huart1, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
#endif

volatile uint32_t timer_overflow_count = 0;  // 溢出次数�???????????????32位扩展）

uint64_t Get_Global_Time_us(void) {
    uint32_t overflow, counter;
    do {
        overflow = timer_overflow_count;
        counter = __HAL_TIM_GET_COUNTER(&htim2);
    } while (overflow != timer_overflow_count); // 无锁校验
    return ((uint64_t)overflow<<32) | counter; // 组合�???????????????64位时间戳
}

void didi()
{
    HAL_TIM_Base_Start_IT(&htim3);
}

uint8_t bezCnt = 0;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2) {
        timer_overflow_count++;
    }
    if (htim->Instance == TIM3) {
        HAL_GPIO_TogglePin(BUZZER_GPIO_Port,BUZZER_Pin);
        bezCnt += 1;
        if(bezCnt>35)
        {
            bezCnt=0;
            HAL_TIM_Base_Stop_IT(&htim3);
        }
    }
}


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* 系统状�?�枚�??????????? */
typedef enum {
    STATE_INIT = 0,
    STATE_CHARGE,
    STATE_BALANCE,
    STATE_MAINTAIN,
    STATE_DISCHARGE
} SystemState;

/* 全局变量 */
SystemState g_state = STATE_INIT;

uint32_t g_adc_buffer[5];            // ADC采样缓冲�???????????
uint16_t g_target_voltage_mv = 0;    // 目标电压 (mV)
uint16_t g_cap_voltage_mv[4] = {0};  // 电容电压 (mV)
uint16_t g_temp = 0;                 // 温度电压 (mv)


uint8_t g_pulse_generating = 0;

uint8_t pre_exit = 0;
uint8_t value_updated = 0;
uint8_t g_type = 0; // 0:AMT,1: AUTO
float g_U = 9.2f;     // 0.2-12.0 v
float g_psqr = 0.1f;  // 0.1-5.0 s
float g_ch1 = 9.6f;   // 0.1-25.0 ms
float g_cool = 1.0f;  // 1.0-9.9 ms
float g_ch2 = 9.6f;   // 0.1-25.0 ms
uint32_t g_count = 0; // 0->>>>>
uint32_t g_cnt =  2000000000;
uint8_t g_btn = 0;

// Pulse Generation Related
uint8_t trig_type=0; //0: no trig; 1: AMT trig; 2: AUTO trig

void SystemStateMachine(void);
void UpdateVoltageMeasurements(void);

uint8_t env_detect()
{
    static uint64_t pretime = 0;
    uint64_t crrtime = Get_Global_Time_us();
    if(crrtime > pretime + 1000000)
    {
//        HAL_GPIO_TogglePin(LED2_GPIO_Port,LED2_Pin);
        pretime = crrtime;
        if(g_temp<1500)
        {
            printf("FAN ON\r\n");
            HAL_GPIO_WritePin(CTR_FAN_GPIO_Port,CTR_FAN_Pin,1);
        }
        if(g_temp>1700)
        {
            printf("FAN OFF\r\n");
            HAL_GPIO_WritePin(CTR_FAN_GPIO_Port,CTR_FAN_Pin,0);
        }
        return 1;
    }
    return 0;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    if(hadc->Instance == ADC1) {
        HAL_GPIO_TogglePin(LED2_GPIO_Port,LED2_Pin);
        UpdateVoltageMeasurements();
    }
}

#define ALPHA 0.1f
static uint16_t filtered_voltage[4] = {0};

/* 更新电压测量并处理串口输�?????????? */
void UpdateVoltageMeasurements(void) {

    // 1. 更新目标电压
    g_target_voltage_mv = (u_int16_t)(g_U/2.0f*1000.0f);

    // 2. 更新温度数据(PA3)
    g_temp = (uint16_t)(g_adc_buffer[0] * 3300.0f / 4096.0f); // <1500,fan on; >1700, fan off

    // 3. 更新电容电压 (PA4-PA7)
    for(int i = 0; i < 4; i++) {
        float adc_voltage = g_adc_buffer[i+1] * 3.3f / 4096.0f;
        uint16_t raw_voltage = (uint16_t)(adc_voltage * (16.0f / 10.0f) * 1000);

        // �??????????阶低通滤�??????????
        if(filtered_voltage[i] == 0) {
            // 第一次计算，直接赋�??
            filtered_voltage[i] = raw_voltage;
        } else {
            // 滤波计算: new = α * current + (1-α) * old
            filtered_voltage[i] = (uint16_t)(ALPHA * raw_voltage + (1 - ALPHA) * filtered_voltage[i]);
        }
        g_cap_voltage_mv[i] = filtered_voltage[i];
    }
    //           输出格式: 时间(ms),目标电压,电容1,电容2,电容3,电容4,状�??
//    printf("Set=%u,C1=%u,C2=%u,C3=%u,C4=%u,St=%d\r\n",
//           g_target_voltage_mv,
//           g_cap_voltage_mv[0],
//           g_cap_voltage_mv[1],
//           g_cap_voltage_mv[2],
//           g_cap_voltage_mv[3],
//           g_state);
}


/* 控制参数 */
#define BALANCE_THRESHOLD    20    // 均衡阈�?�为20mv

/* 状太机核心�?�辑 */
void SystemStateMachine(void) {
    uint8_t i;
    uint16_t max_voltage = 0;
    uint16_t min_voltage = UINT16_MAX;
    uint16_t avg_voltage = 0;
    uint8_t max_id = 0;

    // 计算统计�????????
    for(i = 0; i < 4; i++) {
        if(g_cap_voltage_mv[i] > max_voltage)
        {
            max_voltage = g_cap_voltage_mv[i];
            max_id = i;
        }
        if(g_cap_voltage_mv[i] < min_voltage)
        {
            min_voltage = g_cap_voltage_mv[i];
        }
        avg_voltage += g_cap_voltage_mv[i];
    }
    avg_voltage /= 4;

    // 状�?�转换�?�辑
    switch(g_state) {
        case STATE_INIT:
            g_state = STATE_CHARGE;
            break;

        case STATE_CHARGE: {
            // 打开充电�????????�????????
            HAL_GPIO_WritePin(CTR_SW_GPIO_Port, CTR_SW_Pin, GPIO_PIN_SET);
            // 禁止均衡
            HAL_GPIO_WritePin(CTR_C1_GPIO_Port, CTR_C1_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(CTR_C2_GPIO_Port, CTR_C2_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(CTR_C3_GPIO_Port, CTR_C3_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(CTR_C4_GPIO_Port, CTR_C4_Pin, GPIO_PIN_RESET);

            // 平均电压大于设定值的80%，开始均�????????
            if(avg_voltage > g_target_voltage_mv*0.3) {
                g_state = STATE_BALANCE;
            }
            break;
        }

        case STATE_BALANCE: {
            // 打开充电�????????�????????
            HAL_GPIO_WritePin(CTR_SW_GPIO_Port, CTR_SW_Pin, GPIO_PIN_SET);

            // 均衡控制策略
            uint8_t discharge_flags[4] = {0};

            // 四路电压的最大�?�减去最小�?�大于平衡阈值，�????????始均衡，把最大电压的电容放电
            if(max_voltage-min_voltage > BALANCE_THRESHOLD)
            {
                discharge_flags[max_id] = 1;
            }

            // 设置放电管状�????????
            HAL_GPIO_WritePin(CTR_C1_GPIO_Port, CTR_C1_Pin, discharge_flags[0]);
            HAL_GPIO_WritePin(CTR_C2_GPIO_Port, CTR_C2_Pin, discharge_flags[1]);
            HAL_GPIO_WritePin(CTR_C3_GPIO_Port, CTR_C3_Pin, discharge_flags[2]);
            HAL_GPIO_WritePin(CTR_C4_GPIO_Port, CTR_C4_Pin, discharge_flags[3]);

            // 四路�????????大电压小于设定�?�的80%，转到充�????????
            if(max_voltage<g_target_voltage_mv*0.8)
            {
                g_state = STATE_CHARGE;
            }

            // 平均电压大于设定值，即进入保�????????
            if(avg_voltage>g_target_voltage_mv) {
                g_state = STATE_MAINTAIN;
            }
            break;
        }

        case STATE_MAINTAIN: {
            // 关闭充电
            HAL_GPIO_WritePin(CTR_SW_GPIO_Port, CTR_SW_Pin, GPIO_PIN_RESET);

            // 均衡控制策略
            uint8_t discharge_flags[4] = {0};

            // 四路电压的最大�?�减去最小�?�大于平衡阈值，�????????始均衡，把最大电压的电容放电
            if(max_voltage-min_voltage > BALANCE_THRESHOLD)
            {
                discharge_flags[max_id] = 1;
            }

            // 设置放电管状�????????
            HAL_GPIO_WritePin(CTR_C1_GPIO_Port, CTR_C1_Pin, discharge_flags[0] );
            HAL_GPIO_WritePin(CTR_C2_GPIO_Port, CTR_C2_Pin, discharge_flags[1] );
            HAL_GPIO_WritePin(CTR_C3_GPIO_Port, CTR_C3_Pin, discharge_flags[2] );
            HAL_GPIO_WritePin(CTR_C4_GPIO_Port, CTR_C4_Pin, discharge_flags[3] );

            // 4路电压最大�?�小于设定，转到均衡
            if(max_voltage<g_target_voltage_mv)
            {
                g_state = STATE_BALANCE;
            }

            // 四路电压�????????小�?�大于设定，转到放电
            if(min_voltage>g_target_voltage_mv)
            {
                g_state = STATE_DISCHARGE;
            }
            break;
        }

        case STATE_DISCHARGE: {
            // 关闭充电
            HAL_GPIO_WritePin(CTR_SW_GPIO_Port, CTR_SW_Pin, GPIO_PIN_RESET);

            // 均衡控制策略
            uint8_t discharge_flags[4] = {0};
            uint8_t all_in_range = 1;

            // 大于设（设定+阈�?�）的全部进行放�????????
            for(i = 0; i < 4; i++) {
                if ((g_cap_voltage_mv[i]>=g_target_voltage_mv) && (g_cap_voltage_mv[i] - g_target_voltage_mv > BALANCE_THRESHOLD)) {
                    discharge_flags[i] = 1;
                    all_in_range = 0;
                }
            }

            // 设置放电管状�????????
            HAL_GPIO_WritePin(CTR_C1_GPIO_Port, CTR_C1_Pin, discharge_flags[0] );
            HAL_GPIO_WritePin(CTR_C2_GPIO_Port, CTR_C2_Pin, discharge_flags[1] );
            HAL_GPIO_WritePin(CTR_C3_GPIO_Port, CTR_C3_Pin, discharge_flags[2] );
            HAL_GPIO_WritePin(CTR_C4_GPIO_Port, CTR_C4_Pin, discharge_flags[3] );

            // 放电完成转到保持
            if(all_in_range)
            {
                g_state = STATE_MAINTAIN;
            }
            break;
        }
    }
}

void update_cap_equalizer()
{
    static uint64_t preTimestamp = 0;
    if(Get_Global_Time_us()>preTimestamp+100000)
    {
        SystemStateMachine();
//        HAL_GPIO_TogglePin(LAMP_GPIO_Port,LAMP_Pin);
        preTimestamp = Get_Global_Time_us();

        // U1
        float U1_tem = (float)((float)(g_cap_voltage_mv[0] + g_cap_voltage_mv[1])/1000.f);

        float UC1 = (float)((float)(g_cap_voltage_mv[0])/1000.f); //for debug
        float UC2 = (float)((float)(g_cap_voltage_mv[1])/1000.f); //for debug
        Show_Float_2(170,210, UC1,16,0); //for debug
        Show_Float_2(218,210, UC2,16,0); //for debug

        Show_Float_2(188,242, U1_tem,16,0);
        Show_Str(222,242,"V",16,0);

        // U2
        float U2_tem = (float)((float)(g_cap_voltage_mv[2] + g_cap_voltage_mv[3])/1000.f);

        float UC3 = (float)((float)(g_cap_voltage_mv[2])/1000.f); //for debug
        float UC4 = (float)((float)(g_cap_voltage_mv[3])/1000.f); //for debug
        Show_Float_2(326,210, UC3,16,0); //for debug
        Show_Float_2(374,210, UC4,16,0); //for debug

        Show_Float_2(344,242, U2_tem,16,0);
        Show_Str(378,242,"V",16,0);
    }
}

//------------------------before is for cap equalizer--------------------

static void saveVal()
{
    data_cfg_t dataCfg;
    dataCfg.u = (uint32_t)(g_U*10);
    dataCfg.psqr = (uint32_t)(g_psqr*10);
    dataCfg.ch1 = (uint32_t)(g_ch1*10);
    dataCfg.cool = (uint32_t)(g_cool*10);
    dataCfg.ch2 = (uint32_t)(g_ch2*10);
    dataCfg.type = (uint32_t)(g_type);
    dataCfg.count = (uint32_t)(g_count);
    dataCfg.flag = 12345;
    FlashStore_Save(dataCfg);

    static uint32_t cnt = 0;
    cnt += 1;
    LCD_ShowNum(364,70,cnt,10,16);
}


void Show_Page_Main(void)
{
    POINT_COLOR=GREEN;
    LCD_DrawRectangle(70,35,122,64);
//    POINT_COLOR=WHITE;
//    Show_Str(84,41,"AMT",16,1);
    POINT_COLOR=GREEN;
    LCD_DrawRectangle(362,35,448,64);
    Show_Str(316,41,"COUNT",16,1);

    POINT_COLOR=RED;
    LCD_DrawLine(39,116,74,116);
    LCD_DrawLine(74,116,74,100);
    LCD_DrawLine(74,100,127,100);
    LCD_DrawLine(127,100,127,116);
    LCD_DrawLine(127,116,155,116);
    LCD_DrawLine(155,116,166,100);
    LCD_DrawLine(166,100,245,100);
    LCD_DrawLine(245,100,256,116);
    LCD_DrawLine(256,116,309,116);
    LCD_DrawLine(309,116,320,132);
    LCD_DrawLine(320,132,405,132);
    LCD_DrawLine(405,132,416,116);
    LCD_DrawLine(416,116,461,116);

    LCD_DrawLine(39,115,74,115);
    LCD_DrawLine(73,116,73,100);
    LCD_DrawLine(74,99,127,99);
    LCD_DrawLine(126,100,126,116);
    LCD_DrawLine(127,115,155,115);
    LCD_DrawLine(154,116,165,100);
    LCD_DrawLine(166,99,245,99);
    LCD_DrawLine(244,100,255,116);
    LCD_DrawLine(256,115,309,115);
    LCD_DrawLine(308,116,319,132);
    LCD_DrawLine(320,131,405,131);
    LCD_DrawLine(404,132,415,116);
    LCD_DrawLine(416,115,461,115);

    POINT_COLOR=WHITE;
    Show_Str(80,150,"PSQR",16,1);
    POINT_COLOR=GREEN;
    LCD_DrawRectangle(70,167,122,203);
    POINT_COLOR=WHITE;
    Show_Str(195,150,"CH1",16,1);
    POINT_COLOR=GREEN;
    LCD_DrawRectangle(170,167,241,203);
    POINT_COLOR=WHITE;
    Show_Str(272,150,"COOL",16,1);
    POINT_COLOR=GREEN;
    LCD_DrawRectangle(258,167,315,203);
    POINT_COLOR=WHITE;
    Show_Str(355,150,"CH2",16,1);
    POINT_COLOR=GREEN;
    LCD_DrawRectangle(332,167,404,203);
    POINT_COLOR=WHITE;
    Show_Str(56,243,"U",16,1);
    POINT_COLOR=GREEN;
    LCD_DrawRectangle(70,232,122,268);
    POINT_COLOR=WHITE;
    Show_Str(156,243,"U1",16,1);
    POINT_COLOR=GREEN;
    LCD_DrawRectangle(176,232,247,268);
    POINT_COLOR=WHITE;
    Show_Str(312,243,"U2",16,1);
    POINT_COLOR=GREEN;
    LCD_DrawRectangle(332,232,404,268);
}

void draw_selected(uint8_t index,uint16_t color, uint8_t selected)
{
    switch (index) {
        case 0: // U
            if(selected){
                POINT_COLOR=WHITE;
                LCD_DrawRectangle(69,231,123,269);
            }
            else{
                POINT_COLOR=BLACK;
                LCD_DrawRectangle(69,231,123,269);
            }
            POINT_COLOR = color;
            LCD_DrawRectangle(70,232,122,268);
            break;
        case 1: // PSQR
            if(selected){
                POINT_COLOR=WHITE;
                LCD_DrawRectangle(69,166,123,204);
            }
            else{
                POINT_COLOR=BLACK;
                LCD_DrawRectangle(69,166,123,204);
            }
            POINT_COLOR = color;
            LCD_DrawRectangle(70,167,122,203);
            break;
        case 2: // CH1
            if(selected){
                POINT_COLOR=WHITE;
                LCD_DrawRectangle(169,166,242,204);
            }
            else{
                POINT_COLOR=BLACK;
                LCD_DrawRectangle(169,166,242,204);
            }
            POINT_COLOR = color;
            LCD_DrawRectangle(170,167,241,203);
            break;
        case 3: // COOL
            if(selected){
                POINT_COLOR=WHITE;
                LCD_DrawRectangle(257,166,316,204);
            }
            else{
                POINT_COLOR=BLACK;
                LCD_DrawRectangle(257,166,316,204);
            }
            POINT_COLOR = color;
            LCD_DrawRectangle(258,167,315,203);
            break;
        case 4: // CH2
            if(selected){
                POINT_COLOR=WHITE;
                LCD_DrawRectangle(331,166,405,204);
            }
            else{
                POINT_COLOR=BLACK;
                LCD_DrawRectangle(331,166,405,204);
            }
            POINT_COLOR = color;
            LCD_DrawRectangle(332,167,404,203);
            break;
        case 5: // STORAGE
            if(selected){
                POINT_COLOR=WHITE;
                LCD_DrawRectangle(69,34,123,65);
            }
            else{
                POINT_COLOR=BLACK;
                LCD_DrawRectangle(69,34,123,65);
            }
            POINT_COLOR = color;
            LCD_DrawRectangle(70,35,122,64);
            break;
        default:
            break;
    }
}

void show_select_index(uint8_t index)
{
    static uint8_t preIndex = 0;
    draw_selected(preIndex,GREEN,0); // un select
    draw_selected(index,WHITE,1);  // selected
    preIndex = index;
}

void draw_edited(uint8_t index,uint16_t color, uint8_t edited)
{
    switch (index) {
        case 0: // U
            if(edited){
                POINT_COLOR=WHITE;
                LCD_DrawRectangle(72,234,120,266);
            }
            else{
                POINT_COLOR=BLACK;
                LCD_DrawRectangle(72,234,120,266);
            }
            POINT_COLOR = color;
            LCD_DrawRectangle(70,232,122,268);
            break;
        case 1: // PSQR
            if(edited){
                POINT_COLOR=WHITE;
                LCD_DrawRectangle(72,169,120,201);
            }
            else{
                POINT_COLOR=BLACK;
                LCD_DrawRectangle(72,169,120,201);
            }
            POINT_COLOR = color;
            LCD_DrawRectangle(70,167,122,203);
            break;
        case 2: // CH1
            if(edited){
                POINT_COLOR=WHITE;
                LCD_DrawRectangle(172,169,239,201);
            }
            else{
                POINT_COLOR=BLACK;
                LCD_DrawRectangle(172,169,239,201);
            }
            POINT_COLOR = color;
            LCD_DrawRectangle(170,167,241,203);
            break;
        case 3: // COOL
            if(edited){
                POINT_COLOR=WHITE;
                LCD_DrawRectangle(260,169,313,201);
            }
            else{
                POINT_COLOR=BLACK;
                LCD_DrawRectangle(260,169,313,201);
            }
            POINT_COLOR = color;
            LCD_DrawRectangle(258,167,315,203);
            break;
        case 4: // CH2
            if(edited){
                POINT_COLOR=WHITE;
                LCD_DrawRectangle(334,169,402,201);
            }
            else{
                POINT_COLOR=BLACK;
                LCD_DrawRectangle(334,169,402,201);
            }
            POINT_COLOR = color;
            LCD_DrawRectangle(332,167,404,203);
            break;
        case 5: // STORAGE
            if(edited){
                POINT_COLOR=WHITE;
                LCD_DrawRectangle(72,37,120,62);
            }
            else{
                POINT_COLOR=BLACK;
                LCD_DrawRectangle(72,37,120,62);
            }
            POINT_COLOR = color;
            LCD_DrawRectangle(70,35,122,64);
            break;
        default:
            break;
    }
}

void show_edit_index(uint8_t index)
{
    static uint8_t preIndex = 0;
    draw_selected(index,GREEN,0); // reset

    draw_edited(preIndex,GREEN,0); // un edit
    draw_edited(index,WHITE,1);  // edit
    preIndex = index;
}

void back_to_idle(uint8_t index)
{
    draw_selected(index,GREEN,0); // reset
    draw_edited(index,GREEN,0); // un edit

    if(value_updated)
    {
        saveVal();
        value_updated = 0;
    }
}

void show_U()
{
    // U
    static uint8_t flag = 0;
    POINT_COLOR=WHITE;
    if(g_U<9.99)
    {
        if(flag==1)
        {
            flag = 0;
            POINT_COLOR=BLACK;
            LCD_Fill(73,235,119,265,POINT_COLOR);
            POINT_COLOR=WHITE;
        }
        Show_Float(78,242, g_U,16,0);
        Show_Str(108,242,"V",16,0);
    }else
    {
        if(flag==0)
        {
            flag = 1;
            POINT_COLOR=BLACK;
            LCD_Fill(73,235,119,265,POINT_COLOR);
            POINT_COLOR=WHITE;
        }
        Show_Float(76,242, g_U,16,0);
        Show_Str(110,242,"V",16,0);
    }
}

void show_PSQR()
{
    POINT_COLOR=WHITE;
    //LCD_DrawRectangle(70,167,122,203);
    Show_Float(78,176,g_psqr,16,0);
    Show_Str(104,176,"S",16,0);
}

void show_CH1()
{
    //LCD_DrawRectangle(170,167,241,203);
    POINT_COLOR=WHITE;
    static uint8_t flag = 0;
    if(g_ch1<9.99)
    {
        if(flag==1)
        {
            flag = 0;
            POINT_COLOR=BLACK;
            LCD_Fill(173,170,238,200,POINT_COLOR);
            POINT_COLOR=WHITE;
        }
        Show_Float(184,176,g_ch1,16,0);
        Show_Str(212,176,"MS",16,0);
    }else
    {
        if(flag==0)
        {
            flag = 1;
            POINT_COLOR=BLACK;
            LCD_Fill(173,170,238,200,POINT_COLOR);
            POINT_COLOR=WHITE;
        }
        Show_Float(180,176,g_ch1,16,0);
        Show_Str(216,176,"MS",16,0);
    }
}

void show_COOL()
{
    POINT_COLOR=WHITE;
    // LCD_DrawRectangle(262,167,311,203);
    Show_Float(266,176,g_cool,16,0);
    Show_Str(292,176,"MS",16,0);
}

void show_CH2()
{
    POINT_COLOR=WHITE;
    //LCD_DrawRectangle(332,167,404,203);
    static uint8_t flag = 0;
    if(g_ch2<9.99)
    {
        if(flag==1)
        {
            flag = 0;
            POINT_COLOR=BLACK;
            LCD_Fill(335,170,401,200,POINT_COLOR);
            POINT_COLOR=WHITE;
        }
        Show_Float(346,176,g_ch2,16,0);
        Show_Str(374,176,"MS",16,0);
    }else
    {
        if(flag==0)
        {
            flag = 1;
            POINT_COLOR=BLACK;
            LCD_Fill(335,170,401,200,POINT_COLOR);
            POINT_COLOR=WHITE;
        }
        Show_Float(342,176,g_ch2,16,0);
        Show_Str(378,176,"MS",16,0);
    }
}

int numLength(uint32_t num) {
    int len = 0;

    // 处理0的情�????????
    if (num == 0) {
        return 1;
    }

    // 处理负数（计算绝对�?�）
    if (num < 0) {
        num = -num;
    }

    // 通过循环除以10来计算位�????????
    while (num != 0) {
        len++;
        num /= 10;
    }

    return len;
}

void show_CNT()
{
    POINT_COLOR=GREEN;
    LCD_ShowNum(406-(4*numLength(g_count)),41,g_count,numLength(g_count),16);// 83ms
}

void show_TYPE()
{
    if(g_type==1)
    {
        POINT_COLOR = BLACK;
        LCD_Fill(73,38,119,61,POINT_COLOR);
        POINT_COLOR = WHITE;
        Show_Str(80,41,"AUTO",16,0);
    }else if(g_type == 0)
    {
        POINT_COLOR = BLACK;
        LCD_Fill(73,38,119,61,POINT_COLOR);
        POINT_COLOR = WHITE;
        Show_Str(84,41,"AMT",16,0);
    }
}

void showValue()
{
    POINT_COLOR=WHITE;
    show_U();
    show_PSQR();
    show_CH1();
    show_COOL();
    show_CH2();
    show_TYPE();
    show_CNT();
}

void saveBeforeExit()
{
    if(pre_exit==1)
    {
        if(value_updated)
        {
            saveVal();
            value_updated = 0;
        }
    }
    pre_exit = 0;
}

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
// 状�?�枚�????????
typedef enum {
    IDLE_MODE,      // 空闲状�??
    SELECT_MODE,    // 选择变量状�??
    EDIT_MODE       // 修改变量状�??
} State;

// 全局状�?�变�????????
State current_state = IDLE_MODE;
uint8_t selected_index = 5;  // 当前选中的变量索引：0,1,2,3,4,5 ==> U, PSQR, CH1, COOL, CH2, STORAGE
uint32_t prev_cnt = 2000000000;  // 初始值与你给的g_cnt初始值相�????????
uint64_t last_activity_time = 0; // �????????后一次活动时间戳
const uint64_t IDLE_TIMEOUT = 3000000; // 空闲超时时间3�????????
// 状�?�机处理函数
void update_state_machine(void) {
    uint64_t current_time = Get_Global_Time_us();

    // �????????查空闲超时（除了IDLE_MODE状�?�外�????????
    if (current_state != IDLE_MODE) {
        if (current_time - last_activity_time > IDLE_TIMEOUT) {
            if(current_state==EDIT_MODE)
            {
                current_state = SELECT_MODE;
                draw_edited(selected_index,GREEN,0); // un edit
                show_select_index(selected_index);
                last_activity_time = current_time;
            }
            else if(current_state==SELECT_MODE)
            {
                current_state = IDLE_MODE;
                back_to_idle(selected_index);
                selected_index = 5;
                g_cnt = 2000000000;
                prev_cnt = 2000000000;
            }
            return;
        }
    }

    // 计算编码器计数的变化�????????
    int32_t diff = (int32_t)(g_cnt - prev_cnt);

    // 处理按钮事件（如果按钮被按下�????????
    if (g_btn == 1) {
        printf("btn=ON\r\n");
        last_activity_time = current_time; // 更新活动时间

        if (current_state == IDLE_MODE) {
            // 在空闲模式下按下按钮，无动作
        } else if (current_state == SELECT_MODE) {
            // 在�?�择模式下按下按钮，进入编辑模式
            current_state = EDIT_MODE;
            show_edit_index(selected_index);
        } else if (current_state == EDIT_MODE) {
            // 在编辑模式下按下按钮，返回�?�择模式
            current_state = SELECT_MODE;
            draw_edited(selected_index,GREEN,0); // un edit
            show_select_index(selected_index);
        }
        g_btn = 0;  // 清除按钮状�?�（软件清零�????????
        return;
    }

    // 处理编码器旋转事件（如果有旋转）
    if (diff != 0) {

        last_activity_time = current_time; // 更新活动时间
        prev_cnt = g_cnt;  // 更新上一次的计数

        if (current_state == IDLE_MODE) {
            // 在空闲模式下旋转编码器，进入选择模式
            current_state = SELECT_MODE;
        } else if (current_state == SELECT_MODE) {
            // 在�?�择模式下，旋转编码器改变�?�中的变�????????
            if (diff > 0) {
                // 正转
                selected_index = (selected_index + 1) % 6;
            } else {
                // 反转
                if(selected_index==0) {
                    selected_index = 5;
                }else{
                    selected_index = selected_index - 1;
                }
                selected_index = selected_index % 6;
            }
//            LCD_ShowNum(364,70,selected_index,1,16);
//            Show_Str(364,90,"12.34",16,1);
//            Show_Float(364,90,(float)123,16,1);
            show_select_index(selected_index);
        } else if (current_state == EDIT_MODE) {
            // 在编辑模式下，旋转编码器改变当前选中的变量的�????????
            switch (selected_index) {
                case 0: // g_U: 0.0-6.0 v
                    if(diff>0)
                    {
                        g_U += 0.1f;
                    } else
                    {
                        g_U -= 0.1f;
                    }
                    if(g_U>6.0f)
                    {
                        g_U = 6.0f;
                    }
                    if(g_U<0.01f)
                    {
                        g_U=0.0f;
                    }
                    show_U();
                    break;
                case 1: // g_psqr: 0.1-5.0 s
                    if(diff>0)
                    {
                        g_psqr += 0.1f;
                    } else
                    {
                        g_psqr -= 0.1f;
                    }
                    if(g_psqr>5.0f)
                    {
                        g_psqr = 5.0f;
                    }
                    if(g_psqr<0.11f)
                    {
                        g_psqr=0.1f;
                    }
                    show_PSQR();
                    break;
                case 2: // g_ch1: 0.1-25.0 ms
                    if(diff>0)
                    {
                        g_ch1 += 0.1f;
                    } else
                    {
                        g_ch1 -= 0.1f;
                    }
                    if(g_ch1>25.0f)
                    {
                        g_ch1 = 25.0f;
                    }
                    if(g_ch1<0.11f)
                    {
                        g_ch1=0.1f;
                    }
                    show_CH1();
                    break;
                case 3: // g_cool:  1.0-9.9 ms
                    if(diff>0)
                    {
                        g_cool += 0.1f;
                    } else
                    {
                        g_cool -= 0.1f;
                    }
                    if(g_cool>9.9f)
                    {
                        g_cool = 9.9f;
                    }
                    if(g_cool<1.01f)
                    {
                        g_cool=1.0f;
                    }
                    show_COOL();
                    break;
                case 4: // g_ch2: 0.1-25.0 ms
                    if(diff>0)
                    {
                        g_ch2 += 0.1f;
                    } else
                    {
                        g_ch2 -= 0.1f;
                    }
                    if(g_ch2>25.0f)
                    {
                        g_ch2 = 25.0f;
                    }
                    if(g_ch2<0.11f)
                    {
                        g_ch2=0.1f;
                    }
                    show_CH2();
                    break;
                case 5:
                    // trig type
                    POINT_COLOR=WHITE;
                    if(g_type==0)
                    {
                        g_type = 1; // AUTO
                    }else
                    {
                        g_type = 0; // AMT
                    }
                    show_TYPE();
                    break;
                default:
                    break;
            }
            value_updated = 1;
        }
    }
}
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
void update_pulse_generation()
{
    uint8_t gen_flag = 0;
    uint64_t pre_time_stamp = 0;
    if(g_type==0) // AMT
    {
        if(trig_type == 1) // AMT trig
        {
            gen_flag = 1;
        }
    } else if(g_type == 1) // AUTO
    {
        if(trig_type == 2) // AUTO trig
        {
            gen_flag = 1;
        }
    }

    if(gen_flag == 1)
    {
        // Disable External Inputs
        g_pulse_generating = 1;

        // 1, delay for PSQR
        pre_time_stamp = Get_Global_Time_us();
        while(Get_Global_Time_us() < pre_time_stamp + (uint64_t)(g_psqr*1000000)){};

        // 2, output CH1
        pre_time_stamp = Get_Global_Time_us();
        HAL_GPIO_WritePin(OUT_CH1_GPIO_Port,OUT_CH1_Pin,1);
        while(Get_Global_Time_us() < pre_time_stamp + (uint64_t)(g_ch1*1000)){};
        HAL_GPIO_WritePin(OUT_CH1_GPIO_Port,OUT_CH1_Pin,0);

        // 3, delay for COOL
        pre_time_stamp = Get_Global_Time_us();
        while(Get_Global_Time_us() < pre_time_stamp + (uint64_t)(g_cool*1000)){};

        // 3, output CH2
        pre_time_stamp = Get_Global_Time_us();
        HAL_GPIO_WritePin(OUT_CH2_GPIO_Port,OUT_CH2_Pin,1);
        while(Get_Global_Time_us() < pre_time_stamp + (uint64_t)(g_ch2*1000)){};
        HAL_GPIO_WritePin(OUT_CH2_GPIO_Port,OUT_CH2_Pin,0);

        g_count ++;
        show_CNT();
        value_updated = 1;

        // Enable External Inputs
        g_pulse_generating = 0;
    }

    // clear trig flag
    trig_type = 0;
}


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM2_Init();
  MX_ADC1_Init();
  MX_SPI3_Init();
  MX_USART1_UART_Init();
  MX_CRC_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)g_adc_buffer, 5);
  HAL_TIM_Base_Start_IT(&htim2);
  FlashStore_Init();

    // 初始状�?�设�????????
    HAL_GPIO_WritePin(CTR_SW_GPIO_Port, CTR_SW_Pin, GPIO_PIN_RESET); // 关闭充电
    HAL_GPIO_WritePin(CTR_C1_GPIO_Port, CTR_C1_Pin, GPIO_PIN_RESET);   // 关闭放电
    HAL_GPIO_WritePin(CTR_C2_GPIO_Port, CTR_C2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(CTR_C3_GPIO_Port, CTR_C3_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(CTR_C4_GPIO_Port, CTR_C4_Pin, GPIO_PIN_RESET);

    data_cfg_t dataCfg;
    if(FlashStore_GetLatest(&dataCfg))
    {
        g_U = (float)(dataCfg.u/10.0f);
        g_psqr =  (float)(dataCfg.psqr/10.0f);
        g_ch1 = (float)(dataCfg.ch1/10.0f);
        g_cool = (float)(dataCfg.cool/10.0f);
        g_ch2 = (float)(dataCfg.ch2/10.0f);
        g_type = (uint8_t)dataCfg.type;
        g_count = dataCfg.count;
    }
    else
    {
        saveVal();
    }

    LCD_Init();			   	           //Initialize LCD
    LCD_Display_Dir(USE_LCM_DIR);		   //Screen orientation
    LCD_Clear(BLACK);
    LCD_Turn_On_Backlight();
    Show_Page_Main();
    showValue();

//    data_cfg_t dataCfg;
//    dataCfg.u = 123;
//    dataCfg.psqr = 234;
//    dataCfg.ch1 = 345;
//    dataCfg.cool = 456;
//    dataCfg.ch2 = 567;
//    dataCfg.type = 678;
//    dataCfg.count = 789;
//    dataCfg.flag = 890;
////    FlashStore_Save(dataCfg);
//    data_cfg_t dataCfg1;
//    FlashStore_GetLatest(&dataCfg1);
//    printf("u:     %lu\n", dataCfg1.u);
//    printf("psqr:  %lu\n", dataCfg1.psqr);
//    printf("ch1:   %lu\n", dataCfg1.ch1);
//    printf("cool:  %lu\n", dataCfg1.cool);
//    printf("ch2:   %lu\n", dataCfg1.ch2);
//    printf("type:  %lu\n", dataCfg1.type);
//    printf("count: %lu\n", dataCfg1.count);
//    printf("flag:  %lu\n", dataCfg1.flag);

    Draw_RoundedRectangle(0 * 96, 320 - 1 * 54);
    Draw_RoundedRectangle(1 * 96, 320 - 1 * 54);
    Draw_RoundedRectangle(2 * 96, 320 - 1 * 54);
    Draw_RoundedRectangle(3 * 96, 320 - 1 * 54);
    Draw_RoundedRectangle(4 * 96, 320 - 1 * 54);

    Draw_RoundedRectangle(0 * 96, 320 - 2 * 54);
    Draw_RoundedRectangle(1 * 96, 320 - 2 * 54);
    Draw_RoundedRectangle(2 * 96, 320 - 2 * 54);
    Draw_RoundedRectangle(3 * 96, 320 - 2 * 54);
    Draw_RoundedRectangle(4 * 96, 320 - 2 * 54);

    LCD_Fill(0,30,384,30+180, DARK_RED_BROWN);
    POINT_COLOR = GOLDEN_YELLOW;
    LCD_DrawRectangle(0,30,384,30+180);
    LCD_DrawRectangle(1,30+1,384-1,30+180-1);
    LCD_DrawRectangle(2,30+2,384-2,30+180-2);
    LCD_Fill(0,30,54,30+40, GOLDEN_YELLOW);

    LCD_Fill(384,30,384+96,30+60, DARK_BROWN);
    LCD_Fill(384,30+44,384+22,30+60, OLIVE_GREEN);
    POINT_COLOR = OLIVE_GREEN;
    LCD_DrawRectangle(384,30,384+96,30+60);
    LCD_DrawRectangle(384+1,30+1,384+96-1,30+60-1);
    LCD_DrawRectangle(384+2,30+2,384+96-2,30+60-2);


    LCD_Fill(384,90,384+96,90+60, MIDNIGHT_BLUE);
    LCD_Fill(384,90+44,384+22,90+60, COBALT_BLUE);
    POINT_COLOR = COBALT_BLUE;
    LCD_DrawRectangle(384,90,384+96,90+60);
    LCD_DrawRectangle(384+1,90+1,384+96-1,90+60-1);
    LCD_DrawRectangle(384+2,90+2,384+96-2,90+60-2);

    LCD_Fill(384,150,384+96,150+60, CRIMSON_RED);
    LCD_Fill(384,150+44,384+22,150+60, CORAL_PINK);
    POINT_COLOR = CORAL_PINK;
    LCD_DrawRectangle(384,150,384+96,150+60);
    LCD_DrawRectangle(384+1,150+1,384+96-1,150+60-1);
    LCD_DrawRectangle(384+2,150+2,384+96-2,150+60-2);

    POINT_COLOR=SAGE_GREEN;
    LCD_DrawLine(3,154,380,154);
    LCD_DrawLine(3,168,380,168);
    LCD_DrawLine(3,181,380,181);
    LCD_DrawLine(3,194,380,194);

//    POINT_COLOR=LIME_GREEN;
//    LCD_Draw_Circle(12,181,5);
//    LCD_Fill(12,181-1,90,181, LIME_GREEN);
//    uint16_t x0=91;
//    uint16_t y0=170;
//    LCD_Fill(x0,y0+11,x0+0,y0+2, LIME_GREEN);
//    LCD_Fill(x0+0,y0+10,x0+5,y0+10, LIME_GREEN);
//    LCD_Fill(x0+3,y0+9,x0+6,y0+9, LIME_GREEN);
//    LCD_Fill(x0+5,y0+8,x0+7,y0+8, LIME_GREEN);
//    LCD_Fill(x0+7,y0+7,x0+8,y0+7, LIME_GREEN);
//    LCD_Fill(x0+8,y0+6,x0+9,y0+6, LIME_GREEN);
//    LCD_Fill(x0+8,y0+5,x0+10,y0+5, LIME_GREEN);
//    LCD_Fill(x0+9,y0+4,x0+10,y0+4, LIME_GREEN);
//    LCD_Fill(x0+9,y0+3,x0+10,y0+3, LIME_GREEN);
//    LCD_Fill(x0+10,y0+2,x0+11,y0+2, LIME_GREEN);
//    LCD_Fill(x0+10,y0+1,x0+11,y0+1, LIME_GREEN);
//    LCD_Fill(x0+10,y0+0,x0+11,y0+0, LIME_GREEN);

    Draw_WaveLine();
    Draw_StaticText();




  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint64_t pretime = 0;
  while (1)
  {
      env_detect();
      uint64_t crrtime = Get_Global_Time_us();
      uint64_t buzzertime = crrtime;
      if(crrtime > pretime + 100000)
      {
          pretime = crrtime;
//          输出格式: 时间(ms),目标电压,电容1,电容2,电容3,电容4,状�??
          printf("Set=%u,T=%u,C1=%u,C2=%u,C3=%u,C4=%u,St=%d\r\n",
                 g_target_voltage_mv,
                 g_temp,
                 g_cap_voltage_mv[0],
                 g_cap_voltage_mv[1],
                 g_cap_voltage_mv[2],
                 g_cap_voltage_mv[3],
                 g_state);
      }

      update_state_machine();
      update_pulse_generation();
      update_cap_equalizer();
      saveBeforeExit();

//      HAL_GPIO_TogglePin(LED2_GPIO_Port,LED2_Pin);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 84;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
uint32_t state = 0;
uint64_t BtnTimeStamp = 0;
uint64_t AmtTrigTimeStamp = 0;
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    uint8_t pha;
    uint8_t phb;
    if(g_pulse_generating == 0)
    {
        switch(GPIO_Pin) {
            case SET_Pin:
                if(Get_Global_Time_us()>BtnTimeStamp+500000)
                {
                    didi();
                    g_btn = 1;
                    BtnTimeStamp = Get_Global_Time_us();
                }
                break;
            case PHA_Pin:

                pha = HAL_GPIO_ReadPin(PHA_GPIO_Port,PHA_Pin);
                phb = HAL_GPIO_ReadPin(PHB_GPIO_Port,PHB_Pin);
                printf("pha:%d,phb:%d\r\n",pha,phb);
                if(pha==1 && phb==0) // product xuan niu
                    // if(phb==1 && pha==0) // my xuan niu
                {
                    state = 10;
                }
                if(state == 10 && pha==0 && phb==1)
                {
                    didi();
                    g_cnt ++;
                    state = 0;
                }
                if(pha==0 && phb==0)
                    //if(pha==0 && phb==0) // my xuan niu
                {
                    state = 11;
                }
                if(state == 11 && pha==0 && phb==0)
                {
                    didi();
                    g_cnt --;
                    state = 0;
                }
                break;
            case AMT_TRIG_Pin:
                if(Get_Global_Time_us()>AmtTrigTimeStamp+1000000)
                {
                    trig_type = 1;
                    AmtTrigTimeStamp = Get_Global_Time_us();
                }
                break;
            case AUTO_TRIG_Pin:
                trig_type = 2;
                break;
            case PRE_EXIT_Pin:
                pre_exit = 1;
                break;
            default:
                break;
        }
    }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
