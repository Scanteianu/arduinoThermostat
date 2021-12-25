#include "DHT.h"

#define DHTPIN 2 

#define DHTTYPE DHT11
#define POTPIN 0
#define DOWNPIN 5
#define UPPIN 4 
#define RELAYPIN 13
#define INDICATORPIN 12
DHT dht(DHTPIN, DHTTYPE);
int shouldCool;
float tempTolerance;
int prevDownSwitch;
int prevUpSwitch;
float desiredSwitch;
void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(UPPIN, INPUT);
  pinMode(DOWNPIN,INPUT);
  Serial.begin(9600);
  dht.begin();
  shouldCool=0;
  tempTolerance=1.0;
  prevDownSwitch=HIGH;
  prevUpSwitch=HIGH;
  desiredSwitch=75;
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
  Serial.write("actualTemp: ");
  Serial.println(actualTemp);
  Serial.write("desiredTemp: ");
  Serial.println(desiredTemp);
  if(actualTemp<desiredTemp-tempTolerance){
    shouldCool=0;
  }
  if(actualTemp>desiredTemp+tempTolerance){
    shouldCool=1;
  }
}
void sendSignalToThermostat(){
  if(shouldCool){
    Serial.println("cooling on");
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else{
    Serial.println("cooling off");
    digitalWrite(LED_BUILTIN, LOW);
  }
}
float readDesiredPot(){
  //potentiometer seems to like to max out around 400

  float READINGS=30;
  float POTMAX=1000;
  float TEMP_RANGE=15;
  float TEMP_BASE=68;
  float sum=0;
  // it is also very noisy, so use an average of 10 readings
  for(int i=0; i<READINGS; i++){
    delay(10);
    sum+=analogRead(POTPIN);
  }


  //divide by 40 for desired 10 degree temp range
  float desiredTemp=TEMP_BASE+((sum/READINGS)/(POTMAX/TEMP_RANGE));
  int digitalSignal = analogRead(POTPIN);
  Serial.print("analog signal ");
  Serial.println(sum/READINGS);
  return desiredTemp;
}
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
  //delay is done by temp reader
  Serial.println("-------------------");
  float actualTemp=readTemp();
  //float desiredTemp = readDesiredSwitch();
  float desiredTemp = readDesiredPot();
  setCooling(actualTemp,desiredTemp);
  sendSignalToThermostat();
  //delay(200);
  
}
