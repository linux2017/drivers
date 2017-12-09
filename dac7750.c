/* 
 * driver for Texas Instruments DAC7750 12-Bit SPI DAC
 * 
 * Copyright (C) 2017  eric li
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 * ===============================================================================
 * 
 * What works:
 * - Basic I/O to DAC Registers
 * - interface using IIO
 * - setting range and output enable bits in control register
 * 
 */
#include "lpc177x_8x.h"
#include "cpu_resource_user_map.h"
#include "dac7750.h"
#include "debug.h"
#define MODULE_AUTHOR "eric li<eric.li@znsb.com>" 
#define MODULE_DESCRIPTION "Texas Instruments DAC7750"
#define MODULE_LICENSE "GPL v2"

#define DW_SPI_WORKAROUND
#define DAC7750_CLK_L()  (DAC7750_SPI_GPIO_PORT->CLR |= 1UL << DAC7750_SPI_GPIO_SCK_PIN)
#define DAC7750_CLK_H()  (DAC7750_SPI_GPIO_PORT->SET |= 1UL << DAC7750_SPI_GPIO_SCK_PIN) 

#define DAC7750_SDO_L()  (DAC7750_SPI_GPIO_PORT->CLR |= 1UL << DAC7750_SPI_GPIO_MOSI_PIN)
#define DAC7750_SDO_H()  (DAC7750_SPI_GPIO_PORT->SET |= 1UL << DAC7750_SPI_GPIO_MOSI_PIN)

#define DAC7750_SDI()  	  (DAC7750_SPI_GPIO_PORT->PIN & (1UL << DAC7750_SPI_GPIO_MISO_PIN))

#define DAC7750_1_CS_L()  (DAC7750_1_SPI_SSEL_GPIO_PORT->CLR |= 1UL << DAC7750_1_SPI_GPIO_SSEL_PIN)
#define DAC7750_1_CS_H()  (DAC7750_1_SPI_SSEL_GPIO_PORT->SET |= 1UL << DAC7750_1_SPI_GPIO_SSEL_PIN)

#define DAC7750_2_CS_L()  (DAC7750_2_SPI_SSEL_GPIO_PORT->CLR |= 1UL << DAC7750_2_SPI_GPIO_SSEL_PIN)
#define DAC7750_2_CS_H()  (DAC7750_2_SPI_SSEL_GPIO_PORT->SET |= 1UL << DAC7750_2_SPI_GPIO_SSEL_PIN)

#define DAC7750_3_CS_L()  (DAC7750_3_SPI_SSEL_GPIO_PORT->CLR |= 1UL << DAC7750_3_SPI_GPIO_SSEL_PIN)
#define DAC7750_3_CS_H()  (DAC7750_3_SPI_SSEL_GPIO_PORT->SET |= 1UL << DAC7750_3_SPI_GPIO_SSEL_PIN)



#define DAC7750_1_CLR_L()   (DAC7750_1_SPI_CLR_GPIO_PORT->CLR |= 1UL << DAC7750_1_SPI_GPIO_CLR_PIN)
#define DAC7750_1_CLR_H()   (DAC7750_1_SPI_CLR_GPIO_PORT->SET |= 1UL << DAC7750_1_SPI_GPIO_CLR_PIN)

#define DAC7750_2_CLR_L()   (DAC7750_2_SPI_CLR_GPIO_PORT->CLR |= 1UL << DAC7750_2_SPI_GPIO_CLR_PIN)
#define DAC7750_2_CLR_H()   (DAC7750_2_SPI_CLR_GPIO_PORT->SET |= 1UL << DAC7750_2_SPI_GPIO_CLR_PIN)

#define DAC7750_3_CLR_L()   (DAC7750_3_SPI_CLR_GPIO_PORT->CLR |= 1UL << DAC7750_3_SPI_GPIO_CLR_PIN)
#define DAC7750_3_CLR_H()   (DAC7750_3_SPI_CLR_GPIO_PORT->SET |= 1UL << DAC7750_3_SPI_GPIO_CLR_PIN)

