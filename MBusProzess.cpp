/* ___      ___    ___      ___    _____               _____
  |   \    /   |  |   \    /   |  |_   _|     ____    |_   _|
  | |\ \  / /| |  | |\ \  / /| |    | |      /    \     | |
  | | \ \/ / | |  | | \ \/ / | |    | |     | /  \ |    | |
  | |  \__/  | |  | |  \__/  | |   _| |_    | \__/ /    | |
  |_|        |_|  |_|        |_|  |_____|    \____/     |_|
  
  (c) 2019 MMsIoT#include <Wire.h> 
*/
#include "MBus.h"
#include "MBusProzess.h"

MBus mbus(1);

#define ANZAHL_METER 16

#define MBUS_ADDRESS 0
#define MESSUNG 1
#define SERIEN_NUMMER 2
#define T_VORLAUF 3
#define T_RUECKLAUF 4
#define VOLUMEN_FLOW 5
#define FLOW_EXTERN 6
#define FLOW 7
#define ARBEIT 8
#define LEISTUNG 9


String  strMBusData;
String readString; 
String sMMSERNr;
String sToInt;
String sCom2Winmos32;

String incomingData;
String sWData;

int iCounter; 
int nLoops = 0;
int iMBusData;
int nMeter;
//int iThingSpeak;



//*****************************************************************************

#define DEBUG  
#define WLAN
/*
String testMBusResponse[]= {//"68414168080F7226011570A7320A04200000000C06155700000C14726416000B3B8305000B2D1900000B5A3403000B5E060300046D180C8E240F00000001",
                            "E56841416808107227011570A7320A041E0000000C06026600000C14737925000B3B1105000B2D1300000B5A4603000B5E240300046D190C8E240F00000001",
                            "E5      E568353568080B7214314519E61E3C062200000003744BB400041399CD000044139DBF0000426C9F2302FD7408140F0100E001FD71B002740300BD16",
                            "      E568353568080C7215314519E61E3C061E0000000374FCB3000413912801004413A4130100426C9F2302FD7408140F0100E001FD71B6027409002716",
                            "E56841416808107227011570A7320A041E0000000C06026600000C14737925000B3B1105000B2D1300000B5A4603000B5E240300046D190C8E240F00000001",
                            "      E568353568080D7216314519E61E3C0618000000037461B40004132660000044136F590000426C9F2302FD7408140F0100E001FD71AD02740D006016",
                            "E56845456808117228011570A7320A04190000000C06861400000C14383801000B3B0100F00B2D0000000B5A3902000B5E320200046D070C8E240F0000002F",
                            "68353568080E7217314519E61E3C0617000000037486B3000413C1F00000441356DE0000426C9F2302FD7408140F0100E001FD71AE027401001116"
};
*/
String testMBusResponse[]= {
      "E568353568080C7215314519E61E3C0602000000037479FA000413F12D01004413A4130100426C9F2302FD7405140F0100E001FD71A102740B001E16",
      "E56835356808157218314519E61E3C06020000000374F1FA0004130D770100441360550100426C9F2302FD7405140F0100E001FD71A802740D000F16",
      "E56835356808167219314519E61E3C060200000003744FFA00041352700000441373680000426C9F2302FD7405140F0100E001FD71B902740A00DF16",
      "E56835356808187221314519E61E3C06020000000374DAFA000413606401004413AB400100426C9F2302FD7405140F0100E001FD71A8027401006E16",
      "E568353568081F7222314519E61E3C0602000000037492F90004135A6400004413B4420000426C9F2302FD7405140F0100E001FD71AD02740E004216",
      "E56835356808207223314519E61E3C0602000000037470FA000413293301004413671E0100426C9F2302FD7405140F0100E001FD71A4027408004316",
      "E56835356808217224314519E61E3C0602000000037449FA000413D9830100441319630100426C9F2302FD7405140F0100E001FD71AC027405001A16",
      "E56835356808227225314519E61E3C0602000000037430FA000413EFE3000044135AD00000426C9F2302FD7405140F0100E001FD71AA027402002016",
      "E56835356808237226314519E61E3C0602000000037485FA000413CF55010044130E3B0100426C9F2302FD7405140F0100E001FD71A402740600E816",
      "E56845456808197230011570A7320A04020000000C06916200000C14479020000B3B0100F00B2D0000000B5A3702000B5E360200046D1E1191240F0000002F2F2F2F01FD71AA02743A00E516",
      "E568454568081B7232011570A7320A04020000000C06640000000C14930400000B3B0200F00B2D0000000B5A3102000B5E310200046D1B1191240F0000002F2F2F2F01FD71AE02743B01F316",
      "E568454568081C7233011570A7320A04020000000C06013200000C14012406000B3B0200F00B2D0000000B5A4902000B5E490200046D121191240F0800002F2F2F2F01FD71A70274B000F416",
      "E56841416808247234011570A7320A04020000000C06662500000C14102104000B3B1103000B2D0100000B5A0903000B5E080300046D330788240F00000001FD71A602741E00BA16",
      "E56841416808257235011570A7320A04020000000C06983300000C14630207000B3B0100F00B2D0000000B5A2602000B5E270200046D0D1191240F00000001FD71A7027413002C16",
      "E56845456808267236011570A7320A04020000000C06368400000C14304014000B3B0100F00B2D0000000B5A4102000B5E400200046D1A1191240F0000002F2F2F2F01FD71AB0274B801DC16",
      "E56845456808287238011570A7320A04020000000C06292601000C14160738000B3B0200F00B2D0000000B5A4702000B5E460200046D1F1191240F0000002F2F2F2F01FD71A702741500B116"
  };

