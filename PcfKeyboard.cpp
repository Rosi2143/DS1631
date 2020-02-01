/******************************************************************/
/* Treiberbibliothek fuer eine Matrixtastatur am PCF8574          */
/* Author    : Andr� Helbig (andre.h@cc2net.de / www.CC2Net.de)   */
/* Versionen : 1.1                                                */
/* Datum     : 20. Juli 2003                                      */
/* Getestet  : ja                                                 */
/* Ben�tigt  : i2c.c2 ab V1.2, ports.c2                           */
/******************************************************************/
/* P0= y0(1.2.3)                                                  */
/* P1= y1(4.5.6)                                                  */
/* P2= y2(7.8.9)                                                  */
/* P3= y3(*.0.#)                                                  */
/* P4= x0(3.6.9.#)                                                */
/* P5= x1(2.5.8.0)                                                */
/* P6= x2(1.4.7.*)                                                */
/* P7= frei (bzw. x3 f�r 4x4 Matrix)                              */
/* Die Tastenbelegung kann bei versch. Tastaturen abweichen       */
/* und mu� evtl. angepasst werden.                                */
/******************************************************************/
/* Der INT-Ausgang des PCF8574 mu� mit einem I/O-Port + Pull-UP   */
/* angeschlossen werden                                           */
/******************************************************************/

//const IntPort=15;// Port f�r PCF-Interrupt (0 bis 15)
// Dieser Port ben�tigt einen Pull-Up Widerstand (z.B. 10k)
const Char[] = 0x33, 0x36, 0x39, 0x23, // rechte Spalte
    0x32, 0x35, 0x38, 0x30,            // mittlere Spalte
    0x31, 0x34, 0x37, 0x2A,            // linke Spalte
    'A', 'B', 'C', 'D';                // Nur f�r 4x4 Matrix
                                       //Das Array Char[] mu� f�r eine 4x4 Matrix angepasst werden.
const NUM_OF_PCFKEYB = 3;
byte PCF, pressed, Matrix;
byte IntPort;

/*************************************/
/* Baustein ausw�hlen                */
/* (f�r Mehr-Display-Betrieb)        */
/* 0-7 PCF8574; 8-15 PCF8574A        */
/*************************************/
function setpcf(byte pcfnr)
{
    if (pcfnr > 7)
    {
        PCF = ((pcfnr + 16) shl 1) or 64;
    }
    else
    {
        PCF = (pcfnr shl 1) or 64;
    }
}

/****************************************/
/* PCF8574 f�r Matrix-Tastatur          */
/* initialisieren                       */
/****************************************/
function init(byte pcf, byte matrix, byte intport)
//pcf: 0 bis 7 =PCF8574; 8 bis 15 =PCF8574A
//matrix: 0 = 3x4; 1= 4x4
{
    setpcf(pcf);
    IntPort = intport;

    Matrix = (matrix == 0) and 0x80;
    i2c.cstart(PCF);
    i2c.write(Matrix or 0xF);
    i2c.stop();
}

function getstate() returns int
{
    int chars;
    byte i;
    chars = 0;
 for
     i = 0 ... 3 - (Matrix shr 7)
     {
         i2c.cstart(PCF);
         i2c.write(not(0x10 shl i) or Matrix);
         i2c.stop();
         i2c.cstart(PCF or 1);
         chars = chars or ((not i2c.readlast() and 0xF) shl(4 * i));
         i2c.stop();
     }
 i2c.cstart(PCF);
 i2c.write(0xF or Matrix);
 i2c.stop();
 return chars;
}
/****************************************/
/* Auf Tasteneingabe warten und         */
/* Wert als Int-Bitmaske zur�ckgeben    */
/* Mit dieser Funktion k�nnen auch      */
/* mehrere gleichzeitiggedr�ckte Tasten */
/* abgefragt werden                     */
/****************************************/
function getbin(byte delay) returns int //delay in ms = Wartezeit bis Abfrage
{
    int chars, chars2;
    wait ports.get(IntPort) == 0;
    if
        delay
        {
            chars = getstate();
            sleep delay;
        }
    chars2 = getstate();
    if
        chars2
        {
            chars = chars2;
            pressed = 0xFF;
        }
    else
        pressed = 0;
    return chars;
}

/****************************************/
/* Auf Tasteneingabe warten und         */
/* Wert als ASCII-Zeichen zur�ckgeben   */
/****************************************/
function getchar(byte delay) returns byte //delay in ms = Wartezeit bis Abfrage
{
    int chars, chars2;
    byte i;
    chars = 0;
    wait ports.get(IntPort) == 0;
    if
        delay
        {
            chars = getstate();
            sleep delay;
        }
    chars2 = getstate();
    if
        chars2
        {
            chars = chars2;
            pressed = 0xFF;
        }
    else
        pressed = 0;
 for
     i = 0...15 if chars and (1 shl i) return Char[i]; // R�ckgabe des ASCII-Codes beim Dr�cken einer Taste
 return 0;                                             // R�ckgabe von 0 beim loslassen einer Taste
}

/****************************************/
/* Auf Tasteneingabe warten und         */
/* Wert als String zur�ckgeben          */
/* Mit dieser Funktion k�nnen auch      */
/* mehrere gleichzeitiggedr�ckte Tasten */
/* abgefragt werden. Die Ausgabe erfolgt*/
/* als Zeichenkette.                    */
/****************************************/
function getstr(byte delay, byte s[]) returns byte //delay in ms = Wartezeit bis Abfrage
{
    int chars, chars2;
    byte i, j;
    wait ports.get(IntPort) == 0;
    if
        delay
        {
            chars = getstate();
            sleep delay;
        }
    chars2 = getstate();
    if
        chars2 chars = chars2;
    if
        chars2 pressed = 0xFF;
    else
        pressed = 0;
    j = 0;
 for
     i = 0...15 if chars and (1 shl i)
     {
         s[j] = Char[i];
         j = j + 1;
     }
 s[31] = j;
 return j; //R�ckgabe der Stringl�nge = Anzahl der gedr�ckten Tasten
}

/****************************************/
/* Abfrage, ob alle Tasten losgelassen  */
/* sind.                                */
/* Wenn noch eine Taste gedr�ckt ist,   */
/* wird bei Angabe von "warte" gewartet */
/* bis alle Tasten losgelassen wurden   */
/****************************************/
function released(byte warte) returns int
{
    int chars;
    byte i, j;
    if
        pressed == 0 return -1;
    if
        warte == 0 and ports.get(IntPort) return 0;
    loop
    {
        wait ports.get(IntPort) == 0;
        chars = getstate();
        if
            chars == 0 break;
    }
    pressed = 0;
    return -1;
}
