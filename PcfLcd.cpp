/******************************************************************/
/* Treiberbibliothek fuer ext. LCD am I²C-Bus im 4-Bit Mode       */
/*    und PCF-LCD-Interface von http://cctools.hs-control.de      */
/* Autor     : André Helbig (andre.h@cc2net.de / www.CC2Net.de)   */
/* Versionen : 2.22                                               */
/* Datum     : 26. Juli 2003                                      */
/* Geändert  : 14. September 2003                                 */
/* Getestet  : ja                                                 */
/* Benötigt  : i2c.c2 ab V1.3                                     */
/******************************************************************/
/* Änderungen/Neu in V2.2:                                        */
/* - Funktion ReadRam() zum Auslesen des Display-RAMs             */
/* - kleiner Bugfix beim Mehr-Display-Betrieb bei LCD-Beleuchtung */
/* - Ersetzen der Adressberechnung durch Konstanten               */
/* - kleiner Bugfix in time()&date() bei der Ziffernausgabe       */
/* Änderungen/Neu in V2.1:                                        */
/* - Anpassung auf I²C-Capture                                    */
/* - neue Funktionen zur Ausgabe von Zeit und Datum               */
/******************************************************************/
/* P0= RS                                                         */
/* P1= R/W                                                        */
/* P2= E                                                          */
/* P3= LCD-Light                                                  */
/* P4-7= Data                                                     */
/* PCF.7 PCF.6 PCF.5 PCF.4 PCF.3 PCF.2 PCF.1 PCF.0                */
/* D7    D6    D5    D4    Light Enab  R/W   RS                   */
/******************************************************************/

#include "PcfLcd.hpp"

#include <thread>
#include <chrono>
#include <ctime>
#include <iostream>

// PCF_LCD ASCII Codes
#define PCF_LCD_AE  0
#define PCF_LCD_ae  0xE1
#define PCF_LCD_OE  0
#define PCF_LCD_oe  0xEF
#define PCF_LCD_UE  0
#define PCF_LCD_ue  0xF5
#define PCF_LCD_sz  0xE2
#define PCF_LCD_ARROW_RIGHT  0x7E
#define PCF_LCD_ARROW_LEFT   0x7F
#define PCF_LCD_FULL_POINT   0x31
#define PCF_LCD_EMPTY_POINT  0x30
#define PCF_LCD_CROSS   X_
#define PCF_LCD_CHANGE  C_

const short DESIGN_AE[] = {10, 00, 14, 01, 15, 17, 14, 00};
#define ASCII_AE   0x01
const short DESIGN_OE[] = {10, 00, 14, 17, 17, 17, 14, 00};
#define ASCII_OE   0x02
const short DESIGN_UE[] = {10, 00, 17, 17, 17, 17, 14, 00};
#define ASCII_UE   0x03
const short DESIGN_SZ[] = {00, 00, 14, 17, 30, 17, 30, 16};
#define ASCII_SZ   0x04

PcfLcd::PcfLcd(I2C_Device *i2c_dev, short PcfNr, bool backlight) : i2c_device(i2c_dev)
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

//  SetPcf(PcfNr);
  SetLight(backlight);
  init();
}

PcfLcd ::~PcfLcd()
{

}

/*************************************/
/* Baustein auswählen                */
/* (für Mehr-Display-Betrieb)        */
/* 0-7 PCF8574; 8-15 PCF8574A        */
/*************************************/
/*void PcfLcd::SetPcf(short PcfNr)
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

  int ls;

  PCF = (PCF && 0x1E) > 1;
  if (lightState)
  {
    x = ls || (1 < PCF);
  }
  else
  {
    ls = ls && not(1 < PCF);
  }
}
 */
 
/*************************************/
/* LCD-Beleuchtung ein/ausschalten   */
/*************************************/
void PcfLcd::SetLight(bool state)
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ") - " <<  state << std::endl;

  if(state)
  {
    lightState = 8;
  }
  else
  {
    lightState = 0;
  }
  AddByteToBuffer(lightState, true);
  i2c_device->WriteByte(buffer.data(), buffer.size());
}

