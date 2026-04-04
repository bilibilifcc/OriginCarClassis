/**
  * @file    fdi_host_config.c
  * @brief   FDI设备配置模式主机发送端程序源文件
  */

#include "fdi_host_config.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

/* 缓冲区大小定义 */
#define RX_BUF_SIZE        512
#define RESPONSE_BUF_SIZE  256

/* 全局变量 */
static uint8_t g_rx_buffer[RX_BUF_SIZE];
static uint16_t g_rx_index = 0;
static uint8_t g_rx_complete = 0;

/* 静态函数声明 */
static void Serial_SendByte(uint8_t data);
static void Serial_SendString(char* str);
static void ClearRxBuffer(void);
static CMD_Result_t SendCommandAndWait(char* cmd, uint8_t* response, uint16_t* len, uint32_t timeout_ms);
static uint32_t GetTickCount(void);

/**
  * @brief  单字节解析接口（预留）
  */
void FDI_Host_Parse_Byte(uint8_t data)
{
    if (g_rx_index < RX_BUF_SIZE - 1)
    {
        g_rx_buffer[g_rx_index++] = data;
        
        if (data == '\n' || data == '\r')
        {
            g_rx_buffer[g_rx_index] = '\0';
            g_rx_complete = 1;
        }
    }
}

/**
  * @brief  发送进入配置模式命令
  */
CMD_Result_t FDI_EnterConfigMode(void)
{
    uint8_t response[RESPONSE_BUF_SIZE];
    uint16_t len = 0;
    CMD_Result_t result;
    
    result = SendCommandAndWait("#fconfig\r\n", response, &len, 1000);
    
    if (result == CMD_SUCCESS)
    {
        if (strstr((char*)response, "*#OK") != NULL)
        {
            return CMD_SUCCESS;
        }
    }
    
    return CMD_ERROR;
}

/**
  * @brief  发送退出配置模式命令
  */
CMD_Result_t FDI_ExitConfigMode(void)
{
    uint8_t response[RESPONSE_BUF_SIZE];
    uint16_t len = 0;
    CMD_Result_t result;
    
    result = SendCommandAndWait("#fdeconfig\r\n", response, &len, 2000);
    
    if (result == CMD_SUCCESS)
    {
        if (strstr((char*)response, "*#OK") != NULL)
        {
            return CMD_SUCCESS;
        }
    }
    
    return CMD_ERROR;
}

/**
  * @brief  发送查询安装方向命令
  */
CMD_Result_t FDI_GetAxis(float* axis_x, float* axis_y, float* axis_z)
{
    uint8_t response[RESPONSE_BUF_SIZE];
    uint16_t len = 0;
    CMD_Result_t result;
    
    result = SendCommandAndWait("#faxis\r\n", response, &len, 2000);
    
    if (result == CMD_SUCCESS)
    {
        char* p = (char*)response;
        float x = 0, y = 0, z = 0;
        
        sscanf(p, "X:%f Y:%f Z:%f", &x, &y, &z);
        
        if (axis_x) *axis_x = x;
        if (axis_y) *axis_y = y;
        if (axis_z) *axis_z = z;
        
        return CMD_SUCCESS;
    }
    
    return CMD_ERROR;
}

/**
  * @brief  发送设置安装方向命令
  */
CMD_Result_t FDI_SetAxis(char axis, float angle)
{
    uint8_t response[RESPONSE_BUF_SIZE];
    uint16_t len = 0;
    char cmd[32];
		CMD_Result_t result;
    
    sprintf(cmd, "#faxis %c %.1f\r\n", axis, angle);
    result = SendCommandAndWait(cmd, response, &len, 2000);
    
    if (result == CMD_SUCCESS)
    {
        if (strstr((char*)response, "*#OK") != NULL)
        {
            return CMD_SUCCESS;
        }
    }
    
    return CMD_ERROR;
}

/**
  * @brief  发送保存配置命令
  */
CMD_Result_t FDI_SaveConfig(void)
{
    uint8_t response[RESPONSE_BUF_SIZE];
    uint16_t len = 0;
    
    CMD_Result_t result = SendCommandAndWait("#fsave\r\n", response, &len, 1000);
    
    if (result == CMD_SUCCESS)
    {
        if (strstr((char*)response, "*#OK") != NULL)
        {
            return CMD_SUCCESS;
        }
    }
    
    return CMD_ERROR;
}

/**
  * @brief  发送重启命令
  */
