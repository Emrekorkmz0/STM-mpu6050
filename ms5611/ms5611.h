/*
 * ms5611.h
 *
 *  Created on: 2020年6月17日
 *      Author: yuche
 */
#include <stdint.h>
#include "main.h"

#ifndef COMPONENTS_INC_MS5611_H_
#define COMPONENTS_INC_MS5611_H_

#define MS5611_I2C_ADDR			0x77	//when pin csb is low
#define MS5611_I2C_ADDR2		0x76	//when pin csb is high

#define MS5611_CMD_ADC_READ		0x00
#define MS5611_CMD_CONVERT_D1	0x40	//convert pressure
#define MS5611_CMD_CONVERT_D2	0x50	//convert temperature
#define MS5611_CMD_RESET		0x1E
#define MS5611_CMD_READ_PROM	0xA2	//6 calibration values are stored from 0xA2 to 0xAC with interval 2

enum MS5611_OSR {
	MS5611_OSR_256 = 0,
	MS5611_OSR_512,
	MS5611_OSR_1024,
	MS5611_OSR_2048,
	MS5611_OSR_4096,
};
typedef struct{
	uint32_t D1;
	uint32_t D2;
	uint16_t AC1;
	uint16_t AC2;
	uint16_t AC3;
	uint16_t AC4;
	uint16_t AC5;
	uint16_t AC6;
}params;


//I2C_HandleTypeDef* ms5611_i2cx;

void ms5611_set_i2c(I2C_HandleTypeDef hi2c1;);
void ms5611_init();
int64_t ms5611_press();
void ms5611_osr_select(enum MS5611_OSR osr);

void ms5611_start();
void read_adc_data();
void read_prom_Data();

void ms5611_update_pressure();
void ms5611_update_temperature();

void ms5611_update();

int64_t ms5611_get_temperature();
int64_t ms5611_get_pressure();

#endif /* COMPONENTS_INC_MS5611_H_ */
