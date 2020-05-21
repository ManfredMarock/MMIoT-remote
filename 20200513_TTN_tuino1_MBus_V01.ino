/* ___      ___    ___      ___    _______      ______    _______
  |   \    /   |  |   \    /   |  |__   __|    /  ___  \  |__   __|
  | |\ \  / /| |  | |\ \  / /| |     | |      |  /   \  |    | |
  | | \ \/ / | |  | | \ \/ / | |     | |      |  |    | |    | |
  | |  \__/  | |  | |  \__/  | |   __| |__    |  \__ /  /    | |
  |_|        |_|  |_|        |_|  |_______|    \_______/     |_|
  (c) 2019 MMsIoT
     Author: Manfred Marock
     Brief: Tuino1 Maker's Kit IoT Demo
     Version: 1.0
*/

#include "gmx_lr.h"
#include "Regexp.h"
#include "SeeedOLED.h"
#include "display_utils.h"

#include "MBusProzess.h"

#define MESSINTERVAL 300  //x * ~2s
#define LORA
#define SHOWDATAINTERVALL 4000

String haus1[8] = {"W11 W=", "    H=", "W12 W=", "    H=", "W13 W=", "    H=", "W14 W=", "    H="};
String haus2[8] = {"W21 W=", "    H=", "W22 W=", "    H=", "W23 W=", "    H=", "W24 W=", "    H="};
String haus3a[8] = {"W3 W=", "    H=", "W32 W=", "    H=", "W33 W=", "    H=", "W34 W=", "    H="};
String haus3b[8] = {"W32 W=", "    H=", "W33 W=", "    H=", "W34 W=", "    H=", "W35 W=", "    H="};

word dataArray[30];
int timeCounts = -99;
int dataHaus1[8];
int dataHaus2[8];
int dataHaus3a[8];
int dataHaus3b[8];


int testCounter;                   
 
/****************************************************
* Prototyp
****************************************************/
void fillLoRaData(String dataName,  int dataIndex, int data, byte tx_buf[]);

#include <Wire.h>

// NFC
#include "M24SR.h"
#include "NdefMessage.h"
#include "NdefRecord.h"


// RegExp Engine for NFC parsing
MatchState nfc_ms;
char regexp_buf[512];


#define gpo_pin TUINO_NFC_INTERRUPT

bool writing_nfc = false;
int iGlobal = 0;

M24SR m24sr04(gpo_pin);
// END NFC

//long int timer_period_to_tx = MESSINTERVAL;   //15MIN
//long int timer_millis_lora_tx = 0;
int ledState = 0;

int buttonPin = D4;
int relayPin = D5;
int tempSensorPin = A0;
int testInc =0;
 
String oled_string;
int oledAnzeige = 0;

// Temperature Sensor constants
const int B = 4275;               // B value of the thermistor
const int R0 = 100000;          // R0 = 100k

#define TEMPERATURE_SAMPLING    (20)
#define THERMOSTAT_HYSTERESIS   (1.0f)

float current_temperature = 0.0f;
float temp_temperature = 0;
int temperature_counts = 0;
/*
int wasserWohnung_11;    int waermeWohnung_11;
int wasserWohnung_12;    int waermeWohnung_12;
int wasserWohnung_13;    int waermeWohnung_13;
int wasserWohnung_14;    int waermeWohnung_14;
  
int wasserWohnung_21;    int waermeWohnung_21;
int wasserWohnung_22;    int waermeWohnung_22;
int wasserWohnung_23;    int waermeWohnung_23;
int wasserWohnung_24;    int waermeWohnung_24;
  
int wasserWohnung_31;    int waermeWohnung_31;
int wasserWohnung_32;    int waermeWohnung_32;
int wasserWohnung_33;    int waermeWohnung_33;
int wasserWohnung_34;    int waermeWohnung_34;
int wasserWohnung_35;    int waermeWohnung_35;
*/
 // TTN Projekt meterreading_01
