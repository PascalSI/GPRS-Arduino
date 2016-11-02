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
#define NETLIGHT         3                          // ��������� NETLIGHT
#define STATUS           9                          // ��������� STATUS

#define port1           11                          // ���� ���������� �������� ������������ (��������������)
#define port2           12                          // ���� ���������� �������� ������������ (��������������)


                                                    // ����������  � ������ 7 ������ RX ������ GPRS. ���������� � ���������� SIM800.h
//#define COMMON_ANODE
                                                    // ����������  � ������ 8 ������ TX ������ GPRS. ���������� � ���������� SIM800.h  
#define LED_RED      10                             // ��������� ����������� RED
#define LED_BLUE     15                             // ��������� ����������� BLUE
#define LED_GREEN    14                             // ��������� ����������� GREEN

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
String SMS_center = "";
String zero_tel   = "";
//String imei = "861445030362268";                  // ���� IMEI
//#define DELIM "&"
#define DELIM "@"
//char mydata[] = "t1=861445030362268@04/01/02,15:22:52 00@24.50@25.60";
// ��� ������� +79258110171

unsigned long time;                                 // ���������� ��� ��������� ������
unsigned long time_day = 86400;                     // ���������� ������ � ������
unsigned long previousMillis = 0;
unsigned long interval = 10;                        // �������� �������� ������ 30 ������
//unsigned long interval = 300;                     // �������� �������� ������ 5 �����
bool time_set = false;                              // ����������� �������� �������� ���


int Address_tel1       = 100;                         // ����� � EEPROM �������� 1
int Address_tel2       = 120;                         // ����� � EEPROM �������� 2
int Address_tel3       = 140;                         // ����� � EEPROM �������� 3
int Address_errorAll   = 160;                         // ����� � EEPROM �������� ����� ������
int Address_port1      = 180;                         // ����� � EEPROM ���� ������ (��������������)
int Address_port2      = 190;                         // ����� � EEPROM ���� ������ (��������������)
int Address_interval   = 200;                         // ����� � EEPROM �������� ���������
int Address_SMS_center = 220;                         // ����� � EEPROM SMS ������

char data_tel[13];                                  // ����� ��� ������ �������

int dataport1 = 0;                                  // ���� ������ (��������������)
int dataport2 = 0;                                  // ���� ������ (��������������)


uint8_t oneWirePins[]={16, 17, 4};                     //������ �������� ����������� DS18x20. ����������� ������ ����� ������������� ����������� �������� � ������.
                                                       // ������ ������ ���� ���������� ������.
uint8_t oneWirePinsCount=sizeof(oneWirePins)/sizeof(int);

OneWire ds18x20_1(oneWirePins[0]);
OneWire ds18x20_2(oneWirePins[1]);
OneWire ds18x20_3(oneWirePins[2]);
DallasTemperature sensor1(&ds18x20_1);
DallasTemperature sensor2(&ds18x20_2);
DallasTemperature sensor3(&ds18x20_3);

const char  txt_zero_tel[]                 PROGMEM  = "+79990000000";  
const char  txt_SMS_center[]               PROGMEM  = "SMS.RU";
const char  txt_String_length[]            PROGMEM  = "String length: ";  
const char  txt_uptime[]                   PROGMEM  = "17/01/01,10:10:10 00";
const char  txt_Requesting[]               PROGMEM  = "Requesting ";
const char  txt_Payload[]                  PROGMEM  = "[Payload] ";
const char  txt_nocompare[]                PROGMEM  = "no compare";
const char  txt_Total[]                    PROGMEM  = "Total:";
const char  txt_Errors[]                   PROGMEM  = " Errors:";
const char  txt_Timeset[]                  PROGMEM  = "Timeset";
const char  txt_Restart[]                  PROGMEM  = "Restart";
const char  txt_Timeoff[]                  PROGMEM  = "Timeoff";
const char  txt_setup_start[]              PROGMEM  = " SIM800 setup start";
const char  txt_Resetting[]                PROGMEM  = "Resetting...";
const char  txt_OK1[]                      PROGMEM  = "OK";
const char  txt_Setting_up_network[]       PROGMEM  = "Setting up network...";
const char  txt_ERROR1[]                   PROGMEM  = "ERROR";
const char  txt_Start_clear_EEPROM[]       PROGMEM  = "Start clear EEPROM";
const char  txt_Clear_EEPROM_End[]         PROGMEM  = "Clear EEPROM End";
const char  txt_mytel[]                    PROGMEM  = "+79162632701";
const char  txt_Interval[]                 PROGMEM  = "\nInterval: ";
const char  txt_setup_end[]                PROGMEM  = "\nSIM800 setup end";
const char  txt_SMS[]                      PROGMEM  = "SMS:";
const char  txt_CMT[]                      PROGMEM  = "+CMT";
const char  txt_Interval_sec[]             PROGMEM  = "Interval sec:";
const char  txt_free_memory[]              PROGMEM  = "\nfree memory: ";
const char  txt_No_internet_con[]          PROGMEM  = "No internet connection";
const char  txt_phone_ignored[]            PROGMEM  =  "phone ignored";
const char  txt_commandTel1[]              PROGMEM  = "Commanda tel1";
const char  txt_commandTel2[]              PROGMEM  = "Commanda tel2";