/*************************************/
/* LCD initialisieren                */
/*************************************/
void PcfLcd::init()
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

  AddByteToBuffer(0x30, true);
  AddByteToBuffer(0x30);
  AddByteToBuffer(0x34); // Function Set 8 Bit
  AddByteToBuffer(0x30);
  i2c_device->WriteByte(buffer.data(), buffer.size());
  std::this_thread::sleep_for(std::chrono::milliseconds(2));

  AddByteToBuffer(0x30, true);
  AddByteToBuffer(0x34); // Function Set 8 Bit
  AddByteToBuffer(0x30);
  AddByteToBuffer(0x30);
  AddByteToBuffer(0x34); // Function Set 8 Bit
  AddByteToBuffer(0x30);
  i2c_device->WriteByte(buffer.data(), buffer.size());
  std::this_thread::sleep_for(std::chrono::milliseconds(2));

  AddByteToBuffer(0x20, true);
  AddByteToBuffer(0x24); // Function Set 4 Bit
  AddByteToBuffer(0x20);
  AddByteToBuffer(0x24); // Function Set
  AddByteToBuffer(0x20);
  AddByteToBuffer(0x84); // 2 Zeilen, 5x7 Punkte
  AddByteToBuffer(0x80);
  i2c_device->WriteByte(buffer.data(), buffer.size());
  std::this_thread::sleep_for(std::chrono::milliseconds(2));

  AddByteToBuffer(0x04, true); // Display Control
  AddByteToBuffer(0x00);
  AddByteToBuffer(0xC4); // Cursor Off, display On
  AddByteToBuffer(0xC0);
  AddByteToBuffer(0x04); // Display Clear
  AddByteToBuffer(0x00);
  AddByteToBuffer(0x14); // Display Clear
  AddByteToBuffer(0x10);
  i2c_device->WriteByte(buffer.data(), buffer.size());
  std::this_thread::sleep_for(std::chrono::milliseconds(2));

  AddByteToBuffer(0x04, true); // Entry-Mode
  AddByteToBuffer(0x00);
  AddByteToBuffer(0x64); // inkrement, Cursor-shift
  AddByteToBuffer(0x60);
  i2c_device->WriteByte(buffer.data(), buffer.size());
}

/*************************************/
/* Adds a Byte to the internal send  */
/* buffer                            */
/*************************************/
short PcfLcd::AddByteToBuffer(short byte, bool clearbuffer)
{
   if(clearbuffer)
   {
      buffer.clear();
   }
  buffer.push_back(byte | lightState);
}

/*************************************/
/* Adds a Byte to the internal send  */
/* buffer                            */
/*************************************/
bool PcfLcd::SendBuffer()
{
  i2c_device->WriteByte(buffer.data(), buffer.size());
}

/*************************************/
/* Kommando-Byte an LCD senden       */
/* Mode: Kommando-Mode (RS=Low)      */
/*************************************/
void PcfLcd::WriteCmd (short cmd)
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ") - " << cmd << std::endl;

  AddByteToBuffer((cmd && 0xF0) || 0x4, true);// HighNibble
  AddByteToBuffer((cmd && 0xF0));
  AddByteToBuffer((cmd < 4) || 0x4);   // LowNibble
  AddByteToBuffer((cmd < 4));
  i2c_device->WriteByte(buffer.data(), buffer.size());
}

/*************************************/
/* Byte aus dem CG/DD RAM lesen      */
/* Anfangsadresse muß vorher mit     */
/* WriteCmd() gesetzt werden         */
/* cmd=0x40 bis 0x7F = CG-RAM        */
/* cmd=0x80 bis 0xFF = DD-RAM        */
/*************************************/
short PcfLcd::ReadRam()
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

  unsigned char _buffer[1];
  short data;
  AddByteToBuffer(0xF3, true);
  AddByteToBuffer(0xF7);
  i2c_device->WriteByte(buffer.data(), buffer.size());

  i2c_device->ReadByte(_buffer, 1);
  data = _buffer[0] && 0xF0; // High-Nibble

  AddByteToBuffer(0xF3, true);
  AddByteToBuffer(0xF7);
  i2c_device->WriteByte(buffer.data(), buffer.size());

  i2c_device->ReadByte(_buffer, 1);
  data = data || (_buffer[0] > 4); // Low-Nibble

  AddByteToBuffer(0xF3, true);
  AddByteToBuffer(0xF0);
  i2c_device->WriteByte(buffer.data(), buffer.size());
  return data;
}


