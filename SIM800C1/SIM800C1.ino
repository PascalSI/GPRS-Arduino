/*************************************************************************
* Test sketch for SIM800 library
* Distributed under GPL v2.0
*
*************************************************************************/

#include "SIM800.h"
#include <SoftwareSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <avr/pgmspace.h>
#include <EEPROM.h>

#define con Serial
static const char* url1 = "http://vps3908.vps.host.ru/recieveReadings.php";


#define PWR_On           5                          // ��������� ������� ������ SIM800
#define SIM800_RESET_PIN 6                          // ����� ������ SIM800
#define LED13           13                          // ��������� �����������
#define port1           11                          // ���� ���������� �������� ������������
#define port2           12                          // ���� ���������� �������� ������������


                                                    // ����������  � ������ 7 ������ RX ������ GPRS. ���������� � ���������� SIM800.h
//#define COMMON_ANODE
                                                    // ����������  � ������ 8 ������ TX ������ GPRS. ���������� � ���������� SIM800.h  
#define LED_RED      10                             // ��������� ����������� RED
#define LED_BLUE     14                             // ��������� ����������� BLUE
#define LED_GREEN    15                             // ��������� ����������� GREEN

#define COLOR_NONE LOW, LOW, LOW
#define COLOR_RED HIGH, LOW, LOW
#define COLOR_GREEN LOW, HIGH, LOW
#define COLOR_BLUE LOW, LOW, HIGH

bool start_error = false;                         // ���� ����������� ������ ������ ��� ������.

CGPRS_SIM800 gprs;
uint32_t count  = 0;
uint32_t errors = 0;
String imei = "";
String CSQ = "";                                    // ������� ������� ������
//String header = "";
//String imei = "861445030362268";                  // ���� IMEI
#define DELIM "@"
//char mydata[] = "t1=861445030362268@04/01/02,15:22:52 00@24.50@25.60";
// ��� ������� +79258110171

unsigned long time;                                 // ���������� ��� ��������� ������
unsigned long time_day = 86400000;                  // ���������� ����������� � ������
unsigned long previousMillis = 0;
//unsigned long interval = 10000;                     // �������� �������� ������ 30 ������
unsigned long interval = 300000;                  // �������� �������� ������ 5 �����

//char test_tel[13] = "+79160000000";               // �������  ������� �������
int Address_tel1     = 100;                         // ����� � EEPROM �������� 1
int Address_tel2     = 120;                         // ����� � EEPROM �������� 2
int Address_tel3     = 140;                         // ����� � EEPROM �������� 3
int Address_errorAll = 160;                         // ����� � EEPROM �������� ����� ������
int Address_port1    = 180;                         // ����� � EEPROM �������� 3
int Address_port2    = 190;                         // ����� � EEPROM �������� 3


char data_tel[13];                                  // ����� ��� ������ �������

int dataport1 = 0;
int dataport2 = 0;


uint8_t oneWirePins[]={4, 5, 6};                    //OneWire DS18x20 temperature sensors on these wires
uint8_t oneWirePinsCount=sizeof(oneWirePins)/sizeof(int);

OneWire ds18x20_1(oneWirePins[0]);
OneWire ds18x20_2(oneWirePins[1]);
OneWire ds18x20_3(oneWirePins[2]);
DallasTemperature sensor1(&ds18x20_1);
DallasTemperature sensor2(&ds18x20_2);
DallasTemperature sensor3(&ds18x20_3);


void(* resetFunc) (void) = 0;                       // ��������� ������� reset

 void setColor(bool red, bool green, bool blue)
 {
      #ifdef COMMON_ANODE
        red = !red;
        green = !green;
        blue = !blue;
      #endif
      digitalWrite(LED_RED, red);
      digitalWrite(LED_GREEN, green);
      digitalWrite(LED_BLUE, blue);  
 }


