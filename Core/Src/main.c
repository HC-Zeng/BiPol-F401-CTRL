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

volatile uint32_t timer_overflow_count = 0;  // 溢出次数�????????????????32位扩展）

uint64_t Get_Global_Time_us(void) {
    uint32_t overflow, counter;
    do {
        overflow = timer_overflow_count;
        counter = __HAL_TIM_GET_COUNTER(&htim2);
    } while (overflow != timer_overflow_count); // 无锁校验
    return ((uint64_t)overflow<<32) | counter; // 组合�????????????????64位时间戳
}

void didi()
{
    HAL_TIM_Base_Start_IT(&htim3);
}

data_cfg_t g_dataCfg;

void (*g_blanking_function)(float) = NULL;

#define INIT_VAL 2000000000
#define FLOAT_TO_UINT32(x) ((uint32_t)((x) + 0.5f))
typedef enum {
    MANUAL,
    AUTO,
    NONE
} WELDING_MODE;
uint8_t pre_exit = 0;
WELDING_MODE trig_type=NONE;

uint8_t g_blanking;
uint32_t g_cnt =  INIT_VAL;
uint8_t g_btn = 0;
int32_t btn_delay_cnt = 10000;//500ms base 50us period
int32_t trig_delay_cnt = 10000;//500ms base 50us period
int32_t pre_exit_delay_cnt = 100000;//5s base 50us period
uint8_t bezCnt = 0;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    static int8_t cnt_btn=0;
    static int8_t cnt_a=0;
    static int8_t pre_a=0;
    static int8_t cnt_b=0;
    static int8_t cnt_amt_trig=0;
    static int8_t cnt_auto_trig=0;
    static int8_t cnt_pre_exit=0;
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
    if(htim->Instance == TIM4) // 50us
    {
        cnt_amt_trig += HAL_GPIO_ReadPin(AMT_TRIG_GPIO_Port,AMT_TRIG_Pin)?-1:1;
        cnt_auto_trig += HAL_GPIO_ReadPin(AUTO_TRIG_GPIO_Port,AUTO_TRIG_Pin)?-1:1;
        cnt_pre_exit += HAL_GPIO_ReadPin(PRE_EXIT_GPIO_Port,PRE_EXIT_Pin)?-1:1;
        cnt_btn += HAL_GPIO_ReadPin(SET_GPIO_Port,SET_Pin)?-1:1;
        cnt_a += HAL_GPIO_ReadPin(PHB_GPIO_Port,PHB_Pin)?1:-1;
        cnt_b += HAL_GPIO_ReadPin(PHA_GPIO_Port,PHA_Pin)?1:-1;
        cnt_amt_trig = (int8_t)((cnt_amt_trig < 0) ? 0 : ((cnt_amt_trig > 20) ? 20 : cnt_amt_trig));
        cnt_auto_trig = (int8_t)((cnt_auto_trig < 0) ? 0 : ((cnt_auto_trig > 20) ? 20 : cnt_auto_trig));
        cnt_pre_exit = (int8_t)((cnt_pre_exit < 0) ? 0 : ((cnt_pre_exit > 20) ? 20 : cnt_pre_exit));
        cnt_btn = (int8_t)((cnt_btn < 0) ? 0 : ((cnt_btn > 20) ? 20 : cnt_btn));
        cnt_a = (int8_t)((cnt_a < 0) ? 0 : ((cnt_a > 20) ? 20 : cnt_a));
        cnt_b = (int8_t)((cnt_b < 0) ? 0 : ((cnt_b > 20) ? 20 : cnt_b));
        if(trig_type==NONE)
        {
            trig_delay_cnt -= 1;
            trig_delay_cnt = (trig_delay_cnt > 10000) ? 10000 : ((trig_delay_cnt < 0) ? 0 : trig_delay_cnt);
        }
        btn_delay_cnt -= 1;
        btn_delay_cnt = (btn_delay_cnt > 10000) ? 10000 : ((btn_delay_cnt < 0) ? 0 : btn_delay_cnt);
        if(pre_exit==0)
        {
            pre_exit_delay_cnt -= 1;
            pre_exit_delay_cnt = (pre_exit_delay_cnt > 100000) ? 100000 : ((pre_exit_delay_cnt < 0) ? 0 : pre_exit_delay_cnt);
        }

        if(cnt_amt_trig>10 && trig_delay_cnt==0)
        {
            trig_type = MANUAL;
            trig_delay_cnt = 10000;
        }

        if(cnt_auto_trig>10 && trig_delay_cnt==0)
        {
            trig_type = AUTO;
            trig_delay_cnt = 10000;
        }
        if(cnt_pre_exit>10 && pre_exit_delay_cnt==0)
        {
            pre_exit = 1;
            pre_exit_delay_cnt = 100000;
        }

        if(cnt_btn>10 && btn_delay_cnt==0)
        {
            didi();
            g_btn = 1;
            cnt_btn = 0;
            btn_delay_cnt = 10000;// reset
        }

        if(cnt_a>10)
        {
            pre_a = 1;
        }
        else
        {
            if(pre_a)
            {
                if(cnt_b>10)//CCW
                {
                    didi();
                    g_cnt--;
                }else //CW
                {
                    didi();
                    g_cnt++;
                }
            }
            pre_a = 0;
        }
        //HAL_GPIO_WritePin(LED3_GPIO_Port,LED3_Pin,0);
//        HAL_GPIO_TogglePin(LED2_GPIO_Port,LED2_Pin);
        //HAL_GPIO_WritePin(LED2_GPIO_Port,LED2_Pin,0);
    }
}


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* 系统状�?�枚�???????????? */
typedef enum {
    STATE_INIT = 0,
    STATE_CHARGE,
    STATE_BALANCE,
    STATE_MAINTAIN,
    STATE_DISCHARGE
} SystemState;

