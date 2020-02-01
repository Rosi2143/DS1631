/**
 * @file PcfLcd.hpp
 * @author Michael Rossner (Schrott.Micha@web.de)
 * @brief interface definition for the class to interact with the external interface of CC2-Tools
 * @version 0.1
 * @date 2019-06-10
 * 
 * @copyright Copyright (c) 2019
 * MIT license - see license file
 */

#pragma once

#include "I2C_Device.hpp"

#include <string>
#include <vector>

const short PCF_Addr[16] = {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, //PCF8574-Adressen
                            0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F};

class PcfLcd
{
private:
    I2C_Interface *i2c_device;

public:
    PcfLcd(I2C_Device *i2c_dev, short PcfNr, bool backlight);
    ~PcfLcd();

//    void SetPcf(short PcfNr);
    void SetLight(bool state);
    void init();
    short ReadRam();
    void defineChar(short addr, short chararacter[]);
    void defineGermanChars();
    
    short AddByteToBuffer(short byte, bool clearbuffer = false);
    bool SendBuffer();
    void WriteCmd(short const cmd, bool const fourBitMode = true);
    void WriteData(short const cmd);
    void WriteOut(short const cmd, bool const RegisterSelect);

    void clear();
    void home();
    void delline(short const lineNr);
    void line(short const lineNr);
    void gotopos(short const lineNr, short const col);
    void cursorleft();
    void cursorright();
    void setcursor(short cursor);
    void put(short chararacter);
    
    void print2(std::string text);
    void print(std::string s);
    void printlength(short s[], short len);

    void ziff(short num);
    void zahl2p(int num);
    void zahl3p(int num);
    void zahl3(int num);
    void zahl4p(int num);
    void zahl4(int num);
    void zahl5(int num);
    void zahl(int num);
    void zahl4n1(int num);
    void zahl4n2(int num);
    void zahl5n1(int num);
    void zahl5n2(int num);

    void time(short format);
    void date(short format);

    void def_bargraph();
    void bargraph(short len, short maxlen);
    void def_sanduhr(short ascii);
    void def_arr_up(short ascii);
    void def_arr_down(short ascii);


protected :
    void _ziff(short num);
    void _spc();
    void _neg();
    void _dt();
    void _pt();
    void _dp();

    short lightState;

    std::vector<unsigned char> buffer;

    static const short NumerOfLines = 4;                              // für 4x20 & zweizeilige LCD
    static const short CharsPerLine = 20;                             // für 4x20 & zweizeilige LCD
    const short Line[NumerOfLines] = {0x80, 0xC0, 0x94, 0xD4}; // für 4x20 & zweizeilige LCD
    //const CharsPerLine=16;                  // für 4x16 LCD
    //const Line[]= 0x80,0x80,0xC0,0x90,0xD0; // für 4x16 LCD
};