#define DAC7750_1_ALARM()   (DAC7750_1_SPI_ALARM_GPIO_PORT->PIN & (1UL << DAC7750_1_SPI_GPIO_ALARM_PIN))
#define DAC7750_2_ALARM()   (DAC7750_2_SPI_ALARM_GPIO_PORT->PIN & (1UL << DAC7750_2_SPI_GPIO_ALARM_PIN))
#define DAC7750_3_ALARM()   (DAC7750_3_SPI_ALARM_GPIO_PORT->PIN & (1UL << DAC7750_3_SPI_GPIO_ALARM_PIN))



/*******************
DAC7750_GPIO初始化
********************/

void DAC7750_GPIOInit(uint8 chip_sn)
{
	if(chip_sn>3)return;
	set_pin_dir(DAC7750_SPI_GPIO_PORT,DAC7750_SPI_GPIO_SCK_PIN,PIN_OUTPUT);//设置SCK引脚为输出 
	
    set_pin_dir(DAC7750_SPI_GPIO_PORT,DAC7750_SPI_GPIO_MOSI_PIN,PIN_OUTPUT);//设置SDO引脚为输出
    
    set_pin_dir(DAC7750_SPI_GPIO_PORT,DAC7750_SPI_GPIO_MISO_PIN,PIN_INPUT);//设置SDI引脚为输入
		
   	 DAC7750_CLK_H();
	 DAC7750_SDO_L();
	///////////////////////////////////////////////////////////////////////////////////////////////
	if(chip_sn==1){
		set_pin_dir(DAC7750_1_SPI_SSEL_GPIO_PORT,DAC7750_1_SPI_GPIO_SSEL_PIN,PIN_OUTPUT);//设置CS1
		set_pin_dir(DAC7750_1_SPI_CLR_GPIO_PORT,DAC7750_1_SPI_GPIO_CLR_PIN,PIN_OUTPUT);//设置CLR1
		set_pin_dir(DAC7750_1_SPI_ALARM_GPIO_PORT,DAC7750_1_SPI_GPIO_ALARM_PIN,PIN_INPUT);//设置ALARM1
		DAC7750_1_CS_H();
		DAC7750_1_CLR_H();
		delay(10);
	 	DAC7750_1_CLR_L();
	}else if(chip_sn==2){
		set_pin_dir(DAC7750_2_SPI_SSEL_GPIO_PORT,DAC7750_2_SPI_GPIO_SSEL_PIN,PIN_OUTPUT);//设置CS2
		set_pin_dir(DAC7750_2_SPI_CLR_GPIO_PORT,DAC7750_2_SPI_GPIO_CLR_PIN,PIN_OUTPUT);//设置CLR2
		set_pin_dir(DAC7750_2_SPI_ALARM_GPIO_PORT,DAC7750_2_SPI_GPIO_ALARM_PIN,PIN_INPUT);//设置ALARM2
		DAC7750_2_CS_H();
		DAC7750_2_CLR_H();
		DAC7750_2_CLR_L();
	}else{
		set_pin_dir(DAC7750_3_SPI_SSEL_GPIO_PORT,DAC7750_3_SPI_GPIO_SSEL_PIN,PIN_OUTPUT);//设置CS3
		set_pin_dir(DAC7750_3_SPI_CLR_GPIO_PORT,DAC7750_3_SPI_GPIO_CLR_PIN,PIN_OUTPUT);//设置CLR3
		set_pin_dir(DAC7750_3_SPI_ALARM_GPIO_PORT,DAC7750_3_SPI_GPIO_ALARM_PIN,PIN_INPUT);//设置ALARM3
		DAC7750_3_CS_H();
		DAC7750_3_CLR_H();
		delay(10);
	 	DAC7750_3_CLR_L();
	}
}

void DAC7750_CS_CTRL_H(uint8 chip_id)
{
	if(chip_id>DAC7750_ID3)return ;
	
	if(chip_id==DAC7750_ID1){
		DAC7750_1_CS_H();
	}else if(chip_id==DAC7750_ID2){
		DAC7750_2_CS_H();
	}else{
		DAC7750_3_CS_H();
	}
}