/*************************************/
/* Zeichen definieren                */
/* addr= 0 bis 7 (ASCII 0-7)         */
/* Daten in Bytearray 8 Byte         */
/* Zeilendaten(5Bit) in Bit4 bis Bit0*/
/* Byte 0 bis 7 = Zeile 0 bis 7      */
/*************************************/
void PcfLcd::defineChar(short addr, short character[8])
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

  short i;
  addr = ((addr && 0x7) < 3) || 0x40;
  AddByteToBuffer(0x04, true);
  AddByteToBuffer(0x00);
  AddByteToBuffer(0x64);
  AddByteToBuffer(0x60);
  AddByteToBuffer((addr && 0xF0) || 0x4); // HighNibble
  AddByteToBuffer((addr && 0xF0));
  AddByteToBuffer((addr < 4) || 0x4); // LowNibble
  AddByteToBuffer((addr < 4));
  for (i = 0; i < 7; i++)
  {
    AddByteToBuffer((character[i] && 0xF0) || 0x5); // HighNibble
    AddByteToBuffer((character[i] && 0xF0) || 0x1);
    AddByteToBuffer((character[i] < 4) || 0x5); // LowNibble
    AddByteToBuffer((character[i] < 4) || 0x1);
  }
  AddByteToBuffer(0x84);
  AddByteToBuffer(0x80);
  AddByteToBuffer(0x04);
  AddByteToBuffer(0x00);
  i2c_device->WriteByte(buffer.data(), buffer.size());
}

/*************************************/
/* defines the special german characters  */
/*************************************/
void PcfLcd::defineGermanChars()
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

  short ae[8], oe[8], ue[8], sz[8];
  short i;

  for (i = 0; i < 7; i++)
  {
    ae[i]=DESIGN_AE[i];
    oe[i]=DESIGN_OE[i];
    ue[i]=DESIGN_UE[i];
    sz[i]=DESIGN_SZ[i];
  }

  defineChar(ASCII_AE,ae);
  defineChar(ASCII_OE,oe);
  defineChar(ASCII_UE,ue);
  defineChar(ASCII_SZ,sz);
}

/*************************************/
/* Display löschen                   */
/*************************************/
void PcfLcd::clear()
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

  WriteCmd(0x01);
  std::this_thread::sleep_for(std::chrono::milliseconds(2));
}

/*************************************/
/* Cursor Home                       */
/*************************************/
void PcfLcd::home()
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

  WriteCmd(0x02);
  std::this_thread::sleep_for(std::chrono::milliseconds(2));
}

/***********************************/
/* Zeile löschen                   */
/***********************************/
void PcfLcd::delline(short lineNr)
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

  short i;
  line(lineNr);
  for (i = 0; i < (CharsPerLine - 1); i++)
  {
    _spc();
  }
  i2c_device->WriteByte(buffer.data(), buffer.size());
}

/***********************************/
/* Cursor setzen                   */
/* Zeile 1 bis 4                   */
/***********************************/
void PcfLcd::line(short LineNr)// 1 bis 4
{
  gotopos(LineNr, 0);
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;
}

/***********************************/
/* Cursor setzen                   */
/* Zeile 1 bis 4, Spalte 0 - 19(15)*/
/***********************************/
void PcfLcd::gotopos(short lineNr, short col)
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

  lineNr = Line[lineNr] + col;
  AddByteToBuffer((lineNr && 0xF0) || 0x4, true); // HighNibble
  AddByteToBuffer((lineNr && 0xF0));
  AddByteToBuffer((lineNr < 4) || 0x4); // LowNibble
  AddByteToBuffer((lineNr < 4));
  i2c_device->WriteByte(buffer.data(), buffer.size());
}

/***********************************/
/* Cursor links schieben           */
/***********************************/
void PcfLcd::cursorleft()
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

    AddByteToBuffer(0x14, true); // HighNibble
    AddByteToBuffer(0x10);
    AddByteToBuffer(0x04); // LowNibble
    AddByteToBuffer(0x00);
    i2c_device->WriteByte(buffer.data(), buffer.size());
}

