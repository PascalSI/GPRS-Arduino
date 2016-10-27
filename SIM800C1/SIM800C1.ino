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


#define PWR_On           5                          // Включение питания модуля SIM800
#define SIM800_RESET_PIN 6                          // Сброс модуля SIM800
#define LED13           13                          // Индикация светодиодом
#define port1           11                          // Порт управления внешними устройствами
#define port2           12                          // Порт управления внешними устройствами


                                                    // Подключить  к выводу 7 сигнал RX модуля GPRS. Установить в библиотеке SIM800.h
//#define COMMON_ANODE
                                                    // Подключить  к выводу 8 сигнал TX модуля GPRS. Установить в библиотеке SIM800.h  
#define LED_RED      10                             // Индикация светодиодом RED
#define LED_BLUE     14                             // Индикация светодиодом BLUE
#define LED_GREEN    15                             // Индикация светодиодом GREEN

#define COLOR_NONE LOW, LOW, LOW
#define COLOR_RED HIGH, LOW, LOW
#define COLOR_GREEN LOW, HIGH, LOW
#define COLOR_BLUE LOW, LOW, HIGH

bool start_error = false;                         // флаг компенсации первой ошибки при старте.

CGPRS_SIM800 gprs;
uint32_t count  = 0;
uint32_t errors = 0;
String imei = "";
String CSQ = "";                                    // Уровень сигнала приема
//String header = "";
//String imei = "861445030362268";                  // Тест IMEI
#define DELIM "@"
//char mydata[] = "t1=861445030362268@04/01/02,15:22:52 00@24.50@25.60";
// тел Мегафон +79258110171

unsigned long time;                                 // Переменная для суточного сброса
unsigned long time_day = 86400000;                  // Переменная миллисекунд в сутках
unsigned long previousMillis = 0;
//unsigned long interval = 10000;                     // Интервал передачи данных 30 секунд
unsigned long interval = 300000;                  // Интервал передачи данных 5 минут

//char test_tel[13] = "+79160000000";               // укажите  телефон хозяина
int Address_tel1     = 100;                         // Адрес в EEPROM телефона 1
int Address_tel2     = 120;                         // Адрес в EEPROM телефона 2
int Address_tel3     = 140;                         // Адрес в EEPROM телефона 3
int Address_errorAll = 160;                         // Адрес в EEPROM счетчика общих ошибок
int Address_port1    = 180;                         // Адрес в EEPROM телефона 3
int Address_port2    = 190;                         // Адрес в EEPROM телефона 3


char data_tel[13];                                  // Буфер для номера телефоа

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


