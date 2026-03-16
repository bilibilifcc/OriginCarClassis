#ifndef __N100_H__
#define __N100_H__

#include "sys.h"

//FDlink candata
// #define FRAME_HEADER      0X7B //Frame_header //??
// #define FRAME_TAIL        0X7D //Frame_tail   //?β

#define FRAME_HEAD 0xfc
#define FRAME_END 0xfd
#define TYPE_IMU 0x40
#define TYPE_AHRS 0x41
#define IMU_LEN  0x38   //56+8  8组数据
#define AHRS_LEN 0x30   //48+8  7组数据

// #define SEND_DATA_SIZE    24
#define RECEIVE_DATA_SIZE 11
#define IMU_RS 64
#define AHRS_RS 56
// #define INSGPS_RS 80

typedef struct IMUData_Packet_t{
		float gyroscope_x;          //unit: rad/s
		float gyroscope_y;          //unit: rad/s
		float gyroscope_z;          //unit: rad/s
		float accelerometer_x;      //m/s^2
		float accelerometer_y;      //m/s^2
		float accelerometer_z;      //m/s^2
		float magnetometer_x;       //mG
		float magnetometer_y;       //mG
		float magnetometer_z;       //mG
		float imu_temperature;      //C
		float Pressure;             //Pa
		float pressure_temperature; //C
		u32 Timestamp;          //us
} IMUData_Packet_t;

typedef struct AHRSData_Packet_t
{
	float RollSpeed;   //unit: rad/s
	float PitchSpeed;  //unit: rad/s
	float HeadingSpeed;//unit: rad/s
	float Roll;        //unit: rad
	float Pitch;       //unit: rad
	float Heading;     //unit: rad
	float Qw;//w          //Quaternion
	float Qx;//x
	float Qy;//y
	float Qz;//z
	long long Timestamp; //unit: us
}AHRSData_Packet_t;

u8 TTL_Hex2Dec(void);

extern u8 ttl_receive;
extern u8 Fd_data[64];
extern u8 Fd_rsimu[64];
extern u8 Fd_rsahrs[56];
extern int rs_imutype;
extern int rs_ahrstype;



#endif // __N100_H__