const char* const table_message2[] PROGMEM =
{
 txt_zero_tel,                // 0 "+79990000000";    
 txt_SMS_center,              // 1 "SMS.RU";
 txt_String_length,           // 2 "String length: ";  
 txt_uptime,                  // 3 "17/01/01,10:10:10 00";
 txt_Requesting,              // 4 "Requesting ";
 txt_Payload,                 // 5 "[Payload] ";
 txt_nocompare,               // 6 "no compare";
 txt_Total,                   // 7 "Total:";
 txt_Errors,                  // 8 " Errors:";
 txt_Timeset,                 // 9 "Timeset";
 txt_Restart,                 // 10 "Restart";
 txt_Timeoff,                 // 11 "Timeoff";
 txt_setup_start,             // 12 " SIM800 setup start";
 txt_Resetting,               // 13 "Resetting...";
 txt_OK1,                     // 14 "OK";
 txt_Setting_up_network,      // 15 "Setting up network...";
 txt_ERROR1,                  // 16 "ERROR";
 txt_Start_clear_EEPROM,      // 17 "Start clear EEPROM";
 txt_Clear_EEPROM_End,        // 18 "Clear EEPROM End";
 txt_mytel,                   // 19 "+79162632701";
 txt_Interval,                // 20 "\nInterval: ";
 txt_setup_end,               // 21 "\nSIM800 setup end";
 txt_SMS,                     // 22 "SMS:";
 txt_CMT,                     // 23 "+CMT";
 txt_Interval_sec,            // 24 "Interval sec:";
 txt_free_memory,             // 25 "\nfree memory: ";
 txt_No_internet_con,         // 26 "No internet connection";
 txt_phone_ignored,           // 27 "phone ignored";
 txt_commandTel1,             // 28 "Commanda tel1";
 txt_commandTel2              // 29 "Commanda tel2";

};

char bufmessage[30];


void(* resetFunc) (void) = 0;                         // ��������� ������� reset

 void setColor(bool red, bool green, bool blue)       // ��������� ����� �������� ������������ ����������.
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
	float tsumma = t1+t2+t3+88.88;
	int signal = gprs.getSignalQuality();
	int error_All = 0;
	EEPROM.get(Address_errorAll, error_All);
	//String toSend = formHeader()+DELIM+"temp1="+String(t1)+DELIM+"temp2="+String(t2)+DELIM+"tempint="+String(t3)+ DELIM+"slevel="+String(signal)+DELIM+"ecs="+String(errors)+DELIM+"ec="+String(error_All)+formEnd();
	String toSend = formHeader()+DELIM+String(t1)+DELIM+String(t2)+DELIM+String(t3)+ DELIM+String(signal)+DELIM+String(errors)+DELIM+String(error_All)+formEnd()+DELIM+String(tsumma);
//	Serial.println(toSend);

	strcpy_P(bufmessage, (char*)pgm_read_word(&(table_message2[2])));
    Serial.print(bufmessage);
	Serial.println(toSend.length());
	gprs_send(toSend);
}

