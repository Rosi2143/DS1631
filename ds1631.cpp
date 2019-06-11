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
#include <iomanip>
#include <stdio.h>
#include <stdint.h>
#include <typeinfo>
#include <unistd.h>           //Needed for I2C port
#include <fcntl.h>           //Needed for I2C port
#include <sys/ioctl.h>     //Needed for I2C port
#include <linux/i2c-dev.h> //Needed for I2C port
#include <stdbool.h>
#include <cmath>

#include "ds1631.hpp"

// defines from datasheet
#define DS1631_START_CONVERT_T 0x51
#define DS1631_STOP_CONVERT_T 0x22
#define DS1631_READ_TEMPERATURE 0xAA
#define DS1631_ACCESS_TH 0xA1
#define DS1631_ACCESS_TL 0xA2
#define DS1631_ACCESS_CONFIG 0xAC
#define DS1631_SOFTWARE_POR 0x54

#define DS1631_CONFIG_CONVERSTION_DONE_FLAG (1 << 7)
#define DS1631_CONFIG_CONVERSTION_IN_PROGRESS 0
#define DS1631_CONFIG_CONVERSTION_COMPLETE 1
#define DS1631_CONFIG_TEMP_HIGH_FLAG (1 << 6)
#define DS1631_CONFIG_TEMP_HIGH_OVERFLOW_INACTIVE 0
#define DS1631_CONFIG_TEMP_HIGH_OVERFLOW_ACTIVE 1
#define DS1631_CONFIG_TEMP_LOW_FLAG (1 << 5)
#define DS1631_CONFIG_TEMP_LOW_OVERFLOW_INACTIVE 0
#define DS1631_CONFIG_TEMP_LOW_OVERFLOW_ACTIVE 1
#define DS1631_CONFIG_NVM_BUSY_FLAG (1 << 4)
#define DS1631_CONFIG_NVM_NOT_BUSY 0
#define DS1631_CONFIG_NVM_BUSY 1
#define DS1631_CONFIG_RESOLUTION_BIT1 (1 << 3)
#define DS1631_CONFIG_RESOLUTION_BIT0 (1 << 2)
#define DS1631_CONFIG_09BIT_094MS 0
#define DS1631_CONFIG_10BIT_188MS 1
#define DS1631_CONFIG_11BIT_375MS 2
#define DS1631_CONFIG_12BIT_750MS 3
#define DS1631_CONFIG_TOUT_POLARITY (1 << 1)
#define DS1631_CONFIG_ACTIVE_LOW 0
#define DS1631_CONFIG_ACTIVE_HIGH 1
#define DS1631_CONFIG_1SHOT_CONVERSION (1 << 0)
#define DS1631_CONFIG_CONTINUOUS_MODE 0
#define DS1631_CONFIG_ONE_SHOT_MODE 1

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

void DS1631::ConvertCompl2Byte(const float & complement, short& int_byte, short& float_byte)
{
	std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;
	int_byte = (int)complement;
	float useit = complement;
    float forgetit;
    float_byte = std::modf(useit, &forgetit) * 256;
}

void DS1631::ConvertByte2Compl(const short& int_byte, const short& float_byte, float& complement)
{
	std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;
	complement = int_byte + (float)float_byte / 256;
}

/**************************************
 * DS1631 protcol implementation
 **************************************/
/*!
 * \brief start the conversion of temperature
 *  sudo i2cset -y 1 0x4C 0x51
 */
void DS1631::StartConvert()
{
	std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;
    unsigned char buffer[1] = {0};
    buffer[0] = DS1631_START_CONVERT_T;
    i2c_device->WriteByte(buffer, 1);
}

/*!
 * \brief stops the conversion of temperature
 *  sudo i2cset -y 1 0x4C 0x22
 */
void DS1631::StopConvert()
{
	std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;
    unsigned char buffer[1] = {0};
    buffer[0] = DS1631_STOP_CONVERT_T;
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
	std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;
    unsigned char buffer[2] = {0};
    buffer[0] = DS1631_READ_TEMPERATURE;
    i2c_device->WriteByte(buffer, 1);
    float temperature = 0;
    if (i2c_device->ReadByte(buffer, 2))
    {
        ConvertByte2Compl(buffer[0] , buffer[1], temperature);
		std::cout << "--Data read: " << std::dec << std::setprecision(2) << temperature << "°C" << std::endl;
	}
    return temperature;
}

/*!
 * \brief read the config register and print the result
 */
void DS1631::ReadConfig()
{
	std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;
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
	std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;
    //sudo i2cget -y 1 0x4C 0xa1
    unsigned char buffer[2] = {0};
    buffer[0] = DS1631_ACCESS_TH;
    i2c_device->WriteByte(buffer, 1);
    float templimit = 0;
    if (i2c_device->ReadByte(buffer, 2))
    {
		ConvertByte2Compl(buffer[0], buffer[1], templimit);
		std::cout << "--UpperLimit: " << std::dec << std::setprecision(2) << templimit << "°C" << std::endl;
    }

    return templimit;
}

/*!
 * \brief write the upper temperature limit
 */
bool DS1631::WriteUpperTempTripPoint(float tempLimit)
{
	std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;
	//sudo i2cget -y 1 0x4C 0xa1
	unsigned char buffer[3] = {0};
 	short int_byte, float_byte;
	ConvertCompl2Byte(tempLimit, int_byte, float_byte);
	buffer[0] = DS1631_ACCESS_TH;
	buffer[1] = int_byte;
	buffer[2] = float_byte;
    i2c_device->WriteByte(buffer, 3);
}

/*!
 * \brief read the lower temperature limit
 */
void DS1631::ReadLowerTempTripPoint()
{
	std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;
    //sudo i2cget -y 1 0x4C 0xa1
    unsigned char buffer[2] = {0};
    buffer[0] =  DS1631_ACCESS_TL;
    i2c_device->WriteByte(buffer, 1);
    float templimit = 0;
    if (i2c_device->ReadByte(buffer, 2))
    {
		ConvertByte2Compl(buffer[0], buffer[1], templimit);
		std::cout << "--LowerLimit: " << std::dec << std::setprecision(2) << templimit << "°C" << std::endl;
	}
}

/*!
 * \brief write the lower temperature limit
 */
bool DS1631::WriteLowerTempTripPoint(float tempLimit)
{
	std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;
	//sudo i2cget -y 1 0x4C 0xa1
    unsigned char buffer[3] = {0};
	short int_byte, float_byte;
	ConvertCompl2Byte(tempLimit, int_byte, float_byte);
	buffer[0] = DS1631_ACCESS_TL;
	buffer[1] = int_byte;
	buffer[2] = float_byte;
    i2c_device->WriteByte(buffer, 3);
}