void(* resetFunc) (void) = 0;                       // объявляем функцию reset

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
  GSM_LOCATION loc;                               // Получить время из интернета
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
if(start_error)                        // Корректируем ошибку при первом запуске
{
    errors++;
	errorAllmem();
}
	if (errors > 30)
	  {
			//con.println("Number of transmission errors exceeded");
			resetFunc();          // вызываем reset после 30 ошибок
	  }
    delay(3000);
    return; 
  }
  
  con.println();
  gprs.httpRead();
  int ret;
  while ((ret = gprs.httpIsRead()) == 0) 
  {
    // может сделать что-то здесь, во время ожидания
  }
  if (gprs.httpState == HTTP_ERROR) 
  {
   // con.println("Read error");
//	if(start_error)                      // Корректируем ошибку при первом запуске
//{
    errors++;
	errorAllmem();
//}
	if (errors > 30)
	  {
			//con.println("The number of server errors exceeded");
			resetFunc();         // вызываем reset после 30 ошибок
	  }
    delay(3000);
    return; 
  }

  // Теперь мы получили полезную нагрузку
   con.print("[Payload] ");
   con.println(gprs.buffer);
   String command = gprs.buffer;                       // Получить строку данных с сервера
   String commEXE = command.substring(0, 2);           // Выделить строку с командой
   int var = commEXE.toInt();                          // Получить номер команды. Преобразовать строку команды в число 

   if(var == 1)                                       // Выполнить команду 1
	{
		String commData = command.substring(2, 10);    // Выделить строку с данными
		unsigned long interval1 = commData.toInt();    // Преобразовать строку данных в число 
		con.println(interval1);
		if(interval1 > 5000 && interval1 < 86400001)   // Ограничить интервалы от 5секунд до 24 часов.
		{
          interval = interval1;                        // Переключить интервал передачи на сервер
		}
		con.println(interval);
	}
	
	else if(var == 2)                                  // Выполнить команду 2
	{
	    command.remove(0, 2);                          // Получить данные номера телефона от сервера
		EEPROM.get(Address_tel1, data_tel);            // Получить номер телефона из EEPROM
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
	
	else if(var == 3)                                  // Выполнить команду 3
	{
	    command.remove(0, 2);                          // Получить данные номера телефона от сервера
		EEPROM.get(Address_tel2, data_tel);            // Получить номер телефона из EEPROM
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
	else if(var == 4)                                  // Выполнить команду 4
	{
	    command.remove(0, 2);                          // Получить данные номера телефона от сервера
		EEPROM.get(Address_tel3, data_tel);            // Получить номер телефона из EEPROM
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
	else if(var == 5)                                  // Выполнить команду 5
	{
		  EEPROM.put(Address_errorAll, 0);             // Сбросить счетчик ошибок
	}
	else if(var == 6)                                  // Выполнить команду 6
	{
		//  Здесь и далее можно добавить до 98 команд  
	}
	else if(var == 7)                                  // Выполнить команду 7
	{
		//  Здесь и далее можно добавить до 98 команд  
	}
	else if(var == 8)                                  // Выполнить команду 8
	{
		//  Здесь и далее можно добавить до 98 команд  
	}
	else
	{
		// здесь можно что то выполнить если команда не пришла
	}
	
  // Показать статистику
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
     interval = 20000;    // Установить интервал 20 секунд
  } 
  else if (val.indexOf("Restart") > -1) 
  {
     resetFunc();             //вызываем reset
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

	delay(20000);                           // Ожидаем подключения к станции
	con.print("Setting up network...");

	if (gprs.getIMEI())                     // Получить IMEI
	{
		//con.print("\nIMEI:");
		imei = gprs.buffer;
		gprs.cleanStr(imei);
		//con.println(imei);
	}

		byte ret = gprs.setup();
		if (ret == 0) 
		{
	    	setColor(COLOR_GREEN);  // Включить светодиод
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
			resetFunc();             //вызываем reset при отсутствии регистрации в сети
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
		resetFunc();   //вызываем reset при отсутствии доступа к серверу
	 }
    gprs.httpUninit();
    delay(1000);
  }
    //EEPROM.put(Address_tel2, "+79160000000");  
	//EEPROM.get(Address_tel2, data_tel);            // Получить номер телефона из EEPROM
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
  time = millis();                                       // Старт отсчета суток
}

void loop()
{
 if (gprs.checkSMSU()) 
  {
    //con.print("SMS:");
    //con.println(gprs.val);
	if (gprs.val.indexOf("+CMT") > -1)  //если обнаружен СМС (для определения звонка вместо "+CMT" вписать "RING", трубку он не берет, но реагировать на факт звонка можно)
	{    
	//------------- поиск кодового слова в СМС 
	char buf[13] ;
	EEPROM.get(Address_tel2, buf);        // Восстановить телефон хозяина 1
	String master_tel2(buf);

	EEPROM.get(Address_tel3, buf);         // Восстановить телефон хозяина 2
	String master_tel3(buf);

      if (gprs.val.indexOf(master_tel2) > -1)  //если СМС от хозяина 1
	  {   
		setTime(gprs.val, master_tel2);
      }
	  else if(gprs.val.indexOf(master_tel3) > -1)  //если СМС от хозяина 2
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

    if(millis() - time > time_day) resetFunc();             //вызываем reset интервалом в сутки
	delay(500);
}
