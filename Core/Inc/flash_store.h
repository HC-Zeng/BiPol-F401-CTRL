//
// Created by HC on 2025/9/21.
//

#ifndef BIPOL_F401_CTRL_FLASH_STORE_H
#define BIPOL_F401_CTRL_FLASH_STORE_H

#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include <string.h>

// Flash配置
#define FLASH_USER_SECTOR        FLASH_SECTOR_5   // 使用Sector 5 (128KB)
#define FLASH_USER_ADDR_BASE     0x08020000U      // Sector 5起始地址
#define FLASH_SECTOR_SIZE        (128 * 1024)     // 128KB

// 数据包结构体
#pragma pack(push, 1)  // 确保紧凑排列，无填充字节
typedef struct {
    uint32_t magic;         // 魔数标识：0xDEADBEEF
    uint32_t version;       // 版本号：1, 2, 3, ...
    uint32_t crc32;         // CRC32校验值

    // 8个u32数据
    uint32_t u[5];
    uint32_t psqr[5];
    uint32_t ch1[5];
    uint32_t cool[5];
    uint32_t ch2[5];
    uint32_t idx;
    uint32_t count;
    uint32_t flag[5];
} data_packet_t;
#pragma pack(pop)

typedef struct {
    uint32_t u[5];
    uint32_t psqr[5];
    uint32_t ch1[5];
    uint32_t cool[5];
    uint32_t ch2[5];
    uint32_t idx;
    uint32_t count;
    uint32_t flag[5];
} data_cfg_t;

#define DATA_PACKET_SIZE     sizeof(data_packet_t)  // 数据包大小
#define MAGIC_NUMBER         314159             // 魔数标识

// 函数声明
void FlashStore_Init(void);
bool FlashStore_Save(data_cfg_t dataCfg);
bool FlashStore_GetLatest(data_cfg_t *dataCfg);
uint32_t FlashStore_GetVersion(void);
uint32_t FlashStore_GetUsedSpace(void);
uint32_t FlashStore_GetRemainingSlots(void);
void FlashStore_Format(void);

#endif //BIPOL_F401_CTRL_FLASH_STORE_H
