/**
 * @file DS1631.cpp
 * @author Michael Rossner (Schrott.Micha@web.de)
 * @brief definition file for the DS1631 temperature sensor
 * @version 0.1
 * @date 2019-06-10
 * 
 * @copyright Copyright (c) 2019
 * MIT license - see license file
 */

//https://raspberry-projects.com/pi/programming-in-c/i2c/using-the-i2c-interface

#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>		   //Needed for I2C port
#include <fcntl.h>		   //Needed for I2C port
#include <sys/ioctl.h>     //Needed for I2C port
#include <linux/i2c-dev.h> //Needed for I2C port
#include <stdbool.h>

#include "ds1631.hpp"

/**
 * @brief Construct a new DS1631::DS1631 object
 * 
 * @param i2c_device 
 */
DS1631::DS1631(I2C_Device* i2c_dev) : i2c_device(i2c_dev)
{

}

/**
 * @brief Destroy the DS1631::DS1631 object
 * 
 */
DS1631::~DS1631()
{

}

void DS1631::ConvertCompl2Byte(const int compl &, short int_byte, short float_byte)
{
	int_byte = (int) compl;
	float_byte = std::modf(compl) << 8;
}

void DS1631::ConvertByte2Compl(const short &int_byte, const short &float_byte, int compl)
{
	compl =  int_byte + float_byte / 256;
}

/**************************************
 * DS1631 protcol implementation
 **************************************/
/*!
 * \brief start the conversion of temperature
 *  sudo i2cset -y 1 0x4C 0xee
 */
void DS1631::StartConvert()
{
	unsigned char buffer[1] = {0};
	buffer[0] = DS1631_START_CONVERT_T;
	i2c_device->WriteByte(buffer, 1);
}

/*!
 * \brief read temperature temperature
 * sudo i2cget -y 1 0x4C 0xaa
 * 
 * \return temperature in °C
 */
float DS1631::ReadTemperature()
{
	unsigned char buffer[2] = {0};
	buffer[0] = DS1631_READ_TEMPERATURE;
	i2c_device->WriteByte(buffer, 1);
	int16_t temperature = 0;
	if (i2c_device->ReadByte(buffer, 2))
	{
		temperature = buffer[1] + (buffer[0] << 8);
		std::cout << "--Data read: " << std::hex << temperature << std::endl;
		std::cout << "--current temperature: " << std::dec << (temperature / 256) << "," << (temperature % 256) << "°C" << std::endl;
	}
	return (temperature / 256 + (temperature % 256) / 1000);
}

/*!
 * \brief read the config register and print the result
 */
