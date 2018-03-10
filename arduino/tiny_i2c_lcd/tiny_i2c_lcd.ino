/*
 Demonstration sketch for Adafruit LCD backpack
 using MCP23008 I2C expander
 Uses the ATTiny 85 (replaces Trinket below)
 
 The circuit:
 * 5V to Arduino 5V pin
 * GND to Arduino GND pin
 * Display i2c backpack CLK to Trinket GPIO #2 (Arduino pin 2/attiny85 pin 7)
 * Display i2c backpack DAT to Trinket GPIO #0 (Arduino pin 0/attiny85 pin 5)

If not visible at first, adjust contrast using screw crontrol on back

example adapted from
 - https://github.com/adafruit/TinyLiquidCrystal/blob/master/examples/HelloWorld_i2c/Trinket_HelloWorld_i2c.ino
 - https://github.com/adafruit/TinyLiquidCrystal/blob/master/examples/Trinket_DHT_LCD/Trinket_DHT_LCD.ino
 
ATTiny Core settings in Arduino, using "Arduino as ISP".

Run "Burn bootloader" before uploading sketch the first time.

 - Board: "ATtiny25/45/85
 - Chip: "ATtiny85"
 - Clock: "8 MHz (internal)
 - B.O.D.: "B.O.D. Disabled"
 - LTO (1.6.11+ only): "Disabled"
 - Timer 1 Clock: "CPU"
  
*/

// include the library code
#include <TinyWireM.h>      // Wire/I2C library for ATtiny85
#include <TinyLiquidCrystal.h>
#include <avr/power.h>      // needed to up clock to 16 MHz on 5v Trinket

// Connect display via  i2c, default address #0 (A0-A2 not jumpered)
TinyLiquidCrystal lcd(0);

int16_t t; // loop count

void setup() {
  // put your setup code here, to run once:
 if (F_CPU == 16000000) clock_prescale_set(clock_div_1); // 5V Trinket: run at 16 MHz  
  lcd.begin(16, 2);  // set up the LCD's number of rows and columns: 
  lcd.setBacklight(HIGH); // Set backlight
  lcd.print("hello, world!");  // Print a message to the LCD.
}


void loop() {
  // put your main code here, to run repeatedly:
  lcd.setCursor(0, 0); 
  lcd.print("Bad read on DHT");       //   print error message
  
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  lcd.print(millis()/1000);
  
  delay(2000);  // cycles LCD  backlight every four seconds (2000 ms * 2)
  t++;
  if (t % 2) {
    lcd.setBacklight(HIGH); // Set backlight
  } else {
    lcd.setBacklight(LOW); // Set backlight
  }
}
