#include "flash_store.h"
#include "debug_printf.h"
#include "stm32f4xx_hal_crc.h"  // 使用HAL CRC头文件

// 外部CRC句柄（通常在main.c中定义）
extern CRC_HandleTypeDef hcrc;

// 当前状态
static uint32_t current_version = 0;
static uint32_t next_write_offset = 0;
static bool sector_initialized = false;

// 计算CRC32（使用硬件CRC）
static uint32_t calculate_crc32(const data_packet_t *packet) {
    // 计算除crc32字段外所有数据的CRC
    uint32_t *data_ptr = (uint32_t*)packet;
    uint32_t data_words = (sizeof(data_packet_t) - sizeof(uint32_t)) / sizeof(uint32_t);

    return HAL_CRC_Calculate(&hcrc, data_ptr, data_words);
}

// 查找下一个可写位置
static uint32_t find_next_write_offset(void) {
    uint32_t offset = 0;

    while (offset + DATA_PACKET_SIZE <= FLASH_SECTOR_SIZE) {
        data_packet_t *packet = (data_packet_t*)(FLASH_USER_ADDR_BASE + offset);

        if (packet->magic != MAGIC_NUMBER) {
            return offset;
        }

        offset += DATA_PACKET_SIZE;
    }

    return FLASH_SECTOR_SIZE; // 扇区已满
}

// 查找最新有效数据包
static bool find_latest_packet(data_packet_t *result) {
    uint32_t offset = 0;
    uint32_t max_version = 0;
    uint32_t latest_offset = 0;
    bool found = false;
    while (offset + DATA_PACKET_SIZE <= FLASH_SECTOR_SIZE) {
        data_packet_t packet = *(data_packet_t*)(FLASH_USER_ADDR_BASE + offset);
        uint32_t crc = packet.crc32;
        packet.crc32 = 0;
        if (packet.magic == MAGIC_NUMBER) {
            uint32_t calculated_crc = calculate_crc32(&packet);

            if (crc == calculated_crc) {
                if (packet.version > max_version) {
                    max_version = packet.version;
                    latest_offset = offset;
                    found = true;
                }
            }
            else
            {
                if(offset==0)
                {
                    printf("crc mismatch\r\n");
                }
            }
        }
        else
        {
            if(offset==0)
            {
                printf("magic_num:%ld\r\n",packet.magic);
            }
        }

        offset += DATA_PACKET_SIZE;
    }

    if (found) {
//        printf("found! ver=%ld, offset=%ld\r\n",max_version,latest_offset);
        memcpy(result, (void*)(FLASH_USER_ADDR_BASE + latest_offset), DATA_PACKET_SIZE);
        return true;
    }

    return false;
}

// 初始化Flash存储
void FlashStore_Init(void) {
    // 查找最新数据包来获取当前版本
    data_packet_t latest_packet;
    if (find_latest_packet(&latest_packet)) {
        current_version = latest_packet.version;
        next_write_offset = find_next_write_offset();
        printf("Found existing data, version: %lu, current offset:%lu, next offset: %lu\n",
               current_version, next_write_offset-DATA_PACKET_SIZE, next_write_offset);
    } else {
        current_version = 0;
        next_write_offset = 0;
        printf("Never store anything, formatting...\n");
        FlashStore_Format();
    }

    sector_initialized = true;
}

// 保存数据
bool FlashStore_Save(data_cfg_t dataCfg) {
    if (!sector_initialized) {
        FlashStore_Init();
    }

    // 检查是否需要擦除（空间不足）
    if (next_write_offset + DATA_PACKET_SIZE > FLASH_SECTOR_SIZE) {
        printf("Sector full, formatting...\n");
        FlashStore_Format();
    }

    // 准备数据包
    data_packet_t new_packet;
    new_packet.magic = MAGIC_NUMBER;
    new_packet.version = current_version + 1;
    new_packet.crc32 = 0;

    new_packet.idx = dataCfg.idx;
    new_packet.count = dataCfg.count;
    for(int i=0;i<5;i++)
    {
        new_packet.u[i] = dataCfg.u[i];
        new_packet.psqr[i] = dataCfg.psqr[i];
        new_packet.ch1[i] = dataCfg.ch1[i];
        new_packet.cool[i] = dataCfg.cool[i];
        new_packet.ch2[i] = dataCfg.ch2[i];
        new_packet.flag[i] = dataCfg.flag[i];
    }

    // 计算CRC（令crc32=0来计算，读取的时候也要把crc32置为0再来计算）
    new_packet.crc32 = calculate_crc32(&new_packet);

    HAL_StatusTypeDef status = HAL_FLASH_Unlock();
    if (status != HAL_OK) {
        printf("Flash unlock failed: %d\n", status);
        return false;
    }

    // 按字编程整个结构体
    uint32_t *src = (uint32_t*)&new_packet;
    uint32_t dest_addr = FLASH_USER_ADDR_BASE + next_write_offset;

    for (uint32_t i = 0; i < DATA_PACKET_SIZE / sizeof(uint32_t); i++) {
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, dest_addr + i * 4, src[i]);
        if (status != HAL_OK) {
            printf("Flash program failed at word %lu: %d\n", i, status);
            HAL_FLASH_Lock();
            return false;
        }
    }

    HAL_FLASH_Lock();

    // 更新状态
    current_version++;
    next_write_offset += DATA_PACKET_SIZE;

    printf("Save successful! Version: %lu, Offset: %lu\n", current_version, next_write_offset);
    return true;
}

// 获取最新数据
bool FlashStore_GetLatest(data_cfg_t *dataCfg) {
    data_packet_t packet;
    bool rtn = find_latest_packet(&packet);
    if(rtn)
    {
        dataCfg->idx = packet.idx;
        dataCfg->count = packet.count;
        for(int i=0;i<5;i++)
        {
            dataCfg->u[i] = packet.u[i];
            dataCfg->psqr[i] = packet.psqr[i];
            dataCfg->ch1[i] = packet.ch1[i];
            dataCfg->cool[i] = packet.cool[i];
            dataCfg->ch2[i] = packet.ch2[i];
            dataCfg->flag[i] = packet.flag[i];
        }
    }
    return rtn;
}

// 获取当前版本号
uint32_t FlashStore_GetVersion(void) {
    return current_version;
}

// 获取已使用空间（字节）
uint32_t FlashStore_GetUsedSpace(void) {
    return next_write_offset;
}

// 获取剩余可存储的数据包数量
uint32_t FlashStore_GetRemainingSlots(void) {
    uint32_t remaining_bytes = FLASH_SECTOR_SIZE - next_write_offset;
    return remaining_bytes / DATA_PACKET_SIZE;
}

// 格式化（擦除整个扇区）
void FlashStore_Format(void) {
    printf("Formatting flash sector...\n");

    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef erase_init;
    erase_init.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase_init.Sector = FLASH_USER_SECTOR;
    erase_init.NbSectors = 1;
    erase_init.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    erase_init.Banks = FLASH_BANK_1;

    uint32_t sector_error;
    HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&erase_init, &sector_error);

    HAL_FLASH_Lock();

    if (status != HAL_OK) {
        printf("Format failed: %d\n", status);
    } else {
        printf("Format successful\n");
    }

    // 重置状态
    current_version = 0;
    next_write_offset = 0;
}