/*****************************************************************************
 * Made with beer and late nights in California.
 *
 * (C) Copyright 2017-2018 AND!XOR LLC (http://andnxor.com/).
 *
 * PROPRIETARY AND CONFIDENTIAL UNTIL AUGUST 7th, 2018 then,
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ADDITIONALLY:
 * If you find this source code useful in anyway, use it in another electronic
 * conference badge, or just think it's neat. Consider buying us a beer
 * (or two) and/or a badge (or two). We are just as obsessed with collecting
 * badges as we are in making them.
 *
 * Contributors:
 * 	@andnxor
 * 	@zappbrandnxor
 * 	@hyr0n1
 * 	@exc3ls1or
 * 	@lacosteaef
 * 	@bitstr3m
 *****************************************************************************/
#ifndef COMPONENTS_DRIVERS_DRV_LIS2DE12_H_
#define COMPONENTS_DRIVERS_DRV_LIS2DE12_H_

#define LIS2DE12_REG_WHO_AM_I					0x0F
#define LIS2DE12_REG_WHO_AM_I_RESP				0x33

#define LIS2DE12_CTRL_REG1						0x20
#define LIS2DE12_CTRL_REG1_HR					0x80
#define LIS2DE12_CTRL_REG1_ODR_OFF				0x00
#define LIS2DE12_CTRL_REG1_ODR_1HZ				0x10
#define LIS2DE12_CTRL_REG1_ODR_10HZ				0x20
#define LIS2DE12_CTRL_REG1_ODR_25HZ				0x30
#define LIS2DE12_CTRL_REG1_ODR_50HZ				0x40
#define LIS2DE12_CTRL_REG1_ODR_400HZ			0x70
#define LIS2DE12_CTRL_REG1_LPEN					0x08
#define LIS2DE12_CTRL_REG1_Z_EN					0x04
#define LIS2DE12_CTRL_REG1_Y_EN					0x02
#define LIS2DE12_CTRL_REG1_X_EN					0x01

#define LIS2DE12_CTRL_REG2						0x21
#define LIS2DE12_CTRL_REG2_HPM_NORMAL			0b00000000
#define LIS2DE12_CTRL_REG2_HPM_RESET			0b01000000
#define LIS2DE12_CTRL_REG2_HPM_NORMAL2			0b10000000
#define LIS2DE12_CTRL_REG2_HPM_AUTORESET		0b11000000
#define LIS2DE12_CTRL_REG2_HP_IA1				0b00000001

#define LIS2DE12_CTRL_REG3						0x22
#define LIS2DE12_CTRL_REG3_I1_CLICK				0b10000000
#define LIS2DE12_CTRL_REG3_I1_IA1				0b01000000
#define LIS2DE12_CTRL_REG3_I1_IA2				0b00100000

#define LIS2DE12_REG_CTRL4						0x23
#define LIS2DE12_REG_CTRL4_BDU					0x80
#define LIS2DE12_REG_CTRL4_FS_2G				0x00
#define LIS2DE12_REG_CTRL4_FS_4G				0x10
#define LIS2DE12_REG_CTRL4_FS_8G				0x20
#define LIS2DE12_REG_CTRL4_FS_16G				0x30

#define LIS2DE12_REG_CTRL5						0x24
#define LIS2DE12_REG_CTRL5_RESET				0x40

#define LIS2DE12_REG_X_OUT_L					0x28
#define LIS2DE12_REG_X_OUT_H					0x29
#define LIS2DE12_REG_Y_OUT_L					0x2A
#define LIS2DE12_REG_Y_OUT_H					0x2B
#define LIS2DE12_REG_Z_OUT_L					0x2C
#define LIS2DE12_REG_Z_OUT_H					0x2D

#define LIS2DE12_FIFO_CTRL_REG					0x2E
#define LIS2DE12_FIFO_CTRL_REG_BYPASS			0b00000000
#define LIS2DE12_FIFO_CTRL_REG_FIFO				0b01000000
#define LIS2DE12_FIFO_CTRL_REG_STREAM			0b10000000
#define LIS2DE12_FIFO_CTRL_REG_STREAM_TO_FIFO	0b11000000
#define LIS2DE12_FIFO_CTRL_REG_TR_INT1			0b00000000
#define LIS2DE12_FIFO_CTRL_REG_TR_INT2			0b00100000

#define LIS2DE12_REG_INT1_CFG					0x30
#define LIS2DE12_REG_INT1_THS					0x32
#define LIS2DE12_REG_INT1_DURATION				0x33

#define LIS2DE12_RANGE_DIVIDER_2G				16380
#define LIS2DE12_RANGE_DIVIDER_4G				8190
#define LIS2DE12_RANGE_DIVIDER_8G				4096
#define LIS2DE12_RANGE_DIVIDER_16G				1365

typedef struct {
	int8_t x, y, z;
} accel_axis_t;

extern bool drv_lis2de12_init();

/**
 * Get latest accelerometer data
 *
 * @return accel_axis_t Struct storing x, y, and z data
 */
extern accel_axis_t drv_lis2de12_get();

/**
 * Get the current tilt angle along X/Y axis. 180 is down
 *
 * @return Tilt angle of X/Y vector in degrees. Down is 180
 */
extern uint16_t drv_lis2de12_tilt_get();

/**
 * Start the accelerometer polling task
 */
extern void drv_lis2de12_poll_task_start();

#endif /* COMPONENTS_DRIVERS_DRV_LIS2DE12_H_ */