String Device_EUI_Galenburst = "78:AF:58:03:00:00:08:16";
String App_EUI_Galenburst = "70:B3:D5:7E:D0:02:D7:B5";
String App_EUI_act;
String App_Key_Galenburst = "86:DD:0D:F3:2E:65:BD:0D:21:0E:69:3D:87:60:53:D5";
String App_Key_act;

// LoRa RX interrupt
bool data_received = false;

/****************************************************
*
****************************************************/
void loraRx() {
  data_received = true;
}

/****************************************************
*
****************************************************/
float readTemp() {
  int a = analogRead(tempSensorPin );
  float R = 1023.0 / ((float)a) - 1.0;
  R = 100000.0 * R;

  float temperature = 1.0 / (log(R / 100000.0) / B + 1 / 298.15) - 273.15; //convert to temperature via datasheet ;
  return temperature;
}


/****************************************************
*
****************************************************/
void writeNFC() {
  char string[32];
  String tmpString;
  String nfc_data;

  gmxLR_getDevEui(tmpString);
  nfc_data = "DevEUI:" + tmpString;
  nfc_data = nfc_data + "\n\r";

  nfc_data = nfc_data + "Tempef:" + String(readTemp());
  nfc_data = nfc_data + "\n\r";
  nfc_data = nfc_data + "Text:" + oled_string;
  nfc_data = nfc_data + "\n\r";

  // WriteNFC data
  writing_nfc = true;
  NdefMessage message = NdefMessage();
  message.addTextRecord(nfc_data);
  m24sr04.writeNdefMessage(&message);
  delay(200);
  writing_nfc = false;
}

/****************************************************
*
****************************************************/
void waitButton() {
  while ( digitalRead(buttonPin) == 0 )
  {
  };

  delay(200);
}

void showMeterData(String house[], int dataArray[])
{
  char data0[20];
  char flat0[8];
  house[0].toCharArray(flat0, 9);
  sprintf(data0, "%s %d", flat0, dataArray[0]);

  char data1[20];
  char flat1[8];
  house[1].toCharArray(flat1, 9);
  sprintf(data1,"%s %d", flat1, dataArray[1]);

  char data2[20];
  char flat2[8];
  house[2].toCharArray(flat2, 9);
  sprintf(data2,"%s %d", flat2, dataArray[2]);

  char data3[20];
  char flat3[8];
  house[3].toCharArray(flat3, 9);
  sprintf(data3,"%s %d", flat3, dataArray[3]);

  char data4[20];
  char flat4[8];
  house[4].toCharArray(flat4, 9);
  sprintf(data4,"%s %d", flat4, dataArray[4]);

  char data5[20];
  char flat5[8];
  house[5].toCharArray(flat5, 9);
  sprintf(data5,"%s %d", flat5, dataArray[5]);

  char data6[20];
  char flat6[8];
  house[6].toCharArray(flat6, 9);
  sprintf(data6,"%s %d", flat6, dataArray[6]);

  char data7[20];
  char flat7[8];
  house[7].toCharArray(flat7, 9);
  sprintf(data7,"%s %d", flat7, dataArray[7]);
  
  showOLEDText(data0, data1, data2, data3, data4, data5, data6, data7);
}