//======================================================= 
//======================================================= 
//==           SETUP
//======================================================= 
//======================================================= 
void MBusProzess::setupMBus() {   
  Serial.println("\nMBusProzess-Klassen Erweiterung für LoRa Projekt"); 
  Serial.println("Gampelutz 5+7, Gallenburst 23, Mauren"); 
  Serial.println("25.12.2019"); 
  delay(1000); 
} 


//=======================================================   
//======================================================= 
//==             LOOP
//======================================================= 
//======================================================= 
void MBusProzess::getMBusData(word dataArray[]) {
   Serial.println("getMBusData");
   
// -----------  Zaehler auslesen------------------------
  for (int iMBusAdr = 0; iMBusAdr < ANZAHL_METER ; iMBusAdr++){

 //   nMeter = arMBus_ApiKey[iMBusAdr][MBUS_ADDRESS].toInt();  //MBus address
 //   iThingSpeak = arMBus_ApiKey[iMBusAdr][MBUS_ADDRESS].toInt();  // MBus address
    delay(300);

// ------------  REQUEST DATA Send Command to Meter -----------
 //   mbus.request_MBusData(nMeter); //  Zähleradressen auswerten  // -- 9 --
    mbus.request_MBusSerialNumber(iMBusAdr);

   
//-----------   RECEIVE DATA FROM MBUS  -------------- 
    String  sMBus = mbus.receive_MBusData();   // -- 10 --
// String   sMBus = testMBusResponse[iMBusAdr];
// String   sMBus =" E56841416808257235011570A7320A04020000000C06983300000C14630207000B3B0100F00B2D0000000B5A2602000B5E270200046D0D1191240F00000001FD71A7027413002C16";
     
   Serial.println("--- "+sMBus);
    
    strMBusData =sMBus;// sMBus.substring(iMBusData);   
    delay(100);

// ----------    Any data from UART1 received --------------
    if (strMBusData.length() > 0){ 
//      Serial.println("MBus daten gefunden !!!!!");
// --------  Interpret MBus Data     -------- 
       mbus.search_MBusParameter(strMBusData, iMBusAdr, dataArray);  // -- 0 --
    }  //if (strMBusData.length() > 0){ 
    delay(5000);
  }  
}

///====================================================== 
//  ThingSpeak Übertragung zusammen stellen
//=======================================================
void MBusProzess::get_ThingSpeakData(word dataArray[], int anzahlMeter)
{
/*  String sHTML = "";
  int iMBusAdr;
  int iArrayPos;

  for (int i = 0 ; i < anzahlMeter; i++){
    iMBusAdr = arMBus_ApiKey[i][0].toInt();  //MBusadresse aus Array lesen
    sHTML = mbus.fMBus_Messung_2[iMBusAdr][MESSUNG];  // Value  // Messwert aus Array lesen
 
    dataArray[i] = sHTML.toInt()/ 100;    // 1000 **** MessDaten in Array ablegen
    Serial.print( sHTML); Serial.print("<-->");  Serial.println(dataArray[i]);
  }
  */
}
 
