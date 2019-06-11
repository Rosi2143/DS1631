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

#include <list>
#include "ds1631.hpp"

int main(int ac, char **av)
{
    std::list<DS1631> ds1631_list;

    I2C_Device i2c_device_4b(0x4b);
    DS1631 ds1631_4b(&i2c_device_4b);
    ds1631_list.push_back(ds1631_4b);

    I2C_Device i2c_device_4c(0x4c);
    DS1631 ds1631_4c(&i2c_device_4c);
    ds1631_list.push_back(ds1631_4c);

    I2C_Device i2c_device_4f(0x4f);
    DS1631 ds1631_4f(&i2c_device_4f);
    ds1631_list.push_back(ds1631_4f);

    for (auto &ds1631_elem : ds1631_list)
    {
        ds1631_elem.StartConvert();

        ds1631_elem.ReadTemperature();

        ds1631_elem.ReadConfig();

        ds1631_elem.ReadUpperTempTripPoint();
        ds1631_elem.WriteUpperTempTripPoint(30.4);
        ds1631_elem.ReadUpperTempTripPoint();

        ds1631_elem.ReadLowerTempTripPoint();
        ds1631_elem.WriteLowerTempTripPoint(20.6);
        ds1631_elem.ReadLowerTempTripPoint();
    }
    return(0);
}