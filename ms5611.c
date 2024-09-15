/*
 * ms5611.c
 *
 *	Datasheet: https://www.te.com/commerce/DocumentDelivery/DDEController?Action=showdoc&DocId=Data+Sheet%7FMS5611-01BA03%7FB3%7Fpdf%7FEnglish%7FENG_DS_MS5611-01BA03_B3.pdf%7FCAT-BLPS0036
 *  Created on: 2020年6月17日
 *      Author: yuche
 */

#include "ms5611.h"


#define NUM_CALIBRATION_DATA 6
#define atmPress 101325
uint16_t fc[NUM_CALIBRATION_DATA]; // 6 factory calibration data
uint32_t raw_pressure, raw_temperature;
enum MS5611_OSR selected_osr = MS5611_OSR_256;
double pressure;
params param;


void ms5611_osr_select(enum MS5611_OSR osr){
	selected_osr = osr;
}

void read_prom_Data(){
	uint32_t data_D1[2];
	uint32_t data_D2[2];
	HAL_I2C_Mem_Read(&hi2c1,MS5611_I2C_ADDR,0x40,1, data_D1, 2, HAL_MAX_DELAY);
	HAL_I2C_Mem_Read(&hi2c1,MS5611_I2C_ADDR,0x50,1, data_D2, 2, HAL_MAX_DELAY);
	param.D1 =((data_D1[1]<<8) | data_D1[0]);
	param.D2=((data_D2[1]<<8)| data_D2[0]);
}
void read_adc_data(){
	int16_t data[12];

	HAL_I2C_Mem_Read(&hi2c1, MS5611_I2C_ADDR, 0xA0, 1, data, 12, HAL_MAX_DELAY);
	param.AC1=((data[1]<<8)| data[0]);
	param.AC2=((data[3]<<8)| data[2]);
	param.AC3=((data[5]<<8)| data[4]);
	param.AC4=((data[7]<<8)| data[6]);
	param.AC5=((data[9]<<8)| data[8]);
	param.AC6=((data[11]<<8)| data[10]);
	//HAL_Delay(0.5);

}
void ms5611_start(){
	read_adc_data();
	read_prom_Data();
}

/**
 *	Read raw temperature and pressure from MS5611
 */
void ms5611_update(){
	ms5611_update_temperature();
	ms5611_update_pressure();
}


/**
 * Read raw pressure from MS5611.
 */
void ms5611_update_pressure(){
	uint8_t buffer[3] = {0x00,0x00,0x00};

	//HAL_I2C_Mem_Write(&hi2c1, MS5611_I2C_ADDR, 0x40, 1, buffer, 0, HAL_MAX_DELAY);

	HAL_Delay(12);//time delay necessary for ADC to convert, must be >= 9.02ms

	HAL_I2C_Mem_Read(&hi2c1, MS5611_I2C_ADDR, 0x40, 1, buffer, 3, HAL_MAX_DELAY);

	raw_pressure = ((uint32_t)buffer[0] << 16) | ((uint32_t)buffer[1] << 8) | ((uint32_t)buffer[2]);

}

/**
 * Read raw temperature from MS5611.
 */
void ms5611_update_temperature(){
	uint8_t buffer[3] = {0x00,0x00,0x00};

	//HAL_I2C_Mem_Write(&hi2c1, MS5611_I2C_ADDR, 0x50, 1, buffer, 0, HAL_MAX_DELAY);

	HAL_Delay(12);//time delay necessary for ADC to convert, must be >= 9.02ms

	HAL_I2C_Mem_Read(&hi2c1, MS5611_I2C_ADDR, 0x50, 1, buffer, 3, HAL_MAX_DELAY);
	raw_temperature = ((uint32_t)buffer[0] << 16) | ((uint32_t)buffer[1] << 8) | ((uint32_t)buffer[2]);
}



/**
 * Get calibrated temperature, unit: Celsius degrees
 * @return calibrated temperature
 */
int64_t ms5611_get_temperature(){
	//int32_t dT = raw_temperature - ((uint32_t)param.AC5 * 256);
	uint32_t dT=param.D2-(param.AC5*pow(2,8));
	double TEMP = 2000.0 + dT * (param.AC6/ (8388608.0));//unit 0.01 C

	double T2=0;
	if (TEMP < 2000){
		//temperature < 20 Celsius
		T2 = dT * (dT / (2147483648.0));
	}

	TEMP = TEMP - T2;
	return TEMP / 100;
}

/**
 * Get calibrated pressure, unit: mBar
 * @return calibrated pressure
 */
int64_t ms5611_get_pressure(){

	//uint32_t dT = raw_temperature - ((uint32_t)param.AC5 * 256);
	uint32_t dT=param.D2-(param.AC5*pow(2,8));
	double TEMP = 2000.0 + dT * (param.AC6 / (8388608.0));//unit 0.01 C

	double OFF =  param.AC2* (65536) + param.AC4 * dT / (128);
	double SENS = param.AC1* (32768) + param.AC3 * dT / (256);

	double P = (raw_pressure * SENS / (2097152.0) - OFF) / (32768);//unit 0.01mbar

	double T2=0, OFF2=0, SENS2=0;
	if (TEMP < 2000){
		//temperature < 20 Celsius
		T2 = dT * dT / (2147483648);
		OFF2 = 5 * (TEMP-2000) * (TEMP-2000) / 2;
		SENS2 = 5 * (TEMP-2000) * (TEMP-2000) / 4;

		if (TEMP < -1500){
			//temperature < -15 Celsius
			OFF2 = OFF2 + 7 * (TEMP + 1500) * (TEMP + 1500);
			SENS2 = SENS2 + 11/2 * (TEMP + 1500) * (TEMP + 1500);
		}
	}

	TEMP = TEMP - T2;
	OFF = OFF - OFF2;
	SENS = SENS - SENS2;

	P = (raw_pressure * SENS / (2097152.0) - OFF) / (32768);//unit mbar
	return P;//unit mbar
}

