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
#ifndef COMPONENTS_DRIVERS_HAL_I2C_H_
#define COMPONENTS_DRIVERS_HAL_I2C_H_

extern bool hal_i2c_init();
extern uint8_t hal_i2c_read_byte(uint8_t master, uint8_t address);
extern void hal_i2c_read_bytes(uint8_t master,
                               uint8_t address,
                               uint8_t *data,
                               uint16_t len);
extern void hal_i2c_read_reg_bytes(uint8_t master,
                                   uint8_t address,
                                   uint8_t reg,
                                   uint8_t *data,
                                   uint16_t len);
/**
 * @brief Simple hardware abstraction for I2C on ESP32. This function reads a
 * single byte from a specific register of an I2C device.
 * @param master : ESP32 master number for the I2C bus to use
 * @param address : The 7-bit address of the I2C device to read
 * @param reg : The register in the device to read
 * @return The byte read from the register. Careful, default return value is 0
 * even if there is an error
 */
extern uint8_t hal_i2c_read_reg_byte(uint8_t master,
                                     uint8_t address,
                                     uint8_t reg);
extern bool hal_i2c_write_byte(uint8_t master, uint8_t address, uint8_t data);
extern bool hal_i2c_write_reg_byte(uint8_t master,
                                   uint8_t address,
                                   uint8_t reg,
                                   uint8_t data);
extern bool hal_i2c_write_bytes(uint8_t master,
                                uint8_t address,
                                uint8_t reg,
                                uint8_t *data,
                                size_t length);

#endif /* COMPONENTS_DRIVERS_HAL_I2C_H_ */