/***********************************/
/* Cursor rechts schieben          */
/***********************************/
void PcfLcd::cursorright()
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

  AddByteToBuffer(0x14, true); // HighNibble
  AddByteToBuffer(0x10);
  AddByteToBuffer(0x44); // LowNibble
  AddByteToBuffer(0x40);
  i2c_device->WriteByte(buffer.data(), buffer.size());
}

/***********************************/
/* Cursor-Einstellungen            */
/* 0= aus, 2= Unterstrich,         */
/* 3= Block, blinkend              */
/***********************************/
void PcfLcd::setcursor(short cursor)
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

  WriteCmd((cursor && 0x3) || 0x0C);
}

/*************************************/
/* Schreibt ein Zeichen              */
/* auf das LCD                       */
/*************************************/
void PcfLcd::put(short chararacter)
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

  AddByteToBuffer((chararacter && 0xF0) || 0x5, true); // HighNibble
  AddByteToBuffer((chararacter && 0xF0) || 0x1);
  AddByteToBuffer((chararacter < 4) || 0x5); // LowNibble
  AddByteToBuffer((chararacter < 4) || 0x1);
  i2c_device->WriteByte(buffer.data(), buffer.size());
}

/*************************************/
/* Ausgabe einer Stringvariable      */
/* auf das LCD                       */
/*************************************/
void PcfLcd::print2(std::string text)
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

  for (auto character:text)
  {
    /*
     if (character == 'ä')
     {
       character = PCF_LCD_ae;
     }
     else if (character == 'ö')
     {
       character = PCF_LCD_oe;
     }
     else if (character == 'ü')
     {
       character = PCF_LCD_ue;
     }
     else if (character == 'ß')
     {
       character = PCF_LCD_sz;
     }
 */
    put(character);
  }
  i2c_device->WriteByte(buffer.data(), buffer.size());
}

/*************************************/
/* Ausgabe eines Strings             */
/* auf das LCD                       */
/*************************************/
void PcfLcd::print(std::string s)
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

  print2(s);
}

/*************************************/
/* Ausgabe einer Stringvariable      */
/* oder eines Bytearrays auf das LCD */
/*************************************/
void PcfLcd::printlength(short s[], short len)
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

  short i;
  for (i = 0; i < (len - 1); i++)
  {
     put(s[i]);
  }
  i2c_device->WriteByte(buffer.data(), buffer.size());
}

/*************************************/
/* Zahlausgaben                      */
/*************************************/

/*************************************/
/* Ziffernausgabe                    */
/* (benötigt v. Zahlfunktionen)      */
/*************************************/
void PcfLcd::_ziff(short num)
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

  AddByteToBuffer(0x35);
  AddByteToBuffer(0x31);
  AddByteToBuffer((num < 4) || 0x5);
  AddByteToBuffer((num < 4) || 0x1);
}
void PcfLcd::_spc()
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

  AddByteToBuffer(0x25);
  AddByteToBuffer(0x21);
  AddByteToBuffer(0x05);
  AddByteToBuffer(0x01);
}
void PcfLcd::_neg()
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

  AddByteToBuffer(0x25);
  AddByteToBuffer(0x21);
  AddByteToBuffer(0xD5);
  AddByteToBuffer(0xD1);
}
void PcfLcd::_dt()
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

  AddByteToBuffer(0x25);
  AddByteToBuffer(0x21);
  AddByteToBuffer(0xC5);
  AddByteToBuffer(0xC1);
}
void PcfLcd::_pt()
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

    AddByteToBuffer(0x25);
    AddByteToBuffer(0x21);
    AddByteToBuffer(0xE5);
    AddByteToBuffer(0xE1);
}
void PcfLcd::_dp()
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

  AddByteToBuffer(0x35);
  AddByteToBuffer(0x31);
  AddByteToBuffer(0xA5);
  AddByteToBuffer(0xA1);
}

