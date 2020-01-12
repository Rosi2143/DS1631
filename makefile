CPPFLAGS=-c -std=c++11 -g
LDFLAGS=-g
LDLIBS=-lboost_program_options

ds1631: I2C_Device.o ds1631.o PcfLcd.o main.o 
	c++ $(LDFLAGS) -o ds1631 main.o I2C_Device.o ds1631.o PcfLcd.o $(LDLIBS)

main.o: main.cpp
	c++ $(CPPFLAGS) main.cpp

I2C_Device.o: I2C_Device.cpp
	c++ $(CPPFLAGS) I2C_Device.cpp

ds1631.o: ds1631.cpp
	c++ $(CPPFLAGS) ds1631.cpp

PcfLcd.o: PcfLcd.cpp
	c++ $(CPPFLAGS) PcfLcd.cpp