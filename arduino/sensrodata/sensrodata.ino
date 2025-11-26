#include <SoftwareSerial.h>
#define OK "OK"

#define RX 2
#define TX 3

// Globals
unsigned long current_time = 0;
String gsm_response = "";
SoftwareSerial gsm(RX, TX);

String lastGNSS = "";

void setup() {
  Serial.begin(9600);
  initializeGSM();
  openMQTTConnection();
}

void loop() {
  saveLocation();
  syncMQTT();
  delay(1000);
}


void syncMQTT() {

  String r = sendAndGetResponse(F("AT+QMTPUB=0,0,0,0,\"SUNDAY\""), 1000);
  AT(lastGNSS+ (char)0x1A, OK);
}


String sendAndGetResponse(String cmd, int timeout) {
  Serial.println(cmd);
  current_time = millis();
  gsm_response = "";
  int index = -1;
  int network_status = 0;

  gsm.println(cmd);
  while (true)
  {
    if (gsm.available())
    {
      gsm_response = gsm.readString();
      gsm_response.trim();
      Serial.println(gsm_response);
      return gsm_response;
    }

    if (millis() - current_time >= timeout)
    {
      Serial.println(F("ERROR: GSM response timeout."));
      Serial.println(F("       Press RESET."));
      return "";
    }
  }

  return gsm_response;
}


void saveLocation() {
  String resp = sendAndGetResponse(F("AT+QGPSLOC=1"), 5000);
  String parsedString = parseLocationData(resp);
  Serial.print("->");
  Serial.println(parsedString);
  lastGNSS = parsedString;
}

String parseLocationData(String resp) {
  int sIndx = resp.indexOf("\n\n") + 12;
  String nmea = resp.substring(sIndx, sIndx+76);
  return nmea;
}

void initializeGSM(void)
{
  gsm.begin(115200);
  delay(5000);
  gsm.println(F("AT+IPR=9600"));
  delay(5000);
  gsm.end();
  gsm.begin(9600);
  
  while (!getResponse("AT", OK, 5000))
  {
    // initial 1st time dealy by a module.
    delay(200);
  }
  
  if (connectGSM())
  {
    Serial.println(F("GSM Connected!"));
  }

}

bool getResponse(String cmd, String response, int timeout)
{
  Serial.println(cmd);
  current_time = millis();
  gsm_response = "";
  int index = -1;
  int network_status = 0;

  gsm.println(cmd);
  while (true)
  {
    if (gsm.available())
    {
        gsm_response = gsm.readString();
        gsm_response.trim();
        Serial.println(gsm_response);
        index = gsm_response.indexOf(response);
        if (cmd == "AT+CREG?")
        {
          network_status = gsm_response.substring(gsm_response.indexOf(',') + 1, gsm_response.indexOf(',') + 2).toInt();
          if (network_status == 1 || network_status == 5)
          {
            Serial.println(F("GSM Network registered!"));
            return true;
          }
          else
          {
            Serial.println(F("ERROR: Failed to register network."));
            Serial.println(F("       Press RESET."));
            return false;
          }
        }
        else
        {
          if (index >= 0)
          {
              return true;
          }
        }
      }
      if (millis() - current_time >= timeout)
      {
        Serial.println(F("ERROR: GSM response timeout."));
        Serial.println(F("       Press RESET."));
        return false;
      }
  }
}

void AT(String cmd, String test) {
  while (!getResponse(cmd, test, 120000));
}