/************************************************************************
* SETUP
************************************************************************/
void setup() {
  // put your setup code here, to run once:
  String DevEui;
  String AppEui;
  String AppKey;
  
  String loraClass,LoRaWANClass;
  char string[64];

  String adr, dcs, dxrate;

  byte join_status;
  int join_wait;

  Wire.begin();
  Serial.begin(2400, SERIAL_8E1);
  Serial.println("Starting");
  Serial.println("Datei:           MBusauslesung_Ziel <V1.1../_ 20200513_TTN_tuini1_MBus_V01");
  Serial.println("Prozessor:       Tuino1 - Gimasi Maker's Kit");
  Serial.println("Datum:           13.05. -->  09.04.2020");
  Serial.println("Autor:           Manfred Marock MMIoT");

  // Startupp NFC
  m24sr04._setup();

  // Init Oled
  // GMX-LR init pass callback function
  gmxLR_init(&loraRx);
 
  // Set AppEui and AppKey
  // Uncomment these if you want to change the default keys
    
  LoRaWANClass = "C";


#ifdef MULTIREGION
  gmxLR_getRegion(Region);
  if (NewLoraRegion.length() > 0 )
  {
    Region.trim();    
    Serial.println("**** UPDATING Region ****");
  
    if ( !Region.equals(NewLoraRegion) )
    {
       Serial.println("Setting Region:--"+NewLoraRegion);
       gmxLR_setRegion(NewLoraRegion);
       // reboot GMX_LR1
       gmxLR_Reset();
    }
    else
    {
       Serial.println("Region is already:"+Region);
    }
  }
#endif

  gmxLR_getAppEui(App_EUI_Galenburst);
  if (App_EUI_Galenburst.length() > 0 )
  {
    AppEui.trim();
        
    Serial.println("**** UPDATING AppEUI ****");
    if ( !AppEui.equals(App_EUI_Galenburst) )
    {
      Serial.println("Setting AppEui:--"+App_EUI_Galenburst);
      gmxLR_setAppEui(App_EUI_Galenburst);
      gmxLR_getAppEui(App_EUI_act);
      Serial.println("Set AppEui:    >>"+App_EUI_act);
    }
    else
    {
      Serial.println("AppEui is already:"+App_EUI_Galenburst);
    }
  }

  gmxLR_getAppKey(AppKey);
  if (App_Key_Galenburst.length() > 0 )
  {
    AppKey.trim();
      
    Serial.println("**** UPDATING AppKey ****");
    if ( !AppKey.equals(App_Key_Galenburst) )
    {
      Serial.println("Setting AppKey:--"+App_Key_Galenburst);
      gmxLR_setAppKey(App_Key_Galenburst);
    }
    else
    {
      Serial.println("AppKey is already:"+AppKey);
    }
  }

  // Disable Duty Cycle  ONLY FOR DEBUG!
  gmxLR_setDutyCycle("0");

  // Set LoRaWAN Class
  gmxLR_setClass(LoRaWANClass);
  
  // Show Splash Screen on OLED
   char data[100];
   sprintf(data, "W11= %d - %d ",11, 3);
   showOLEDText("MMIoT MBus", "", "Ziel V1.x", "","18.04.2020","","","");

 //  splashScreen();
  delay(2000);

  // Show LoRaWAN Params on OLED
  gmxLR_getDevEui(DevEui);
  gmxLR_getAppKey(AppKey);
  gmxLR_getAppEui(AppEui);
//  displayLoraWanParams(DevEui, AppEui, AppKey);

  delay(2000);
  
  // MMIot SeeedOled.clearDisplay();
  // MMIot SeeedOled.setTextXY(0, 0);
  // MMIot SeeedOled.putString("Joining...");

  Serial.println("Joining...");
  join_wait = 0;

#ifdef LORA
  while ((join_status = gmxLR_isNetworkJoined()) != LORA_NETWORK_JOINED) {
    if ( join_wait == 0 )
    {
      Serial.println("LoRaWAN Params:");
      Serial.println("DevEui:" + DevEui);
      gmxLR_getAppEui(AppEui);
      Serial.println("AppEui:" + AppEui);
      gmxLR_getAppKey(AppKey);
      Serial.println("AppKey:" + AppKey);
      gmxLR_getClass(loraClass);
      Serial.println("Class:" + loraClass);
      adr = String( gmxLR_getADR() );
      Serial.println("ADR:" + adr);
      dcs = String( gmxLR_getDutyCycle() );
      Serial.println("DCS:" + dcs);
      gmxLR_getRX2DataRate(dxrate);
      Serial.println("RX2 DataRate:" + dxrate);

      gmxLR_Join();
    } //if

    // MMIot SeeedOled.setTextXY(1, 0);
    sprintf(string, "Attempt: %d", join_wait);
    // MMIot SeeedOled.putString(string);

    join_wait++;

    if (!( join_wait % 100 )) {
      gmxLR_Reset();
      join_wait = 0;
    }

    delay(5000);
  }; //while
#endif
  // MMIot SeeedOled.setTextXY(2, 0);
  // MMIot SeeedOled.putString("Joined!");

//  writeNFC();
  delay(2000);
  // MMIot SeeedOled.clearDisplay();

  // Init Temperature
  current_temperature = readTemp() * 100;
  oled_string = "-----test----";

  MBusProzess::setupMBus();
}

