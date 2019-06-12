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

class DS1631
{
private:
    I2C_Interface* i2c_device;

public:
    DS1631(I2C_Device* i2c_dev);
    ~DS1631();

    bool StartConvert();
    bool StopConvert();
    float ReadTemperature();
    short ReadConfig();
    void EvalConfig();
    bool WriteConfig(short config);
    float ReadUpperTempTripPoint();
    bool WriteUpperTempTripPoint(float tempLimit);
    float ReadLowerTempTripPoint();
    bool WriteLowerTempTripPoint(float tempLimit);

    // config read
    bool ConfigIsConversionDone();
    bool ConfigIsTempHighFlagSet();
    bool ConfigIsTempLowFlagSet();
    bool ConfigIsNvMBusy();
    bool ConfigIsToutPolarityHigh();
    bool ConfigIs1ShotModeActive();
    short ConfigGetResolutionAndConversionTime();

    // config set
    bool SetConfigConversionDone(bool state);
    bool SetConfigTempHighFlagSet(bool state);
    bool SetConfigTempLowFlagSet(bool state);
    bool SetConfigNvMBusy(bool state);
    bool SetConfigToutPolarityHigh(bool state);
    bool SetConfig1ShotModeActive(bool state);
    bool ConfigSetResolutionAndConversionTime(short ResolutionAndConverstionTime);

protected:
    void ConvertCompl2Byte(const float &complement, short& int_byte, short& float_byte);
    void ConvertByte2Compl(const short &int_byte, const short &float_byte, float& complement);
};