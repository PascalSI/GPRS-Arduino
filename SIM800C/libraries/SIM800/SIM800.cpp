/*************************************************************************
* SIM800 GPRS/HTTP Library
* Distributed under GPL v2.0
* Written by Stanley Huang <stanleyhuangyc@gmail.com>
* For more information, please visit http://arduinodev.com
*************************************************************************/

#include "SIM800.h"
#include <SoftwareSerial.h>
#include <avr/pgmspace.h>

SoftwareSerial SIM_SERIAL(PIN_RX, PIN_TX);                // RX, TX

bool CGPRS_SIM800::init(int PWR_On,int SIM800_RESET_PIN,int LED13)
{
	_PWR_On           = PWR_On;                           // ��������� ������� ������ SIM800
    _SIM800_RESET_PIN = SIM800_RESET_PIN;                 // ����� ������ SIM800
    _LED13            = LED13;                            // ��������� �����������

 	SIM_SERIAL.begin(19200);
	pinMode(_SIM800_RESET_PIN, OUTPUT);
	pinMode(_LED13,    OUTPUT);
	pinMode(_PWR_On,   OUTPUT);
	digitalWrite(_SIM800_RESET_PIN,   LOW);               // ������ ����� � �������� ���������
	digitalWrite(_LED13,    LOW);
	digitalWrite(_PWR_On,   HIGH);                        // �������������� ��������� ������� ������ GPRS
	delay(1000);
	digitalWrite(_LED13,    HIGH);
	digitalWrite(_PWR_On,   LOW);                         // �������� ������� ������
	delay(2000);                                          // ������� ���������� ��������
	digitalWrite(_LED13,    LOW);
	delay(300);  
	digitalWrite(_PWR_On,   HIGH); 
	//digitalWrite(_SIM800_RESET_PIN,   HIGH);              // ���������� ����� ������ ����� ��������� �������
	//delay(1000);
	//digitalWrite(_SIM800_RESET_PIN,   LOW);    
	//digitalWrite(_LED13,    LOW);
	//delay(300);  
	//digitalWrite(_PWR_On,   HIGH); 
    delay(3000);

    if (sendCommand("AT")) 
	{
		strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[1])));
 	//	sendCommand("AT+IPR=19200");                    // ���������� �������� ������
		sendCommand(bufcom);                            // ���������� �������� ������
		strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[0])));
        //sendCommand("ATE0");                            // ��������� ��� 
        sendCommand(bufcom);   
		strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[2])));
		sendCommand(bufcom);                              // 1 � ���������� ����� (�� ���������). ������ �������� 1
		//sendCommand("AT+CFUN=1");                       // 1 � ���������� ����� (�� ���������). ������ �������� 1 � ������������� (�������� ������ � ���������� ������, �.�. ��������� = 1,1)
  	    strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[3])));
		sendCommand(bufcom);                              // ����� ��������� ��� - ������� (��� ����.)
		//sendCommand("AT+CMGF=1");                       // ����� ��������� ��� - ������� (��� ����.)
		strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[4])));
		sendCommand(bufcom);                                         // �������� ���
		//sendCommand("AT+CLIP=1");                        // �������� ���
		strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[5])));
		sendCommand(bufcom);                                         // ����� ��������� ������
		//sendCommand("AT+CSCS=\"GSM\"");                            // ����� ��������� ������
		strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[6])));
		sendCommand(bufcom);                                        // ����������� ��� � ��������� ����� ����� ������ (��� ����� 
		//sendCommand("AT+CNMI=2,2");                               // ����������� ��� � ��������� ����� ����� ������ (��� ����� ��������� ����� ������ � ������)tln("AT+CSCS=\"GSM\""); 
	  //  sendCommand("AT+IFC=2,2");   
		return true;
    }
    return false;
}
byte CGPRS_SIM800::setup()
{
  bool success = false;
  for (byte n = 0; n < 30; n++) 
  {
    strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[7])));
	if (sendCommand(bufcom, 2000)) 
   // if (sendCommand("AT+CREG?", 2000))  // ��� ����������� ����
										// ������ ��������:
										// 0 � ��� ���� ����������� ����
										// 1 � ���� ��� ����������� ����
										// 2 � ���� ��� ����������� ���� + ��� ���������
										// ������ ��������:
										// 0 � �� ���������������, ������ ���� ���
										// 1 � ���������������, �������� ����
										// 2 � �� ���������������, ��� ����� ����� ����
										// 3 � ����������� ���������
										// 4 � ����������
										// 5 � �������
	{
        char *p = strstr(buffer, "0,");
        if (p) 
		{
          char mode = *(p + 2);
#if DEBUG
          DEBUG.print("Mode:");
          DEBUG.println(mode);
#endif
          if (mode == '1' || mode == '5') 
		  {
            strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[8])));
            sendCommand(bufcom,1000); 	//sendCommand("AT+CSQ",1000); 
			char *p = strstr(buffer, "CSQ: ");
	/*		Serial.println();
			Serial.println(p);   */   
            success = true;
            break;
          }
        }
    }
    delay(1000);
  }
  		
  if (!success)
    return 1;
	strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[9])));
	if (!sendCommand(bufcom)) return 2;   // if (!sendCommand("AT+CGATT?"))     // ����������� � GPRS
 
    strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[10])));
    if (!sendCommand(bufcom)) return 3;  // 
    // if (!sendCommand("AT+SAPBR=3,1,\"Contype\",\"GPRS\""))     return 3;// 

     getOperatorName();
	 String OperatorName = buffer;
	 cleanStr(OperatorName);
     strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[12]))); //"MTS"
	if (OperatorName.indexOf(bufcom) > -1) 
	{
		strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[11])));
		apn  = bufcom;
		//apn  = "internet.mts.ru";
		strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[13])));
		user = bufcom;
		pwd  = bufcom;
	/*	user = "mts";
		pwd  = "mts";*/
		strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[11])));
		cont = bufcom;
		//cont = "internet.mts.ru";
		strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[12]))); //"MTS"
		Serial.println(bufcom);
	}
    else if (OperatorName.indexOf("Beeline") > -1) 
	{
	    strcpy_P(bufcom1, (char*)pgm_read_word(&(table_message[15])));
		apn = bufcom1;                                                 //apn = "internet.beeline.ru";
	    strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[16])));
		user = bufcom;                                                 //user = "beeline";
		pwd = bufcom;                                                  //pwd = "beeline";
		cont = bufcom1;                                                //cont = "internet.beeline.ru";
		strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[14])));
		Serial.println(bufcom);                                        //Serial.println("Beeline");
	}
	else if (OperatorName.indexOf("MegaFon") > -1) 
	{
	strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[17])));
	strcpy_P(bufcom1, (char*)pgm_read_word(&(table_message[18])));
	apn = bufcom1;	                                                   //apn = "internet";
	user = "";
	pwd = "";
	cont = bufcom1;	                                                   //cont = "internet";
	Serial.println(bufcom);                                            //Serial.println("MEGAFON");
	}
	else if (OperatorName.indexOf("TELE2") > -1) 
	{
		strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[20])));
		strcpy_P(bufcom1, (char*)pgm_read_word(&(table_message[21])));
		apn = bufcom1;                                                 //apn = "internet.TELE2.ru";
		user = "";
		pwd = "";
		apn = bufcom1;                                                 //cont = "internet.TELE2.ru";
		Serial.println(bufcom);	                                       // Serial.println("TELE2");
	}

	//  ��������� ��� ����������:
	//  ��� - APN internet.mts.ru ��� ������������ � ������ mts , ����� ������� *99#
	//  ������� - APN internet ��� ������������ � ������ internet , ����� ������� *99#
	//  ����2 - APN internet.tele2.ru ��� ������������ � ������ ����� , ����� ������� *99#
	//  ������ - APN internet.beeline.ru ��� ������������ � ������ beeline , ����� ������� *99# - ��� ��� ����� �� ��������
	//  ������ - APN home.beeline.ru ��� ������������ � ������ beeline , ����� ������� *99# - ��� ����������� ��� ��� ������

	//	AT+CGDCONT=1,"IP","home.beeline.ru" � ���������. 
	//  ��� ��� �� �������� ������ AT+CGDCONT=1,"IP","internet.beeline.ru" 
	//  ��� �������� AT+CGDCONT=1,"IP","internet"
	//  ��� ��� AT+CGDCONT=1,"IP","internet.mts.ru"
	//  ��� ����2 AT+CGDCONT=1,"IP","internet.tele2.ru"

	strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[22])));
	SIM_SERIAL.print(bufcom);                                       //SIM_SERIAL.print("AT+SAPBR=3,1,\"APN\",\"");
	SIM_SERIAL.print(apn);
	SIM_SERIAL.println('\"');
	if (!sendCommand(0))   return 4;

	strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[23])));
	SIM_SERIAL.print(bufcom);                                       //SIM_SERIAL.print("AT+SAPBR=3,1,\"USER\",\"");
	SIM_SERIAL.print(user);
	SIM_SERIAL.println('\"');
	if (!sendCommand(0))   return 4;

	strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[24])));
	SIM_SERIAL.print(bufcom);                                       //SIM_SERIAL.print("AT+SAPBR=3,1,\"PWD\",\"");
	SIM_SERIAL.print(pwd);
	SIM_SERIAL.println('\"');
	if (!sendCommand(0))   return 4;

	strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[25])));
	SIM_SERIAL.print(bufcom);                                      //SIM_SERIAL.print("AT+CGDCONT=1,\"IP\",\"");
	SIM_SERIAL.print(cont);
	SIM_SERIAL.println('\"');
	if (!sendCommand(0))   return 4;

	strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[26])));
	sendCommand(bufcom, 10000);                                    //sendCommand("AT+SAPBR=1,1", 10000);                     // ��������� GPRS �����
	strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[27])));
	sendCommand(bufcom, 10000);                                    //sendCommand("AT+SAPBR=2,1", 10000);                     // ���������� IP �����

	strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[28])));
	sendCommand(bufcom);                                           //sendCommand("AT+CMGF=1");                               // sets the SMS mode to text
	strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[29])));
	sendCommand(bufcom);                                           //sendCommand("AT+CPMS=\"SM\",\"SM\",\"SM\"");            // selects the memory

	if (!success)   return 5;
	return 0;
}

