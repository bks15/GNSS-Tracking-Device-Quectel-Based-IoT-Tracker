#include <SoftwareSerial.h>
#define OK "OK"
/// Macros - GSM Related
// #define AT "AT"
//// RX Pin
#define RX 2
#define TX 3
#define PHONE_NUMBER "+91988888887"


// Globals
unsigned long current_time = 0;

/// Globals - GSM Related
String gsm_response = "";
String phone_number = PHONE_NUMBER;
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
  /*
    Publish Messages
      AT+QMTPUBEX=<client_idx>,<msgid>,<qos>,<retain>,<topic>,<length>

  */
  String r = sendAndGetResponse(F("AT+QMTPUB=0,0,0,0,\"test\""), 1000);
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
  
  //sendSMS(phone_number, "Hello from EC200U GSM!");
  // readSMS();
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
  

  AT(F("AT+QICSGP=1,1,\"airtelgprs.com\",\"\",\"\",1"), OK);
  /*
      0 Detached, 1 Attached
  */
  AT(F("AT+CGATT=1"), OK);
  /*
    Activate or Deactivate PDP Context
  */
  AT(F("AT+CGACT=1,1"), OK);
  /*
    Activate a specified PDP context
  */
  AT(F("AT+QIACT=1"), OK);
  /*
    Configure the PDP to be used by the MQTT client
  */
  AT(F("AT+QMTCFG=\"pdpcid\",0,1"), OK);
  /*
    Configure the MQTT SSL mode and SSL context index
  */
  AT(F("AT+QMTCFG=\"ssl\",0,0,0"), OK);
  /*
    Configure the SSL version for the specified SSL context:
  */
  AT(F("AT+QSSLCFG=\"sslversion\",0,4"), OK);
  /*
    Configure the SSL cipher suites for the specified SSL context:
  */
  AT(F("AT+QSSLCFG=\"ciphersuite\",0,0XFFFF"), OK);
  /*
    Configure the authentication mode for the specified SSL context:
  */
  AT(F("AT+QSSLCFG=\"seclevel\",0,2"), OK);
  /*
    Configure whether to ignore certificate validity period check for the specified SSL context:
  */
  AT(F("AT+QSSLCFG=\"ignorelocaltime\",0,1"), OK);
  /*
    Configure Server Name Indication feature for the specified SSL context:
  */
  AT(F("AT+QSSLCFG=\"sni\",0,1"), OK);
  /*
    Configure the session type
  */
  AT(F("AT+QMTCFG=\"session\",0,1"), OK);
  /*
    Configure the MQTT protocol version
  */
  AT(F("AT+QMTCFG=\"version\",0,4"), OK);
  /*
    Configure the version t 0,4
  */
  AT(F("AT+QMTCFG=\"keepalive\",0,120"), OK);
  /*
    heartbeat of 120 sec
  */
  //AT(F("AT+QSSLCFG=\"UFS:ca.pem\""), OK);
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
  /*
    Open a Network for MQTT client, connect using MQTT broker like HiveMQ, URL, Login & password
  */
  AT(F("AT+QMTOPEN=0,\"18.100.124.119\",1883"), OK);
  AT(F("AT+QMTCONN=0,\"client001\",\"test\",\"test\""), OK);  
}

void sendSMS(String number, String sms)
{
    while (!getResponse(F("AT+CMGF=1"), OK, 1000))
        ;
    while (!getResponse("AT+CMGS=\"" + number + "\"\r", ">", 1000))
        ;
    gsm.print(sms);
    delay(1000);
    gsm.println((char)26);
}

bool readSMS(void)
{
    while (!getResponse(F("AT+CNMI=2,2,0,0,0"), OK, 1000))
        ;
    gsm_response = "";
    while (true)
    {
        if (gsm.available())
        {
            gsm_response = gsm.readString();
            gsm_response.trim();
            Serial.println(gsm_response);
        }
    }
}

