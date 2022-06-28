// SPDX-FileCopyrightText: 2019 Anne Barela for Adafruit Industries
//
// SPDX-License-Identifier: MIT

/*
The reason for the fork is to add code led dome lights. Both lights could probably be powered from a single pin 
but this is being used in a prop I don't have physical access to so after sending away the hardware the user has 
a better chance of maintining it if the wiring appears simpiler.

Dalek voice effect using Wave Shield.  This is based on the adavoice sketch and
doesn't do any pitch bending, just the Dalek voice modulation...you'll need to perform your own monotone & British accent. :)

 
*/


#define ADC_CHANNEL 0 // Microphone on Analog pin 0

// LED pins
const int led1Pin =  13; // the number of the LED 1 pin
const int led2Pin =  12; // the number of the LED 2 pin

int ledState = LOW; // ledState used to set the LED

// Wave shield DAC: digital pins 2, 3, 4, 5
#define DAC_CS_PORT    PORTD
#define DAC_CS         PORTD2
#define DAC_CLK_PORT   PORTD
#define DAC_CLK        PORTD3
#define DAC_DI_PORT    PORTD
#define DAC_DI         PORTD4
#define DAC_LATCH_PORT PORTD
#define DAC_LATCH      PORTD5

uint8_t adc_save;




//////////////////////////////////// SETUP

void setup() {
  uint8_t i;

  Serial.begin(9600);    

           
  pinMode(led1Pin, OUTPUT); // set the digital pin 1 as led output
  pinMode(led2Pin, OUTPUT); // set the digital pin 2 as led output
  pinMode(2, OUTPUT);    // Chip select
  pinMode(3, OUTPUT);    // Serial clock
  pinMode(4, OUTPUT);    // Serial data
  pinMode(5, OUTPUT);    // Latch
  digitalWrite(2, HIGH); // Set chip select high


  // Optional, but may make sampling and playback a little smoother:
  // Disable Timer0 interrupt.  This means delay(), millis() etc. won't
  // work.  Comment this out if you really, really need those functions.
  TIMSK0 = 0;

  // Set up Analog-to-Digital converter:
  analogReference(EXTERNAL); // 3.3V to AREF
  adc_save = ADCSRA;         // Save ADC setting for restore later

  startDalek();          // and start the Dalek effect
}


//////////////////////////////////// LOOP



void loop() {  
}


//////////////////////////////////// DALEK MODULATION CODE

void startDalek() {

  // Start up ADC in free-run mode for audio sampling:
  DIDR0 |= _BV(ADC0D);  // Disable digital input buffer on ADC0
  ADMUX  = ADC_CHANNEL; // Channel sel, right-adj, AREF to 3.3V regulator
  ADCSRB = 0;           // Free-run mode
  ADCSRA = _BV(ADEN) |  // Enable ADC
    _BV(ADSC)  |        // Start conversions
    _BV(ADATE) |        // Auto-trigger enable
    _BV(ADIE)  |        // Interrupt enable
    _BV(ADPS2) |        // 128:1 prescale...
    _BV(ADPS1) |        //  ...yields 125 KHz ADC clock...
    _BV(ADPS0);         //  ...13 cycles/conversion = ~9615 Hz
}

void stopDalek() {
  ADCSRA = adc_save; // Disable ADC interrupt and allow normal use
}

// Dalek sound is produced by a 'ring modulator' which multiplies microphone
// input by a 30 Hz sine wave.  sin() is a time-consuming floating-point
// operation so instead a canned 8-bit integer table is used...the number of
// elements here takes into account the ADC sample rate (~9615 Hz) and the
// desired sine wave frequency (traditionally ~30 Hz for Daleks).
// This is actually abs(sin(x)) to slightly simplify some math later.

volatile uint16_t ringPos = 0; // Current index into ring table below