void DAC7750_CS_CTRL_L(uint8 chip_id)
{
	if(chip_id>DAC7750_ID3)return;
		
	if(chip_id==DAC7750_ID1){
		DAC7750_1_CS_L();
	}else if(chip_id==DAC7750_ID2){
		DAC7750_2_CS_L();
	}else{
		DAC7750_3_CS_L();
	}
}

u8 DAC7750_SPI_Write(uint8 chip_id,DAC7750_TRANS_FRAME TX_data)
{
	u8 temp_i=0;
	uint8 addr;
	uint16 data;
	addr=TX_data.addr;
	data=TX_data.data;
	OS_ENTER_CRITICAL();//进入临界区(无法被中断打断)
	DAC7750_CS_CTRL_L(chip_id);
	DAC7750_CLK_H();
	delay(2);
	///发地址
	for(temp_i=0;temp_i<8;temp_i++)
	{
		if(addr &0x80)
			DAC7750_SDO_H();
		else
			DAC7750_SDO_L();
		
		DAC7750_CLK_L();		
		delay(2);
		DAC7750_CLK_H();
		
		addr=addr<<1;	
	}

	//////发数据
	for(temp_i=0;temp_i<16;temp_i++)
	{
		if(data &0x8000)
			DAC7750_SDO_H();
		else
			DAC7750_SDO_L();
		
		DAC7750_CLK_L();		
		delay(2);
		DAC7750_CLK_H();
		
		data=data<<1;	
	}
	DAC7750_CS_CTRL_H(chip_id);
	OS_EXIT_CRITICAL();//进入临界区(无法被中断打断)
	return ;
	
}

u16 DAC7750_SPI_Read(uint8 chip_id,DAC7750_TRANS_FRAME TX_data)
{
	u8 temp_i=0;
	uint8 addr;
	uint16 data;
	addr=TX_data.addr;
	data=TX_data.data;
	OS_ENTER_CRITICAL();//进入临界区(无法被中断打断)
	DAC7750_CS_CTRL_L(chip_id);
	DAC7750_CLK_H();
	delay(2);
	///发地址
	for(temp_i=0;temp_i<8;temp_i++)
	{
		if(addr &0x80)
			DAC7750_SDO_H();
		else
			DAC7750_SDO_L();
		
		DAC7750_CLK_L();		
		delay(2);
		DAC7750_CLK_H();
		addr<<=1;	
	}

	//////发数据
	for(temp_i=0;temp_i<16;temp_i++)
	{
		if(data &0x8000)
			DAC7750_SDO_H();
		else
			DAC7750_SDO_L();
		
		DAC7750_CLK_L();		
		delay(1);
		DAC7750_CLK_H();
		data<<=1;	
	}
	DAC7750_CS_CTRL_H(chip_id);
	delay(5);
	DAC7750_CS_CTRL_L(chip_id);
	DAC7750_CLK_H();
	//读数据 先发8个时钟
	delay(2);
	data=0;
	for(temp_i=0;temp_i<8;temp_i++)
	{
		DAC7750_CLK_L();
		delay(2);
		DAC7750_CLK_H();	
	}
	for(temp_i=0;temp_i<16;temp_i++)
	{
		data<<=1;	
		DAC7750_CLK_L();
		delay(1);
		if(DAC7750_SDI()){
			data|=1;
		}
		delay(1);
		DAC7750_CLK_H();
	}
	DAC7750_CS_CTRL_H(chip_id);
	OS_EXIT_CRITICAL();//进入临界区(无法被中断打断)
	return data;
	
}

uint16 DAC7750_read_status(uint8 chip_id)
{
	uint16 st;
	DAC7750_TRANS_FRAME DAC7750_Frame;
	DAC7750_Frame.addr=DAC7750_ADDR_READ;
	DAC7750_Frame.data=DAC7750_READ_STATUS;
	st=DAC7750_SPI_Read(chip_id,DAC7750_Frame);
	return st&0x001f;
}
uint16 DAC7750_read_congfigure_reg(uint8 chip_id)
{
	uint16 data;
	DAC7750_TRANS_FRAME DAC7750_Frame;
	DAC7750_Frame.addr=DAC7750_ADDR_READ;
	DAC7750_Frame.data=DAC7750_READ_CFG;
	data=DAC7750_SPI_Read(chip_id,DAC7750_Frame);
	return data;
}
uint16 DAC7750_set_congfigure_reg(uint8 chip_id)
{
	DAC7750_TRANS_FRAME DAC7750_Frame;
	DAC7750_Frame.addr=DAC7750_ADDR_WR_CFG;
	DAC7750_Frame.data=DAC7750_WDPD_204MS;
	DAC7750_SPI_Write(chip_id,DAC7750_Frame);
}