bool connectGSM(void)
{
  while (!getResponse(F("AT"), OK, 1000))
      ;
  //while (!getResponse(F("AT+CFUN=1,1"), OK, 10000))
  //    ;
  //delay(10000);  
  while (!getResponse(F("ATE0"), OK, 1000))
      ;
  while (!getResponse(F("AT+CLIP=1"), OK, 1000))
      ;
  while (!getResponse(F("AT+CMEE=2"), OK, 1000))
      ;
  while (!getResponse(F("AT+CIMI"), OK, 1000))
      ;
  while (!getResponse(F("AT+CSQ"), OK, 1000))
      ;
  while (!getResponse(F("AT+CVHU=0"), OK, 1000))
      ;  
  while (!getResponse(F("AT+CTZU=1"), OK, 1000))
      ;
  while (!getResponse(F("AT+CMGF=1"), OK, 1000))
      ;
  while (!getResponse(F("AT+CSQ"), OK, 1000))
      ;
  while (!getResponse(F("AT+CREG?"), OK, 1000))
      ;
  while (!getResponse(F("AT+CGREG?"), OK, 1000))
      ;
  while (!getResponse(F("AT+CMGD=1,4"), OK, 1000))
      ;

//AT(F("AT+QICSGP=1,\"airtelgprs.com\",\"\",\"\",0"), OK);
//AT(F("AT+QICSGP=1,1,\"airtelgprs.com\",\"\",\"\",1"), OK);
//AT(F("AT+QICSGP=1,\"airtelgprs.com\",\"\",\"\",1"), OK);
  AT(F("AT+CGATT=1"), OK);
  AT(F("AT+CGACT=1,1"), OK);
//AT(F("AT+QIACT=1"), OK);
  AT(F("AT+QIACT=?"), OK);

  AT(F("AT+QMTCFG=\"pdpcid\",0,1"), OK);

  // SSL Block
  AT(F("AT+QMTCFG=\"ssl\",0,1,0"), OK);
  AT(F("AT+QSSLCFG=\"sslversion\",0,4"), OK);        // TLS 1.2
  AT(F("AT+QSSLCFG=\"ciphersuite\",0,0XFFFF"), OK);  // allow all
  AT(F("AT+QSSLCFG=\"seclevel\",0,0"), OK);          // verify server cert
  AT(F("AT+QSSLCFG=\"ignorelocaltime\",0,1"), OK);   // ignore RTC for validity
  AT(F("AT+QSSLCFG=\"sni\",0,1"), OK);               // enable SNI
  //AT(F("AT+QSSLCFG=\"cacert\",0,\"UFS:ca.pem\""), OK); // certificate config
  AT(F("AT+QMTCFG=\"session\",0,1"), OK);
  AT(F("AT+QMTCFG=\"version\",0,4"), OK);            // MQTT 3.1.1
  AT(F("AT+QMTCFG=\"keepalive\",0,120"), OK);        // 120s heartbeat


  // GNSS
  while (!getResponse(F("AT+QGPSCFG=\"autogps\",1"), OK, 1000))
      ;
  while (!getResponse(F("AT+QGPSCFG=\"gnssconfig\",7"), OK, 1000))
      ;
  while (!getResponse(F("AT+QGPSGNMEA=\"GGA\""), OK, 1000))
      ;
  while (!getResponse(F("AT+QGPSGNMEA=\"GSV\""), OK, 1000))
      ;
  while (!getResponse(F("AT+QGPSGNMEA=\"RMC\""), OK, 1000))
      ;
  while (!getResponse(F("AT+QGPSGNMEA=\"VTG\""), OK, 1000))
      ;
  while (!getResponse(F("AT+QGPSGNMEA=\"GSA\""), OK, 1000))
      ;
  while (!getResponse(F("AT+QGPSCFG=\"nmeasrc\",1"), OK, 1000))
      ;
  // Configure URC port to UART1
  if (getResponse(F("AT+QURCCFG=\"urcport\",\"uart1\""), OK, 1000)) 
  {
    Serial.println(F("URC port configured to UART1"));
  }
  else 
  {
    Serial.println(F("Failed to configure URC port"));
  }

  return true;
}

void openMQTTConnection() {
  // check for +QMTOPEN: 0,0
  while (!getResponse(
             F("AT+QMTOPEN=0,\"6a99624836bd49aeaff2cd08268824d0.s1.eu.hivemq.cloud\",8883"),
             F("+QMTOPEN: 0,0"),
             120000)) {
    Serial.println(F("Retrying QMTOPEN..."));
  }

  while (!getResponse(
             F("AT+QMTCONN=0,\"client001\",\"bksan\",\"P@ssw0rd\""),
             F("+QMTCONN: 0,0,0"),
             120000)) {
    Serial.println(F("Retrying QMTCONN..."));
  }

  Serial.println(F("MQTT connected to HiveMQ Cloud!"));
}
