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
#include <thread>
#include <chrono>
#include <ctime>

#include "ds1631.hpp"
#include "PcfLcd.hpp"

namespace po = boost::program_options;

int main(int ac, char **av)
{
    boost::uint32_t ds1631_device_address  = -1;
    boost::uint32_t display_device_address = -1;
    bool verbose = false;

    try
    {

        po::options_description desc("Allowed options");
        desc.add_options()("help,h", "produce help message")
                          ("t_device,t", po::value<std::string>(), "set used DS1631 device (hex value) - 0 for none")
                          ("d_device,d", po::value<int>(), "set used display device (dec value 0..16)")
                          ("verbose,v", "set trace to verbose");

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

        if (vm.count("t_device"))
        {
            std::stringstream interpreter;

            interpreter << std::hex << vm["t_device"].as<std::string>();

            interpreter >> ds1631_device_address;
            if (verbose)
                std::cout << "used DS1631 device is 0x" << std::hex << ds1631_device_address << ".\n";
        }
        else
        {
            if (verbose)
                std::cout << "using all DS1631 devices.\n";
        }

        if (vm.count("d_device"))
        {
            display_device_address = vm["d_device"].as<int>();
            if (verbose)
                std::cout << "used display device is " << std::dec << display_device_address << ".\n";
        }
        else
        {
            if (verbose)
                std::cout << "no display device used.\n";
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

    if(ds1631_device_address != -1)
    {
        for (auto &ds1631_elem : ds1631_map)
        {
            if ((ds1631_device_address == ds1631_elem.first) || (ds1631_device_address == 0))
            {
                ds1631_elem.second.StartConvert();
                if (verbose)
                {
                    std::cout << "(0x" << std::hex << ds1631_elem.first << "): Temp="   << ds1631_elem.second.ReadTemperature() << std::endl;
                    std::cout << "(0x" << std::hex << ds1631_elem.first << "): Config=" << ds1631_elem.second.ReadConfig()      << std::endl;
                }
                else
                {
                    std::cout << ds1631_device_address << ":" << std::setprecision(4) << ds1631_elem.second.ReadTemperature() << std::endl;
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
    }

    std::map<short, PcfLcd> PcfLcd_map;

    if (display_device_address != -1)
    {
        short I2C_Address = PCF_Addr[display_device_address];
        std::cout << "Display (" << display_device_address << ") == (0x" << std::hex << I2C_Address << ") is used."  << std::endl;
        I2C_Device display_device(I2C_Address, verbose);
        PcfLcd display(&display_device, display_device_address, true);
        PcfLcd_map.insert(std::pair<short, PcfLcd>(I2C_Address, display));

        display.SetLight(true);
        std::this_thread::sleep_for(std::chrono::seconds(2));
        display.SetLight(false);
        std::this_thread::sleep_for(std::chrono::seconds(2));
        display.SetLight(true);
        display.home();
        display.clear();
        display.put('A');
        std::this_thread::sleep_for(std::chrono::seconds(2));
        std::this_thread::sleep_for(std::chrono::seconds(2));
        display.SetLight(false);
    }
    return (0);
}