/* 全局变量 */
SystemState g_state = STATE_INIT;

uint32_t g_adc_buffer[5];            // ADC采样缓冲�????????????
uint16_t g_target_voltage_mv = 0;    // 目标电压 (mV)
uint16_t g_cap_voltage_mv[4] = {0};  // 电容电压 (mV)
uint16_t g_temp = 0;                 // 温度电压 (mv)


uint8_t value_updated = 0;



WELDING_MODE g_weldingMode = 0; // 0:AMT,1: AUTO
uint32_t g_idx = 0;   // 1- 5
float g_U = 9.2f;     // 0.2-12.0 v
float g_psqr = 0.1f;  // 0.1-5.0 s
float g_ch1 = 9.6f;   // 0.1-25.0 ms
float g_cool = 1.0f;  // 1.0-9.9 ms
float g_ch2 = 9.6f;   // 0.1-25.0 ms
uint32_t g_count = 0; // 0->>>>>


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
//            printf("FAN ON\r\n");
            HAL_GPIO_WritePin(CTR_FAN_GPIO_Port,CTR_FAN_Pin,1);
        }
        if(g_temp>1800)
        {
//            printf("FAN OFF\r\n");
            HAL_GPIO_WritePin(CTR_FAN_GPIO_Port,CTR_FAN_Pin,0);
        }
        return 1;
    }
    return 0;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    if(hadc->Instance == ADC1) {
//        HAL_GPIO_TogglePin(LED2_GPIO_Port,LED2_Pin);
        UpdateVoltageMeasurements();
    }
}

