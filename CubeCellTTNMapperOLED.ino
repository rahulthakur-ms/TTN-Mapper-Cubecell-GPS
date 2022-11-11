#include "LoRaWan_APP.h"
#include "Arduino.h"
#include "GPS_Air530.h"
#include "GPS_Air530Z.h"
#include <Wire.h>  
#include "HT_SSD1306Wire.h"

#define POI_BUTTON (GPIO7)

Air530ZClass GPS;
SSD1306Wire  screen(0x3c, 500000, SDA, SCL, GEOMETRY_128_64, GPIO10 ); // addr , freq , SDA, SCL, resolution , rst

int pktSent = 1;

uint8_t devEui[] = { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x05, 0x74, 0x9A };  //Change this
uint8_t appEui[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };  //Change this
uint8_t appKey[] = { 0xDF, 0x84, 0xCB, 0x13, 0xF5, 0x5F, 0x36, 0x39, 0x78, 0x96, 0x34, 0xCA, 0xEC, 0xAE, 0xBA, 0xD9 };  //Change this

uint32_t devAddr = ( uint32_t)0x26000000;
uint8_t nwkSKey[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t appSKey[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

extern bool IsLoRaMacNetworkJoined;

uint16_t userChannelsMask[6] = { 0xFF00, 0x0000, 0x0000, 0x0000, 0x0002, 0x0000 }; // Subchannel 2 for AU915

/*
   set LoraWan_RGB to Active,the RGB active in loraWan
   RGB red means sending;
   RGB purple means joined done;
   RGB blue means RxWindow1;
   RGB yellow means RxWindow2;
   RGB green means received done;
*/

/*LoraWan Class*/
DeviceClass_t  loraWanClass = LORAWAN_CLASS;
/*OTAA or ABP*/
bool overTheAirActivation = LORAWAN_NETMODE;
/*ADR enable*/
bool loraWanAdr = LORAWAN_ADR;
/* set LORAWAN_Net_Reserve ON, the node could save the network info to flash, when node reset not need to join again */
bool keepNet = LORAWAN_NET_RESERVE;
/*LoraWan REGION*/
LoRaMacRegion_t loraWanRegion = ACTIVE_REGION;

/* Indicates if the node is sending confirmed or unconfirmed messages */
bool isTxConfirmed = false; //LORAWAN_UPLINKMODE;

uint8_t confirmedNbTrials = 8;

/* Application port */
uint8_t appPort = 2;

/*the application data transmission duty cycle.  value in [ms].*/
uint32_t appTxDutyCycle = 1000 * 20;

uint32_t LatitudeBinary, LongitudeBinary;
uint16_t altitudeGps;
uint8_t hdopGps;
bool poi; // point of interest
bool DRfast; // slow or fast DR. Flip-flip between them.

#define DR_SLOW DR_2 // SF10
#define DR_FAST DR_5 // SF7

void poiButtonPress()
{
  poi = true;
}

// Set the Datarate (interrelated to SF. DR_0 = SF12, mostly. DR_2 = SF10 for AU915, DR_5 is SF7)
void setDR(int8_t datarate) {
  MibRequestConfirm_t mibSet;
  
  mibSet.Type = MIB_CHANNELS_DATARATE;
  mibSet.Param.ChannelsDatarate = datarate;
  LoRaMacMibSetRequestConfirm( &mibSet );
}

/* Prepares the payload of the frame */
static void prepareTxFrame( uint8_t port )
{
  appDataSize = 0;//AppDataSize max value is 64

  Serial.printf("GPS data is %ds old - %d sats\n", GPS.location.age()/1000, GPS.satellites.value());
  
  if (GPS.location.age() < 1000) {
    
    appDataSize = 10;

    LatitudeBinary = ((GPS.location.lat() + 90) / 180) * 16777215;
    LongitudeBinary = ((GPS.location.lng() + 180) / 360) * 16777215;

    appData[0] = ( LatitudeBinary >> 16 ) & 0xFF;
    appData[1] = ( LatitudeBinary >> 8 ) & 0xFF;
    appData[2] = LatitudeBinary & 0xFF;

    appData[3] = ( LongitudeBinary >> 16 ) & 0xFF;
    appData[4] = ( LongitudeBinary >> 8 ) & 0xFF;
    appData[5] = LongitudeBinary & 0xFF;

    altitudeGps = GPS.altitude.meters();
    appData[6] = ( altitudeGps >> 8 ) & 0xFF;
    appData[7] = altitudeGps & 0xFF;

    hdopGps = GPS.hdop.hdop() * 10;
    appData[8] = hdopGps & 0xFF;

    appData[9] = GPS.satellites.value(); // < 128 sats. Could probably steal another bit.
    appData[9] &= ~(1 << 7);
    if (poi) {
      appData[9] |= 1 << 7;
      poi = false;
      Serial.println("POI");
    }
  } else {
    Serial.print("No GPS data found:");
    Serial.printf("%04d-%02d-%02dT%02d:%02d:%02d\n", GPS.date.year(), GPS.date.month(), GPS.date.day(),
                  GPS.time.hour(), GPS.time.minute(), GPS.time.second());
  }
}

void setup() {
  pinMode(POI_BUTTON,INPUT_PULLUP);
  attachInterrupt(POI_BUTTON, poiButtonPress, FALLING);
  
  boardInitMcu();

  screen.init();
  screen.clear();
  screen.display();
  screen.setFont(ArialMT_Plain_16);
  screen.drawString(0, 2, "Starting..");
  screen.drawString(0, 40, "Locking GPS");
  screen.display();
  screen.setFont(ArialMT_Plain_24);
  delay(1000);

  Serial.begin(115200);
#if(AT_SUPPORT)
  enableAt();
#endif
//  LoRaWAN.displayMcuInit();
  deviceState = DEVICE_STATE_INIT;
  LoRaWAN.ifskipjoin();
//  GPS.setmode(MODE_GPS_GLONASS_BEIDOU);
  GPS.setNMEA(NMEA_RMC);
  GPS.begin();
  poi = false;
}

void get_gps() {
  if (GPS.available() > 0)
  {
    GPS.encode(GPS.read());
  }
}

void loop()
{
  get_gps(); // get something useful
  switch ( deviceState )
  {
    case DEVICE_STATE_INIT:
      {
#if(AT_SUPPORT)
        getDevParam();
#endif
        printDevParam();
        Serial.printf("LoRaWan Class%X  start! \r\n", loraWanClass + 10);
        LoRaWAN.init(loraWanClass, loraWanRegion);
        deviceState = DEVICE_STATE_JOIN;
        break;
      }
    case DEVICE_STATE_JOIN:
      {
        //setTTNBand();
        LoRaWAN.join();
        break;
      }
    case DEVICE_STATE_SEND:
      {
        prepareTxFrame( appPort );
        if (appDataSize > 0) {
          if(DRfast) {
            Serial.println("Setting DRfast");
            setDR(DR_FAST);
          } else {
            Serial.println("Setting DRslow");
            setDR(DR_SLOW);
          }
          DRfast = !DRfast;
          LoRaWAN.send();
          screen.clear();
          screen.drawString(0, 2, "Pkt Sent ");
          screen.drawString(0, 30, String(pktSent));
          screen.display();
          pktSent++;
        }
        deviceState = DEVICE_STATE_CYCLE;
        
        break;
      }
    case DEVICE_STATE_CYCLE:
      {
        // Schedule next packet transmission
        txDutyCycleTime = appTxDutyCycle + randr( 0, APP_TX_DUTYCYCLE_RND );
        LoRaWAN.cycle(txDutyCycleTime);
        deviceState = DEVICE_STATE_SLEEP;
        break;
      }
    case DEVICE_STATE_SLEEP:
      {
        //LoRaWAN.sleep();
        Radio.IrqProcess( );
        break;
      }
    default:
      {
        deviceState = DEVICE_STATE_INIT;
        break;
      }
  }
}
