#define BLYNK_PRINT Serial
#include <FastLED.h>
#include <ESP8266WiFi.h>
#define BLYNK_TIMEOUT_MS  750  // must be BEFORE BlynkSimpleEsp8266.h doesn't work !!!
#define BLYNK_HEARTBEAT   17   // must be BEFORE BlynkSimpleEsp8266.h works OK as 17s
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>
#include <Wire.h>

#define BLYNK_PRINT Serial    

#define LED_PIN     14
#define NUM_LEDS    36
#define BRIGHTNESS  100
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

#define LED_PIN2     12
#define NUM_LEDS2    36
#define BRIGHTNESS2  100
#define LED_TYPE2    WS2811
#define COLOR_ORDER2 GRB
CRGB leds2[NUM_LEDS2];


// Power
#define BH1750_POWER_DOWN 0x00  // No active state
#define BH1750_POWER_ON 0x01  // Waiting for measurement command
#define BH1750_RESET 0x07  // Reset data register value - not accepted in POWER_DOWN mode

// Measurement Mode
#define CONTINUOUS_HIGH_RES_MODE 0x10  // Measurement at 1 lux resolution. Measurement time is approx 120ms
#define CONTINUOUS_HIGH_RES_MODE_2 0x11  // Measurement at 0.5 lux resolution. Measurement time is approx 120ms
#define CONTINUOUS_LOW_RES_MODE 0x13  // Measurement at 4 lux resolution. Measurement time is approx 16ms
#define ONE_TIME_HIGH_RES_MODE 0x20  // Measurement at 1 lux resolution. Measurement time is approx 120ms
#define ONE_TIME_HIGH_RES_MODE_2 0x21  // Measurement at 0.5 lux resolution. Measurement time is approx 120ms
#define ONE_TIME_LOW_RES_MODE 0x23  // Measurement at 4 lux resolution. Measurement time is approx 16ms

// I2C Address
#define BH1750_1_ADDRESS 0x23  // Sensor 1 connected to GND
#define BH1750_2_ADDRESS 0x5C  // Sensor 2 connected to VCC


char auth[] = "a1d7ea85ee93444486e42c8faa7e008c";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Y15";
char pass[] = "12345678";
char server[]          = "blynk-cloud.com";
unsigned int port      = 80;



CRGBPalette16 currentPalette;



int terang = 255;
int terang2 = 255;



// Definition of Variable
int16_t RawData;
int16_t SensorValue[2];
unsigned long previouTime = 0;
const unsigned long EventInterval = 1000;
unsigned long previouTime2 = 0;
const unsigned long EventInterval2 = 500;
unsigned long previouTime3 = 0;
const unsigned long EventInterval3 = 10000;
unsigned long previouTime4 = 0;
const unsigned long EventInterval4 = 11000;
unsigned long previouTime5 = 0;
const unsigned long EventInterval5 = 120000;
unsigned long previouTime6 = 0;
const unsigned long EventInterval6 = 18000;
unsigned long previouTime7 = 0;
const unsigned long EventInterval7 = 19000;







void setup() {
    Serial.begin(115200);
    Blynk.connectWiFi(ssid, pass);
    Blynk.config(auth, server, port);
    Blynk.connect();    
    Wire.begin(D1,D2);
    
    delay( 3000 ); // power-up safety delay
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(  BRIGHTNESS );
    FastLED.addLeds<LED_TYPE2, LED_PIN2, COLOR_ORDER>(leds2, NUM_LEDS2).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(  BRIGHTNESS2 );
    
    SetupBlackAndWhiteStripedPalette();
    
    
    
}

BLYNK_WRITE(V7)// set Setpoint to V7 of blynk.
{
  terang = param.asInt();
}
BLYNK_WRITE(V6)// set Setpoint to V7 of blynk.
{
  terang2 = param.asInt();
}