void DS1631::ReadConfig()
{
	//sudo i2cget -y 1 0x4C 0xac
	unsigned char buffer[1] = {0};
	buffer[0] = DS1631_ACCESS_CONFIG;
	i2c_device->WriteByte(buffer, 1);
	int8_t config = 0;
	if (i2c_device->ReadByte(buffer, 1))
	{
		config = buffer[0];
		std::cout << "--Config: " << config << std::endl;
		if (config & DS1631_CONFIG_CONVERSTION_DONE_FLAG)
		{
			std::cout << "\tconfig: Conversion done" << std::endl;
		}
		else
		{
			std::cout << "\tconfig: Conversion in progress" << std::endl;
		}

		if (config & DS1631_CONFIG_TEMP_HIGH_FLAG)
		{
			std::cout << "\tconfig: HighTemp overflow active" << std::endl;
		}
		else
		{
			std::cout << "\tconfig: HighTemp overflow inactive" << std::endl;
		}

		if (config & DS1631_CONFIG_TEMP_LOW_FLAG)
		{
			std::cout << "\tconfig: LowTemp overflow active" << std::endl;
		}
		else
		{
			std::cout << "\tconfig: LowTemp overflow inactive" << std::endl;
		}

		if (config & DS1631_CONFIG_NVM_BUSY_FLAG)
		{
			std::cout << "\tconfig: NvM write in progress" << std::endl;
		}
		else
		{
			std::cout << "\tconfig: NvM write done" << std::endl;
		}

		int8_t ResolutionAndTime = config & (DS1631_CONFIG_RESOLUTION_BIT1 | DS1631_CONFIG_RESOLUTION_BIT0);
		ResolutionAndTime = ResolutionAndTime >> 2;
		if (ResolutionAndTime == DS1631_CONFIG_09BIT_094MS)
		{
			std::cout << "\tconfig: accuracy 8Bit, cycle 93.75ms" << std::endl;
		}
		else if (ResolutionAndTime == DS1631_CONFIG_10BIT_188MS)
		{
			std::cout << "\tconfig: accuracy 9Bit, cycle 187.5ms" << std::endl;
		}
		else if (ResolutionAndTime == DS1631_CONFIG_11BIT_375MS)
		{
			std::cout << "\tconfig: accuracy 11Bit, cycle 375ms" << std::endl;
		}
		else if (ResolutionAndTime == DS1631_CONFIG_12BIT_750MS)
		{
			std::cout << "\tconfig: accuracy 12Bit, cycle 750ms" << std::endl;
		}

		if (config & DS1631_CONFIG_TOUT_POLARITY)
		{
			std::cout << "\tconfig: Polarity is HIGH" << std::endl;
		}
		else
		{
			std::cout << "\tconfig: Polarity is LOW" << std::endl;
		}
		if (config & DS1631_CONFIG_1SHOT_CONVERSION)
		{
			std::cout << "\tconfig: OneShot conversion is active" << std::endl;
		}
		else
		{
			std::cout << "\tconfig: continuous conversion is active" << std::endl;
		}
	}
}

/*!
 * \brief read the upper temperature limit
 * 
 * \param temperature of upper limit in °C
 */
float DS1631::ReadUpperTempTripPoint()
{
	//sudo i2cget -y 1 0x4C 0xa1
	unsigned char buffer[2] = {0};
	buffer[0] = DS1631_ACCESS_TH;
	i2c_device->WriteByte(buffer, 1);
	int16_t templimit = 0;
	if (i2c_device->ReadByte(buffer, 2))
	{
		templimit = buffer[1] + (buffer[0] << 8);
		std::cout << "--Data read: " << std::hex << templimit << std::endl;
		std::cout << "--UpperLimit: " << std::dec << (templimit / 256) << "." << (templimit % 256) << "°C" << std::endl;
	}

	return (templimit / 256 + (templimit % 256) / 1000);
}

/*!
 * \brief write the upper temperature limit
 */
bool DS1631::WriteUpperTempTripPoint(float tempLimit)
{
	//sudo i2cget -y 1 0x4C 0xa1
	unsigned char buffer[3] = {0};
	buffer[0] = DS1631_ACCESS_TH;
	buffer[1] = (int8_t)tempLimit;
	buffer[2] = 0;
	i2c_device->WriteByte(buffer, 3);
}

/*!
 * \brief read the lower temperature limit
 */
void DS1631::ReadLowerTempTripPoint()
{
	//sudo i2cget -y 1 0x4C 0xa1
	unsigned char buffer[2] = {0};
	buffer[0] =  DS1631_ACCESS_TL;
	i2c_device->WriteByte(buffer, 1);
	int16_t templimit = 0;
	if (i2c_device->ReadByte(buffer, 2))
	{
		templimit = buffer[1] + (buffer[0] << 8);
		std::cout << "--Data read: " << std::hex << templimit << std::endl;
		std::cout << "--LowerLimit: " << std::dec << (templimit / 256) << "," << (templimit % 256) << "°C" << std::endl;
	}
}

/*!
 * \brief write the lower temperature limit
 */
bool DS1631::WriteLowerTempTripPoint(float tempLimit)
{
	//sudo i2cget -y 1 0x4C 0xa1
	unsigned char buffer[3] = {0};
	buffer[0] = DS1631_ACCESS_TL;
	buffer[1] = (int8_t)tempLimit;
	buffer[2] = 0;
	i2c_device->WriteByte(buffer, 3);
}
