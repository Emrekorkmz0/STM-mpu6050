//define parameters are below
#define MPU6050_ADDR 0x68<<1
#define PWR_MGMT_1_REG 0x6B
#define SMPLRT_DIV_REG 0x19
#define GYRO_CNFG_REG 0x1B
#define ACC_CNFG_REG 0x1C
/////////

//global variables are below
uint8_t data;
uint8_t buffer[2],tuffer[6],cuffer[6];
int16_t gyro_raw[3],acc_raw[3];
float gyro_cal[3]; //calibrasyon ofsetleri tutar
int16_t acc_total_vector;
float angle_pitch_gyro,angle_roll_gyro;
float angle_pitch_acc,angle_roll_acc;
float angle_pitch,angle_roll;
int16_t raw_temp;
int i;
float temp;
float prevtime,prevtime1,time1,elepsedtime1,prevtime2,time2,elepsedtime2;
HAL_StatusTypeDef set_gyro;



//////////////////////////////////

  data=0x00;
  HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, PWR_MGMT_1_REG, 1, &data,1,HAL_MAX_DELAY);

  data=0x08;
  HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, GYRO_CNFG_REG, 1, &data,1,HAL_MAX_DELAY);

  data=0x10;
  HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, ACC_CNFG_REG, 1, &data,1,HAL_MAX_DELAY);

  for(i=0;i<200;i++){
  	  prevtime2=time2;
  	  time2=HAL_GetTick();


  	  cuffer[0]=0x43;
  	  HAL_I2C_Master_Transmit(&hi2c1, MPU6050_ADDR,cuffer ,1 ,HAL_MAX_DELAY );
  	  HAL_I2C_Master_Receive(&hi2c1, MPU6050_ADDR, cuffer, 6, HAL_MAX_DELAY);

  	  gyro_raw[0]=(cuffer[0]<<8 | cuffer[1]);
  	  gyro_raw[1]=(cuffer[2]<<8 | cuffer[3]);
  	  gyro_raw[2]=(cuffer[4]<<8 | cuffer[5]);

  	  gyro_cal[0]+=gyro_raw[0];
  	  gyro_cal[1]+=gyro_raw[1];
  	  gyro_cal[2]+=gyro_raw[2];

  	  HAL_Delay(3);
    }

    gyro_cal[0]/=2000;
    gyro_cal[1]/=2000;
    gyro_cal[2]/=2000;

    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    HAL_Delay(1000);



  while (1)
  {

	  
	  HAL_Delay(1000);

	  prevtime1=time1;
	  time1=HAL_GetTick();
	  elepsedtime1=(time1-prevtime1)*1000;


	  tuffer[0]=0x3B;
	  HAL_I2C_Master_Transmit(&hi2c1, MPU6050_ADDR,tuffer ,1,HAL_MAX_DELAY );
	  HAL_I2C_Master_Receive(&hi2c1, MPU6050_ADDR, tuffer, 6, HAL_MAX_DELAY);

	  acc_raw[0]=(tuffer[0] << 8 | tuffer[1]);
	  acc_raw[1]=(tuffer[2] << 8 | tuffer[3]);
	  acc_raw[2]=(tuffer[4] << 8 | tuffer[5]);


	  buffer[0]=0x41;
	  HAL_I2C_Master_Transmit(&hi2c1, MPU6050_ADDR,buffer ,1,HAL_MAX_DELAY );
	  HAL_I2C_Master_Receive(&hi2c1, MPU6050_ADDR, buffer, 2, HAL_MAX_DELAY);

	  raw_temp=(buffer[0] << 8 | buffer[1]);
	  temp=(raw_temp/340.0)+36.53;

	  cuffer[0]=0x43;
	  HAL_I2C_Master_Transmit(&hi2c1, MPU6050_ADDR,cuffer ,1,HAL_MAX_DELAY );
	  HAL_I2C_Master_Receive(&hi2c1, MPU6050_ADDR, cuffer, 6, HAL_MAX_DELAY);

	  gyro_raw[0]=(cuffer[0]<<8 | cuffer[1]);
	  	  gyro_raw[1]=(cuffer[2]<<8 | cuffer[3]);
	  	  gyro_raw[2]=(cuffer[4]<<8 | cuffer[5]);

	  	  gyro_raw[0]-=gyro_cal[0];
	  	  gyro_raw[1]-=gyro_cal[1];
	  	  gyro_raw[2]-=gyro_cal[2];


	  	  angle_pitch_gyro +=gyro_raw[0]*0.0000611;
	  	  angle_roll_gyro +=gyro_raw[0]*0.0000611;

	  	  angle_pitch_gyro +=angle_roll_gyro *sin(gyro_raw[2]*0.000001066);
	  	  angle_roll_gyro +=angle_pitch_gyro * sin(gyro_raw[2]*0.000001066);


	  	  acc_total_vector =sqrt((acc_raw[0]*acc_raw[0])+(acc_raw[1]*acc_raw[1])+(acc_raw[2]*acc_raw[2]));

	  	  //57.296=1/(3.142/180)

	  	  angle_pitch_acc= asin((float)acc_raw[1]/acc_total_vector)*57.296;
	  	  angle_roll_acc= asin((float)acc_raw[0]/acc_total_vector)*57.296;

	  	  angle_pitch_acc -=0.00; //0.05
	  	  angle_roll_acc -=0.00; //-1,32

	  	  if(set_gyro){
	  		  angle_pitch= angle_pitch_gyro *0.9996 + angle_pitch_acc *0.0004;
	  		  angle_roll=angle_roll_gyro*0.9996 + angle_roll_acc *0.0004;
	  	  }
	  	  else{
	  		  angle_pitch=angle_pitch_acc;
	  		  set_gyro=true;
	  	  }

	  	  while((HAL_GetTick() - prevtime)*1000 < 4000);
	  	  prevtime=HAL_GetTick();