/************************************************************************
* LOOP
************************************************************************/
void loop() {
  
  // put your main code here, to run repeatedly:

   
  String rx_data;
  long int delta_lora_tx;
  int temperature_int;
  char lora_data[104];
  char lora_data_W35[16];
  char lora_rx_string[256];
  byte tx_buf[120];
  byte rx_buf[128];
  int buf_len;

  String tx_data;
  String lora_string;
  int port;


   for(int i = 0; i < 8; i++){
     dataHaus1[i] = dataArray[i];  // Daten  für OLED abfüllen
     dataHaus2[i] = dataArray[i+8];
     dataHaus3a[i] = dataArray[i+16];
     dataHaus3b[i] = dataArray[i+18];
   }

   
  switch (oledAnzeige){ 
  case 0:showMeterData
    (haus1, dataHaus1);
    delay(SHOWDATAINTERVALL);
    oledAnzeige=1;
    break;
    
  case 1:
    showMeterData(haus2, dataHaus2);
    delay(SHOWDATAINTERVALL);
    oledAnzeige=2;
    break;
    
  case 2:  
    showMeterData(haus3a, dataHaus3a);
    delay(SHOWDATAINTERVALL);
    oledAnzeige=3;
    break;
    
  case 3:  
    showMeterData(haus3b, dataHaus3b);
    delay(SHOWDATAINTERVALL);
    oledAnzeige=0;
    break;  
    
  default:    Serial.println("def..");  break; 
  }

//--------------------------------------------------------------
 timeCounts++;
   
if ((timeCounts > MESSINTERVAL ) ||(timeCounts < 0))
 { 
   timeCounts =0;
   MBusProzess::getMBusData(dataArray);

//for (int i=1; i <= 27; i++)
//      dataArray[i] = i + testCounter++; 
      
// ************ Zähler Gampelutz 5   ************ 
   fillLoRaData("Wasserstand W11", 0, dataArray[0], tx_buf);
   fillLoRaData("Wärme W11", 1, dataArray[1], tx_buf);
   fillLoRaData("Wasserstand W12", 2, dataArray[2], tx_buf);
   fillLoRaData("Wärme W12", 3, dataArray[3], tx_buf);
   fillLoRaData("Wasserstand W13", 4, dataArray[4], tx_buf);
   fillLoRaData("Wärme W13", 5, dataArray[5], tx_buf);
   fillLoRaData("Wasserstand W14", 6, dataArray[6], tx_buf);
   fillLoRaData("Wärme W14", 7, dataArray[7], tx_buf);
   getLoRaData(tx_buf, lora_data,0);    

#ifdef LORA  
   writeNFC_Data(lora_data);
#endif   
   delay(10000);
   
//  ************ Zähler Gampelutz 7 ************  
   fillLoRaData("Wasserstand W21--", 8, dataArray[8], tx_buf);
   fillLoRaData("Wärme W21--", 9, dataArray[9], tx_buf);
   fillLoRaData("Wasserstand W22--", 10, dataArray[10], tx_buf);
   fillLoRaData("Wärme W22--", 11, dataArray[11], tx_buf);
   fillLoRaData("Wasserstand W23--", 12, dataArray[12], tx_buf);
   fillLoRaData("Wärme W23--", 13, dataArray[13], tx_buf);
   fillLoRaData("Wasserstand W24--", 14, dataArray[14], tx_buf);
   fillLoRaData("Wärme W24--", 15, dataArray[15], tx_buf);   
   getLoRaData(tx_buf, lora_data,8);  
#ifdef LORA  
   writeNFC_Data(lora_data);
#endif
   delay(10000);

// ************  Zähler Galenburst 23  ************ 
   fillLoRaData("Wasserstand W31--", 16, dataArray[16], tx_buf);
   fillLoRaData("Wärme W31--", 17, dataArray[17], tx_buf);
   fillLoRaData("Wasserstand W32--", 18, dataArray[18], tx_buf);
   fillLoRaData("Wärme W32--", 19, dataArray[19], tx_buf);
   
   fillLoRaData("Wasserstand W33--", 20, dataArray[20], tx_buf);
   fillLoRaData("Wärme W33--", 21, dataArray[21], tx_buf);
   fillLoRaData("Wasserstand W34--", 22, dataArray[22], tx_buf);
   fillLoRaData("Wärme W34--", 23, dataArray[23], tx_buf);
   getLoRaData(tx_buf, lora_data,16); 
//   getLongLoRaData(tx_buf, lora_data);
#ifdef LORA  
   writeNFC_Data(lora_data);
#endif  
  delay(10000);
   fillLoRaData("Wasserstand W35--", 24, dataArray[24], tx_buf);
   fillLoRaData("Wärme W35--", 25, dataArray[25], tx_buf);
   fillLoRaData("Wärme W35--", 26, 0,tx_buf);
   fillLoRaData("Wärme W35--", 27, (int)current_temperature, tx_buf);
   
   fillLoRaData("Wärme W35--", 28, 0, tx_buf);
   fillLoRaData("Wärme W35--", 29, 0, tx_buf);
   fillLoRaData("Wärme W35--", 30, 0, tx_buf);
   fillLoRaData("Wärme W35--", 31, 0, tx_buf);
   
   getLoRaData(tx_buf, lora_data,24); 
   writeNFC_Data(lora_data);
    delay(10000);
  }
  else
  {
    Serial.print(timeCounts); 
 //   displayTime2TX(timer_period_to_tx - delta_lora_tx);
  }
  // End Transmission

  if (data_received)
  {
    displayLoraRX(true);

    gmxLR_RXData(rx_data, &port);
    gmxLR_StringToHex(rx_data, rx_buf, &buf_len );

    Serial.println("LORA RX DATA:" + rx_data);
    Serial.print("LORA RX LEN:");
    Serial.println(buf_len);
    Serial.print("LORA RX PORT:");
    Serial.println(port);

    rx_buf[buf_len] = 0;
    sprintf(lora_rx_string,"%s",rx_buf);
    Serial.println( lora_rx_string );
    oled_string = String( lora_rx_string );

    data_received = false;

    delay(1000);
    displayLoraRX(false);

  }

//  displayTemp(current_temperature,oled_string );

  // update temperature
  // we sample and make an average of the temperature - since in this demo we use a NTC sensor that fluctuates
  temp_temperature += readTemp();
  temperature_counts ++;

  if ( temperature_counts >= TEMPERATURE_SAMPLING )
  {
    current_temperature = temp_temperature / TEMPERATURE_SAMPLING;
    temperature_counts = 0;
    temp_temperature = 0;
  }    //if
 // timer_millis_lora_tx = millis();
}