void CGPRS_SIM800::cleanStr(String & str) 
{
  str.replace("OK", "");
  str.replace("\"", "");
  str.replace("\n", "");
  str.replace("\r", "");
  str.trim();
}

bool CGPRS_SIM800::getIMEI()
{

	strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[30])));
    sendCommand(bufcom);                                          //sendCommand("AT+GSN");
    delay(1000);

  //if (sendCommand("AT+GSN", "OK\r", "ERROR\r") == 1) 
 // {
 //   char *p = strstr(buffer, "\r");          //������� strstr() ���������� ��������� �� ������ ��������� � ������, 
	                                         //�� ������� ��������� str1, ������, ��������� str2 (�������� ����������� ������� ������).
	                                         //���� ���������� �� ����������, ������������ NULL.
     // if (p) 
	 // {
   //     p += 2;
		
		 ////  char *s = strstr(buffer, "OK");  // ���� ���������� ��������
   //      char *s = strchr(p, '\r');       // ������� strchr() ���������� ��������� �� ������ ��������� ������� ch � ������, 
			//							    //�� ������� ��������� str. ���� ������ ch �� ������,
			//							    //������������ NULL. 
   //      if (s) *s = 0;   strcpy(buffer, p);
	     return true;
      //}
 // }
 // return false;
}

