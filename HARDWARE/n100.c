#include "n100.h"
#include "usartx.h"

u8 ttl_receive;
u8 Fd_data[64];
u8 Fd_rsimu[64];
u8 Fd_rsahrs[56];
int rs_imutype =0;
int rs_ahrstype =0;

AHRSData_Packet_t AHRSData_Packet;
IMUData_Packet_t IMUData_Packet;

float DATA_Trans(u8 Data_1,u8 Data_2,u8 Data_3,u8 Data_4)
{
  u32 transition_32;
	float tmp=0;
	int sign=0;
	int exponent=0;
	float mantissa=0;
  transition_32 = 0;
  transition_32 |=  Data_4<<24;   
  transition_32 |=  Data_3<<16; 
	transition_32 |=  Data_2<<8;
	transition_32 |=  Data_1;
  sign = (transition_32 & 0x80000000) ? -1 : 1;//符号位
	//先右移操作，再按位与计算，出来结果是30到23位对应的e
	exponent = ((transition_32 >> 23) & 0xff) - 127;
	//将22~0转化为10进制，得到对应的x系数 
	mantissa = 1 + ((float)(transition_32 & 0x7fffff) / 0x7fffff);
	tmp=sign * mantissa * pow(2, exponent);
	return tmp;
}

// 解析IMU数据
u8 TTL_Hex2Dec(void)  
{
	if(rs_ahrstype==1)
	{
		if(Fd_rsahrs[1]==TYPE_AHRS&&Fd_rsahrs[2]==AHRS_LEN)
		{	
			//AHRSData_Packet.RollSpeed=DATA_Trans(Fd_rsahrs[7],Fd_rsahrs[8],Fd_rsahrs[9],Fd_rsahrs[10]);       //横滚角速度
			//AHRSData_Packet.PitchSpeed=DATA_Trans(Fd_rsahrs[11],Fd_rsahrs[12],Fd_rsahrs[13],Fd_rsahrs[14]);   //俯仰角速度
			//AHRSData_Packet.HeadingSpeed=DATA_Trans(Fd_rsahrs[15],Fd_rsahrs[16],Fd_rsahrs[17],Fd_rsahrs[18]); //偏航角速度
				
			//AHRSData_Packet.Roll=DATA_Trans(Fd_rsahrs[19],Fd_rsahrs[20],Fd_rsahrs[21],Fd_rsahrs[22]);      //横滚角
			//AHRSData_Packet.Pitch=DATA_Trans(Fd_rsahrs[23],Fd_rsahrs[24],Fd_rsahrs[25],Fd_rsahrs[26]);     //俯仰角
			//AHRSData_Packet.Heading=DATA_Trans(Fd_rsahrs[27],Fd_rsahrs[28],Fd_rsahrs[29],Fd_rsahrs[30]);	 //偏航角
			if(xSemaphoreTake(N100DataSem, portMAX_DELAY) == pdPASS)
			{
				Send_Data.Sensor_Str.N100_all.q_W_data=DATA_Trans(Fd_rsahrs[31],Fd_rsahrs[32],Fd_rsahrs[33],Fd_rsahrs[34]);  //四元数
				Send_Data.Sensor_Str.N100_all.q_X_data=DATA_Trans(Fd_rsahrs[35],Fd_rsahrs[36],Fd_rsahrs[37],Fd_rsahrs[38]);
				Send_Data.Sensor_Str.N100_all.q_Y_data=DATA_Trans(Fd_rsahrs[39],Fd_rsahrs[40],Fd_rsahrs[41],Fd_rsahrs[42]);
				Send_Data.Sensor_Str.N100_all.q_Z_data=DATA_Trans(Fd_rsahrs[43],Fd_rsahrs[44],Fd_rsahrs[45],Fd_rsahrs[46]);
				xSemaphoreGive(N100DataSem);
				return 1;
			}
			//AHRSData_Packet.Timestamp=timestamp(Fd_rsahrs[47],Fd_rsahrs[48],Fd_rsahrs[49],Fd_rsahrs[50],Fd_rsahrs[51],Fd_rsahrs[52],Fd_rsahrs[53],Fd_rsahrs[54]);   //时间戳
			// AHRSData2PC();
		}
		rs_ahrstype=0;
	}
	if(rs_imutype==1)
	{
		if(Fd_rsimu[1]==TYPE_IMU&&Fd_rsimu[2]==IMU_LEN)
		{
			
			if(xSemaphoreTake(N100DataSem, portMAX_DELAY) == pdPASS)
			{
				Send_Data.Sensor_Str.N100_all.gyro_X_data=DATA_Trans(Fd_rsimu[7],Fd_rsimu[8],Fd_rsimu[9],Fd_rsimu[10]);  //角速度
				Send_Data.Sensor_Str.N100_all.gyro_Y_data=-DATA_Trans(Fd_rsimu[11],Fd_rsimu[12],Fd_rsimu[13],Fd_rsimu[14]);
				Send_Data.Sensor_Str.N100_all.gyro_Z_data=DATA_Trans(Fd_rsimu[15],Fd_rsimu[16],Fd_rsimu[17],Fd_rsimu[18]);
					
				Send_Data.Sensor_Str.N100_all.acc_X_data=DATA_Trans(Fd_rsimu[19],Fd_rsimu[20],Fd_rsimu[21],Fd_rsimu[22]);  //线加速度
				Send_Data.Sensor_Str.N100_all.acc_Y_data=-DATA_Trans(Fd_rsimu[23],Fd_rsimu[24],Fd_rsimu[25],Fd_rsimu[26]);
				Send_Data.Sensor_Str.N100_all.acc_Z_data=DATA_Trans(Fd_rsimu[27],Fd_rsimu[28],Fd_rsimu[29],Fd_rsimu[30]);
				xSemaphoreGive(N100DataSem);
			}
		// IMUData_Packet.magnetometer_x=DATA_Trans(Fd_rsimu[31],Fd_rsimu[32],Fd_rsimu[33],Fd_rsimu[34]);  //磁力计数据
		// IMUData_Packet.magnetometer_y=DATA_Trans(Fd_rsimu[35],Fd_rsimu[36],Fd_rsimu[37],Fd_rsimu[38]);
		// IMUData_Packet.magnetometer_z=DATA_Trans(Fd_rsimu[39],Fd_rsimu[40],Fd_rsimu[41],Fd_rsimu[42]);
			
		// IMUData_Packet.Timestamp=timestamp(Fd_rsimu[55],Fd_rsimu[56],Fd_rsimu[57],Fd_rsimu[58],Fd_rsimu[59],Fd_rsimu[60],Fd_rsimu[61],Fd_rsimu[62]);   //时间戳
		// IMUData2PC();
		}
		rs_imutype=0;
	}
	return 0;
}

