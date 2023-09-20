#include <Arduino.h>
#include <EEPROM.h>
#include <TM1637Display.h>

// Module connection pins (Digital Pins)
#define CLK 10
#define DIO 11

TM1637Display display(CLK, DIO);

//Configuration
int PWMpin = 3;        //Arduino digital pin for PWM output

float pwm = 0;

bool bnMode = false;
int twobtntime = 0;
int upbtntime = 0;
int downbtntime = 0;
int btndelay = 12;
volatile int brightness = 0xFF;

void setup() {
  //Enable serial information output
  //Serial.begin(9600);

  //PWM Setup for 25KHz
  TCCR2A = 0;                               // TC2 Control Register A
  TCCR2B = 0;                               // TC2 Control Register B
  TIMSK2 = 0;                               // TC2 Interrupt Mask Register
  TIFR2 = 0;                                // TC2 Interrupt Flag Register
  TCCR2A |= (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);  // OC2B cleared/set on match when up/down counting, fast PWM
  TCCR2B |= (1 << WGM22) | (1 << CS21);     // prescaler 8
  OCR2A = 79;

  pinMode(4, INPUT_PULLUP); 
  pinMode(5, INPUT_PULLUP);

  //Enable PWM output pin
  pinMode(PWMpin, OUTPUT);
  pwm=EEPROM.read(1);
  if(pwm>100) pwm=100;
  //Set PWM duty cycle
  OCR2B = (pwm/100*OCR2A);
  brightness=EEPROM.read(0);
  if(brightness>7) brightness=7;

  display.clear();
  display.setBrightness(brightness);
  display.showNumberDec(pwm);
}

void displayBrightness() {
  display.setBrightness(brightness);
  display.showNumberHexEx(0xB,0,false,1,0);
  display.showNumberDecEx(brightness+1,0,false,String(brightness).length(),1);
}

void PWMup() {
  pwm+=1;
  if(pwm>100) pwm=100;
  display.showNumberDec(pwm);
  //Set PWM duty cycle
  OCR2B = (pwm/100*OCR2A);
}

void PWMdown() {
  pwm-=1;
  if(pwm<0)pwm=0;
  display.showNumberDec(pwm);
  //Set PWM duty cycle
  OCR2B = (pwm/100*OCR2A);
}

void Bup() {
  brightness+=1;
  if(brightness>7) brightness=7;
  displayBrightness();
}

void Bdown() {
  brightness-=1;
  if(brightness<0)brightness=0;
  displayBrightness();
}

void loop() {
  int btnup, btndown;
  btnup=digitalRead(4);
  btndown=digitalRead(5);
  
  if(!bnMode) {
    //Change PWM
    if(btndown==HIGH&&btnup==LOW) {
      if(upbtntime==0) {
        PWMup();
        upbtntime+=1;
      } else if(upbtntime<btndelay) {
        upbtntime+=1;
      } else {
        PWMup();
        upbtntime=btndelay;
      }
    }
    if(btndown==LOW&&btnup==HIGH){
      if(downbtntime==0) {
        PWMdown();
        downbtntime+=1;
      } else if(downbtntime<btndelay) {
        downbtntime+=1;
      } else {
        PWMdown();
        downbtntime=btndelay;
      }
    }
    if(btndown==HIGH&&btnup==HIGH&&(downbtntime>0||upbtntime>0)){
      downbtntime=0;
      upbtntime=0;
      EEPROM.write(1,uint8_t(pwm));
    }
    if(upbtntime==btndelay||downbtntime==btndelay) {
      delay(25);
    }
    else {
      delay(50);
    }
  } else {
    //Change brightness
    if(btndown==LOW&&btnup==HIGH) {
      if(downbtntime==0) {
        Bdown();
        downbtntime+=1;
      }
    }
    if(btndown==HIGH&&btnup==LOW) {
      if(upbtntime==0) {
        Bup();
        upbtntime+=1;
      }
    }
    if(btndown==HIGH&&btnup==HIGH&&(downbtntime>0||upbtntime>0)){
      downbtntime=0;
      upbtntime=0;
    }
    delay(50);
  }
  if(digitalRead(4)==LOW&&digitalRead(5)==LOW) {
    twobtntime++;
    if(twobtntime>10) {
      bnMode=!bnMode;
      twobtntime=0;
      if(bnMode) {
        downbtntime=0;
        upbtntime=0;
        display.clear();
        uint8_t data[] = { 0x00, 0x00, 0x00, 0x00 };
        display.setSegments(data);
        displayBrightness();
      } else {
        uint8_t data[] = { 0x00, 0x00, 0x00, 0x00 };
        display.setSegments(data);
        display.showNumberDec(pwm);
        EEPROM.write(0,uint8_t(brightness));
      }
    }
  } else {
    twobtntime=0;
  }
}