bool CGPRS_SIM800::getOperatorName()
{
  // display operator name
	strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[31])));
	strcpy_P(bufcom1, (char*)pgm_read_word(&(table_message[33])));

  if (sendCommand(bufcom, "OK\r", bufcom1) == 1)   // if (sendCommand("AT+COPS?", "OK\r", "ERROR\r") == 1) 
  {
      char *p = strstr(buffer, ",\"");
      if (p) 
	  {
          p += 2;
          char *s = strchr(p, '\"');
          if (s) *s = 0;
          strcpy(buffer, p);
          return true;
      }
  }
  return false;
}

bool CGPRS_SIM800::checkSMSU()
{
 if (SIM_SERIAL.available())             //���� ������ �� GSM ������
 {          
    delay(100);                          //������, ����� ������ ������ ������� � ���� ������� ������ ��� ����� �������
    while (SIM_SERIAL.available())       //���� ������ �� GSM ������
	{    
      ch = SIM_SERIAL.read();
      val += char(ch);                   //��������� ������� ������ � ���������� val
      delay(10);
    }
    return true;
  }
  return false; 
}


int CGPRS_SIM800::getSignalQuality()
{
  strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[8])));
  sendCommand(bufcom);                             // sendCommand("AT+CSQ");
  char *p = strstr(buffer, "CSQ:");
  if (p) {
    int n = atoi(p+5);
    if (n == 99 || n == -1) return 0;
    return n ;
  } else {
   return 0; 
  }
}

