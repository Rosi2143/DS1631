/**
 * @file main.cpp
 * @author Michael Rossner (Schrott.Micha@web.de)
 * @brief main file for testing
 * @version 0.1
 * @date 2019-06-10
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include "ds1631.hpp"

int main(int ac, char **av)
{
    I2C_Device i2c_device(72 /*0x48 */);
    DS1631 ds1631(&i2c_device);

    ds1631.StartConvert();

    ds1631.ReadTemperature();

    ds1631.ReadConfig();

    ds1631.ReadUpperTempTripPoint();
    ds1631.WriteUpperTempTripPoint(30.4);
    ds1631.ReadUpperTempTripPoint();

    ds1631.ReadLowerTempTripPoint();
    ds1631.WriteLowerTempTripPoint(30.4);
    ds1631.ReadLowerTempTripPoint();

    return(0);
}