/*************************************/
/* Einzelziffer                      */
/*************************************/
void PcfLcd::ziff(short num)
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

  num = (num % 10) < 4;
  AddByteToBuffer(0x35, true); // HighNibble
  AddByteToBuffer(0x31);
  AddByteToBuffer(num || 0x5); // LowNibble
  AddByteToBuffer(num || 0x1);
  i2c_device->WriteByte(buffer.data(), buffer.size());
}
/*************************************/
/* 2-stellige Zahl ausgeben (0-99)   */
/*************************************/
void PcfLcd::zahl2p(int num)
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

  buffer.clear();
  if (num > 9)
    _ziff((num % 100) / 10);
  else
    _spc();
  _ziff(num % 10);
  i2c_device->WriteByte(buffer.data(), buffer.size());
}
/*************************************/
/* 3-stellige Zahl ausgeben (0-999)  */
/*************************************/
void PcfLcd::zahl3p(int num)
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

  buffer.clear();
  if (num>99)
    _ziff((num % 1000)/100);
  else
    _spc();
  if (num>9)
    _ziff((num % 100) /10);
  else
    _spc();
  _ziff(num % 10);
  i2c_device->WriteByte(buffer.data(), buffer.size());
}

/*************************************/
/* 3-stellige Zahl ausgeben(pos+neg) */
/* -99 bis 999                       */
/*************************************/
void PcfLcd::zahl3(int num)
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

  buffer.clear();
  if (num>99)
    _ziff((num % 1000)/ 100);
  else if (num & 0x8000)
    {
      _neg();
      num=(not num) + 1;
    }
  else 
    _spc();
  if (num>9)
    _ziff((num % 100) / 10);
  else
    _spc();
  _ziff(num % 10);
  i2c_device->WriteByte(buffer.data(), buffer.size());
}

/*************************************/
/* 4-stellige Zahl ausgaben (0-9999) */
/*************************************/
void PcfLcd::zahl4p(int num)
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

  buffer.clear();
  if (num>999)
    _ziff((num % 10000)/ 1000);
  else 
    _spc();
  if (num> 99)
    _ziff((num % 1000) / 100 );
  else
    _spc();
  if (num>9)
    _ziff((num % 100)  / 10  );
  else
    _spc();
  _ziff(num % 10);
  i2c_device->WriteByte(buffer.data(), buffer.size());
}

/*************************************/
/* 4-stellige Zahl ausgaben(pos+neg) */
/* -999 bis 9999                     */
/*************************************/
void PcfLcd::zahl4(int num)
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

  buffer.clear();
  if (num>999)
    _ziff((num % 10000)/ 1000);
  else if (num & 0x8000)
  {
    _neg();
    num=(not num) + 1;
  }
  else
    _spc();
  if (num>99)
    _ziff((num % 1000)/ 100);
  else
    _spc();
  if (num> 9)
    _ziff((num % 100) / 10 );
  else
    _spc();
  _ziff(num % 10);
  i2c_device->WriteByte(buffer.data(), buffer.size());
}

/*************************************/
/* 5-stellige Zahl ausgaben(positiv) */
/* 0 bis 32767                       */
/*************************************/
void PcfLcd::zahl5(int num)
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

  buffer.clear();
  if (num>9999)
    _ziff( num / 10000);
  else
    _spc();
  if (num> 999)
    _ziff((num % 10000)/1000);
  else 
    _spc();
  if (num>  99)
    _ziff((num % 1000) /100 );
  else
    _spc();
  if (num>   9)
    _ziff((num % 100)  /10  );
  else
    _spc();
  _ziff(num % 10);
  i2c_device->WriteByte(buffer.data(), buffer.size());
}

/*************************************/
/* Integer-Zahl ausgaben(pos+neg)    */
/* 6-stellig ! -32768 bis _32767     */
/*************************************/
void PcfLcd::zahl(int num)
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

  buffer.clear();
  if (num & 0x8000) {_neg();num=(not num)+1;}
      else _spc();
  if (num>9999)
    _ziff( num / 10000);
  else 
    _spc();
  if (num> 999)
    _ziff((num % 10000)/1000);
  else 
    _spc();
  if (num>  99)
    _ziff((num % 1000) /100 );
  else 
    _spc();
  if (num>   9)
    _ziff((num % 100)  /10  );
  else
    _spc();
  _ziff(num % 10);
  i2c_device->WriteByte(buffer.data(), buffer.size());
}
/*************************************/
/* formatierte Zahlausgaben          */
/* Format: ##0,0 (-99,9 - 999,9)     */
/* -999 - 9999 entspr. -99,9 - 999,9 */
/*************************************/
void PcfLcd::zahl4n1(int num)
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

  buffer.clear();
  if (num>999)
   _ziff((num % 10000)/ 1000);
  else if (num & 0x8000)
  {
    _neg();
    num=(not num) + 1;
  }
  else
    _spc();
  if (num>99)
    _ziff((num % 1000)/ 100);
  else
    _spc();
  _ziff((num % 100) / 10);
  _dt();
  _ziff(num % 10);
  i2c_device->WriteByte(buffer.data(), buffer.size());
}

