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
#include <iomanip>
#include <map>
#include <boost/program_options.hpp>
#include <exception>

#include "ds1631.hpp"

namespace po = boost::program_options;

int main(int ac, char **av)
{
    boost::uint32_t device_address = 0;
    bool verbose = false;

    try
    {

        po::options_description desc("Allowed options");
        desc.add_options()("help", "produce help message")("device", po::value<std::string>(), "set used device (hex value)")("verbose", "set trace to verbose");

        po::variables_map vm;
        po::store(po::parse_command_line(ac, av, desc), vm);
        po::notify(vm);

        if (vm.count("verbose"))
        {
            verbose = true;
        }

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
            if (verbose)
                std::cout << "used device is " << std::hex << device_address << ".\n";
        }
        else
        {
            if (verbose)
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

    I2C_Device i2c_device_48(0x48, verbose);
    DS1631 ds1631_48(&i2c_device_48);
    ds1631_map.insert(std::pair<short, DS1631>(0x48, ds1631_48));

    I2C_Device i2c_device_4b(0x4b, verbose);
    DS1631 ds1631_4b(&i2c_device_4b);
    ds1631_map.insert(std::pair<short, DS1631>(0x4b, ds1631_4b));

    I2C_Device i2c_device_4c(0x4c, verbose);
    DS1631 ds1631_4c(&i2c_device_4c);
    ds1631_map.insert(std::pair<short, DS1631>(0x4c, ds1631_4c));

    I2C_Device i2c_device_4f(0x4f, verbose);
    DS1631 ds1631_4f(&i2c_device_4f);
    ds1631_map.insert(std::pair<short, DS1631>(0x4f, ds1631_4f));

    for (auto &ds1631_elem : ds1631_map)
    {
        if ((device_address == ds1631_elem.first) || (device_address == 0))
        {
            ds1631_elem.second.StartConvert();
            if (verbose)
            {
                std::cout << "(0x" << std::hex << ds1631_elem.first << "): Temp=" << ds1631_elem.second.ReadTemperature() << std::endl;
                std::cout << "(0x" << std::hex << ds1631_elem.first << "): Config=" << ds1631_elem.second.ReadConfig() << std::endl;
            }
            else
            {
                std::cout << std::setprecision(4) << ds1631_elem.second.ReadTemperature();
            }
            
    /*
            ds1631_elem.second.ReadUpperTempTripPoint();
            ds1631_elem.second.WriteUpperTempTripPoint(30.4);
            ds1631_elem.second.ReadUpperTempTripPoint();

            ds1631_elem.second.ReadLowerTempTripPoint();
            ds1631_elem.second.WriteLowerTempTripPoint(20.6);
            ds1631_elem.second.ReadLowerTempTripPoint();
            */
        }
    }
    
    return(0);
}