/****************************************************
*  LoRa Daten bereitstellen für Transfer
****************************************************/
void getLoRaData(byte tx_buf[], char lora_data[], int offset)
{
  offset = offset * 4;
  sprintf(lora_data,"%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",  
  tx_buf[0 + offset], tx_buf[1 + offset], tx_buf[2 + offset], tx_buf[3 + offset], 
  tx_buf[4 + offset], tx_buf[5 + offset], tx_buf[6 + offset], tx_buf[7 + offset],
  tx_buf[8 + offset], tx_buf[9 + offset], tx_buf[10 + offset], tx_buf[11 + offset], 
  tx_buf[12 + offset], tx_buf[13 + offset], tx_buf[14 + offset], tx_buf[15 + offset],
    
  tx_buf[16 + offset], tx_buf[17 + offset], tx_buf[18 + offset], tx_buf[19 + offset],
  tx_buf[20 + offset], tx_buf[21 + offset], tx_buf[22 + offset], tx_buf[23 + offset],
  tx_buf[24 + offset], tx_buf[25 + offset], tx_buf[26 + offset], tx_buf[27 + offset],
  tx_buf[28 + offset], tx_buf[29 + offset], tx_buf[30 + offset], tx_buf[31 + offset]);  
 
  Serial.println("********** LORA DATA *********");
  Serial.println(lora_data);
}

