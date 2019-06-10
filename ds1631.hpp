/**
 * @file ds1631.hpp
 * @author Michael Rossner (Schrott.Micha@web.de)
 * @brief interface definition for the class to interact with the DS1631 chip
 * @version 0.1
 * @date 2019-06-10
 * 
 * @copyright Copyright (c) 2019
 * MIT license - see license file
 */

#include "I2C_Device.hpp"

/* command line commands
set up continuous measurement
sudo i2cset -y 1 0x4C 0xac 0x00 b

start measurements
sudo i2cset -y 1 0x4C 0xee

read the temperature
sudo i2cget -y 1 0x4C 0xaa

 */

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

class DS1631
{
private:
    I2C_Interface* i2c_device;

public:
    DS1631(I2C_Device* i2c_dev);
    ~DS1631();

    void StartConvert();
    float ReadTemperature();
    void ReadConfig();
    float ReadUpperTempTripPoint();
    bool WriteUpperTempTripPoint(float tempLimit);
    void ReadLowerTempTripPoint();
    bool WriteLowerTempTripPoint(float tempLimit);

protected:
    void ConvertInt2Compl(const int value &, short byte1, short byte2);
    void ConvertInt2Compl(const short &byte1, const short &byte2, int value);
};