uint16 DAC7750_read_control_reg(uint8 chip_id)
{
	uint16 data;
	DAC7750_TRANS_FRAME DAC7750_Frame;
	DAC7750_Frame.addr=DAC7750_ADDR_READ;
	DAC7750_Frame.data=DAC7750_READ_CTRL;
	data=DAC7750_SPI_Read(chip_id,DAC7750_Frame);
	return data;
}
uint16 DAC7750_set_control_reg(uint8 chip_id)
{
	DAC7750_TRANS_FRAME DAC7750_Frame;
	DAC7750_Frame.addr=DAC7750_ADDR_WR_CTRL;
	//DAC7750_Frame.data=DAC7750_RANGE_4_20|DAC7750_MASK_CTRL_REXT|DAC7750_MASK_CTRL_OUTEN;
	DAC7750_Frame.data=DAC7750_RANGE_4_20|DAC7750_MASK_CTRL_OUTEN;
	DAC7750_SPI_Write(chip_id,DAC7750_Frame);
}

uint16 DAC7750_read_DAC_reg(uint8 chip_id)
{
	uint16 data;
	DAC7750_TRANS_FRAME DAC7750_Frame;
	DAC7750_Frame.addr=DAC7750_ADDR_READ;
	DAC7750_Frame.data=DAC7750_READ_DATA;
	data=DAC7750_SPI_Read(chip_id,DAC7750_Frame);
	return data;
}

uint16 DAC7750_set_DAC_reg(uint8 chip_id,uint16 data)
{
	DAC7750_TRANS_FRAME DAC7750_Frame;
	if(data>4096) return 0;
	DAC7750_Frame.addr=DAC7750_ADDR_WR_REG;
	DAC7750_Frame.data=(data<<4);
	DAC7750_SPI_Write(chip_id,DAC7750_Frame);
	return 1;
}

uint16 DAC7750_read_gain_calibration_reg(uint8 chip_id)
{
	uint16 data;
	DAC7750_TRANS_FRAME DAC7750_Frame;
	DAC7750_Frame.addr=DAC7750_ADDR_READ;
	DAC7750_Frame.data=DAC7750_READ_DAC_GAIN;
	data=DAC7750_SPI_Read(chip_id,DAC7750_Frame);
	return data;
}

uint16 DAC7750_set_gain_calibration_reg(uint8 chip_id,uint16 data)
{
	DAC7750_TRANS_FRAME DAC7750_Frame;
	DAC7750_Frame.addr=DAC7750_ADDR_WR_DAC_GAIN;
	DAC7750_Frame.data=data;
	DAC7750_SPI_Write(chip_id,DAC7750_Frame);
	return 1;
}

