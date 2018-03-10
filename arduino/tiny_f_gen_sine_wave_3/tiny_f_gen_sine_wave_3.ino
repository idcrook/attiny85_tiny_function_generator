/* 
 *  Adapted by @idcrook for 
 *  - Adafruit LCD 16x2 I2C/SPI Backpack
 *  - KY-040 Rotary Encoder with a switch
 *  
 *  Tiny Function Generator with Sine Wave

   David Johnson-Davies - www.technoblogy.com - 7th March 2018
   ATtiny85 @ 8 MHz (internal PLL; BOD disabled)
   
   http://www.technoblogy.com/show?20W6
   http://www.technoblogy.com/list?22ML
   
   CC BY 4.0
   Licensed under a Creative Commons Attribution 4.0 International license: 
   http://creativecommons.org/licenses/by/4.0/
*/

// include the library code
#include <TinyWireM.h>      // Wire/I2C library for ATtiny85
#include <TinyLiquidCrystal.h>
#include <avr/power.h>      // needed to up clock to 16 MHz on 5v Trinket

// Connect display via  i2c, default address #0 (A0-A2 not jumpered)
TinyLiquidCrystal lcd(0);

#define NOINIT __attribute__ ((section (".noinit")))

// Don't initialise these on reset, since we use warm reset to switch waveform
int Wave NOINIT;
unsigned int Freq NOINIT;
int8_t Sinewave[256] NOINIT;

typedef void (*wavefun_t)();

// Direct Digital Synthesis **********************************************

volatile unsigned int Acc, Jump;
volatile signed int X, Y;

void SetupDDS () {
  // Enable 64 MHz PLL and use as source for Timer1
  PLLCSR = 1<<PCKE | 1<<PLLE;

  // Set up Timer/Counter1 for PWM output
  TIMSK = 0;                               // Timer interrupts OFF
  TCCR1 = 1<<PWM1A | 2<<COM1A0 | 1<<CS10;  // PWM A, clear on match, 1:1 prescale
  pinMode(1, OUTPUT);                      // Enable PWM output pin

  // Set up Timer/Counter0 for 20kHz interrupt to output samples.
  TCCR0A = 3<<WGM00;                       // Fast PWM
  TCCR0B = 1<<WGM02 | 2<<CS00;             // 1/8 prescale
  TIMSK = 1<<OCIE0A;                       // Enable compare match, disable overflow
  OCR0A = 60;                              // Divide by 61
}

// Calculate sine wave
void CalculateSine () {
  int X=0, Y=8180;
  for (int i=0; i<256; i++) {
    X = X + (Y*4)/163;
    Y = Y - (X*4)/163;
    Sinewave[i] = X>>6;
  }
}

void Sine () {
  Acc = Acc + Jump;
  OCR1A = Sinewave[Acc>>8] + 128;
}

void Sawtooth () {
  Acc = Acc + Jump;
  OCR1A = Acc >> 8;
}

void Square () {
  Acc = Acc + Jump;
  int8_t temp = Acc>>8;
  OCR1A = temp>>7;
}

void Rectangle () {
  Acc = Acc + Jump;
  int8_t temp = Acc>>8;
  temp = temp & temp<<1;
  OCR1A = temp>>7;
}

void Triangle () {
  int8_t temp, mask;
  Acc = Acc + Jump;
  temp = Acc>>8;
  mask = temp>>7;
  temp = temp ^ mask;
  OCR1A = temp<<1;
}

void Chainsaw () {
  int8_t temp, mask, top;
  Acc = Acc + Jump;
  temp = Acc>>8;
  mask = temp>>7;
  top = temp & 0x80;
  temp = (temp ^ mask) | top;
  OCR1A = temp;
}

void Pulse () {
  Acc = Acc + Jump;
  int8_t temp = Acc>>8;
  temp = temp & temp<<1 & temp<<2 & temp<<3;
  OCR1A = temp>>7;
}

void Noise () {
  int8_t temp = Acc & 1;
  Acc = Acc >> 1;
  if (temp == 0) Acc = Acc ^ 0xB400;
  OCR1A = Acc;
}

const int nWaves = 8;
wavefun_t Waves[nWaves] = {Sine, Triangle, Sawtooth, Square, Rectangle, Pulse, Chainsaw, Noise};
wavefun_t Wavefun;

ISR(TIMER0_COMPA_vect) {
  Wavefun();
}


// LCD Display **************************************************

// Character set for waveform icons - stored in program memory
// Two characters per waveform icon on 5x8 dot position
// Eight (8) waveforms = 8 byte/char * 2 char/wave * 8 wave = 128 bytes
const uint8_t CharMap[128] PROGMEM = {
  // sine
  0b00110,
  0b01001,
  0b10000,
  0b10000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  
  0b00000,
  0b00000,
  0b00000,
  0b10000,
  0b10000,
  0b10000,
  0b01001,
  0b00110,

  // Triangle
  0b00000,
  0b00010,
  0b00101,
  0b01000,
  0b10000,
  0b00000,
  0b00000,
  0b00000,
  
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b10000,
  0b01000,
  0b00101,
  0b00010,

  //Sawtooth
  0b00000,
  0b00001,
  0b00011,
  0b00101,
  0b01001,
  0b10001,
  0b00001,
  0b00000,

  0b00000,
  0b00001,
  0b00011,
  0b00101,
  0b01001,
  0b10001,
  0b00001,
  0b00000,

  // Square
  0b00000,
  0b11111,
  0b10001,
  0b10001,
  0b10001,
  0b10001,
  0b10001,
  0b00000,
  
  0b00000,
  0b00111,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b11100,
  0b00000,

  // Rectangle
  0b00000,
  0b11000,
  0b11000,
  0b11000,
  0b11000,
  0b11000,
  0b11111,
  0b00000,
  
  0b00000,
  0b00110,
  0b00110,
  0b00110,
  0b00110,
  0b00110,
  0b11111,
  0b00000,
  
  // Pulse
  0b00000,
  0b01000,
  0b01000,
  0b01000,
  0b01000,
  0b01000,
  0b11111,
  0b00000,

  0b00000,
  0b00001,
  0b00001,
  0b00001,
  0b00001,
  0b00001,
  0b11111,
  0b00000,

  // Chainsaw
  0b00010,
  0b00110,
  0b01010,
  0b10010,
  0b10001,
  0b10000,
  0b10000,
  0b00000,

  0b00001,
  0b00011,
  0b00101,
  0b01001,
  0b01001,
  0b11000,
  0b01000,
  0b00000,

  // Noise
  0b00000,
  0b10100,
  0b10110,
  0b11101,
  0b01101,
  0b01001,
  0b01000,
  0b00000,

  0b00000,
  0b00100,
  0b01100,
  0b01101,
  0b01111,
  0b11011,
  0b10010,
  0b00000,
  
};