CMD_Result_t FDI_Reboot(uint8_t confirm)
{
    uint8_t response[RESPONSE_BUF_SIZE];
    uint16_t len = 0;
    
    CMD_Result_t result = SendCommandAndWait("#freboot\r\n", response, &len, 2000);
    
    if (result == CMD_SUCCESS)
    {
        if (strstr((char*)response, "y/n") != NULL || strstr((char*)response, "?") != NULL)
        {
            if (confirm)
            {
                result = SendCommandAndWait("y\r\n", response, &len, 3000);
                if (result == CMD_SUCCESS)
                {
                    return CMD_SUCCESS;
                }
            }
            else
            {
                SendCommandAndWait("n\r\n", response, &len, 1000);
                return CMD_WAIT_CONFIRM;
            }
        }
        else if (strstr((char*)response, "*#OK") != NULL)
        {
            return CMD_SUCCESS;
        }
    }
    
    return CMD_ERROR;
}

/**
  * @brief  发送恢复出厂设置命令
  */
CMD_Result_t FDI_ResetToFactory(uint8_t confirm)
{
    uint8_t response[RESPONSE_BUF_SIZE];
    uint16_t len = 0;
    
    CMD_Result_t result = SendCommandAndWait("#freset\r\n", response, &len, 2000);
    
    if (result == CMD_SUCCESS)
    {
        if (strstr((char*)response, "y/n") != NULL || strstr((char*)response, "?") != NULL)
        {
            if (confirm)
            {
                result = SendCommandAndWait("y\r\n", response, &len, 3000);
                if (result == CMD_SUCCESS)
                {
                    return CMD_SUCCESS;
                }
            }
            else
            {
                SendCommandAndWait("n\r\n", response, &len, 1000);
                return CMD_WAIT_CONFIRM;
            }
        }
    }
    
    return CMD_ERROR;
}

/**
  * @brief  设置数据包发送频率
  */
CMD_Result_t FDI_SetMsgFrequency(uint8_t packet_id, float freq)
{
    uint8_t response[RESPONSE_BUF_SIZE];
    uint16_t len = 0;
    char cmd[32];
		CMD_Result_t result;
    
    sprintf(cmd, "#fmsg %02X %.1f\r\n", packet_id, freq);
    
    result = SendCommandAndWait(cmd, response, &len, 2000);
    
    if (result == CMD_SUCCESS)
    {
        if (strstr((char*)response, "*#OK") != NULL)
        {
            return CMD_SUCCESS;
        }
    }
    
    return CMD_ERROR;
}

/**
  * @brief  查询数据包配置
  */
CMD_Result_t FDI_GetMsgConfig(uint8_t* output_buffer, uint16_t buf_size)
{
    uint8_t response[RESPONSE_BUF_SIZE];
    uint16_t len = 0;
    
    CMD_Result_t result = SendCommandAndWait("#fmsg\r\n", response, &len, 2000);
    
    if (result == CMD_SUCCESS)
    {
        if (output_buffer != NULL && buf_size > 0)
        {
            uint16_t copy_len = (len < buf_size) ? len : buf_size - 1;
            memcpy(output_buffer, response, copy_len);
            output_buffer[copy_len] = '\0';
        }
        return CMD_SUCCESS;
    }
    
    return CMD_ERROR;
}

/**
  * @brief  获取参数值
  */
CMD_Result_t FDI_GetParam(char* param_name, float* value)
{
    uint8_t response[RESPONSE_BUF_SIZE];
    uint16_t len = 0;
    char cmd[64];
		CMD_Result_t result;
    
    sprintf(cmd, "#fparam get %s\r\n", param_name);
    
    result = SendCommandAndWait(cmd, response, &len, 2000);
    
    if (result == CMD_SUCCESS)
    {
        char* p = strchr((char*)response, '=');
        if (p != NULL && value != NULL)
        {
            *value = atof(p + 1);
            return CMD_SUCCESS;
        }
    }
    
    return CMD_ERROR;
}

/**
  * @brief  设置参数值
  */
CMD_Result_t FDI_SetParam(char* param_name, float value)
{
    uint8_t response[RESPONSE_BUF_SIZE];
    uint16_t len = 0;
    char cmd[64];
		CMD_Result_t result;
    
    sprintf(cmd, "#fparam set %s %.6f\r\n", param_name, value);
    
    result = SendCommandAndWait(cmd, response, &len, 2000);
    
    if (result == CMD_SUCCESS)
    {
        if (strstr((char*)response, "*#OK") != NULL)
        {
            return CMD_SUCCESS;
        }
    }
    
    return CMD_ERROR;
}

