/******************************************************************/
/* CCTools Routinen für die C-Control Pro                         */
/* zum PCF-LCD-Interface und I2C-Terminal                         */
/* (http://cctools.hs-control.de)                                 */
/* Autor     : André Helbig                                       */
/* Versionen : 1.0                                                */
/* Datum     : 09. Oktober 2005                                   */
/* Getestet  : ja                                                 */
/* Benötigt  : IntFunc_Lib.cc (Library)                           */
/******************************************************************/
/* Portbelegung PCF8574                                           */
/* P.0= RS                                                        */
/* P.1= R/W                                                       */
/* P.2= E                                                         */
/* P.3= LCD-Light                                                 */
/* P.4= D4                                                        */
/* P.5= D5                                                        */
/* P.6= D6                                                        */
/* P.7= D7                                                        */
/******************************************************************/
/* Adressen: (nur gerade Werte !)                                 */
/*  0x40-0x4E : PCF8574   Format 0b 0 1 0 0 A2 A1 A0 0            */
/*  0x70-0x7E : PCF8574A  Format 0b 0 1 1 1 A2 A1 A0 0            */
/******************************************************************/

byte PCFLCDlight;

//------------------------------------------------------------------------------
// Beleuchtung schalten
//------------------------------------------------------------------------------
void PCFLCD_SetLight(byte Adresse, int state)
{
 PCFLCDlight=(state!=0) & 8;
 Adresse=Adresse & 0xFE;
 I2C_Start();
 I2C_Write(Adresse);
 I2C_Write(PCFLCDlight);
 I2C_Stop();
}

//------------------------------------------------------------------------------
// LCD initialisieren
//------------------------------------------------------------------------------
void PCFLCD_Init(byte Adresse)
{
 PCFLCDlight=PCFLCDlight & 8;
 Adresse=Adresse & 0xFE;
 I2C_Start();
 I2C_Write(Adresse);
 I2C_Write(0x30 | PCFLCDlight);
 I2C_Write(0x30 | PCFLCDlight);
 I2C_Write(0x34 | PCFLCDlight); // Function Set 8 Bit
 I2C_Write(0x30 | PCFLCDlight);
 AbsDelay(2);

 I2C_Write(0x30 | PCFLCDlight);
 I2C_Write(0x34 | PCFLCDlight); // Function Set 8 Bit
 I2C_Write(0x30 | PCFLCDlight);
 I2C_Write(0x30 | PCFLCDlight);
 I2C_Write(0x34 | PCFLCDlight); // Function Set 8 Bit
 I2C_Write(0x30 | PCFLCDlight);

 I2C_Write(0x20 | PCFLCDlight);
 I2C_Write(0x24 | PCFLCDlight); // Function Set 4 Bit
 I2C_Write(0x20 | PCFLCDlight);
 I2C_Write(0x24 | PCFLCDlight); // Function Set
 I2C_Write(0x20 | PCFLCDlight);
 I2C_Write(0x84 | PCFLCDlight); // 2 Zeilen, 5x7 Punkte
 I2C_Write(0x80 | PCFLCDlight);

 I2C_Write(0x04 | PCFLCDlight); // Display Control
 I2C_Write(0x00 | PCFLCDlight);
 I2C_Write(0xC4 | PCFLCDlight); // Cursor Off, display On
 I2C_Write(0xC0 | PCFLCDlight);
 I2C_Write(0x04 | PCFLCDlight); // Display Clear
 I2C_Write(0x00 | PCFLCDlight);
 I2C_Write(0x14 | PCFLCDlight); // Display Clear
 I2C_Write(0x10 | PCFLCDlight);
 AbsDelay(2);
 I2C_Write(0x04 | PCFLCDlight); // Entry-Mode
 I2C_Write(0x00 | PCFLCDlight);
 I2C_Write(0x64 | PCFLCDlight); // inkrement, Cursor-shift
 I2C_Write(0x60 | PCFLCDlight);
 I2C_Stop();
}

//------------------------------------------------------------------------------
// Kommando senden
//------------------------------------------------------------------------------
void PCFLCD_WriteCmd (byte Adresse, byte cmd)
{
 Adresse=Adresse & 0xFE;
 I2C_Start();
 I2C_Write(Adresse);
 I2C_Write((cmd & 0xF0) | PCFLCDlight | 0x4);// HighNibble
 I2C_Write((cmd & 0xF0) | PCFLCDlight);
 I2C_Write((cmd << 4) | PCFLCDlight | 0x4);   // LowNibble
 I2C_Write((cmd << 4) | PCFLCDlight);
 I2C_Stop();
}

//------------------------------------------------------------------------------
// Zeichen ausgeben
//------------------------------------------------------------------------------
void PCFLCD_Put(byte Adresse, char Zeichen)
{
 Adresse=Adresse & 0xFE;
 I2C_Start();
 I2C_Write(Adresse);
 I2C_Write((Zeichen & 0xF0) | PCFLCDlight | 0x5);// HighNibble
 I2C_Write((Zeichen & 0xF0) | PCFLCDlight | 0x1);
 I2C_Write((Zeichen << 4) | PCFLCDlight | 0x5);   // LowNibble
 I2C_Write((Zeichen << 4) | PCFLCDlight | 0x1);
 I2C_Stop();
}



//------------------------------------------------------------------------------
// Hauptprogramm
//
void main(void)
{
 I2C_Init(I2C_100kHz);                   // I2C Bit Rate: 100 kHz

 PCFLCD_Init(0x40);
 PCFLCD_Put(0x40,'T');
 PCFLCD_Put(0x40,'e');
 PCFLCD_Put(0x40,'s');
 PCFLCD_Put(0x40,'t');
 PCFLCD_WriteCmd(0x40,0xC0);
 PCFLCD_Put(0x40,'1');
 PCFLCD_Put(0x40,'2');
 PCFLCD_Put(0x40,'3');
 PCFLCD_Put(0x40,'4');


 while (1)
 {
 }
}
