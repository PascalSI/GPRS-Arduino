
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SoftwareSerial.h>



#define PIN_TX        7                             // Подключить  к выводу 7 сигнал RX модуля GPRS
#define PIN_RX        8                             // Подключить  к выводу 8 сигнал TX модуля GPRS
#define LED13        13                             // Индикация светодиодом
#define PWR_On       12                             // Включение питания модуля SIM800
#define RST_GPRS      3                             // Сброс модуля SIM800
#define LED_RED       9                             // Индикация светодиодом RED
#define LED_BLUE     11                             // Индикация светодиодом BLUE
#define LED_GREEN    10                             // Индикация светодиодом GREEN
#define STANDARD_DELAY 1000


#define BAUDRATE 19200
int ch = 0;
String val = "";

#define ONE_WIRE_BUS 4
#define STARTUP_DELAY 10000
#define STANDARD_DELAY 1000
#define DELIM "@"

const String getTime = "AT+CCLK?";
const String getOperator = "AT+CSPN?";  
const String getIME = "AT+CGSN";

String imei = "";
String mobop = "";

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress thermometer1;
DeviceAddress thermometer2;

SoftwareSerial GSMport(PIN_RX, PIN_TX);            // RX, TX

unsigned long previousMillis = 0;
const long interval = 60000;

String execCommand(String comm, int d) ;
String formHeader() ;
void cleanStr(String & str) ;

void initGSM() 
{
  String apn = "";
  String user = "";
  String pwd = "";
  String cont = "";
  if (mobop.indexOf("MTS") > -1) 
  {
    apn = "internet.mts.ru";
    user = "mts";
    pwd = "mts";
    cont = "internet.mts.ru";
  }
  else if (mobop.indexOf("BEELINE") > -1) 
  {
    apn = "internet.beeline.ru";
    user = "beeline";
    pwd = "beeline";
    cont = "internet.beeline.ru";
  } 
  else if (mobop.indexOf("MEGAFON") > -1) 
  {
    apn = "internet";
    user = "megafon";
    pwd = "megafon";
    cont = "internet";
  }
  else if (mobop.indexOf("TELE2") > -1) 
  {
    apn = "internet.TELE2.ru";
    user = "";
    pwd = "";
    cont = "internet.tele2.ru";
  }

  execCommand("AT+CSQ", STANDARD_DELAY);

  execCommand("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"", STANDARD_DELAY * 4);
  execCommand("AT+SAPBR=3,1,\"APN\",\"" + apn + "\"", STANDARD_DELAY);
  execCommand("AT+SAPBR=3,1,\"USER\",\"" + user + "\"", STANDARD_DELAY);
  execCommand("AT+SAPBR=3,1,\"PWD\",\"" + pwd + "\"", STANDARD_DELAY);
  execCommand("AT+CGDCONT=1,\"IP\",\"" + cont + "\"", STANDARD_DELAY);

 // execCommand("AT+CIICR", STANDARD_DELAY);
 // execCommand("AT+CIPPING=\"www.ntmp.ru\"", STANDARD_DELAY);  // проверка подключения к интернету (пока не работает)

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
	Serial.println(data);
}

String ReadGSM(void) 
{
  int c;
  String v;
  while (GSMport.available())
  {
    c = GSMport.read();
    v += char(c);
    delay(10);
  }
  return v;
}

void initTemps(void)
{
  sensors.begin();
  sensors.getAddress(thermometer1, 0);
  sensors.getAddress(thermometer2, 1);
  sensors.setResolution(thermometer1, 10);
  sensors.setResolution(thermometer2, 10);
}

void sendTemps(void) 
{
  sensors.requestTemperatures();
   Serial.println("\nTemp");

  float t1 = sensors.getTempC(thermometer1);
  t1 = 25.5;
  Serial.println(t1);
  float t2 = sensors.getTempC(thermometer2);
  t2 = 25.6;
  Serial.println(t2);
  String toSend = formHeader() + DELIM + String(t1) + DELIM + String(t2);
  gprs_send(toSend);
}

String execCommand(String comm, int d) 
{
  Serial.println("Send: " + ReadGSM());
  GSMport.println(comm);
  delay(d);
  String response = ReadGSM();
  Serial.println(response);
  return response;
}

String formHeader() 
{
  String uptime = execCommand(getTime, STANDARD_DELAY);
  uptime.replace("+CCLK:", "");
  uptime.replace(getTime, "");
  cleanStr(uptime);
  return "t1=" + imei + DELIM + uptime;
}

void cleanStr(String & str) 
{
  str.replace("OK", "");
  str.replace("\"", "");
  str.replace("\n", "");
  str.replace("\r", "");
  str.trim();
}
int freeRam () 
{
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

void setup()
{
	Serial.begin(9600);                           //подключаем порт компьютера
	Serial.println("** GSM Start **");
	GSMport.begin(19200);                         //подключаем порт модема
	pinMode(LED13,    OUTPUT);
	pinMode(PWR_On,   OUTPUT);
	pinMode(RST_GPRS, OUTPUT);
	pinMode(LED_RED,  OUTPUT);
	pinMode(LED_BLUE, OUTPUT);
	pinMode(LED_GREEN,OUTPUT);
	digitalWrite(RST_GPRS,   LOW);                // Сигнал сброс в исходное состояние
	digitalWrite(LED13,    LOW);
	digitalWrite(PWR_On,   HIGH);                 // Кратковременно отключаем питание модуля GPRS
	delay(300);
	digitalWrite(LED13,    HIGH);
	digitalWrite(PWR_On,   LOW);
	delay(100);                                  // Ожидаем отключения модема

	digitalWrite(RST_GPRS,   HIGH);               // Производим сброс модема после включения питания
	delay(300);
	digitalWrite(RST_GPRS,   LOW);               

	digitalWrite(LED_RED, HIGH);                  // Кратковременно зажигаем цветные светододы для проверки
	delay(300);
	digitalWrite(LED_RED,  LOW);
	delay(300);
	digitalWrite(LED_BLUE, HIGH);
	delay(300);
	digitalWrite(LED_BLUE, LOW);
	delay(300);
	digitalWrite(LED_GREEN,HIGH);
	delay(300);
	digitalWrite(LED_GREEN,LOW);

	delay(6000);

	execCommand("AT", STANDARD_DELAY * 5);
	mobop = execCommand(getOperator, STANDARD_DELAY * 3);
	mobop.replace("+CSPN:", "");
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

	//GPRS_setup();
}
void loop()
{
  unsigned long currentMillis = millis();
  
  if ((unsigned long)(currentMillis - previousMillis) >= interval)
  {
//   setColor(COLOR_BLUE);
    previousMillis = currentMillis;
    sendTemps();
 //   setColor(COLOR_GREEN);
    Serial.print("\nfree memory:");
    Serial.println(freeRam());
  }
}