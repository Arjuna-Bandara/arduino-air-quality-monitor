#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"
#include "MQ7.h"

MQ7 mq7(A0,5.0);

#include <SoftwareSerial.h>
#define RX 10
#define TX 11

#define DHTPIN 2
#define DHTTYPE DHT22

String AP = "Arjuna";       // CHANGE ME
String PASS = "arjuna12345"; // CHANGE ME
String API = "ELIB1XW18XYTXE2W";   // CHANGE ME
String HOST = "api.thingspeak.com";
String PORT = "80";

int countTrueCommand;
int countTimeCommand; 
boolean found = false; 
int valSensor = 1;

String sensorDataString = "";

String COvalue = "";

//dht sensor
DHT dht(DHTPIN, DHTTYPE);

SoftwareSerial esp8266(RX,TX); 

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

void setup()
{

  displayText("Hello from Air!","Connecting..wifi",true);

  Serial.begin(9600);

  dhtSetup();
  
  esp8266.begin(115200);
  sendCommand("AT",5,"OK");
  sendCommand("AT+CWMODE=1",5,"OK");
  sendCommand("AT+CWJAP=\""+ AP +"\",\""+ PASS +"\"",20,"OK");  

}

void mq7read(){
//    Serial.println("CO value");
//    Serial.println(mq7.getPPM());
  COvalue = (String)mq7.getPPM();
  delay(2000);    
}

void dhtSetup(){
  Serial.println("DHTxx test!"); 
  dht.begin();  
}

void dhtRead(){
  delay(2000); // 2 seconds to load
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);    

  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");

  displayText("RH:" + (String)h + "%" + ",T:" + (String)t + " C", "CO:"+ COvalue+" ppm",true);
  sensorDataString = getSensorData((String)h,(String)t,COvalue);

  Serial.print(sensorDataString);
  
  delay(1000);
  
}




void displayText(String row1, String row2, boolean backlightStatus){
  lcd.init(); 
  
  if(backlightStatus == false){
      lcd.noBacklight();  
  }
  else{
      lcd.backlight();
  }
  
  lcd.backlight();
  lcd.print(row1);
  lcd.setCursor(0,1);
  lcd.print(row2); 
 }


void loop()
{

 syncSensorData();
 
 mq7read();
 dhtRead();
 
 //delay(15000);
}



String getSensorData(String val1,String val2,String val3){
  String x = "field1="+val1+"&" + "field2="+val2 + "&" + "field3="+val3;
  return x; 
}

void sendCommand(String command, int maxTime, char readReplay[]) {
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while(countTimeCommand < (maxTime*1))
  {
    esp8266.println(command);//at+cipsend
    if(esp8266.find(readReplay))//ok
    {
      found = true;
      break;
    }
  
    countTimeCommand++;
  }
  
  if(found == true)
  {
    Serial.println("OYI");
    //displayText("Server connected","Data sent", true);
    countTrueCommand++;
    countTimeCommand = 0;
  }
  
  if(found == false)
  {
    Serial.println("Fail");
    //displayText("Server not connected","Data fail", true);
    countTrueCommand = 0;
    countTimeCommand = 0;
  }
  
  found = false;
}

//function to sync data with server
void syncSensorData(){
  
 String getData = "GET /update?api_key="+ API +"&"+ sensorDataString;
 sendCommand("AT+CIPMUX=1",5,"OK");
 sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
 sendCommand("AT+CIPSEND=0," +String(getData.length()+4),4,">");
 esp8266.println(getData);
 delay(1500);
 countTrueCommand++;
 sendCommand("AT+CIPCLOSE=0",5,"OK");  
}