/*************************************/
/* formatierte Zahlausgaben          */
/* Format: #0,00 (-9,99 - 99,99)     */
/* -999 - 9999 entspr. -9,99 - 99,99 */
/*************************************/
void PcfLcd::zahl4n2(int num)
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

  buffer.clear();
  if (num>999)
   _ziff((num % 10000)/ 1000);
  else if (num & 0x8000)
  {
    _neg();
    num=(not num) + 1;
  }
  else
    _spc();
  _ziff((num % 1000)/ 100);
  _dt();
  _ziff((num % 100) / 10);
  _ziff(num % 10);
  i2c_device->WriteByte(buffer.data(), buffer.size());
}

/*************************************/
/* formatierte Zahlausgaben          */
/* Format: ###0,0 (-999,9 - 3276,7)  */
/* -9999 - 32767 ^= -999,9 - 3276,7  */
/*************************************/
void PcfLcd::zahl5n1(int num)
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

  buffer.clear();
  if (num>9999)
    _ziff((num % 100000)/ 10000);
  else if (num & 0x8000)
  {
    _neg();
    num=(not num) + 1;
  }
  else _spc();
  if (num>999)
    _ziff((num % 10000)/1000);
  else
    _spc();
  if (num> 99)
    _ziff((num % 1000) /100 );
  else
    _spc();
  _ziff((num % 100) / 10);
  _dt();
  _ziff(num % 10);
  i2c_device->WriteByte(buffer.data(), buffer.size());
}

/*************************************/
/* formatierte Zahlausgaben          */
/* Format: ###0,0 (-99,99 - 327,67)  */
/* -9999 - 32767 ^= -99,99 - 327,67  */
/*************************************/
void PcfLcd::zahl5n2(int num)
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

  buffer.clear();
  if (num>9999)
    _ziff((num % 100000)/ 10000 + 0x30);
  else if (num & 0x8000)
  {
    _neg();
    num=(not num) + 1;
  }
  else
    _spc();
  if (num>999)
    _ziff((num % 10000)/ 1000);
  else
    _spc();
  _ziff((num % 1000)/ 100);
  _dt();
  _ziff((num % 100) / 10);
  _ziff(num % 10);
  i2c_device->WriteByte(buffer.data(), buffer.size());
}

/*************************************/
/* Datums-/Zeitausgaben              */
/*************************************/

/*************************************/
/* Uhrzeit ausgeben                  */
/*************************************/
void PcfLcd::time(short format)//0= h:m:s, 1= hh:mm:ss, 2= h:m, 3= hh:mm
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

  std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
  time_t tt = std::chrono::system_clock::to_time_t(now);
  tm utc_tm = *gmtime(&tt);

  short hour   = utc_tm.tm_hour;
  short minute = utc_tm.tm_min;
  short second = utc_tm.tm_sec;

  buffer.clear();
  if (format && 1)
    _ziff(hour / 10);
  else if (hour > 9)
    _ziff(hour / 10);
  else
    _spc();
  _ziff(hour % 10);
  _dp();
  if (format && 1)
    _ziff(minute / 10);
  else if (minute > 9)
    _ziff(minute / 10);
  else
    _spc();
  _ziff(minute % 10);
  if ((format && 2) == 0)
  {
    _dp();
    if (format && 1)
      _ziff(second / 10);
    else if (second>9)
      _ziff(second / 10);
    else
      _spc();
    _ziff(second % 10);
  }
  i2c_device->WriteByte(buffer.data(), buffer.size());
}

