#include <SoftwareSerial.h>

#define PIN_TX        7                             // Подключить  к выводу 7 сигнал RX модуля GPRS
#define PIN_RX        8                             // Подключить  к выводу 8 сигнал TX модуля GPRS
#define LED13        13                             // Индикация светодиодом
#define PWR_On        4                             // Включение питания модуля А6
#define RST_A6        3                             // Сброс модуля А6
#define LED_RED       9                             // Индикация светодиодом RED
#define LED_BLUE     10                             // Индикация светодиодом BLUE
#define LED_GREEN    11                             // Индикация светодиодом GREEN
#define STANDARD_DELAY 1000


#define BAUDRATE 19200
int ch = 0;
String val = "";

const String getTime = "AT+CCLK?";
const String getOperator = "AT+COPS?";
const String getIME = "AT+CGSN";
String imei = "";
String mobop = "";

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
  if (mobop.indexOf("25001") > -1) 
  {
    apn = "internet.mts.ru";
    user = "mts";
    pwd = "mts";
    cont = "internet.mts.ru";
  }
  else if (mobop.indexOf("25099") > -1) 
  {
    apn = "internet.beeline.ru";
    user = "beeline";
    pwd = "beeline";
    cont = "internet.beeline.ru";
  }
  else if (mobop.indexOf("25002") > -1) 
  {
    apn = "internet";
    user = "megafon";
    pwd = "megafon";
    cont = "internet";
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

String ReadGSM() 
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

String execCommand(String comm, int d) 
{
  Serial.println("Flushed: " + ReadGSM());
  GSMport.println(comm);
  delay(d);
  String response = ReadGSM();
  Serial.println(response);
  return response;
}

void cleanStr(String & str) 
{
  str.replace("OK", "");
  str.replace("\"", "");
  str.replace("\n", "");
  str.replace("\r", "");
  str.trim();
}


void GPRS_setup()
{
	String apn = "";
	String user = "";
	String pwd = "";
	String cont = "";

	//digitalWrite(RST_A6,   HIGH);
	//digitalWrite(LED13,    HIGH);
	//delay(200);
	//digitalWrite(RST_A6,   LOW);
	//digitalWrite(LED13,    LOW);
	//delay(10000);

	//
	digitalWrite(RST_A6,   HIGH);
	digitalWrite(LED13,    HIGH);
	delay(200);
	digitalWrite(RST_A6,   LOW);
	digitalWrite(LED13,    LOW);
	delay(1000);




	digitalWrite(PWR_On,   LOW);
	digitalWrite(LED13,    HIGH);
	delay(3000);
	digitalWrite(PWR_On,   HIGH);
	digitalWrite(LED13,    LOW);
	Serial.println("Start 10sec.");
	delay(10000);
	Serial.println("Stop 10sec.");
	GSMport.begin(BAUDRATE); 
	Serial.println("Start AT");
	GSMport.println("AT"); 
	delay(3000);  
	Serial.println("AT+IPR=19200");
	GSMport.println("AT+IPR=19200"); 
	delay(100);   
	print_Serial();
	Serial.println("ATI");
	GSMport.println("ATI"); 
	delay(100);    
	print_Serial();
	GSMport.println("AT+ccid"); 
	delay(100);  
	print_Serial();
	GSMport.println("AT+creg?"); 
	delay(100);  
	print_Serial();



	mobop = execCommand(getOperator, STANDARD_DELAY * 3);
	Serial.println(mobop);

	if (mobop.indexOf("25001") > -1)                 // "MTS"
	{
		execCommand("AT+CSTT=\"internet.mts.ru\",\"mts\",\"mts\"", STANDARD_DELAY);
		//print_Serial();
	}
	else if (mobop.indexOf("25002") > -1)            // "MegaFon"
	{
		 execCommand("AT+CSTT=\"internet\",\"\",\"\"", STANDARD_DELAY);
		//GSMport.println("AT+CSTT=\"internet\",\"megafon\",\"megafon\"");
		//print_Serial();
	}
	else if (mobop.indexOf("25099") > -1)            // "BeeLine"
	{
		//execCommand("AT+CSTT=\"internet.beeline.ru\",\"\",\"\"", STANDARD_DELAY);
		execCommand("AT+CSTT=\"internet.beeline.ru\",\"beeline\",\"beeline\"", STANDARD_DELAY);
		//print_Serial();
	}

	
	    execCommand("AT+CGACT=1,1", STANDARD_DELAY);
	
	    execCommand("AT+CGATT=1", STANDARD_DELAY);
		//execCommand("AT+CIFSR", STANDARD_DELAY);
	
		execCommand("AT+CGATT?", STANDARD_DELAY);
		execCommand("AT+CIPSTART=?", STANDARD_DELAY);
		execCommand("AT+CIPSTART=\"TCP\",\"speedtest.tele2.net\",21", STANDARD_DELAY);

		//execCommand("AT+CGDCONT=1, \"IP\", \"www.ntmp.ru\"", STANDARD_DELAY);
		execCommand("at+cipstatus", STANDARD_DELAY);
	

		//delay(1000); 
		//execCommand("AT+CGDCONT=1, \"IP\", \"u55750.ftp.masterhost.ru\"", STANDARD_DELAY);
		execCommand("AT+CDNSGIP?", STANDARD_DELAY);


		/*
		u55750.ftp.masterhost.ru
		u55750
		uncophy5is


		*/

	//cleanStr(mobop);

	//Serial.println(mobop);

	//GSMport.println("AT+CGSN"); 
	//delay(100);   
	//print_Serial();
	/*
	GSMport.println("AT+cipstatus?"); 
	delay(100);
	print_Serial();
	GSMport.println("AT+CSQ"); 
	delay(100);    
	print_Serial();
	GSMport.println("AT+COPS?"); 
	delay(100);    
	print_Serial();
	GSMport.println("AT+COPS=?"); 
	delay(5000);    
	print_Serial();
	*/


}
void print_Serial()
{
	while(GSMport.available())
	{
	if(GSMport.available())
		{
			ch = GSMport.read();
			val += char(ch);                           //с охраняем входную строку в переменную val
			//delay(10);
		}
		Serial.print(val);
		val="";
		//delay(10);
	}
}

void setup()
{
	Serial.begin(9600);                           //подключаем порт компьютера
	Serial.println("GSM Start");
	GSMport.begin(115200);                     //подключаем порт модема
	delay(2000);
	pinMode(LED13,    OUTPUT);
	pinMode(PWR_On,   OUTPUT);
	pinMode(RST_A6,   OUTPUT);
	pinMode(LED_RED,  OUTPUT);
	pinMode(LED_BLUE, OUTPUT);
	pinMode(LED_GREEN,OUTPUT);

	digitalWrite(LED13,    LOW);
	digitalWrite(PWR_On,   HIGH);
	digitalWrite(RST_A6,   LOW);
	digitalWrite(LED_RED,  LOW);
	digitalWrite(LED_BLUE, LOW);
	digitalWrite(LED_GREEN,LOW);

	GPRS_setup();
}
void loop()
{
 	while(GSMport.available())
	{
    if(GSMport.available())
	{
		ch = GSMport.read();
		val += char(ch);                           //с охраняем входную строку в переменную val
	   // delay(10);
	}
	Serial.print(val);
	val="";
	//delay(10);
  }
}