// I2C pins implicit in <TinyWireM.h> for ATtiny85
//const int SDAPin = 0;
//const int SCLPin = 2;

// borrowed concept from https://forum.arduino.cc/index.php?topic=499365.msg3429492#msg3429492
byte lcdbuffer[8]; // progmem data in for pickup by lcd.create
byte nocustomchr = 8;

void initializeLCD(int bank) {

  lcd.begin(16, 2);  // set up the LCD's number of rows and columns: 
  lcd.setBacklight(HIGH); // Set backlight
  /*
      Initialize custom graphic characters for LCD from PROGMEM
  */
  byte i;
  uint8_t * ccpoint;
  ccpoint = lcdbuffer;
  for ( i = 0; i < nocustomchr; i++) {
    if (bank == 1) {
      memcpy_P (&lcdbuffer, &CharMap[(i * 8)], 8);
    } else if (bank == 2) {
      memcpy_P (&lcdbuffer, &CharMap[(i * 8) + sizeof CharMap / 2], 8);
    }
    lcd.createChar(i, ccpoint);
  }
}

void PlotIcon (int wave, int line, int column) {
  if (0) {
    lcd.setCursor(column, 0);
    // TODO: print text desc string of waveform
    lcd.print(wave);
  }

  int wave_in_bank = wave % 4; // 4 waveforms * 2 chars each gets to max of 8 custom display chars
  
  lcd.setCursor(column, line);
  lcd.write(2*wave_in_bank);
  lcd.setCursor(column+1, line);
  lcd.write(2*wave_in_bank + 1);
}

// Display a 5-digit frequency starting at line, column
void PlotFreq (unsigned int freq, int line, int column) {
  lcd.setCursor(column, line);
  lcd.print(freq);
  lcd.print("Hz   ");
}

// Rotary encoder **********************************************
const int EncoderAPin = 3;
const int EncoderBPin = 4;

const int MinFreq = 1;        // Hz
const int MaxFreq = 5000;     // Hz

volatile int Count = -2; // rotary encoder state machine steady/starting state
volatile byte abOld;     // Initialize state

void SetupRotaryEncoder () {
  // Encoder pins connect to ground through a 10k and pulled up to VCC/5V when not in contact
  pinMode(EncoderAPin, INPUT);  
  pinMode(EncoderBPin, INPUT);
  
  PCMSK = 1<<EncoderAPin | 1<<EncoderBPin;     // Configure pin change interrupt on A and B
  GIMSK = 1<<PCIE;            // Enable interrupt
  GIFR  = 1<<PCIF;            // Clear interrupt flag
}

// Called when encoder value changes
void IncreaseFreq (bool Up) {
  if (Up) {
    Count++;
  } else {
    Count--;
  }
  if (Count % 4 == 0) {
    int step = 1;
    if     (Freq >= 1000) step = 100;
    else if (Freq >=100)  step = 10;
    else if (Freq >=30)   step = 5;

    Freq = max(min((Freq + (Up ? step : -step)), MaxFreq), MinFreq);
    PlotFreq(Freq, 1, 7);
    Jump = Freq*4;
  }
}

// borrowed idea from: http://domoticx.com/arduino-rotary-encoder-keyes-ky-040/
//  - when tested on attiny85 @ 8MHz using both encoder A/B on change interrupts,
//    generates four counts each full detention sweep 
//    (KY-040 model is 20 detentions / revolution)

// pin change interrupt handler
ISR (PCINT0_vect) {
  enum { upMask = 0x66, downMask = 0x99 };

  byte abNew = (digitalRead(EncoderAPin) << 1) | digitalRead(EncoderBPin);
  byte criterion = abNew^abOld;

  if (criterion==0x1 || criterion==0x2) { 
    if (upMask & (1 << (2*abOld + abNew/2)))
      IncreaseFreq(false);
    else IncreaseFreq(true);       // upMask = ~downMask
  }
  abOld = abNew;        // Save new state
}

void ChangeFunction() {
  Wavefun = Waves[Wave];
  MCUSR = 0;
  SetupDDS();
  SetupRotaryEncoder();
  Jump = Freq*4;
  PlotFreq(Freq, 1, 7);
  PlotIcon(Wave, 1, 0);
}
  
void setup() {


  // Is it a power-on reset?
  if (MCUSR & 1) {
    Wave = 0; Freq = 100;     // Start with 100Hz Sine (wave 0)
    CalculateSine();
    lcd.clear();
  } else {
    Wave = (Wave+1) % nWaves;  
  }

  if (Wave >= 4) {
    initializeLCD(2);
  } else {
    initializeLCD(1);
  }

  ChangeFunction();
}


void loop() {
}