/****AT Commands reference Parameters****

<client_idx> Integer type. MQTT client identifier. Range: 0–5.

<SSL_enable> Integer type. Configure the MQTT SSL mode.
    0 Use normal TCP connection for MQTT
    1 Use SSL TCP secure connection for MQTT

<SSL_ctx_idx> Integer type. SSL context index. Range: 0–5.

<SSL_ctxID> Integer type. SSL context ID. Range: 0–5.

<SSL_version> Integer type. SSL version.
  0 SSL 3.0
  1 TLS 1.0
  2 TLS 1.1
  3 TLS 1.2
  4 All

<cipher_suites> Numeric type in HEX format. SSL cipher suites.
  0X0035 TLS_RSA_WITH_AES_256_CBC_SHA
  0X002F TLS_RSA_WITH_AES_128_CBC_SHA
  0X0005 TLS_RSA_WITH_RC4_128_SHA
  0X0004 TLS_RSA_WITH_RC4_128_MD5
  0X000A TLS_RSA_WITH_3DES_EDE_CBC_SHA
  0X003D TLS_RSA_WITH_AES_256_CBC_SHA256
  0XC002 TLS_ECDH_ECDSA_WITH_RC4_128_SHA
  0XC003 TLS_ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA
  0XC004 TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA
  0XC005 TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA
  0XC007 TLS_ECDHE_ECDSA_WITH_RC4_128_SHA
  0XC008 TLS_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA
  0XC009 TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA
  0XC00A TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA
  0XC011 TLS_ECDHE_RSA_WITH_RC4_128_SHA
  0XC012 TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA
  0XC013 TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA
  0XC014 TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA
  0xC00C TLS_ECDH_RSA_WITH_RC4_128_SHA
  0XC00D TLS_ECDH_RSA_WITH_3DES_EDE_CBC_SHA
  0XC00E TLS_ECDH_RSA_WITH_AES_128_CBC_SHA
  0XC00F TLS_ECDH_RSA_WITH_AES_256_CBC_SHA
  0XC023 TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256
  0xC024 TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384
  0xC025 TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256
  0xC026 TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA384
  0XC027 TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256
  0XC028 TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384
  0xC029 TLS_ECDH_RSA_WITH_AES_128_CBC_SHA256
  0XC02A TLS_ECDH_RSA_WITH_AES_256_CBC_SHA384
  0XC02F TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256
  0xC030 MBEDTLS_TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384
  0XFFFF Support all cipher suites

<seclevel> Integer type. The authentication mode.
  0 No authentication
  1 Perform server authentication
  2 Perform server and client authentication if requested by the remote
  server

<ignore_ltime> Integer type. Whether or not to ignore validity period check of certificate.
  0 Not to ignore
  1 Ignore

<SNI> Integer type. Disable/enable Server Name Indication feature.
  0 Disable
  1 Enable

<clean_session> Integer type. Configure the session type.
  0 The server must store the subscriptions of the client after it disconnects.
    (effective only when the server supports the operation of storing session information)
  1 The server must discard any previously maintained information about the client and treat the connection as "clean".

<vsn> Integer type. MQTT protocol version.
  3 MQTT protocol v3.1
  4 MQTT protocol v3.1.1

<contextID> Integer type. The context ID. Range: 1–7.

<state> Integer type. Indicate the state of PDP context activation.
  0 Deactivated
  1 Activated

<cid> Integer type. Specify a particular PDP context definition.

<PDP_type> String type. Packet data protocol type, a string parameter which specifies the type of packet data protocol.
  "IP" IPv4. Internet protocol (IETF STD 5)
  "PPP" Point to Point Protocol (IETF STD 51)
  "IPV6" Internet Protocol, version 6 (see RFC 2460)
  "IPV4V6" Virtual introduced to handle dual IP stack UE capability (see RFC 2460)

<APN> String type. Access point name, a string parameter that is a logical name used to select the GGSN or the external packet data network. If the value is null or omitted, then the subscription value will be requested.

<PDP_addr> String type. Identifies the MT in the address space applicable to the PDP. If the value is null or omitted, then a value may be provided by the TE during the PDP startup procedure or, failing that, a dynamic address will be requested. The allocated address may be read with AT+CGPADDR.

<data_comp> Integer type. Controls PDP data compression (applicable for SNDCP only) (refer to 3GPP TS 44.065).
  0* Off
  1 On (Manufacturer preferred compression)
  2 V.42bis

<head_comp> Integer type. Control PDP header compression (refer to 3GPP TS 44.065 and 3GPP TS 25.323).
  0* Off
  1 On
  2 RFC1144
  3 RFC2507
  4 RFC3095

<IPv4_addr_alloc> Integer type. Control how the MT/TA requests to get the IPv4 address information.
  0* IPv4 address allocation through NAS signaling
  1 IPv4 address allocated through DHCP

<request_type> Integer type. Indicate the type of PDP context activation request for the PDP context.
  0* PDP context is for new PDP context establishment or for handover from a non-3GPP access network (how the MT decides whether the PDP context is for new PDP context establishment or for handover is implementation specific)
  1 PDP context is for emergency bearer services

<keep_alive_time> Integer type. Keep-alive time. Range: 0–3600. Default value: 120. Unit: s. It defines the maximum time interval between messages received from a client. If the server does not receive a message (Interactive data or keep-alive package) from the client within 1.5 times the keep-alive time period, it disconnects the client as if the client has sent a DISCONNECT message.
0 The client is not disconnected

<host_name> String type. The address of the server. It could be an IP address or a domain name. The maximum size is 100 bytes.

<port> Integer type. The port of the server. Range: 1–65535.

<filename> String type. Name of the file to be deleted. <filename> can include the file path (that is the directory name) and file name. The maximum length of the file path is 58 bytes, and the maximum length of the file name is 63 bytes.
  "*" Delete all the files in UFS (not delete the directory)
  "<filename>" Delete the specified file <filename> in UFS
  "UFS:*" Delete all the files in UFS (not delete the directory)
  "UFS:<filename>" Delete the specified file <filename> in UFS
  "SFS:*" Delete all the files in SFS (not delete the directory)）
  "SFS:<filename>" Delete the specified file <filename> in SFS
  "EFS:*" Delete all the files in EFS (not delete the directory)
  "EFS:<filename>" Delete the specified file <filename> in EFS
  "SD:*" Delete all the files in SD card (not delete the directory)
  "SD:<filename>" Delete the specified file <filename> in SD card

<clientID> String type. The client identifier string.

<username> String type. User name of the client. It can be used for authentication.

<password> String type. Password corresponding to the user name of the client. It can be used for authentication.

<msgid> Integer type. Message identifier of packet. Range: 0–65535. It is 0 only when <qos>=0.

<qos> Integer type. The QoS level at which the client wants to publish the messages.
  0 At most once
  1 At least once
  2 Exactly once

<retain> Integer type. Whether or not the server will retain the message after it has been delivered to the current subscribers.
  0 Not retain
  1 Retain

<topic> String type. Topic that needs to be published.

<length> Integer type. Length of message to be published.
*******************************/