void getLongLoRaData(byte tx_buf[], char lora_data[])
{
  sprintf(lora_data,"%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",  
  //Gampelutz 5
  tx_buf[0], tx_buf[1], tx_buf[2], tx_buf[3], 
  tx_buf[4], tx_buf[5], tx_buf[6], tx_buf[7],
  tx_buf[8], tx_buf[9], tx_buf[10], tx_buf[11], 
  tx_buf[12], tx_buf[13], tx_buf[14], tx_buf[15],
    
  tx_buf[16], tx_buf[17], tx_buf[18], tx_buf[19],
  tx_buf[20], tx_buf[21], tx_buf[22], tx_buf[23],
  tx_buf[24], tx_buf[25], tx_buf[26], tx_buf[27],
  tx_buf[28], tx_buf[29], tx_buf[30], tx_buf[31],  
 
   //Gampelutz 5
  tx_buf[32], tx_buf[33], tx_buf[34], tx_buf[35], 
  tx_buf[36], tx_buf[37], tx_buf[38], tx_buf[39],
  tx_buf[40], tx_buf[41], tx_buf[42], tx_buf[43], 
  tx_buf[44], tx_buf[42], tx_buf[46], tx_buf[47],
    
  tx_buf[48], tx_buf[49], tx_buf[50], tx_buf[51],
  tx_buf[52], tx_buf[53], tx_buf[54], tx_buf[55],
  tx_buf[56], tx_buf[57], tx_buf[58], tx_buf[59],
  tx_buf[60], tx_buf[61], tx_buf[62], tx_buf[63], 
  
    //Galenburst 23
  tx_buf[64], tx_buf[65], tx_buf[66], tx_buf[67], 
  tx_buf[68], tx_buf[69], tx_buf[70], tx_buf[71],
  tx_buf[72], tx_buf[73], tx_buf[74], tx_buf[75], 
  tx_buf[76], tx_buf[77], tx_buf[78], tx_buf[79],    
  tx_buf[80], tx_buf[81], tx_buf[82], tx_buf[83],
  
  tx_buf[84], tx_buf[85], tx_buf[86], tx_buf[87],
  tx_buf[88], tx_buf[89], tx_buf[90], tx_buf[91],
  tx_buf[92], tx_buf[93], tx_buf[94], tx_buf[95],  
  tx_buf[96], tx_buf[97], tx_buf[98], tx_buf[99],
  tx_buf[100], tx_buf[101], tx_buf[102], tx_buf[103]); 

  Serial.println("********** LORA DATA *********");
  Serial.println(lora_data);
}
/****************************************************
*  LoRa Daten bereitstellen für Transfer
****************************************************/

/****************************************************
*  Transfer Array abfüllen
************* ***************************************/
void fillLoRaData(String dataName,  int dataIndex, int data, byte tx_buf[])
{
  Serial.println("Undex "+dataIndex);
  tx_buf[4*dataIndex] = dataIndex+1; // packet header - multiple data
  tx_buf[4*dataIndex+1] = 0x02;  // Analogmesswert
  tx_buf[4*dataIndex+2] = (data & 0xff00 ) >> 8;
  tx_buf[4*dataIndex+3] = data & 0x00ff;      
}

/****************************************************
*  Daten via LoRa Kommunication versenden
****************************************************/
void writeNFC_Data(char lora_data[])
{  
  String tx_data;
  displayLoraTX(true);
  tx_data = String(lora_data);
  Serial.println("TX DATA:" + tx_data);
  gmxLR_TXData(tx_data);

  // Update NFC
  writeNFC();

  displayLoraTX(false);
}