void updateParameter(uint32_t idx)
{
    g_U = (float)(g_dataCfg.u[idx]/100.0f);
    g_psqr =  (float)(g_dataCfg.psqr[idx]/10.0f);
    g_ch1 = (float)(g_dataCfg.ch1[idx]/10.0f);
    g_cool = (float)(g_dataCfg.cool[idx]/10.0f);
    g_ch2 = (float)(g_dataCfg.ch2[idx]/10.0f);

    ShowSelectedChannel(g_idx,0);
    ShowSetVoltage(g_U,0);
    ShowSQDuration(g_psqr,0);
    ShowWE1Duration(g_ch1,0);
    ShowCOOLDuration(g_cool,0);
    ShowWE2Duration(g_ch2,0);
}


#define ALPHA 0.1f
static uint16_t filtered_voltage[4] = {0};


void UpdateVoltageMeasurements(void) {

    g_target_voltage_mv = (u_int16_t)(g_U/2.0f*1000.0f);

    // Temperature Sensor Input. channel(PA3)
    g_temp = (uint16_t)(g_adc_buffer[0] * 3300.0f / 4096.0f); // <1500,fan on; >1700, fan off

    // Four Capacitor Voltage Inputs. channel(PA4-PA7)
    for(int i = 0; i < 4; i++) {
        float adc_voltage = g_adc_buffer[i+1] * 3.3f / 4096.0f;
        // scaling
        uint16_t raw_voltage = (uint16_t)(adc_voltage * (16.0f / 10.0f) * 1000);

        // Low-pass filter
        if(filtered_voltage[i] == 0) {
            filtered_voltage[i] = raw_voltage;
        } else {
            filtered_voltage[i] = (uint16_t)(ALPHA * raw_voltage + (1 - ALPHA) * filtered_voltage[i]);
        }
        g_cap_voltage_mv[i] = filtered_voltage[i];
    }

//    printf("Set=%u,C1=%u,C2=%u,C3=%u,C4=%u,St=%d\r\n",
//           g_target_voltage_mv,
//           g_cap_voltage_mv[0],
//           g_cap_voltage_mv[1],
//           g_cap_voltage_mv[2],
//           g_cap_voltage_mv[3],
//           g_state);
}


#define BALANCE_THRESHOLD    20    // unit(mv)

