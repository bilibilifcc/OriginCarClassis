/**
  * @file    fdi_host_config.h
  * @brief   FDI设备配置模式主机发送端程序头文件
  */

#ifndef __FDI_HOST_CONFIG_H
#define __FDI_HOST_CONFIG_H

#include "system.h"

/* 命令执行结果 */
typedef enum {
    CMD_SUCCESS = 0,
    CMD_TIMEOUT,
    CMD_ERROR,
    CMD_WAIT_CONFIRM
} CMD_Result_t;

/* 初始化函数 */
void FDI_Host_Init(uint32_t baudrate);

/* 单字节解析接口（预留） */
void FDI_Host_Parse_Byte(uint8_t data);

/* 配置模式控制 */
CMD_Result_t FDI_EnterConfigMode(void);
CMD_Result_t FDI_ExitConfigMode(void);

/* 安装方向配置 */
CMD_Result_t FDI_GetAxis(float* axis_x, float* axis_y, float* axis_z);
CMD_Result_t FDI_SetAxis(char axis, float angle);

/* 配置保存与恢复 */
CMD_Result_t FDI_SaveConfig(void);
CMD_Result_t FDI_Reboot(uint8_t confirm);
CMD_Result_t FDI_ResetToFactory(uint8_t confirm);

/* 数据包配置 */
CMD_Result_t FDI_SetMsgFrequency(uint8_t packet_id, float freq);
CMD_Result_t FDI_GetMsgConfig(uint8_t* output_buffer, uint16_t buf_size);

/* 参数配置 */
CMD_Result_t FDI_GetParam(char* param_name, float* value);
CMD_Result_t FDI_SetParam(char* param_name, float value);

/* 天线配置 */
CMD_Result_t FDI_SetAntenna(float head_bias, float baseline, float arm_x, float arm_y, float arm_z);

/* 自定义命令 */
CMD_Result_t FDI_SendCustomCommand(char* cmd, uint8_t* response, uint16_t* response_len, uint32_t timeout_ms);

/* 上电自校准 */
CMD_Result_t FDI_AutoCalibration(void);

#endif /* __FDI_HOST_CONFIG_H */