uint16 DAC7750_read_zero_calibration_reg(uint8 chip_id)
{
	uint16 data;
	DAC7750_TRANS_FRAME DAC7750_Frame;
	DAC7750_Frame.addr=DAC7750_ADDR_READ;
	DAC7750_Frame.data=DAC7750_READ_DAC_ZERO;
	data=DAC7750_SPI_Read(chip_id,DAC7750_Frame);
	return data;
}
uint16 DAC7750_set_zero_calibration_reg(uint8 chip_id,uint16 data)
{
	DAC7750_TRANS_FRAME DAC7750_Frame;
	DAC7750_Frame.addr=DAC7750_ADDR_WR_DAC_ZERO;
	DAC7750_Frame.data=data;
	DAC7750_SPI_Write(chip_id,DAC7750_Frame);
	return 1;
}
uint16 DAC7750_set_reset_reg(uint8 chip_id)
{
	DAC7750_TRANS_FRAME DAC7750_Frame;
	DAC7750_Frame.addr=DAC7750_ADDR_WR_RESET;
	DAC7750_Frame.data=1;
	DAC7750_SPI_Write(chip_id,DAC7750_Frame);
	return 1;
}
void DAC7750_Init(uint8 chip_id)
{
	uint16 data;
	DAC7750_GPIOInit(chip_id);
	DAC7750_set_reset_reg(chip_id);
	DAC7750_set_zero_calibration_reg(chip_id,0);
	DAC7750_set_gain_calibration_reg(chip_id,0);
	DAC7750_set_DAC_reg(chip_id,0);
	DAC7750_set_control_reg(chip_id);
	DAC7750_set_congfigure_reg(chip_id);
	
	data=DAC7750_read_status(chip_id);
	DEBUG_PRINT(DEBUG_ERROR, 0x2000,"DAC7750[%d] status=%04x\r\n",chip_id,data);
	data=DAC7750_read_congfigure_reg(chip_id);
	DEBUG_PRINT(DEBUG_ERROR, 0x2000,"DAC7750[%d] config=%04x\r\n",chip_id,data);

	data=DAC7750_read_control_reg(chip_id);
	DEBUG_PRINT(DEBUG_ERROR, 0x2000,"DAC7750[%d] contrl=%04x\r\n",chip_id,data);

	data=DAC7750_read_DAC_reg(chip_id);
	DEBUG_PRINT(DEBUG_ERROR, 0x2000,"DAC7750[%d] DATA=%04x\r\n",chip_id,data);

	data=DAC7750_read_gain_calibration_reg(chip_id);
	DEBUG_PRINT(DEBUG_ERROR, 0x2000,"DAC7750[%d] gain=%04x\r\n",chip_id,data);

	data=DAC7750_read_zero_calibration_reg(chip_id);
	DEBUG_PRINT(DEBUG_ERROR, 0x2000,"DAC7750[%d] zero=%04x\r\n",chip_id,data);
}
void DAC7750_set_output_on(uint8 chip_id)
{
	DAC7750_TRANS_FRAME DAC7750_Frame;
	DAC7750_Frame.addr=DAC7750_ADDR_WR_CTRL;
	DAC7750_Frame.data=DAC7750_RANGE_4_20|DAC7750_MASK_CTRL_OUTEN;
	DAC7750_SPI_Write(chip_id,DAC7750_Frame);
}
void DAC7750_set_output_off(uint8 chip_id)
{
	DAC7750_TRANS_FRAME DAC7750_Frame;
	DAC7750_Frame.addr=DAC7750_ADDR_WR_CTRL;
	DAC7750_Frame.data=DAC7750_RANGE_4_20;
	DAC7750_SPI_Write(chip_id,DAC7750_Frame);
}
void DAC7750_Monitor(uint8 chip_id)
{
	uint16 data;
	static uint16 st[3]=0;
	#define CRC_FLT (1<<4)
	#define WD_FLT (1<<3)
	#define I_FLT (1<<2)
	#define SR_ON (1<<1)	
	#define T_LFT (1<<0)
	data=DAC7750_read_status(chip_id);
	if(st[chip_id-1]!=data){
		st[chip_id-1]=data;
		if(((st[chip_id-1]&I_FLT)!=0)||((st[chip_id-1]&T_LFT)!=0)){
			if((st[chip_id-1]&I_FLT)!=0){
				DEBUG_PRINT(DEBUG_ERROR, 0x2000,"DAC7750[%d] output circuit open \r\n",chip_id);
				//DAC7750_set_output_off(chip_id);
			}
			if((st[chip_id-1]&T_LFT)!=0){
				DEBUG_PRINT(DEBUG_ERROR, 0x2000,"DAC7750[%d] die temperature is over\r\n",chip_id);
				//DAC7750_set_output_off(chip_id);
			}
		}else{
				//DAC7750_set_output_on(chip_id);
				DEBUG_PRINT(DEBUG_ERROR, 0x2000,"DAC7750[%d]output normal\r\n",chip_id);
		}
		
	}
	
}


