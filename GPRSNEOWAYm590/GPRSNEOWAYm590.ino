//#define RESET_TIMER 3540000UL
#define RESET_TIMER 300000UL

#include <OneWire.h>
#include <DallasTemperature.h>
#include <SoftwareSerial.h>

#define LED_RED_PIN A2
#define LED_GREEN_PIN 12
#define LED_BLUE_PIN 11
     
//#define COMMON_ANODE

#define COLOR_NONE LOW, LOW, LOW
#define COLOR_RED HIGH, LOW, LOW
#define COLOR_GREEN LOW, HIGH, LOW
#define COLOR_BLUE LOW, LOW, HIGH

#define ONE_WIRE_BUS 10
#define STARTUP_DELAY 10000
#define STANDARD_DELAY 1000
#define DELIM "@"

const String getTime = "AT+CCLK?";
const String getOperator = "AT+COPS?";
const String getIME = "AT+CGSN";

String imei = "";
String mobop = "";

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress thermometer1;
DeviceAddress thermometer2;
SoftwareSerial GSMport(8, 7); // RX, TX

#define SIMBOOT 6  // пин сброс SIM


unsigned long previousMillis = 0;
const long interval = 60000;

String execCommand(String comm, int d) ;
String formHeader() ;
void cleanStr(String & str) ;

void setColor(bool red, bool green, bool blue){
      #ifdef COMMON_ANODE
        red = !red;
        green = !green;
        blue = !blue;
      #endif
      digitalWrite(LED_RED_PIN, red);
      digitalWrite(LED_GREEN_PIN, green);
      digitalWrite(LED_BLUE_PIN, blue);  
 }
    

void initGSM() 
{
  String apn = "";
  String user = "";
  String pwd = "";
  String cont = "";
  if (mobop.indexOf("MTS-RUS") > -1) {
    apn = "internet.mts.ru";
    user = "mts";
    pwd = "mts";
    cont = "internet.mts.ru";
  } else if (mobop.indexOf("BEELINE") > -1) {
    apn = "internet.beeline.ru";
    user = "beeline";
    pwd = "beeline";
    cont = "internet.beeline.ru";
  } else if (mobop.indexOf("MEGAFON") > -1) {
    apn = "internet";
    user = "megafon";
    pwd = "megafon";
    cont = "internet";
  } else if (mobop.indexOf("TELE2") > -1) {
    apn = "internet.TELE2.ru";
    user = "";
    pwd = "";
    cont = "internet.tele2.ru";
  }
 // execCommand("AT+XGAUTH=,1,\"CONTYPE\",\"GPRS\"", STANDARD_DELAY * 4);
  execCommand("AT+XGAUTH=1,1,\"APN\",\"" + apn + "\"", STANDARD_DELAY);
  execCommand("AT+XGAUTH=1,1,\"USER\",\"" + user + "\"", STANDARD_DELAY);
  execCommand("AT+XGAUTH=1,1,\"PWD\",\"" + pwd + "\"", STANDARD_DELAY);


  //execCommand("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"", STANDARD_DELAY * 4);
  //execCommand("AT+SAPBR=3,1,\"APN\",\"" + apn + "\"", STANDARD_DELAY);
  //execCommand("AT+SAPBR=3,1,\"USER\",\"" + user + "\"", STANDARD_DELAY);
  //execCommand("AT+SAPBR=3,1,\"PWD\",\"" + pwd + "\"", STANDARD_DELAY);
  execCommand("AT+CGDCONT=1,\"IP\",\"" + cont + "\"", STANDARD_DELAY);
 // execCommand("AT+XIIC=1""\"", STANDARD_DELAY);
  execCommand("AT+TCPSETUP=0,213.180.193.3,80""\"", STANDARD_DELAY);


 // execCommand("AT+SAPBR=1,1", STANDARD_DELAY * 2);
}

