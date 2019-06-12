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

#include <iostream>
#include <map>
#include <boost/program_options.hpp>
#include <exception>

#include "ds1631.hpp"

namespace po = boost::program_options;

int main(int ac, char **av)
{
    boost::uint32_t device_address = 0;

    try
    {

        po::options_description desc("Allowed options");
        desc.add_options()("help", "produce help message")("device", po::value<std::string>(), "set used device (hex value)");

        po::variables_map vm;
        po::store(po::parse_command_line(ac, av, desc), vm);
        po::notify(vm);

        if (vm.count("help"))
        {
            std::cout << desc << "\n";
            return 0;
        }

        if (vm.count("device"))
        {
            std::stringstream interpreter;

            interpreter << std::hex << vm["device"].as<std::string>();

            interpreter >> device_address;
            std::cout << "used device is " << std::hex << device_address << ".\n";
        }
        else
        {
            std::cout << "using all devices.\n";
        }
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
    catch (...)
    {
        std::cerr << "Exception of unknown type!\n";
        return 1;
    }

    std::map<short, DS1631> ds1631_map;

    I2C_Device i2c_device_48(0x48);
    DS1631 ds1631_48(&i2c_device_48);
    ds1631_map.insert(std::pair<short, DS1631>(0x48, ds1631_48));

    I2C_Device i2c_device_4b(0x4b);
    DS1631 ds1631_4b(&i2c_device_4b);
    ds1631_map.insert(std::pair<short, DS1631>(0x4b, ds1631_4b));

    I2C_Device i2c_device_4c(0x4c);
    DS1631 ds1631_4c(&i2c_device_4c);
    ds1631_map.insert(std::pair<short, DS1631>(0x4c, ds1631_4c));

    I2C_Device i2c_device_4f(0x4f);
    DS1631 ds1631_4f(&i2c_device_4f);
    ds1631_map.insert(std::pair<short, DS1631>(0x4f, ds1631_4f));

    if (device_address == 0)
    {
        for (auto &ds1631_elem : ds1631_map)
        {
            ds1631_elem.second.StartConvert();
            ds1631_elem.second.ReadTemperature();
            ds1631_elem.second.ReadConfig();

            ds1631_elem.second.ReadUpperTempTripPoint();
            ds1631_elem.second.WriteUpperTempTripPoint(30.4);
            ds1631_elem.second.ReadUpperTempTripPoint();

            ds1631_elem.second.ReadLowerTempTripPoint();
            ds1631_elem.second.WriteLowerTempTripPoint(20.6);
            ds1631_elem.second.ReadLowerTempTripPoint();
        }
    }
    else
    {
        std::map<short, DS1631>::iterator it;
        it = ds1631_map.find(device_address);
        if (it != ds1631_map.end())
        {
            ds1631_map.find(device_address)->second.ReadTemperature();
            ;
        }
        else
        {
            std::cout << "no device with address 0x" << std::hex << device_address << " found." << std::endl;
        }
    }
    
    return(0);
}