String formHeader() 
{
  strcpy_P(bufmessage, (char*)pgm_read_word(&(table_message2[3])));
  String uptime = bufmessage;
  GSM_LOCATION loc;                               // �������� ����� �� ���������
  if (gprs.getLocation(&loc)) 
  {
   uptime  = String(loc.year)+'/'+ String(loc.month)+'/'+ String(loc.day)+','+String(loc.hour)+':'+ String(loc.minute)+':'+String(loc.second)+" 00";
  //  uptime  = "date="+ String(loc.year)+'_'+ String(loc.month)+'_'+ String(loc.day)+','+String(loc.hour)+':'+ String(loc.minute)+':'+String(loc.second)+"00";
  }
 // return "imei=" + imei + DELIM + uptime;
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

	 //EEPROM.get(Address_tel1, master_tel1); 
	 //EEPROM.get(Address_tel2, master_tel3); 
	 //EEPROM.get(Address_tel2, master_tel3); 


	 EEPROM.get(Address_SMS_center, SMS_center);   //�������� �� EEPROM ��� �����


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
String mytel = "mytel=" + master_tel1;
String tel1 = "tel1=" + master_tel2;
String tel2 = "tel2=" + master_tel3;

//return DELIM + mytel + DELIM +tel1 + DELIM + tel2;
return DELIM + master_tel1 + DELIM + master_tel2 + DELIM + master_tel3 + DELIM + SMS_center;

}