static const uint8_t PROGMEM ring[] = {
  0x00, 0x03, 0x05, 0x08, 0x0A, 0x0D, 0x0F, 0x12,
  0x14, 0x17, 0x19, 0x1B, 0x1E, 0x20, 0x23, 0x25,
  0x28, 0x2A, 0x2D, 0x2F, 0x32, 0x34, 0x37, 0x39,
  0x3C, 0x3E, 0x40, 0x43, 0x45, 0x48, 0x4A, 0x4C,
  0x4F, 0x51, 0x54, 0x56, 0x58, 0x5B, 0x5D, 0x5F,
  0x62, 0x64, 0x66, 0x68, 0x6B, 0x6D, 0x6F, 0x72,
  0x74, 0x76, 0x78, 0x7A, 0x7D, 0x7F, 0x81, 0x83,
  0x85, 0x87, 0x89, 0x8C, 0x8E, 0x90, 0x92, 0x94,
  0x96, 0x98, 0x9A, 0x9C, 0x9E, 0xA0, 0xA2, 0xA4,
  0xA6, 0xA8, 0xA9, 0xAB, 0xAD, 0xAF, 0xB1, 0xB3,
  0xB4, 0xB6, 0xB8, 0xBA, 0xBB, 0xBD, 0xBF, 0xC0,
  0xC2, 0xC4, 0xC5, 0xC7, 0xC8, 0xCA, 0xCB, 0xCD,
  0xCE, 0xD0, 0xD1, 0xD3, 0xD4, 0xD5, 0xD7, 0xD8,
  0xD9, 0xDB, 0xDC, 0xDD, 0xDE, 0xE0, 0xE1, 0xE2,
  0xE3, 0xE4, 0xE5, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB,
  0xEC, 0xED, 0xED, 0xEE, 0xEF, 0xF0, 0xF1, 0xF2,
  0xF3, 0xF3, 0xF4, 0xF5, 0xF5, 0xF6, 0xF7, 0xF7,
  0xF8, 0xF9, 0xF9, 0xFA, 0xFA, 0xFB, 0xFB, 0xFB,
  0xFC, 0xFC, 0xFD, 0xFD, 0xFD, 0xFE, 0xFE, 0xFE,
  0xFE, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE,
  0xFE, 0xFE, 0xFE, 0xFE, 0xFD, 0xFD, 0xFD, 0xFC,
  0xFC, 0xFB, 0xFB, 0xFB, 0xFA, 0xFA, 0xF9, 0xF9,
  0xF8, 0xF7, 0xF7, 0xF6, 0xF5, 0xF5, 0xF4, 0xF3,
  0xF3, 0xF2, 0xF1, 0xF0, 0xEF, 0xEE, 0xED, 0xED,
  0xEC, 0xEB, 0xEA, 0xE9, 0xE8, 0xE7, 0xE5, 0xE4,
  0xE3, 0xE2, 0xE1, 0xE0, 0xDE, 0xDD, 0xDC, 0xDB,
  0xD9, 0xD8, 0xD7, 0xD5, 0xD4, 0xD3, 0xD1, 0xD0,
  0xCE, 0xCD, 0xCB, 0xCA, 0xC8, 0xC7, 0xC5, 0xC4,
  0xC2, 0xC0, 0xBF, 0xBD, 0xBB, 0xBA, 0xB8, 0xB6,
  0xB4, 0xB3, 0xB1, 0xAF, 0xAD, 0xAB, 0xA9, 0xA8,
  0xA6, 0xA4, 0xA2, 0xA0, 0x9E, 0x9C, 0x9A, 0x98,
  0x96, 0x94, 0x92, 0x90, 0x8E, 0x8C, 0x89, 0x87,
  0x85, 0x83, 0x81, 0x7F, 0x7D, 0x7A, 0x78, 0x76,
  0x74, 0x72, 0x6F, 0x6D, 0x6B, 0x68, 0x66, 0x64,
  0x62, 0x5F, 0x5D, 0x5B, 0x58, 0x56, 0x54, 0x51,
  0x4F, 0x4C, 0x4A, 0x48, 0x45, 0x43, 0x40, 0x3E,
  0x3C, 0x39, 0x37, 0x34, 0x32, 0x2F, 0x2D, 0x2A,
  0x28, 0x25, 0x23, 0x20, 0x1E, 0x1B, 0x19, 0x17,
  0x14, 0x12, 0x0F, 0x0D, 0x0A, 0x08, 0x05, 0x03 };

ISR(ADC_vect, ISR_BLOCK) { // ADC conversion complete

  uint8_t  hi, lo, bit;
  int32_t  v; // Voice in
  uint16_t r; // Ring in
  uint32_t o; // Output
  

  lo = ADCL;
  hi = ADCH;

  // Multiply signed 10-bit input by abs(sin(30 Hz)):
  v = ((int32_t)hi << 8 | lo) - 512;               // voice = -512 to +511
  r = (uint16_t)pgm_read_byte(&ring[ringPos]) + 1; // ring = 1 to 256
  o = v * r + 131072;                              // 0-261888 (18-bit)
  hi = (o >> 14);                                  // Scale 18- to 12-bit
  lo = (o >> 16) | (o >> 6);

    // Change the state of the leds based on voice.
    if (v > 450) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }

    // set the LEDs state.
    digitalWrite(led1Pin, ledState);
    digitalWrite(led2Pin, ledState);

  if(++ringPos >= sizeof(ring)) ringPos = 0; // Cycle through table

  // Issue result to DAC:
  DAC_CS_PORT  &= ~_BV(DAC_CS);
  DAC_DI_PORT  &= ~_BV(DAC_DI);
  DAC_CLK_PORT |=  _BV(DAC_CLK); DAC_CLK_PORT &= ~_BV(DAC_CLK);
  DAC_CLK_PORT |=  _BV(DAC_CLK); DAC_CLK_PORT &= ~_BV(DAC_CLK);
  DAC_DI_PORT  |=  _BV(DAC_DI);
  DAC_CLK_PORT |=  _BV(DAC_CLK); DAC_CLK_PORT &= ~_BV(DAC_CLK);
  DAC_CLK_PORT |=  _BV(DAC_CLK); DAC_CLK_PORT &= ~_BV(DAC_CLK);
  for(bit=0x08; bit; bit>>=1) {
    if(hi & bit) DAC_DI_PORT |=  _BV(DAC_DI);
    else         DAC_DI_PORT &= ~_BV(DAC_DI);
    DAC_CLK_PORT |=  _BV(DAC_CLK); DAC_CLK_PORT &= ~_BV(DAC_CLK);
  }
  for(bit=0x80; bit; bit>>=1) {
    if(lo & bit) DAC_DI_PORT |=  _BV(DAC_DI);
    else         DAC_DI_PORT &= ~_BV(DAC_DI);
    DAC_CLK_PORT |=  _BV(DAC_CLK); DAC_CLK_PORT &= ~_BV(DAC_CLK);
  }
  DAC_CS_PORT    |=  _BV(DAC_CS);
}