void sendTemps() 
{
	//Serial.println("\nTemp");
	sensor1.requestTemperatures();
	sensor2.requestTemperatures();
	sensor3.requestTemperatures();
	float t1 = sensor1.getTempCByIndex(0);
	float t2 = sensor2.getTempCByIndex(0);
	float t3 = sensor3.getTempCByIndex(0);
	int signal = gprs.getSignalQuality();
	int error_All = 0;
	EEPROM.get(Address_errorAll, error_All);
	String toSend = formHeader()+DELIM+String(t1)+DELIM+String(t2)+DELIM+String(t3)+ DELIM+String(signal)+DELIM+String(errors)+DELIM+String(error_All)+formEnd();
	Serial.println(toSend.length());
	gprs_send(toSend);
}

String formHeader() 
{
  String uptime = "17/01/01,10:10:10 00";
  GSM_LOCATION loc;                               // �������� ����� �� ���������
  if (gprs.getLocation(&loc)) 
  {
   uptime  = String(loc.year)+'/'+ String(loc.month)+'/'+ String(loc.day)+','+String(loc.hour)+':'+ String(loc.minute)+':'+String(loc.second)+" 00";
  }
  return "t1=" + imei + DELIM + uptime;
}
String formEnd() 
{
	char buf[13] ;
	EEPROM.get(Address_tel1, buf);
	String master_tel1(buf);
	//Serial.println(master_tel1);

	EEPROM.get(Address_tel2, buf);
	String master_tel2(buf);
	//Serial.println(master_tel2);
	
	EEPROM.get(Address_tel3, buf);
	String master_tel3(buf);
	//Serial.println(master_tel3);
if(EEPROM.read(Address_port1))
 {
 
 }
 else
 {
	//dataport1 = digitalRead(port1);
	//Serial.println(dataport1);
 }

 if(EEPROM.read(Address_port2))
 {
 
 }
 else
 {
	/*dataport2 = digitalRead(port2);
	Serial.println(dataport2);*/
 }


	return DELIM + master_tel1 + DELIM + master_tel2 + DELIM + master_tel3;
}



