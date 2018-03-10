/*
 Demonstration sketch for Adafruit LCD backpack
 using MCP23008 I2C expander
 Uses the ATTiny 85 (replaces Trinket below)
 Also hooked up a rotary encoder with a button.
 
 The circuit:
 * 5V to Arduino 5V pin
 * GND to Arduino GND pin
 * Display i2c backpack CLK to Trinket GPIO #2 (Arduino pin 2/attiny85 pin 7)
 * Display i2c backpack DAT to Trinket GPIO #0 (Arduino pin 0/attiny85 pin 5)

KY-040 equivalent rotary encoder (20 detentions/revolution):

 * CLK to Arduino #3/attiny85 pin 2
 * DT  to Arduino #4/attiny85 pin 3
 * SW  to Arduino #A0/attiny85 pin 1 (also _RESET_)
 * +   to 5V
 * GND to GND

If text on display not visible at first, adjust contrast using screw crontrol on back

ATTiny Core settings in Arduino, using "Arduino as ISP".

Run "Burn bootloader" before uploading sketch the first time.

 - Board: "ATtiny25/45/85
 - Chip: "ATtiny85"
 - Clock: "8 MHz (internal)
 - B.O.D.: "B.O.D. Disabled"
 - LTO (1.6.11+ only): "Disabled"
 - Timer 1 Clock: "CPU"

Adapted from:
 - http://www.technoblogy.com/list?1YVE
   - via http://www.technoblogy.com/show?1YHJ
   NOTE: Since the example uses different rotary dial hardware, had to adjust some things

*/

// include the library code
#include <TinyWireM.h>      // Wire/I2C library for ATtiny85
#include <TinyLiquidCrystal.h>
#include <avr/power.h>      // needed to up clock to 16 MHz on 5v Trinket

// Connect display via  i2c, default address #0 (A0-A2 not jumpered)
TinyLiquidCrystal lcd(0);

//const int SDAPin = 0;
//const int SCLPin = 2;
const int LEDPin = 1;
const int EncoderAPin = 3;
const int EncoderBPin = 4;

const int EncoderSwitchAnalogPin = A0; // (PB5: PCINT5/_RESET_/ADC0/dW/Digital Pin 0)

int16_t t; // loop count

// Display count
void DisplayCount (int number) {
  lcd.setCursor(0, 0);
  lcd.print(number/4);  // each detention gets counted four times
  lcd.print("  ");
}

// Rotary encoder **********************************************

volatile int a0;
volatile int c0;
volatile int Count = -2; // rotary encoder state machine steady/starting state
volatile byte abOld;     // Initialize state

// borrowed idea from: http://domoticx.com/arduino-rotary-encoder-keyes-ky-040/
//  - when tested on atinny @ 8MHz using both encoder A/B on change interrupts,
//    generates four counts each detention 
ISR (PCINT0_vect) {
  enum { upMask = 0x66, downMask = 0x99 };

  byte abNew = (digitalRead(EncoderAPin) << 1) | digitalRead(EncoderBPin);
  byte criterion = abNew^abOld;

  if (criterion==0x1 || criterion==0x2) { 
    if (upMask & (1 << (2*abOld + abNew/2)))
      Count--;
    else Count++;       // upMask = ~downMask
  }
  abOld = abNew;        // Save new state
}



// Setup demo **********************************************

void setup() {
  pinMode(LEDPin, OUTPUT);
  pinMode(EncoderAPin, INPUT);  // Connects to ground through a 10k
  pinMode(EncoderBPin, INPUT);
  
  PCMSK = 1<<EncoderAPin | 1<<EncoderBPin;     // Configure pin change interrupt on A and B
  GIMSK = 1<<PCIE;            // Enable interrupt
  GIFR  = 1<<PCIF;            // Clear interrupt flag

  
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1); // 5V Trinket: run at 16 MHz  
  
  lcd.begin(16, 2);  // set up the LCD's number of rows and columns: 
  lcd.setBacklight(HIGH); // Set backlight
  lcd.print("   ");  // Print a message to the LCD.

  // initialize rotary state
  abOld = 0x0;
}

void loop() {
  DisplayCount(Count);
  delay(100);

  // Since pin A0/0 is also active-low RESET, it needs to stay digital high.
  // Wire VCC to it using 1k / 10k voltage divider (1k high-side)
  // Rotary encoder Button press pulls voltage divider toward ground (1023 is
  // highest value on 10-bit ADC)

  // When Rotary encoder's button is pressed, light LED 
  //  - LED Connected on ATtiny pin 6 thru current-limiting resister to ground
  if (analogRead(EncoderSwitchAnalogPin) > 960 ) {   // reset pin is near Vcc
    digitalWrite( LEDPin , LOW );   // turn led off
  } else {                       // reset pin is less than 960/1023 * 5 vcc
    digitalWrite( LEDPin , HIGH );   // turn led on
  }
   
  // set the cursor to line 1 (second line)
  lcd.setCursor(0, 1);
  lcd.print("T=");
  // print the number of seconds since reset:
  lcd.print(millis()/1000);
  
}
