/**
 * @file I2C_Interface.hpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2019-06-10
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#pragma once

class I2C_Interface
{
public:
    I2C_Interface(){};
    ~I2C_Interface(){};

    virtual bool WriteByte(unsigned char const *buffer, const int length) = 0;
    virtual bool ReadByte(unsigned char *buffer, const int length) = 0;

    virtual int getAddress() = 0;
    virtual bool isVerbose() = 0;
};