void gprs_send(String data) 
{
  con.print("Requesting ");
  con.print(url1);
  con.print('?');
  con.println(data);
  gprs.httpConnectStr(url1, data);
  count++;
  if(count >1)
  {
      start_error = true;
  }
  while (gprs.httpIsConnected() == 0) 
  {
    con.write('.');
    for (byte n = 0; n < 25 && !gprs.available(); n++) 
	{
      delay(10);
    }
  }
  if (gprs.httpState == HTTP_ERROR) 
  {
  //  con.println("Connect error");
if(start_error)                        // ������������ ������ ��� ������ �������
{
    errors++;
	errorAllmem();
}
	if (errors > 30)
	  {
			//con.println("Number of transmission errors exceeded");
			resetFunc();          // �������� reset ����� 30 ������
	  }
    delay(3000);
    return; 
  }
  
  con.println();
  gprs.httpRead();
  int ret;
  while ((ret = gprs.httpIsRead()) == 0) 
  {
    // ����� ������� ���-�� �����, �� ����� ��������
  }
  if (gprs.httpState == HTTP_ERROR) 
  {
   // con.println("Read error");
//	if(start_error)                      // ������������ ������ ��� ������ �������
//{
    errors++;
	errorAllmem();
//}
	if (errors > 30)
	  {
			//con.println("The number of server errors exceeded");
			resetFunc();         // �������� reset ����� 30 ������
	  }
    delay(3000);
    return; 
  }

  // ������ �� �������� �������� ��������
   con.print("[Payload] ");
   con.println(gprs.buffer);
   String command = gprs.buffer;                       // �������� ������ ������ � �������
   String commEXE = command.substring(0, 2);           // �������� ������ � ��������
   int var = commEXE.toInt();                          // �������� ����� �������. ������������� ������ ������� � ����� 

   if(var == 1)                                       // ��������� ������� 1
	{
		String commData = command.substring(2, 10);    // �������� ������ � �������
		unsigned long interval1 = commData.toInt();    // ������������� ������ ������ � ����� 
		con.println(interval1);
		if(interval1 > 5000 && interval1 < 86400001)   // ���������� ��������� �� 5������ �� 24 �����.
		{
          interval = interval1;                        // ����������� �������� �������� �� ������
		}
		con.println(interval);
	}
	
	else if(var == 2)                                  // ��������� ������� 2
	{
	    command.remove(0, 2);                          // �������� ������ ������ �������� �� �������
		EEPROM.get(Address_tel1, data_tel);            // �������� ����� �������� �� EEPROM
		String num_tel(data_tel);
		//Serial.println(num_tel);
		if (command != num_tel) 
		{
			Serial.println("no compare");
			for(int i=0;i<13;i++)
			{
				EEPROM.write(i+Address_tel1,command[i]);
			}
		}
	}
	
	else if(var == 3)                                  // ��������� ������� 3
	{
	    command.remove(0, 2);                          // �������� ������ ������ �������� �� �������
		EEPROM.get(Address_tel2, data_tel);            // �������� ����� �������� �� EEPROM
		String num_tel(data_tel);
		//Serial.println(num_tel);
		if (command != num_tel) 
		{
			//Serial.println("no compare");
			for(int i=0;i<13;i++)
			{
				EEPROM.write(i+Address_tel2,command[i]);
			}
		}
	}
	else if(var == 4)                                  // ��������� ������� 4
	{
	    command.remove(0, 2);                          // �������� ������ ������ �������� �� �������
		EEPROM.get(Address_tel3, data_tel);            // �������� ����� �������� �� EEPROM
		String num_tel(data_tel);
		//Serial.println(num_tel);
		if (command != num_tel) 
		{
			//Serial.println("no compare");
			for(int i=0;i<13;i++)
			{
				EEPROM.write(i+Address_tel3,command[i]);
			}
		}
	}
	else if(var == 5)                                  // ��������� ������� 5
	{
		  EEPROM.put(Address_errorAll, 0);             // �������� ������� ������
	}
	else if(var == 6)                                  // ��������� ������� 6
	{
		//  ����� � ����� ����� �������� �� 98 ������  
	}
	else if(var == 7)                                  // ��������� ������� 7
	{
		//  ����� � ����� ����� �������� �� 98 ������  
	}
	else if(var == 8)                                  // ��������� ������� 8
	{
		//  ����� � ����� ����� �������� �� 98 ������  
	}
	else
	{
		// ����� ����� ��� �� ��������� ���� ������� �� ������
	}
	
  // �������� ����������
  con.print("Total:");
  con.print(count);
  if (errors) 
  {
    con.print(" Errors:");
    con.print(errors);
  }
  con.println();
}

void errorAllmem()
{
  int error_All;
  EEPROM.get(Address_errorAll, error_All);
  error_All++;
  EEPROM.put(Address_errorAll, error_All);            
}