/**
  * @brief  设置天线相关参数
  */
CMD_Result_t FDI_SetAntenna(float head_bias, float baseline, float arm_x, float arm_y, float arm_z)
{
    uint8_t response[RESPONSE_BUF_SIZE];
    uint16_t len = 0;
    char cmd[128];
    CMD_Result_t result;
    
    if (head_bias >= 0)
    {
        sprintf(cmd, "#fanteheadbias %.1f\r\n", head_bias);
        result = SendCommandAndWait(cmd, response, &len, 2000);
        if (result != CMD_SUCCESS || strstr((char*)response, "*#OK") == NULL)
        {
            return CMD_ERROR;
        }
    }
    
    if (baseline > 0)
    {
        sprintf(cmd, "#fantebaseline %.3f\r\n", baseline);
        result = SendCommandAndWait(cmd, response, &len, 2000);
        if (result != CMD_SUCCESS || strstr((char*)response, "*#OK") == NULL)
        {
            return CMD_ERROR;
        }
    }
    
    sprintf(cmd, "#fantearm %.3f %.3f %.3f\r\n", arm_x, arm_y, arm_z);
    result = SendCommandAndWait(cmd, response, &len, 2000);
    
    if (result == CMD_SUCCESS && strstr((char*)response, "*#OK") != NULL)
    {
        return CMD_SUCCESS;
    }
    
    return CMD_ERROR;
}

/**
  * @brief  发送自定义命令
  */
CMD_Result_t FDI_SendCustomCommand(char* cmd, uint8_t* response, uint16_t* response_len, uint32_t timeout_ms)
{
    return SendCommandAndWait(cmd, response, response_len, timeout_ms);
}

/**
  * @brief  发送命令并等待响应
  */
static CMD_Result_t SendCommandAndWait(char* cmd, uint8_t* response, uint16_t* len, uint32_t timeout_ms)
{
		uint32_t start_time;
    g_rx_complete = 0;
    g_rx_index = 0;
		
		// ClearRxBuffer();
    Serial_SendString(cmd);
    
		delay_ms(timeout_ms);
    
    return CMD_TIMEOUT;
}

/**
  * @brief  上电自动校准（需设备保持水平静止）
  * @return 执行结果
  */
CMD_Result_t FDI_AutoCalibration(void)
{
    uint8_t response[RESPONSE_BUF_SIZE];
    uint16_t len = 0;
    CMD_Result_t result;
    
    /* 1. 进入配置模式 */
    result = SendCommandAndWait("#fconfig\r\n", response, &len, 1000);
    
    /* 2. 执行调平校准（需水平静止） */
    result = SendCommandAndWait("#fimucal_level\r\n", response, &len, 1000);
    
    /* 3. 执行加速度计零偏校准（需水平静止） */
    result = SendCommandAndWait("#fimucal_acec\r\n", response, &len, 1000);
    
    /* 4. 执行陀螺仪零偏校准（需静止） */
    result = SendCommandAndWait("#fimucal_gyro\r\n", response, &len, 1000);
    
    /* 5. 保存配置 */
    result = SendCommandAndWait("#fsave\r\n", response, &len, 1000);
    /* 6. 重启设备 */
    result = SendCommandAndWait("#freboot\r\n", response, &len, 1000);
    
    return result;
}

/**
  * @brief  清空接收缓冲区
  */
static void ClearRxBuffer(void)
{
    g_rx_index = 0;
    g_rx_complete = 0;
    memset(g_rx_buffer, 0, RX_BUF_SIZE);
}

/**
  * @brief  串口发送单字节
  */
static void Serial_SendByte(uint8_t data)
{		while( (USART_GetFlagStatus(UART5, USART_FLAG_TXE) == RESET) && (USART_GetFlagStatus(UART5, USART_FLAG_TC) == RESET) ) //  判断忙
    USART_SendData(UART5, data);
    while (USART_GetFlagStatus(UART5, USART_FLAG_TXE) == RESET);
}

/**
  * @brief  串口发送字符串
  */
static void Serial_SendString(char* str)
{
    while (*str)
    {
        Serial_SendByte((uint8_t)*str++);
    }
}


/**
  * @brief  获取系统运行时间
  */
static uint32_t GetTickCount(void)
{
    static uint32_t tick = 0;
    tick++;
    return tick;
}