void SystemStateMachine(void) {
    uint8_t i;
    uint16_t max_voltage = 0;
    uint16_t min_voltage = UINT16_MAX;
    uint16_t avg_voltage = 0;
    uint8_t max_id = 0;

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

    switch(g_state) {
        case STATE_INIT:
            g_state = STATE_CHARGE;
            break;

        case STATE_CHARGE: {
            HAL_GPIO_WritePin(CTR_SW_GPIO_Port, CTR_SW_Pin, GPIO_PIN_SET);

            HAL_GPIO_WritePin(CTR_C1_GPIO_Port, CTR_C1_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(CTR_C2_GPIO_Port, CTR_C2_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(CTR_C3_GPIO_Port, CTR_C3_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(CTR_C4_GPIO_Port, CTR_C4_Pin, GPIO_PIN_RESET);

            if(avg_voltage > g_target_voltage_mv*0.3) {
                g_state = STATE_BALANCE;
            }
            break;
        }

        case STATE_BALANCE: {
            HAL_GPIO_WritePin(CTR_SW_GPIO_Port, CTR_SW_Pin, GPIO_PIN_SET);

            uint8_t discharge_flags[4] = {0};

            if(max_voltage-min_voltage > BALANCE_THRESHOLD)
            {
                discharge_flags[max_id] = 1;
            }

            HAL_GPIO_WritePin(CTR_C1_GPIO_Port, CTR_C1_Pin, discharge_flags[0]);
            HAL_GPIO_WritePin(CTR_C2_GPIO_Port, CTR_C2_Pin, discharge_flags[1]);
            HAL_GPIO_WritePin(CTR_C3_GPIO_Port, CTR_C3_Pin, discharge_flags[2]);
            HAL_GPIO_WritePin(CTR_C4_GPIO_Port, CTR_C4_Pin, discharge_flags[3]);

            if(max_voltage<g_target_voltage_mv*0.8)
            {
                g_state = STATE_CHARGE;
            }

            if(avg_voltage>g_target_voltage_mv) {
                g_state = STATE_MAINTAIN;
            }
            break;
        }

        case STATE_MAINTAIN: {
            HAL_GPIO_WritePin(CTR_SW_GPIO_Port, CTR_SW_Pin, GPIO_PIN_RESET);

            uint8_t discharge_flags[4] = {0};

            if(max_voltage-min_voltage > BALANCE_THRESHOLD)
            {
                discharge_flags[max_id] = 1;
            }

            HAL_GPIO_WritePin(CTR_C1_GPIO_Port, CTR_C1_Pin, discharge_flags[0] );
            HAL_GPIO_WritePin(CTR_C2_GPIO_Port, CTR_C2_Pin, discharge_flags[1] );
            HAL_GPIO_WritePin(CTR_C3_GPIO_Port, CTR_C3_Pin, discharge_flags[2] );
            HAL_GPIO_WritePin(CTR_C4_GPIO_Port, CTR_C4_Pin, discharge_flags[3] );

            if(max_voltage<g_target_voltage_mv)
            {
                g_state = STATE_BALANCE;
            }

            if(min_voltage>g_target_voltage_mv)
            {
                g_state = STATE_DISCHARGE;
            }
            break;
        }

        case STATE_DISCHARGE: {
            HAL_GPIO_WritePin(CTR_SW_GPIO_Port, CTR_SW_Pin, GPIO_PIN_RESET);

            uint8_t discharge_flags[4] = {0};
            uint8_t all_in_range = 1;

            for(i = 0; i < 4; i++) {
                if ((g_cap_voltage_mv[i]>=g_target_voltage_mv) && (g_cap_voltage_mv[i] - g_target_voltage_mv > BALANCE_THRESHOLD)) {
                    discharge_flags[i] = 1;
                    all_in_range = 0;
                }
            }

            HAL_GPIO_WritePin(CTR_C1_GPIO_Port, CTR_C1_Pin, discharge_flags[0] );
            HAL_GPIO_WritePin(CTR_C2_GPIO_Port, CTR_C2_Pin, discharge_flags[1] );
            HAL_GPIO_WritePin(CTR_C3_GPIO_Port, CTR_C3_Pin, discharge_flags[2] );
            HAL_GPIO_WritePin(CTR_C4_GPIO_Port, CTR_C4_Pin, discharge_flags[3] );

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
        preTimestamp = Get_Global_Time_us();
        float UC1 = (float)((float)(g_cap_voltage_mv[0])/1000.f); //for debug
        float UC2 = (float)((float)(g_cap_voltage_mv[1])/1000.f); //for debug
        ShowC1C2(UC1,UC2);
        float UC3 = (float)((float)(g_cap_voltage_mv[2])/1000.f); //for debug
        float UC4 = (float)((float)(g_cap_voltage_mv[3])/1000.f); //for debug
        ShowC3C4(UC3,UC4);
    }
}

//------------------------before is for cap equalizer--------------------

static void saveVal()
{
    g_dataCfg.u[g_idx] = FLOAT_TO_UINT32(g_U*100);
    g_dataCfg.psqr[g_idx] = FLOAT_TO_UINT32(g_psqr*10);
    g_dataCfg.ch1[g_idx] = FLOAT_TO_UINT32(g_ch1*10);
    g_dataCfg.cool[g_idx] = FLOAT_TO_UINT32(g_cool*10);
    g_dataCfg.ch2[g_idx] = FLOAT_TO_UINT32(g_ch2*10);
    g_dataCfg.idx = g_idx;
    g_dataCfg.count = (uint32_t)(g_count);
    g_dataCfg.flag[g_idx] = 12345;

//    printf("dataCfg.psqr=%lu, psqr=%f",dataCfg.psqr,g_psqr);

    FlashStore_Save(g_dataCfg);

    static uint32_t cnt = 0;
    cnt += 1;
}

void back_to_idle()
{
    printf("back to idle\r\n");
    ShowSelectedButton(255); // 255 means no button selected
}

void saveBeforeExit()
{
    if(pre_exit==1)
    {
        printf("pre exit.\r\n");
        if(value_updated)
        {
            printf("save before exit.\r\n");
            g_dataCfg.idx = g_idx;
            g_dataCfg.count = (uint32_t)(g_count);
            FlashStore_Save(g_dataCfg);
            value_updated = 0;
        }
    }
    pre_exit = 0;
}

void edit_state(uint8_t selected_index,uint8_t editing)
{
    switch (selected_index) {
        case 0:
            ShowSQDuration(g_psqr, editing);
            break;
        case 1:
            ShowWE1Duration(g_ch1,editing);
            break;
        case 2:
            ShowCOOLDuration(g_cool,editing);
            break;
        case 3:
            ShowWE2Duration(g_ch2,editing);
            break;
        case 4:
            ShowSetVoltage(g_U,editing);
            break;
        case 6:
            ShowSelectedChannel(g_idx, editing);
            break;
        case 7:
            ShowWeldingMode(g_weldingMode,editing);
            break;
        case 8:
            ShowWeldingMode(g_weldingMode,editing);
            break;
        default:
            break;
    }
}

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
// State enumeration
typedef enum {
    IDLE_MODE,      // Idle state
    SELECT_MODE,    // Variable selection state
    EDIT_MODE       // Variable modification state
} State;

// Global state variable
State current_state = IDLE_MODE;
uint8_t selected_index = 9;  // Currently selected variable index:0,1,2,3,4,5 6,7,8,9==> PSQR, CH1, COOL, CH2, SOV, SAVE, PARAM, AUTO, MAN, RCL
uint32_t prev_cnt = INIT_VAL;  // Same initial value as g_cnt.
uint64_t last_activity_time = 0; // Last activity timestamp
uint64_t last_saving_time = 0;
const uint64_t IDLE_TIMEOUT = 3000000; // Idle timeout: 3s
const uint64_t SAVING_TIMEOUT = 1000000; // 1s
// State machine handler
void update_state_machine(void) {
    uint64_t current_time = Get_Global_Time_us();

    if(last_saving_time!=0)
    {
        if(current_time-last_saving_time>SAVING_TIMEOUT)
        {
            ShowSaving(0);
            last_saving_time=0;
        }
    }

    // Check for idle timeout (except in IDLE_MODE state).
    if (current_state != IDLE_MODE) {
        if (current_time - last_activity_time > IDLE_TIMEOUT) {
            if(current_state==EDIT_MODE)
            {
                edit_state(selected_index,0);
                current_state = SELECT_MODE;
                ShowSelectedButton(selected_index);
                last_activity_time = current_time;
            }
            else if(current_state==SELECT_MODE)
            {
                current_state = IDLE_MODE;
                back_to_idle();
                selected_index = 9;
                g_cnt = INIT_VAL;
                prev_cnt = INIT_VAL;
            }
            return;
        }
    }

    // Calculate the change in encoder count
    int32_t diff = (int32_t)(g_cnt - prev_cnt);

    // Process button event (if button is pressed)
    if (g_btn == 1) {
        printf("btn=ON\r\n");
        last_activity_time = current_time; // Update activity time

        if (current_state == IDLE_MODE)
        {
            // do nothing
        }else if (current_state == SELECT_MODE)
        {
            current_state = EDIT_MODE;
            if(selected_index==8)
            {
                printf("mode=MANUAL\r\n");
                g_weldingMode = MANUAL;
            }else if(selected_index==7)
            {
                printf("mode=AUTO\r\n");
                g_weldingMode = AUTO;
            }else if(selected_index==5) // save parameter,no need enter edit mode
            {
                ShowSaving(1);
                saveVal();
                ShowSelectedChannel(g_idx,0);
                current_state = SELECT_MODE;
                last_saving_time = Get_Global_Time_us();
            }
            edit_state(selected_index,1);


        } else if (current_state == EDIT_MODE)
        {
            current_state = SELECT_MODE;
            edit_state(selected_index,0);
            ShowSelectedButton(selected_index);
        }
        g_btn = 0;
        return;
    }

    // Process encoder rotation event (if rotated)
    if (diff != 0) {
        last_activity_time = current_time;
        prev_cnt = g_cnt;
        if (current_state == IDLE_MODE) {
            current_state = SELECT_MODE;
        } else if (current_state == SELECT_MODE) {
            if (diff > 0) {
                selected_index = (selected_index + 1) % 10;
            } else {
                if(selected_index==0) {
                    selected_index = 9;
                }else{
                    selected_index = selected_index - 1;
                }
                selected_index = selected_index % 10;
            }
            ShowSelectedButton(selected_index);
        } else if (current_state == EDIT_MODE) {
            switch (selected_index) {
                case 0: // g_psqr: 0.1-5.0 s
                    g_psqr += (diff > 0) ? 0.1f : -0.1f;
                    g_psqr = (g_psqr > 5.0f) ? 5.0f : g_psqr;
                    g_psqr = (g_psqr < 0.11f) ? 0.1f : g_psqr;
                    ShowSQDuration(g_psqr,1);
                    break;
                case 1: // g_ch1: 0.1-25.0 ms
                    g_ch1 += (diff > 0) ? 0.1f : -0.1f;
                    g_ch1 = (g_ch1 > 25.0f) ? 25.0f : g_ch1;
                    g_ch1 = (g_ch1 < 0.11f) ? 0.1f : g_ch1;
                    ShowWE1Duration(g_ch1,1);
                    break;
                case 2: // g_cool:  1.0-9.9 ms
                    g_cool += (diff > 0) ? 0.1f : -0.1f;
                    g_cool = (g_cool > 9.9f) ? 9.9f : g_cool;
                    g_cool = (g_cool < 1.01f) ? 1.0f : g_cool;
                    ShowCOOLDuration(g_cool,1);
                    break;
                case 3: // g_ch2: 0.1-25.0 ms
                    g_ch2 += (diff > 0) ? 0.1f : -0.1f;
                    g_ch2 = (g_ch2 > 25.0f) ? 25.0f : g_ch2;
                    g_ch2 = (g_ch2 < 0.11f) ? 0.1f : g_ch2;
                    ShowWE2Duration(g_ch2, 1);
                    break;
                case 4: // g_U: 0.0-6.0 v
                    g_U += (diff > 0) ? 0.02f : -0.02f;
                    g_U = (g_U > 6.0f) ? 6.0f : g_U;
                    g_U = (g_U < 0.01f) ? 0.0f : g_U;
                    ShowSetVoltage(g_U, 1);
                    break;
                case 6: // param
                    if(diff > 0)
                    {
                        g_idx += 1;
                        g_idx = (g_idx>=5)? 0 : g_idx;
                    }else if(diff<0)
                    {
                        if(g_idx>0)
                        {
                            g_idx -= 1;
                        }else
                        {
                            g_idx=4;
                        }
                    }
                    updateParameter(g_idx);
                    ShowSelectedChannel(g_idx,1);
                    break;
                case 7:
                    g_weldingMode = AUTO;
                    ShowWeldingMode(g_weldingMode, 1);
                    break;
                case 8:
                    g_weldingMode = MANUAL;
                    ShowWeldingMode(g_weldingMode, 1);
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
    if(g_weldingMode==trig_type)
    {
        gen_flag = 1;
        printf("gen trig\r\n");
    }

    if(gen_flag == 1)
    {
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
        ShowCnt(g_count);
        value_updated = 1;
    }

    // clear trig flag
    trig_type = NONE;
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
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)g_adc_buffer, 5);
    HAL_TIM_Base_Start_IT(&htim2);
    HAL_TIM_Base_Start_IT(&htim4);

    FlashStore_Init();

    // all switch off
    HAL_GPIO_WritePin(CTR_SW_GPIO_Port, CTR_SW_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(CTR_C1_GPIO_Port, CTR_C1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(CTR_C2_GPIO_Port, CTR_C2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(CTR_C3_GPIO_Port, CTR_C3_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(CTR_C4_GPIO_Port, CTR_C4_Pin, GPIO_PIN_RESET);


    if(FlashStore_GetLatest(&g_dataCfg))
    {
        g_idx = (uint32_t)g_dataCfg.idx;

        for(int i=0;i<5;i++)
        {
            if(g_dataCfg.u[i]%2!=0)
            {
                g_dataCfg.u[i]+=1;
            }
        }

        g_U = (float)(g_dataCfg.u[g_idx]/100.0f);
        g_psqr =  (float)(g_dataCfg.psqr[g_idx]/10.0f);
        g_ch1 = (float)(g_dataCfg.ch1[g_idx]/10.0f);
        g_cool = (float)(g_dataCfg.cool[g_idx]/10.0f);
        g_ch2 = (float)(g_dataCfg.ch2[g_idx]/10.0f);

        g_count = g_dataCfg.count;
        g_weldingMode = 0;
        printf("Get data from NVM success.\r\n");
    }
    else
    {
        printf("Get data from NVM failed. Creating init value.\r\n");
        g_U=1.23f;
        g_psqr=0.8f;
        g_ch1=9.5f;
        g_cool=8.0f;
        g_ch2=9.5f;
        g_idx=0;
        g_count=0;
        for(int i=0;i<5;i++)
        {
            g_dataCfg.u[i] = FLOAT_TO_UINT32(g_U*100);
            g_dataCfg.psqr[i] = FLOAT_TO_UINT32(g_psqr*10);
            g_dataCfg.ch1[i] = FLOAT_TO_UINT32(g_ch1*10);
            g_dataCfg.cool[i] = FLOAT_TO_UINT32(g_cool*10);
            g_dataCfg.ch2[i] = FLOAT_TO_UINT32(g_ch2*10);
            g_dataCfg.flag[i] = 12345;
        }
        g_dataCfg.idx = g_idx;
        g_dataCfg.count = (uint32_t)(g_count);

        saveVal();
    }

    LCD_Init();
    LCD_Display_Dir(USE_LCM_DIR); //Screen orientation
    LCD_Clear(BLACK);

    initUI();

    ShowCnt(g_count);
    ShowSelectedChannel(g_idx,0);
//    ShowC1C2(1.23f,2.3f);
//    ShowC3C4(2.34f,4.50f);
    ShowWeldingMode(g_weldingMode,0);
    ShowSetVoltage(g_U,0);
    ShowSQDuration(g_psqr,0);
    ShowWE1Duration(g_ch1,0);
    ShowCOOLDuration(g_cool,0);
    ShowWE2Duration(g_ch2,0);
    ShowWeldingCurrent(4381);
    ShowSelectedButton(255);


    LCD_Turn_On_Backlight();


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint64_t pretime = 0;
  while (1)
  {
      env_detect();
      uint64_t crrtime = Get_Global_Time_us();
      if(crrtime > pretime + 100000)
      {
          pretime = crrtime;
//          printf("Set=%u,T=%u,C1=%u,C2=%u,C3=%u,C4=%u,St=%d\r\n",
//                 g_target_voltage_mv,
//                 g_temp,
//                 g_cap_voltage_mv[0],
//                 g_cap_voltage_mv[1],
//                 g_cap_voltage_mv[2],
//                 g_cap_voltage_mv[3],
//                 g_state);
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