void gprs_send(String data) 
{
	execCommand("AT+HTTPINIT", STANDARD_DELAY);
	execCommand("AT+HTTPPARA=\"CID\",1", STANDARD_DELAY);

	execCommand("AT+HTTPPARA=\"URL\",\"http://vps3908.vps.host.ru/recieveReadings.php\"", STANDARD_DELAY * 2);

	execCommand("AT+HTTPPARA=\"CONTENT\",\"application/x-www-form-urlencoded\"",
	STANDARD_DELAY);
	execCommand("AT+HTTPDATA=" + String(data.length()) + ",10000",
	STANDARD_DELAY*2);
	execCommand(data, STANDARD_DELAY*3);
	execCommand("AT+HTTPACTION=1", STANDARD_DELAY*5);
	execCommand("AT+HTTPTERM", STANDARD_DELAY * 2);
}

String ReadGSM(void) {
  int c;
  String v;
  while (GSMport.available()) {
    c = GSMport.read();
    v += char(c);
    delay(10);
  }
  return v;
}

void initTemps(void) {
  sensors.begin();
  sensors.getAddress(thermometer1, 0);
  sensors.getAddress(thermometer2, 1);
  sensors.setResolution(thermometer1, 10);
  sensors.setResolution(thermometer2, 10);
}

void sendTemps(void) {
  sensors.requestTemperatures();
   Serial.println("\nTemp");

  float t1 = sensors.getTempC(thermometer1);
  Serial.println(t1);
  float t2 = sensors.getTempC(thermometer2);
  Serial.println(t2);
  String toSend = formHeader() + DELIM + String(t1) + DELIM + String(t2);
  gprs_send(toSend);
}

String execCommand(String comm, int d) 
{
  Serial.println("Flushed: " + ReadGSM());
  GSMport.println(comm);
  delay(d);
  String response = ReadGSM();
  Serial.println(response);
  return response;
}

String formHeader() {
  String uptime = execCommand(getTime, STANDARD_DELAY);
  uptime.replace("+CCLK:", "");
  uptime.replace(getTime, "");
  cleanStr(uptime);
  return "t1=" + imei + DELIM + uptime;
}

void cleanStr(String & str) {
  str.replace("OK", "");
  str.replace("\"", "");
  str.replace("\n", "");
  str.replace("\r", "");
  str.trim();
}

void(* resetFunc) (void) = 0;

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

void setup(void){
	pinMode(LED_RED_PIN, OUTPUT);
	pinMode(LED_GREEN_PIN, OUTPUT);
	pinMode(LED_BLUE_PIN, OUTPUT); 
	pinMode(SIMBOOT, OUTPUT); 
	delay(STARTUP_DELAY);
	Serial.begin(9600);
	Serial.println("Start setup");
	GSMport.begin(19200);

	execCommand("AT", STANDARD_DELAY * 3);
	execCommand("ATI", STANDARD_DELAY * 3);
	//execCommand("AT+CPAS", STANDARD_DELAY * 3);
	//execCommand("AT+CREG=1", STANDARD_DELAY * 3);
	//execCommand("AT+CREG?", STANDARD_DELAY * 3);
	//execCommand("AT+COPS?", STANDARD_DELAY * 3);
	mobop = execCommand(getOperator, STANDARD_DELAY * 3);
	mobop.replace("+COPS:", "");
	mobop.replace(getOperator, "");
	cleanStr(mobop);
	mobop.toUpperCase();
	imei = execCommand(getIME, STANDARD_DELAY * 3);
	imei.replace(getIME, "");
	cleanStr(imei);
	Serial.println("Operator " + mobop);
	Serial.println("IMEI " + imei);
	initTemps();
	initGSM();
  setColor(COLOR_GREEN);
   Serial.println("\n[memCheck]");
    Serial.println(freeRam());
}

void loop()
{
  if (GSMport.available())   Serial.write(GSMport.read());

  if (Serial.available())  GSMport.write(Serial.read());
  
  unsigned long currentMillis = millis();
  
  if ((unsigned long)(currentMillis - previousMillis) >= interval) {
    setColor(COLOR_BLUE);
    previousMillis = currentMillis;
    sendTemps();
    setColor(COLOR_GREEN);
    Serial.print("\nfree memory:");
    Serial.println(freeRam());
  }
  
  if(millis() > (unsigned long)RESET_TIMER){
    resetFunc();
  }
}

