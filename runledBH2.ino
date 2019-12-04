
#include <FastLED.h>
#include <ESP8266WiFi.h>
#define BLYNK_TIMEOUT_MS  750  // must be BEFORE BlynkSimpleEsp8266.h doesn't work !!!
#define BLYNK_HEARTBEAT   30   // must be BEFORE BlynkSimpleEsp8266.h works OK as 17s
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>
#include <Wire.h>
#include <PID_v1.h>
#include <Servo.h>
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
SSD1306  display(0x3c, 5, 4); // Initialize the OLED display using Wire library



#define BLYNK_PRINT Serial    
#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
#define LED_PIN     3
#define NUM_LEDS    36
#define BRIGHTNESS  100
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

#define LED_PIN2     4
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
double sp = 100 ;
double sp2 = 100 ;
double sum;
double xsum;
int man1 = 0;
int man2 = 0;
double Outx = 100;
double Outx2 = 100;
double Outy = 100;
double Outy2 = 100;



// Definition of Variable
int16_t RawData;
int16_t SensorValue[2];

double Setpoint , Input, Output;
double Setpoint2 , Input2, Output2;
PID myPID(&Input, &Output, &Setpoint ,1.2,0.9,1, DIRECT);
PID myPID2(&Input2, &Output2, &Setpoint2 ,1.2,0.9,1, DIRECT);
Servo servo;
Servo servo2;
BlynkTimer timer;
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
    timer.setInterval(13L, LED);
    timer.setInterval(500L, sensor);
    timer.setInterval(8000L, v1);
    timer.setInterval(30000L, check);
    myPID.SetMode(AUTOMATIC);
    myPID2.SetMode(AUTOMATIC);
    display.init(); // Initialising the UI will init the display too.
    display.flipScreenVertically();    
}



BLYNK_WRITE(V7)// set Setpoint to V7 of blynk.
{
  sp = param.asDouble();
}
BLYNK_WRITE(V6)// set Setpoint to V7 of blynk.
{
  sp2 = param.asDouble();
}
BLYNK_WRITE(V8)// set Setpoint to V7 of blynk.
{
  man1 = param.asInt();
}
BLYNK_WRITE(V9)// set Setpoint to V7 of blynk.
{
  man2 = param.asInt();
}
BLYNK_WRITE(V10)// set Setpoint to V7 of blynk.
{
  Outx = param.asDouble();
}
BLYNK_WRITE(V11)// set Setpoint to V7 of blynk.
{
  Outx2 = param.asDouble();
}


void drawlux()
{
  int x=0;
  int y=0;

  int lux = SensorValue[0] ;
  lux = map (lux,0,10000,0,100);

  display.setFont(ArialMT_Plain_24);
  String hum = String(lux) + " PSIG";
  display.drawString(0 + x, 15 + y, hum);
  int humWidth = display.getStringWidth(hum);
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
    uint8_t brightness = xsum;
    
    for( int i = 1; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness );
        colorIndex += 3;
    }
}
void FillLEDsFromPaletteColors2( uint8_t colorIndex)
{
    uint8_t brightness = xsum;
    
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

void LED()
{
    leds[0].g =xsum; 
    leds2[0].g =xsum;
    static uint8_t startIndex = 1;
    startIndex = startIndex +1; /* motion speed */
    FillLEDsFromPaletteColors( startIndex);
    FillLEDsFromPaletteColors2( startIndex);
    FastLED.show();
    myPID.Compute();
    myPID2.Compute();
    sum = Outy + Outy2;
    xsum = constrain(sum,0,255);
    if (man1 == 1 )
    {
      Outy = Outx;
      
    }else{Outy = Output;}
    if (man2 == 1 )
    {
      Outy2 = Outx2;
      
    }else{Outy2 = Output2;}
    
}
void sensor ()
{
  init_BH1750(BH1750_1_ADDRESS, CONTINUOUS_HIGH_RES_MODE);
    RawData_BH1750(BH1750_1_ADDRESS);
    SensorValue[0] = RawData / 120;  
    init_BH1750(BH1750_2_ADDRESS, CONTINUOUS_HIGH_RES_MODE);
    RawData_BH1750(BH1750_2_ADDRESS);
    SensorValue[1] = RawData / 120;
    Input = SensorValue[0];
    Input2 = SensorValue[1];
    display.clear();
    drawlux();
    display.display();
    Setpoint = sp;
    Setpoint2 = sp2;

    

}


void v1()
{
  Blynk.virtualWrite(4, SensorValue[0]);
  Blynk.virtualWrite(5, SensorValue[1]);
  Blynk.virtualWrite(3, Outy);
  Blynk.virtualWrite(2, Outy2);
  Blynk.virtualWrite(1, xsum);
  }
void check (){
  CheckConnection();
  
}
void loop()
{
    
   if(Blynk.connected()){
    Blynk.run();
  }
  timer.run();
      
}
