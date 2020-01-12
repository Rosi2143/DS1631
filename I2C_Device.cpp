/**
 * @file I2C_Device.cpp
 * @author Michael Rossner (Schrott.Micha@web.de)
 * @brief Implementation of the I2C interaction
 * @version 0.1
 * @date 2019-06-10
 * 
 * @copyright Copyright (c) 2019
 * MIT license - see license file
 */

#include <iostream>
#include <iomanip>
#include <typeinfo> 
#include <unistd.h>
#include <sys/ioctl.h>     //Needed for I2C port
#include <linux/i2c-dev.h> //Needed for I2C port
#include <fcntl.h>

#include "I2C_Device.hpp"

/**
 * @brief Construct a new i2c device::i2c device object
 * 
 */
I2C_Device::I2C_Device(int device_id, bool verb) : addr(device_id),verbose(verb) 
{
    //----- OPEN THE I2C BUS -----
    char *filename = (char *)"/dev/i2c-1";
    if ((file_i2c = open(filename, O_RDWR)) < 0)
    {
        //ERROR HANDLING: you can check errno to see what went wrong
        std::cout << "Failed to open the i2c bus" << std::endl;
        return;
    }

    if (ioctl(file_i2c, I2C_SLAVE, addr) < 0)
    {
        std::cout << "Failed to acquire bus access and / or talk to slave." << std::endl;
        //ERROR HANDLING; you can check errno to see what went wrong
        return;
    }
}

/**
 * @brief Destroy the i2c device::i2c device object
 * 
 */
I2C_Device::~I2C_Device()
{
}

/**
 * @brief 
 * 
 * @param buffer 
 * @param length 
 */
bool I2C_Device::WriteByte(unsigned char const *buffer, const int length)
{
    bool ret = true;
    if (verbose)
        std::cout << "   " << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << getAddress() << ")" << std::endl;

    if (write(file_i2c, buffer, length) != length) //write() returns the number of bytes actually written, if it doesn't match then an error occurred (e.g. no response from the device)
    {
        /* ERROR HANDLING: i2c transaction failed */
        std::cout << "Failed to write to the i2c bus." << std::endl;
        ret = false;
    }
    return ret;
}

/**
 * @brief 
 * 
 * @param buffer 
 * @param length 
 * @return int 
 */
bool I2C_Device::ReadByte(unsigned char *buffer, const int length)
{
    if (verbose)
        std::cout << "   " << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << getAddress() << ")" << std::endl;
    int16_t result = 0;
    if (length > 2)
    {
        return false; // there is no reply for DS1621 with more than 2 bytes
    }

    if (read(file_i2c, buffer, length) != length) //read() returns the number of bytes actually read, if it doesn't match then an error occurred (e.g. no response from the device)
    {
        //ERROR HANDLING: i2c transaction failed
        std::cout << "Failed to read from the i2c bus." << std::endl;
        return false;
    }
    else
    {
        if (verbose)
            std::cout << "Data read 0: " << std::hex << std::setw(2) << std::setfill('0') << (int)buffer[0] << std::endl;
        if (length == 2)
            if (verbose)
                std::cout << "Data read 1: " << std::hex << std::setw(2) << std::setfill('0') << (int)buffer[1] << std::endl;
    }
    return true;
}