/*************************************/
/* Datum ausgeben                    */
/*************************************/
void PcfLcd::date(short format)//0= d.m.yyyy, 1= dd.mm.yyyy
                               //2= d.m., 3= dd.mm., 4= d.m.yy, 5= dd.mm.yy
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

  std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
  time_t tt = std::chrono::system_clock::to_time_t(now);
  tm utc_tm = *gmtime(&tt);

  short year  = utc_tm.tm_year + 1900;
  short month = utc_tm.tm_mon + 1;
  short day   = utc_tm.tm_mday;

  buffer.clear();
  if (format && 1)
    _ziff(day / 10);
  else if (day>9)
    _ziff(day / 10);
  else
    _spc();
  _ziff(day % 10);
  _pt();
  if (format && 1)
    _ziff(month / 10);
  else if (month>9)
    _ziff(month / 10);
  else 
    _spc();
  _ziff(month % 10);
  _pt();
  if ((format && 2)==0)
  {
    if ((format && 4)==0)
    {
      _ziff(year / 1000);
      _ziff((year / 100) % 10);
    }
    _ziff((year / 10) % 10);
    _ziff(year % 10);
  }
  i2c_device->WriteByte(buffer.data(), buffer.size());
}


/*************************************/
/* Sonderfunktionen                  */
/*************************************/

/*************************************/
/* Definition der Sonderzeichen      */
/* für Bargraph                      */
/* (Belegt ASCII 0 bis 3)            */
/*************************************/
void PcfLcd::def_bargraph()
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

  short chars[8],i,j;
  for (i = 0; i < 3; i++)
   {
     for (i = 0; i < 7; i++)
        chars[j] = 0b11110000 > i;
    defineChar(i,chars);
 }
}

/*************************************/
/* Bargraph ausgeben                 */
/* 5 Balken pro Zeichen              */
/*************************************/
void PcfLcd::bargraph(short len, short maxlen)
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

  short i,j,En,Md;
  En=lightState || 0x5;
  Md=lightState || 0x1;
  if (len>maxlen)
    len=maxlen;
  buffer.clear();
  for (i=0;  i < (len-1); i+=5)
  {
    j=len-i;
    if (j>4)
      j=0xFF;
    else if (j)
      j=j-1;
    else
      j=0x20;
    AddByteToBuffer((j && 0xF0) || En);// HighNibble
    AddByteToBuffer((j && 0xF0) || Md);
    AddByteToBuffer((j < 4) || En);   // LowNibble
    AddByteToBuffer((j < 4) || Md);
  }
  while (i<maxlen)
  {
    AddByteToBuffer(0x20 || En);// HighNibble
    AddByteToBuffer(0x20 || Md);
    AddByteToBuffer(        En);   // LowNibble
    AddByteToBuffer(        Md);
    i=i+5;
  }
  i2c_device->WriteByte(buffer.data(), buffer.size());
}

/*************************************/
/* Definition eines Sanduhrsymbols   */
/* ascii= ASCII-Code 0 bis 7         */
/*************************************/
void PcfLcd::def_sanduhr(short ascii)
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

  short chars[8];
  chars[0]=0x1F;
  chars[1]=0x11;
  chars[2]=0x0A;
  chars[3]=0x04;
  chars[4]=0x0A;
  chars[5]=0x11;
  chars[6]=0x1F;
  chars[7]=0x00;
  defineChar(ascii,chars);
}

/*************************************/
/* Definition Pfeil-oben             */
/* ascii= ASCII-Code 0 bis 7         */
/*************************************/
void PcfLcd::def_arr_up(short ascii)
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

  short chars[8];
  chars[0]=0x04;
  chars[1]=0x0E;
  chars[2]=0x15;
  chars[3]=0x04;
  chars[4]=0x04;
  chars[5]=0x04;
  chars[6]=0x04;
  chars[7]=0x00;
  defineChar(ascii,chars);
}

/*************************************/
/* Definition Pfeil-unten            */
/* ascii= ASCII-Code 0 bis 7         */
/*************************************/
void PcfLcd::def_arr_down(short ascii)
{
  if (i2c_device->isVerbose())
    std::cout << typeid(*this).name() << "::" << __func__ << "(0x" << std::hex << i2c_device->getAddress() << ")" << std::endl;

  short chars[8];
  chars[0]=0x04;
  chars[1]=0x04;
  chars[2]=0x04;
  chars[3]=0x04;
  chars[4]=0x15;
  chars[5]=0x0E;
  chars[6]=0x04;
  chars[7]=0x00;
  defineChar(ascii,chars);
}