void CheckConnection(){    // check every 11s if connected to Blynk server
  if(!Blynk.connected()){
    Serial.println("Not connected to Blynk server"); 
    Blynk.connect();  // try to connect to server with default timeout
  }
  else{
    Serial.println("Connected to Blynk server");     
  }
}

void init_BH1750(int ADDRESS, int MODE){
  //BH1750 Initializing & Reset
  Wire.beginTransmission(ADDRESS);
  Wire.write(MODE);  // PWR_MGMT_1 register
  Wire.endTransmission(true);
}

void RawData_BH1750(int ADDRESS){
  Wire.beginTransmission(ADDRESS);
  Wire.requestFrom(ADDRESS,2,true);  // request a total of 2 registers
  RawData = Wire.read() << 8 | Wire.read();  // Read Raw Data of BH1750
  Wire.endTransmission(true);
}
void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    uint8_t brightness = terang;
    
    for( int i = 1; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness );
        colorIndex += 3;
    }
}
void FillLEDsFromPaletteColors2( uint8_t colorIndex)
{
    uint8_t brightness = terang2;
    
    for( int i = 1; i < NUM_LEDS2; i++) {
        leds2[i] = ColorFromPalette( currentPalette, colorIndex, brightness );
        colorIndex += 3;
    }
}


void SetupBlackAndWhiteStripedPalette()
{
    // 'black out' all 16 palette entries...
    fill_solid( currentPalette, 5, CRGB::Black);
    // and set every fourth one to white.
    currentPalette[0] = CRGB::Red;
    currentPalette[4] = CRGB::Red;
    currentPalette[8] = CRGB::Red;
    currentPalette[12] = CRGB::Red;
    
}
void loop()
{
    
   if(Blynk.connected()){
    Blynk.run();
  }
    
    unsigned long currentMillis = millis();
    leds[0].r = terang; 
    leds2[0].r = terang2;
    FastLED.show();

    if (currentMillis - previouTime >= 10)
    {
    
    
    static uint8_t startIndex = 1;
    startIndex = startIndex +1; /* motion speed */
    
    FillLEDsFromPaletteColors( startIndex);
    FillLEDsFromPaletteColors2( startIndex);
    
    previouTime = currentMillis; 
    }

    
    if (currentMillis - previouTime2 >= EventInterval2)
    {
    previouTime2 = currentMillis;
    init_BH1750(BH1750_1_ADDRESS, CONTINUOUS_HIGH_RES_MODE);
    RawData_BH1750(BH1750_1_ADDRESS);
    SensorValue[0] = RawData / 1.2;  
    init_BH1750(BH1750_2_ADDRESS, CONTINUOUS_HIGH_RES_MODE);
    RawData_BH1750(BH1750_2_ADDRESS);
    SensorValue[1] = RawData / 1.2;
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
   //Read the Temp and Humidity from DHT
    Serial.print("Sensor_1 = "); Serial.print(SensorValue[0]);
    Serial.print(" | Sensor_2 = "); Serial.println(SensorValue[1]);
  
    
     
    }
    if (currentMillis - previouTime3 >= EventInterval3)
    {
    previouTime3 = currentMillis;
    Blynk.virtualWrite(4, SensorValue[0]);
    
   
           
    }
    if (currentMillis - previouTime4 >= EventInterval4)
    {
    previouTime4 = currentMillis;
    Blynk.virtualWrite(5, SensorValue[1]);
      
    }
   if (currentMillis - previouTime5 >= EventInterval5)
    {
      previouTime5 = currentMillis;
    CheckConnection();
      
    }
    if (currentMillis - previouTime6 >= EventInterval6)
    {
    previouTime6 = currentMillis;
    Blynk.virtualWrite(3, SensorValue[0]);
    
   
           
    }
    if (currentMillis - previouTime7 >= EventInterval7)
    {
    previouTime7 = currentMillis;
    Blynk.virtualWrite(2, SensorValue[0]);
    
   
           
    }
}
