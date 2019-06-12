/**
 * @file I2C_Device.hpp
 * @author Michael Rossner (Schrott.Micha@web.de)
 * @brief declaration of i2c device
 * @version 0.1
 * @date 2019-06-10
 * 
 * @copyright Copyright (c) 2019
 * MIT license - see license file
 */

#include "I2C_Interface.hpp"

class I2C_Device : public I2C_Interface
{
public:
    I2C_Device(int device_id);
    ~I2C_Device();

    virtual void WriteByte(unsigned char const *buffer, const int length);
    virtual bool ReadByte(unsigned char *buffer, const int length);

    virtual int getAddress(){return addr;}
private:
    int file_i2c;
    /**
     * @brief i2c adress of the device
     * 
     */
    int addr;
};