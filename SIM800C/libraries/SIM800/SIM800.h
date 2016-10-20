/*************************************************************************
* SIM800 GPRS/HTTP Library
* Distributed under GPL v2.0
* Written by Stanley Huang <stanleyhuangyc@gmail.com>
* For more information, please visit http://arduinodev.com
*************************************************************************/

#include <Arduino.h>
#include <avr/pgmspace.h>

// change this to the pin connect with SIM800 reset pin
#define PIN_TX           7                             // Подключить  к выводу 7 сигнал RX модуля GPRS
#define PIN_RX           8                             // Подключить  к выводу 8 сигнал TX модуля GPRS

// change this to the serial UART which SIM800 is attached to
//#define SIM_SERIAL Serial1

// define DEBUG to one serial UART to enable debug information output
//#define DEBUG Serial

typedef enum {
    HTTP_DISABLED = 0,
    HTTP_READY,
    HTTP_CONNECTING,
    HTTP_READING,
    HTTP_ERROR,
} HTTP_STATES;

typedef struct {
  float lat;
  float lon;
  uint8_t year; /* year past 2000, e.g. 15 for 2015 */
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
} GSM_LOCATION;

class CGPRS_SIM800 {
public:
    CGPRS_SIM800():httpState(HTTP_DISABLED) {}
    // initialize the module
    bool init(int PWR_On,int SIM800_RESET_PIN,int LED13);
    // setup network
    byte setup();
	// byte setup(const char* apn, const char* user, const char* pwd);
    // get network operator name
    bool getOperatorName();

	bool getIMEI();
    // check for incoming SMS
    bool checkSMS();
	bool checkSMSU();
    // get signal quality level (in dB)
    int getSignalQuality();
    // get GSM location and network time
    bool getLocation(GSM_LOCATION* loc);
    // initialize HTTP connection
    bool httpInit();
    // terminate HTTP connection
    void httpUninit();
    // connect to HTTP server
    bool httpConnect(const char* url, const char* args = 0);
	bool httpConnectStr(const char* url, String args = "");

    // check if HTTP connection is established
    // return 0 for in progress, 1 for success, 2 for error
    byte httpIsConnected();
    // read data from HTTP connection
    void httpRead();
    // check if HTTP connection is established
    // return 0 for in progress, -1 for error, bytes of http payload on success
    int httpIsRead();
    // send AT command and check for expected response
    byte sendCommand(const char* cmd, unsigned int timeout = 2000, const char* expected = 0);
    // send AT command and check for two possible responses
    byte sendCommand(const char* cmd, const char* expected1, const char* expected2, unsigned int timeout = 2000);
    // toggle low-power mode
    bool sleep(bool enabled)
    {
      return sendCommand(enabled ? "AT+CFUN=0" : "AT+CFUN=1");
    }
    // check if there is available serial data
    bool available();
	void cleanStr(String & str);

    //{
    // // return SIM_SERIAL.available(); 
    //}
    char buffer[128];
    byte httpState;
	String val = "";
	//String telefon = "";
private:
    byte checkbuffer(const char* expected1, const char* expected2 = 0, unsigned int timeout = 2000);
    void purgeSerial();
    byte m_bytesRecv;
    uint32_t m_checkTimer;
	String apn  = "";
    String user = "";
    String pwd  = "";
    String cont = "";
	int _PWR_On           ;                            // Включение питания модуля SIM800
    int _SIM800_RESET_PIN ;                            // Сброс модуля SIM800
    int _LED13            ;                             // Индикация светодиодом
};