void gprs_send(String data) 
{

  strcpy_P(bufmessage, (char*)pgm_read_word(&(table_message2[4])));
  con.print(bufmessage);               //con.print("Requesting ");
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
	if (errors > 20)
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
    errors++;
	errorAllmem();
	if (errors > 20)
	  {
			//con.println("The number of server errors exceeded");
			resetFunc();         // �������� reset ����� 30 ������
	  }
    delay(3000);
    return; 
  }

  // ������ �� �������� ��������� �� �����.
   strcpy_P(bufmessage, (char*)pgm_read_word(&(table_message2[5])));
   con.print(bufmessage);                              //con.print("[Payload] ");
   con.println(gprs.buffer);
   String command = gprs.buffer;                       // �������� ������ ������ � �������
   String commEXE = command.substring(0, 2);           // �������� ������ � ��������
   int var = commEXE.toInt();                          // �������� ����� �������. ������������� ������ ������� � ����� 

   if(var == 1)                                        // ��������� ������� 1
	{
		String commData = command.substring(2, 10);    // �������� ������ � �������
		unsigned long interval1 = commData.toInt();    // ������������� ������ ������ � ����� 
		con.println(interval1);
		if(interval1 > 10 && interval1 < 86401)         // ���������� ��������� �� 10  ������ �� 24 �����.
		{
		  if(interval1!=interval)                       // ���� ���������� �� ���������� - �� ������ � EEPROM
		  {
			 if(!time_set)                              // ���� ��� ������� �������� ��������� �� ��� 
			 {
			    interval = interval1;                    // ����������� �������� �������� �� ������
			    EEPROM.put(Address_interval, interval);  // �������� �������� EEPROM , ���������� �� �������
			 }
		  }
		}
		con.println(interval);
	}
	
	else if(var == 2)                                  // ��������� ������� 2
	{
	    command.remove(0, 2);                          // �������� ������ ������ �������� �� �������
		EEPROM.get(Address_tel1, data_tel);            // �������� ����� �������� �� EEPROM
		String num_tel(data_tel);
		if (command != num_tel)                        // ���� ���������� �� ���������� - �� ������ � EEPROM
		{
			 strcpy_P(bufmessage, (char*)pgm_read_word(&(table_message2[6])));

			 Serial.println(bufmessage);               //Serial.println("no compare");
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
		if (command != num_tel)                        // ���� ���������� �� ���������� - �� ������ � EEPROM
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
		if (command != num_tel)                        // ���� ���������� �� ���������� - �� ������ � EEPROM
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
	    command.remove(0, 2);                          // �������� ������ ������ �������� �� �������
		EEPROM.get(Address_SMS_center, data_tel);      // �������� �� EEPROM ��� �����
		String num_tel(data_tel);
		if (command != num_tel)                        // ���� ���������� �� ���������� - �� ������ � EEPROM
		{
			//Serial.println("no compare");
			for(int i=0;i<13;i++)
			{
				EEPROM.write(i+Address_SMS_center,command[i]);
			}
		}
	}
	else if(var == 7)                                  // ��������� ������� 7
	{
		time_set = false;                              // ����� �������� ��������� ��������� ���
	}
	else if(var == 8)                                  // ��������� ������� 8
	{
		//  ����� � ����� ����� �������� �� 90 ������  
	}
	else
	{
		// ����� ����� ��� �� ��������� ���� ������� �� ������
	}
	
  // �������� ����������
  strcpy_P(bufmessage, (char*)pgm_read_word(&(table_message2[7])));
  con.print(bufmessage);                  //con.print("Total:");
  con.print(count);
  if (errors) 
  {

    strcpy_P(bufmessage, (char*)pgm_read_word(&(table_message2[8])));
    con.print(bufmessage); //con.print(" Errors:");
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
   strcpy_P(bufmessage, (char*)pgm_read_word(&(table_message2[9])));

  if (val.indexOf(bufmessage) > -1)         // (val.indexOf("Timeset") > -1) 
  {
     interval = 20;                                     // ���������� �������� 20 ������
	 time_set = true;                                   // ���������� �������� ��������� ��������� ���
	 Serial.println(interval);
  } 
  else if (val.indexOf("Restart") > -1) 
  {
	  strcpy_P(bufmessage, (char*)pgm_read_word(&(table_message2[10])));
	  Serial.print(f_phone);
	  Serial.print("..");
	  Serial.println(bufmessage);
      resetFunc();                                        //�������� reset
  } 
  else if (val.indexOf("Timeoff") > -1) 
  {

     time_set = false;                              // ����� �������� ��������� ��������� ���
  } 
}

void setup()
{
	con.begin(19200);

	strcpy_P(bufmessage, (char*)pgm_read_word(&(table_message2[12])));
	con.println(bufmessage);            // con.println(" SIM800 setup start");     
	pinMode(LED_RED,  OUTPUT);
	pinMode(LED_BLUE, OUTPUT);
	pinMode(LED_GREEN,OUTPUT);
	pinMode(NETLIGHT ,INPUT);                      // ��������� NETLIGHT
	pinMode(STATUS ,INPUT);                        // ��������� STATUS

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

	 strcpy_P(bufmessage, (char*)pgm_read_word(&(table_message2[13])));

	for (;;) 
	{
		con.print(bufmessage);           // "Resetting...";
		while (!gprs.init(PWR_On, SIM800_RESET_PIN, LED13))
	{
		con.write('.');
	}
	 strcpy_P(bufmessage, (char*)pgm_read_word(&(table_message2[14])));
	con.println(bufmessage);             // con.println("OK");

	delay(20000);                           // ������� ����������� � �������
	strcpy_P(bufmessage, (char*)pgm_read_word(&(table_message2[15])));
	con.print(bufmessage);        // con.print("Setting up network...");

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

	strcpy_P(bufmessage, (char*)pgm_read_word(&(table_message2[14])));
  con.println(bufmessage);           // con.println("OK");
  for (;;) 
  {
    if (gprs.httpInit()) break;
	con.print(">");
    con.println(gprs.buffer);
	String stringError = gprs.buffer;
	strcpy_P(bufmessage, (char*)pgm_read_word(&(table_message2[16])));
	if (stringError.indexOf(bufmessage) > -1)                   // if (stringError.indexOf("ERROR") > -1)
	 {
		strcpy_P(bufmessage, (char*)pgm_read_word(&(table_message2[26])));
		con.print(bufmessage);              //con.print("No internet connection");
		delay(1000);
		resetFunc();   //�������� reset ��� ���������� ������� � �������
	 }
    gprs.httpUninit();
    delay(1000);
  }

 if(EEPROM.read(0)!=31)
 {
	 strcpy_P(bufmessage, (char*)pgm_read_word(&(table_message2[17])));
	 Serial.println (bufmessage);               //  Serial.println ("Start clear EEPROM");
	 for(int i = 0; i<1023;i++)
	 {
		 EEPROM.write(i,0);
	 }
	  EEPROM.write(0,31);
	  EEPROM.put(Address_interval, interval);                     // ������ ��������� ��������� ����������
	  strcpy_P(bufmessage, (char*)pgm_read_word(&(table_message2[0])));
	  EEPROM.put(Address_tel1,bufmessage);   
	  EEPROM.put(Address_tel2,bufmessage);   
	  EEPROM.put(Address_tel3,bufmessage); 
	  EEPROM.put(Address_SMS_center, bufmessage); 
	  strcpy_P(bufmessage, (char*)pgm_read_word(&(table_message2[18])));
	  Serial.println (bufmessage);                                //  Serial.println ("Clear EEPROM End");

 }
     strcpy_P(bufmessage, (char*)pgm_read_word(&(table_message2[1])));
     SMS_center = bufmessage;                                     //  SMS_center = "SMS.RU";
 	// EEPROM.put(Address_interval, interval);                    // ��������������� ������ ����� ��������� ����������
     EEPROM.put(Address_SMS_center, SMS_center);                  // ��������������� ������ ����� ��������� ��� ������
	 strcpy_P(bufmessage, (char*)pgm_read_word(&(table_message2[19])));
    // EEPROM.put(Address_tel3, bufmessage);                      //  EEPROM.put(Address_tel3, "+79162632701");   

 	 EEPROM.get(Address_interval, interval);                      //�������� �� EEPROM ��������
	 EEPROM.get(Address_SMS_center, SMS_center);                  //�������� �� EEPROM ��� �����

	 strcpy_P(bufmessage, (char*)pgm_read_word(&(table_message2[20])));
	 con.print(bufmessage);                                       //  con.print("\nInterval: ");
	 con.println(interval);

	 con.println(SMS_center);
	 
	setColor(COLOR_BLUE);
	sendTemps();
	setColor(COLOR_GREEN);
	strcpy_P(bufmessage, (char*)pgm_read_word(&(table_message2[21])));
	con.println(bufmessage);                                     // con.println("\nSIM800 setup end");
	time = millis();                                             // ����� ������� �����
}

void loop()
{
 if (gprs.checkSMSU()) 
  {
	//strcpy_P(bufmessage, (char*)pgm_read_word(&(table_message2[22])));
 //   con.print(bufmessage);                    //  con.print("SMS:");
 //   con.println(gprs.val);
	strcpy_P(bufmessage, (char*)pgm_read_word(&(table_message2[23])));
	if (gprs.val.indexOf("+CMT") > -1)  //���� ��������� ��� (��� ����������� ������ ������ "+CMT" ������� "RING", ������ �� �� �����, �� ����������� �� ���� ������ �����)
	{    
	//------------- ����� �������� ����� � ��� 
	char buf[13] ;

	EEPROM.get(Address_tel2, buf);                                         // ������������ ������� ������� 1
	String master_tel2(buf);
	EEPROM.get(Address_tel3, buf);                                         // ������������ ������� ������� 2
	String master_tel3(buf);
	EEPROM.get(Address_SMS_center, buf);                                   // ������������ ������� ��� ������
	String master_SMS_center(buf);

      if (gprs.val.indexOf(master_tel2) > -1)                              //���� ��� �� ������� 1
	  {   
		strcpy_P(bufmessage, (char*)pgm_read_word(&(table_message2[28])));
		Serial.println(bufmessage);
		setTime(gprs.val, master_tel2);
      }
	  else if(gprs.val.indexOf(master_tel3) > -1)                          //���� ��� �� ������� 2
	  {
      	strcpy_P(bufmessage, (char*)pgm_read_word(&(table_message2[29])));
		Serial.println(bufmessage);
		setTime(gprs.val, master_tel3);
      }
	  else if(gprs.val.indexOf(master_SMS_center) > -1)                    //���� ��� �� ������� 2
	  {
 		Serial.println("SMS centr");
		setTime(gprs.val, master_SMS_center);
      }
	  else
	  {
		  strcpy_P(bufmessage, (char*)pgm_read_word(&(table_message2[27])));
		  Serial.println(bufmessage);            //   Serial.println("phone ignored");
	  }
     }
	    
		gprs.val = "";
  }
	unsigned long currentMillis = millis();
	if(!time_set)                                                               // 
	{
         EEPROM.get( Address_interval, interval);                               // �������� �������� �� EEPROM Address_interval
	}
	if ((unsigned long)(currentMillis - previousMillis) >= interval*1000) 
	{
        strcpy_P(bufmessage, (char*)pgm_read_word(&(table_message2[24])));
		Serial.print(bufmessage);                                         // Serial.print("Interval sec:"); 
		Serial.println((currentMillis-previousMillis)/1000);
		setColor(COLOR_BLUE);
		previousMillis = currentMillis;
		sendTemps();
		setColor(COLOR_GREEN);
		strcpy_P(bufmessage, (char*)pgm_read_word(&(table_message2[25])));
		Serial.print(bufmessage);                                  // Serial.print("\nfree memory: ");
		Serial.println(freeRam());
	}

    if(millis() - time > time_day*1000) resetFunc();                       //�������� reset ���������� � �����
	delay(500);
}