bool CGPRS_SIM800::getLocation(GSM_LOCATION* loc)
{
  strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[34])));
  if (sendCommand(bufcom, 10000)) do         // if (sendCommand("AT+CIPGSMLOC=1,1", 10000)) do 
  {
    char *p;
    if (!(p = strchr(buffer, ':'))) break;
    if (!(p = strchr(p, ','))) break;
    loc->lon = atof(++p);
    if (!(p = strchr(p, ','))) break;
    loc->lat = atof(++p);
    if (!(p = strchr(p, ','))) break;
    loc->year = atoi(++p) - 2000;
    if (!(p = strchr(p, '/'))) break;
    loc->month = atoi(++p);
    if (!(p = strchr(p, '/'))) break;
    loc->day = atoi(++p);
    if (!(p = strchr(p, ','))) break;
    loc->hour = atoi(++p);
    if (!(p = strchr(p, ':'))) break;
    loc->minute = atoi(++p);
    if (!(p = strchr(p, ':'))) break;
    loc->second = atoi(++p);
    return true;
  } while(0);
  return false;
}

void CGPRS_SIM800::httpUninit()
{
  strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[35])));
  sendCommand(bufcom);          // sendCommand("AT+HTTPTERM");
}

bool CGPRS_SIM800::httpInit()
{
	strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[36])));
	strcpy_P(bufcom1, (char*)pgm_read_word(&(table_message[37])));
    if  (!sendCommand(bufcom, 10000) || !sendCommand(bufcom1, 5000))  //if  (!sendCommand("AT+HTTPINIT", 10000) || !sendCommand("AT+HTTPPARA=\"CID\",1", 5000)) 
	{
	httpState = HTTP_DISABLED;
	return false;
	}
	httpState = HTTP_READY;
	return true;
}
bool CGPRS_SIM800::httpConnect(const char* url, const char* args)
{
  	strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[38])));
    SIM_SERIAL.print(bufcom);                    //SIM_SERIAL.print("AT+HTTPPARA=\"URL\",\"");
    SIM_SERIAL.print(url);
    if (args) 
	{
        SIM_SERIAL.print('?');
        SIM_SERIAL.print(args);
    }

    SIM_SERIAL.println('\"');
    if (sendCommand(0))
    {
        // Starts GET action
		strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[39])));
        SIM_SERIAL.println(bufcom);                         //SIM_SERIAL.println("AT+HTTPACTION=0");
        httpState = HTTP_CONNECTING;
        m_bytesRecv = 0;
        m_checkTimer = millis();
    }
	else 
	{
        httpState = HTTP_ERROR;
    }
    return false;
}

bool CGPRS_SIM800::httpConnectStr(const char* url, String args)
{

 	strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[40])));
    SIM_SERIAL.print(bufcom);    //SIM_SERIAL.print("AT+HTTPPARA=\"URL\",\"");
    SIM_SERIAL.print(url);
    if (args) 
	{
        SIM_SERIAL.print('?');
        SIM_SERIAL.print(args);
    }

    SIM_SERIAL.println('\"');
	delay(500);
    if (sendCommand(0))
    {
        strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[41])));
        SIM_SERIAL.println(bufcom);              //SIM_SERIAL.println("AT+HTTPACTION=0");
        httpState = HTTP_CONNECTING;
        m_bytesRecv = 0;
        m_checkTimer = millis();
    }
	else 
	{
        httpState = HTTP_ERROR;
    }
    return false;
}

// check if HTTP connection is established
// return 0 for in progress, 1 for success, 2 for error
// ���������, ���� ���������� HTTP �����������
// ���������� 0 ��� ��������� �, 1 ��� ������, 2 ��� ������

byte CGPRS_SIM800::httpIsConnected()
{
	strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[42])));
	strcpy_P(bufcom1, (char*)pgm_read_word(&(table_message[43])));
    byte ret = checkbuffer(bufcom,bufcom1, 10000);           // byte ret = checkbuffer("0,200", "0,60", 10000);
    if (ret >= 2) 
	{
        httpState = HTTP_ERROR;
        return -1;
    }
    return ret;
}
void CGPRS_SIM800::httpRead()
{
	strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[44])));
	SIM_SERIAL.println(bufcom);     //SIM_SERIAL.println("AT+HTTPREAD");
    httpState = HTTP_READING;
    m_bytesRecv = 0;
    m_checkTimer = millis();
}
// check if HTTP connection is established
// return 0 for in progress, -1 for error, number of http payload bytes on success
// ���������, ���� ���������� HTTP �����������
// ���������� �������� 0 ��� ������������, -1 ��� ������, ���������� ������ �������� �������� HTTP �� �����

int CGPRS_SIM800::httpIsRead()
{
	strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[45])));
	strcpy_P(bufcom1, (char*)pgm_read_word(&(table_message[46])));
    byte ret = checkbuffer(bufcom, bufcom1, 10000) == 1;//byte ret = checkbuffer("+HTTPREAD: ", "Error", 10000) == 1;
    if (ret == 1) 
	{
        m_bytesRecv = 0;
        // read the rest data
        sendCommand(0, 100, "\r\n");
        int bytes = atoi(buffer);
        sendCommand(0);
        bytes = min(bytes, sizeof(buffer) - 1);
        buffer[bytes] = 0;
        return bytes;
    } else if (ret >= 2) 
	{
        httpState = HTTP_ERROR;
        return -1;
    }
    return 0;
}
byte CGPRS_SIM800::sendCommand(const char* cmd, unsigned int timeout, const char* expected)
{
  if (cmd) 
  {
    purgeSerial();
#ifdef DEBUG
    DEBUG.print('>');
    DEBUG.println(cmd);
#endif
    SIM_SERIAL.println(cmd);
  }
  uint32_t t = millis();
  byte n = 0;
  do {
    if (SIM_SERIAL.available()) 
	{
      char c = SIM_SERIAL.read();
      if (n >= sizeof(buffer) - 1) 
	  {
        // buffer full, discard first half
        n = sizeof(buffer) / 2 - 1;
        memcpy(buffer, buffer + sizeof(buffer) / 2, n);
      }
      buffer[n++] = c;
      buffer[n] = 0;
	  strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[32])));
    if (strstr(buffer, expected ? expected : bufcom))    // if (strstr(buffer, expected ? expected : "OK\r")) 
	  {
#ifdef DEBUG
       DEBUG.print("[1]");
       DEBUG.println(buffer);
#endif
      return n;
      }
    }
  } while (millis() - t < timeout);
#ifdef DEBUG
   DEBUG.print("[0]");
   DEBUG.println(buffer);
#endif
  return 0;
}
byte CGPRS_SIM800::sendCommand(const char* cmd, const char* expected1, const char* expected2, unsigned int timeout)
{
  if (cmd) 
  {
	purgeSerial();
	#ifdef DEBUG
	    DEBUG.print('>');
	    DEBUG.println(cmd);
	#endif
	SIM_SERIAL.println(cmd);
  }
  uint32_t t = millis();
  byte n = 0;
  do {
    if (SIM_SERIAL.available()) 
	{
      char c = SIM_SERIAL.read();
      if (n >= sizeof(buffer) - 1) 
	  {
        // buffer full, discard first half
        n = sizeof(buffer) / 2 - 1;
        memcpy(buffer, buffer + sizeof(buffer) / 2, n);
      }
      buffer[n++] = c;
      buffer[n] = 0;
      if (strstr(buffer, expected1)) {
#ifdef DEBUG
       DEBUG.print("[1]");
       DEBUG.println(buffer);
#endif
       return 1;
      }
      if (strstr(buffer, expected2)) {
#ifdef DEBUG
       DEBUG.print("[2]");
       DEBUG.println(buffer);
#endif
       return 2;
      }
    }
  } while (millis() - t < timeout);
#if DEBUG
   DEBUG.print("[0]");
   DEBUG.println(buffer);
#endif
  return 0;
}

byte CGPRS_SIM800::checkbuffer(const char* expected1, const char* expected2, unsigned int timeout)
{
    while (SIM_SERIAL.available()) 
	{
        char c = SIM_SERIAL.read();
        if (m_bytesRecv >= sizeof(buffer) - 1) 
		{
            // buffer full, discard first half ����� ��������, ��������� ������ ��������
            m_bytesRecv = sizeof(buffer) / 2 - 1;
            memcpy(buffer, buffer + sizeof(buffer) / 2, m_bytesRecv);
        }
        buffer[m_bytesRecv++] = c;
        buffer[m_bytesRecv] = 0;
        if (strstr(buffer, expected1)) 
		{
            return 1;
        }
        if (expected2 && strstr(buffer, expected2)) 
		{
            return 2;
        }
    }
    return (millis() - m_checkTimer < timeout) ? 0 : 3;
}

void CGPRS_SIM800::purgeSerial()
{
  while (SIM_SERIAL.available()) SIM_SERIAL.read();
}
bool CGPRS_SIM800::available()
{
    return SIM_SERIAL.available(); 
}
