/* MLX90640_I2C_Driver.c */

#include "bsp.h"
#include "MLX90640_I2C_Driver.h"
#include "stm32h7xx_hal.h"

extern I2C_HandleTypeDef I2cHandle; // 使用您提供的 I2C 句柄

/* 初始化 MLX90640 I2C 接口 */
void MLX90640_I2CInit(void)
{
    /* 调用本地驱动的初始化函数 */
    bsp_InitI2CBus();

    /* 修改 I2C 配置为主机模式 */
    I2cHandle.Init.OwnAddress1 = 0x00; // 主机模式下自地址可设为 0x00

    /* 重新初始化 I2C 外设 */
    if (HAL_I2C_Init(&I2cHandle) != HAL_OK)
    {
        /* 初始化错误处理 */
        Error_Handler(__FILE__, __LINE__);
    }
}

/* 发送 I2C 通用复位命令 */
int MLX90640_I2CGeneralReset(void)
{
    uint8_t cmd = 0x06; // 通用复位命令
    HAL_StatusTypeDef result;

    /* 发送通用调用复位命令 */
    result = HAL_I2C_Master_Transmit(&I2cHandle, 0x00, &cmd, 1, HAL_MAX_DELAY);
    if (result != HAL_OK)
    {
        return -1; // 发送失败
    }

    return 0; // 发送成功
}

/* 从 MLX90640 读取数据 */
int MLX90640_I2CRead(uint8_t slaveAddr, uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data)
{
    HAL_StatusTypeDef result;
    uint8_t i2cData[2];
    uint8_t rxBuffer[I2C_BUFFER_SIZE];

    /* 检查读取长度是否超过缓冲区大小 */
    if ((nMemAddressRead * 2) > I2C_BUFFER_SIZE)
    {
        return -1; // 缓冲区溢出
    }

    /* 准备要发送的起始地址（大端模式） */
    i2cData[0] = (startAddress >> 8) & 0xFF;
    i2cData[1] = startAddress & 0xFF;

    /* 发送起始地址 */
    result = HAL_I2C_Master_Transmit(&I2cHandle, slaveAddr << 1, i2cData, 2, HAL_MAX_DELAY);
    if (result != HAL_OK)
    {
        return -1; // 发送失败
    }

    /* 接收数据 */
    result = HAL_I2C_Master_Receive(&I2cHandle, slaveAddr << 1, rxBuffer, nMemAddressRead * 2, HAL_MAX_DELAY);
    if (result != HAL_OK)
    {
        return -1; // 接收失败
    }

    /* 将接收到的字节数据转换为 uint16_t 数组 */
    for (uint16_t i = 0; i < nMemAddressRead; i++)
    {
        data[i] = (rxBuffer[2 * i] << 8) | rxBuffer[2 * i + 1];
    }

    return 0; // 读取成功
}

/* 向 MLX90640 写入数据 */
int MLX90640_I2CWrite(uint8_t slaveAddr, uint16_t writeAddress, uint16_t data)
{
    HAL_StatusTypeDef result;
    uint8_t i2cData[4];

    /* 准备写入的地址和数据（大端模式） */
    i2cData[0] = (writeAddress >> 8) & 0xFF;
    i2cData[1] = writeAddress & 0xFF;
    i2cData[2] = (data >> 8) & 0xFF;
    i2cData[3] = data & 0xFF;

    /* 发送数据 */
    result = HAL_I2C_Master_Transmit(&I2cHandle, slaveAddr << 1, i2cData, 4, HAL_MAX_DELAY);
    if (result != HAL_OK)
    {
        return -1; // 发送失败
    }

    return 0; // 写入成功
}

/* 设置 I2C 通信频率 */
void MLX90640_I2CFreqSet(int freq)
{
    uint32_t timing;

    /* 根据频率选择合适的定时配置 */
    if (freq == 100000) // 100kHz
    {
        timing = 0x10D07DB5; // 示例值，请根据实际情况调整
    }
    else if (freq == 400000) // 400kHz
    {
        timing = 0x00C0216C; // 示例值，请根据实际情况调整
    }
    else
    {
        /* 不支持的频率 */
        return;
    }

    /* 更新 I2C 定时寄存器 */
    I2cHandle.Init.Timing = timing;

    /* 重新初始化 I2C 外设 */
    if (HAL_I2C_Init(&I2cHandle) != HAL_OK)
    {
        /* 初始化错误处理 */
        Error_Handler(__FILE__, __LINE__);
    }
}


