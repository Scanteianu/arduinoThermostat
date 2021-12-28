// Dependencies:

// uses dht 11 temperature sensor, using dht sensor library by adafruit, 
// which also installs the other unified sensor lib

// uses LiquidCrystal I2C library by Marco Schwartz for a 16x2 lcd using 4 pin i2c connection, pins 20 and 21 are sda and scl on arduino mega, respectively

// it is assumed that, if temperature buttons are used, they are HIGH by default, and LOW when depressed
// the user can feel free to change it to be the other way around if that's desired
// I personally use a 20k potentiometer for the temperature setting, which allows all 1023 analog resolution to be used
// when cooling is on, a 10 amp relay like HiLetgo 2pcs 5V One Channel Relay Module Relay Switch with OPTO Isolation High Low Level Trigger is turned on
// this can be connected to something like a window fan
// temperature sensor is dht11, but others could bbe substituted

#include "DHT.h"
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#define DHTPIN 2 //temp sensor

#define DHTTYPE DHT11 // temp sensor
#define POTPIN 0 // for potentiometer temperature setting
#define DOWNPIN 5 // for temperature reduction button with button temp setting
#define UPPIN 4  // for temperature increase with button temp setting
#define RELAYPIN 12 // the pin that turns on the temperature changing device

DHT dht(DHTPIN, DHTTYPE); // temp sensor initialization
LiquidCrystal_I2C lcd(0x27,16,2); 

int shouldCool; //boolean to turn on the temperature changer
float tempTolerance; // how far from the desired temperature you have to be to change device state
int prevDownSwitch; // previous state of down button
int prevUpSwitch; //previous state of up button
float desiredSwitch; // current temperature that is desired, if buttons are used to change temp
float prevDesiredTemp;
float prevActualTemp;
void setup() {
  // put your setup code here, to run once:
  pinMode(UPPIN, INPUT);
  pinMode(DOWNPIN, INPUT);
  pinMode(RELAYPIN,OUTPUT);
  Serial.begin(9600);
  dht.begin();
  shouldCool=0;
  tempTolerance=0.75;
  prevDownSwitch=HIGH;
  prevUpSwitch=HIGH;
  desiredSwitch=75;
  prevDesiredTemp=75;
  prevActualTemp=75;
  lcd.init(); 
  lcd.backlight();
}
float readTemp(){
  float f = dht.readTemperature(true);
  if (isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return 0;
  }
  return f;
}
void setCooling(float actualTemp, float desiredTemp){
  if(actualTemp<desiredTemp-tempTolerance){
    shouldCool=0;
  }
  if(actualTemp>desiredTemp+tempTolerance){
    shouldCool=1;
  }
}

void displayStatusSerialPort(int shouldCool, float actualTemp, float desiredTemp){
  if(actualTemp!=prevActualTemp || desiredTemp!=prevDesiredTemp){
    Serial.println("-------------------");
    Serial.write("actualTemp: ");
    Serial.println(actualTemp);
    Serial.write("desiredTemp: ");
    Serial.println(desiredTemp);
    if(shouldCool){
      Serial.println("cooling on");
    }
    else{
      Serial.println("cooling off");
    }
  }
  prevActualTemp=actualTemp;
  prevDesiredTemp=desiredTemp;
}
void displayStatusLCD(int shouldCool, float actualTemp, float desiredTemp){
  lcd.setCursor(0,0);
  lcd.print("Actual ");
  lcd.setCursor(7,0);
  lcd.print(actualTemp);
  lcd.setCursor(0,1);
  lcd.print("Target ");
  lcd.setCursor(7,1);
  lcd.print(desiredTemp);
  lcd.print("*");
  if(shouldCool){
    lcd.print("ON ");
  }
  else{
    lcd.print("OFF");
  }

  
}
void sendSignalToThermostat(){
  if(shouldCool){
    digitalWrite(RELAYPIN, HIGH);
  }
  else{
    digitalWrite(RELAYPIN, LOW);
  }
}
// get desired temperature from potentiometer
float readDesiredPot(){
  float READINGS=30; // number of samples to take
  float POTMAX=1000; // max readout of potentiometer (used for scaling, can be approximate)
  float TEMP_RANGE=15; // range of temperatures that can be set
  float TEMP_BASE=68; // minimum temperature that can be set
  float sum=0;
  // an older potentiometer was noisy, so use an average over a few readings to reduce noise
  for(int i=0; i<READINGS; i++){
    delay(10);
    sum+=analogRead(POTPIN);
  }

  float desiredTemp=TEMP_BASE+((sum/READINGS)/(POTMAX/TEMP_RANGE));
  desiredTemp=((int)(desiredTemp*10))/10.0;
  int digitalSignal = analogRead(POTPIN);
  //  Serial.print("analog signal ");
  //  Serial.println(sum/READINGS);
  return desiredTemp;
}
// get desired temperature from a switch
float readDesiredSwitch(){
  int up = digitalRead(UPPIN);
  int down = digitalRead(DOWNPIN);
  if(up==LOW && prevUpSwitch==HIGH){
    desiredSwitch+=1;
    
  }
  if(down==LOW && prevDownSwitch==HIGH){
    desiredSwitch-=1;
    
  }
  prevDownSwitch=down;
  prevUpSwitch=up;
  return desiredSwitch;
}
void loop() {
  //delay is done by temp reader in potentiometer mode
  float actualTemp=readTemp();
  //float desiredTemp = readDesiredSwitch();
  float desiredTemp = readDesiredPot();
  setCooling(actualTemp,desiredTemp);
  displayStatusSerialPort(shouldCool, actualTemp, desiredTemp);
  displayStatusLCD(shouldCool, actualTemp, desiredTemp);
  sendSignalToThermostat();
  //delay(200);
  
}