int freeRam ()
{
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

void setTime(String val, String f_phone)
{
  if (val.indexOf("Timeset") > -1) 
  {
     interval = 20000;    // ���������� �������� 20 ������
  } 
  else if (val.indexOf("Restart") > -1) 
  {
     resetFunc();             //�������� reset
  } 
}

void setup()
{
	con.begin(19200);
	con.println("SIM800 setup start");

	pinMode(LED_RED,  OUTPUT);
	pinMode(LED_BLUE, OUTPUT);
	pinMode(LED_GREEN,OUTPUT);

	setColor(COLOR_RED);
	delay(300);
	setColor(COLOR_GREEN); 
	delay(300);
	setColor(COLOR_BLUE);
	delay(300);
    setColor(COLOR_RED);
	DeviceAddress deviceAddress;
	sensor1.setOneWire(&ds18x20_1);
	sensor2.setOneWire(&ds18x20_2);
	sensor3.setOneWire(&ds18x20_3);
	sensor1.begin();
	sensor2.begin();
	sensor3.begin();
	if (sensor1.getAddress(deviceAddress, 0)) sensor1.setResolution(deviceAddress, 12);
	if (sensor2.getAddress(deviceAddress, 0)) sensor2.setResolution(deviceAddress, 12);
	if (sensor3.getAddress(deviceAddress, 0)) sensor2.setResolution(deviceAddress, 12);

	for (;;) 
	{
		con.print("Resetting...");
		while (!gprs.init(PWR_On, SIM800_RESET_PIN, LED13))
	{
		con.write('.');
	}
	con.println("OK");

	delay(20000);                           // ������� ����������� � �������
	con.print("Setting up network...");

	if (gprs.getIMEI())                     // �������� IMEI
	{
		//con.print("\nIMEI:");
		imei = gprs.buffer;
		gprs.cleanStr(imei);
		//con.println(imei);
	}

		byte ret = gprs.setup();
		if (ret == 0) 
		{
	    	setColor(COLOR_GREEN);  // �������� ���������
			break;
		}
	/*	con.print("Error code:");
		con.println(ret);
		con.println(gprs.buffer);*/
		delay(2000);
		if (ret == 4 ||ret == 5) 
		{
            setColor(COLOR_RED);
			//con.print("The network is not defined");
			delay(1000);
			resetFunc();             //�������� reset ��� ���������� ����������� � ����
		}
	}
  con.println("OK");
  for (;;) 
  {
    if (gprs.httpInit()) break;
	//con.print(">");
    //con.println(gprs.buffer);
	String stringError = gprs.buffer;

	if (stringError.indexOf("ERROR") > -1) 
	 {
		//con.print("No internet connection");
		delay(1000);
		resetFunc();   //�������� reset ��� ���������� ������� � �������
	 }
    gprs.httpUninit();
    delay(1000);
  }
    //EEPROM.put(Address_tel2, "+79160000000");  
	//EEPROM.get(Address_tel2, data_tel);            // �������� ����� �������� �� EEPROM
	//con.print("data_tel  ");
	//con.println(data_tel);

 // EEPROM.write(Address_port1,255);
 // EEPROM.write(Address_port2,255);

 //if(EEPROM.read(Address_port1))
 //{
 //   pinMode(port1,   OUTPUT);
 //}
 //else
 //{
	//pinMode(port1,  INPUT); 
 //}

 //if(EEPROM.read(Address_port2))
 //{
 //   pinMode(port2,  OUTPUT);
 //}
 //else
 //{
	//pinMode(port2,   INPUT); 
 //}

  con.println("SIM800 setup end");
  time = millis();                                       // ����� ������� �����
}

void loop()
{
 if (gprs.checkSMSU()) 
  {
    //con.print("SMS:");
    //con.println(gprs.val);
	if (gprs.val.indexOf("+CMT") > -1)  //���� ��������� ��� (��� ����������� ������ ������ "+CMT" ������� "RING", ������ �� �� �����, �� ����������� �� ���� ������ �����)
	{    
	//------------- ����� �������� ����� � ��� 
	char buf[13] ;
	EEPROM.get(Address_tel2, buf);        // ������������ ������� ������� 1
	String master_tel2(buf);

	EEPROM.get(Address_tel3, buf);         // ������������ ������� ������� 2
	String master_tel3(buf);

      if (gprs.val.indexOf(master_tel2) > -1)  //���� ��� �� ������� 1
	  {   
		setTime(gprs.val, master_tel2);
      }
	  else if(gprs.val.indexOf(master_tel3) > -1)  //���� ��� �� ������� 2
	  {
        setTime(gprs.val, master_tel3);
      }
    }
	    
		gprs.val = "";
  }
	unsigned long currentMillis = millis();
  
	if ((unsigned long)(currentMillis - previousMillis) >= interval) 
	{
		setColor(COLOR_BLUE);
		previousMillis = currentMillis;
		sendTemps();
		setColor(COLOR_GREEN);
		//Serial.println();
		Serial.print("\nfree memory: ");
		Serial.println(freeRam());
	}

    if(millis() - time > time_day) resetFunc();             //�������� reset ���������� � �����
	delay(500);
}
