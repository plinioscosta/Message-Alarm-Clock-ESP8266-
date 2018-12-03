// Program to exercise the MD_Parola library
//
// NOTE: MD_MAX72xx library must be installed and configured for the LED
// matrix type being used. Refer documentation included in the MD_MAX72xx
// library or see this link:
// https://majicdesigns.github.io/MD_MAX72XX/page_hardware.html
//

#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h> // must be included here so that Arduino library object file references work
#include <RtcDS3231.h>
#include <BlynkSimpleEsp8266.h>

#include "Font_Data.h"

RtcDS3231<TwoWire> Rtc(Wire);

// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
#define MAX_DEVICES 4

#define CLK_PIN   14
#define DATA_PIN  13
#define CS_PIN    15



const int AnalogIn  = A0;
int readingLight = 16;

#define BUZZER    2
#define BUTTON    16
int countButton=0;

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "49ddd52105de4ee39514b8bcad234837";

// Your WiFi credentials.
// Set password to "" for open networks.
 char ssid[] = "MyASUS";
 char pass[] = "27699659";

boolean connected = false;

WidgetLCD lcd(V4);

// ******************* String form to sent to the client-browser ************************************
String form =
  "<p>"
  "<center>"
  "<h1>ESP8266 Web Server</h1>"
  "<form action='msg'><p>Type your message <input type='text' name='msg' size=50 autofocus> <input type='submit' value='Submit'></form>"
  "<p>"
  "<form action='clock'><p><input type='submit' name='clock' size=50 value='Clock'></form>"
  "<p>"
  "<form action='temp'><p><input type='submit' name='temp' size=50 value='Temperature'></form>"
  "</center>";

ESP8266WebServer server(80);                             // HTTP server will listen at port 80  

// HARDWARE SPI
MD_Parola P = MD_Parola(CS_PIN, MAX_DEVICES);


#define PAUSE_TIME  0
#define SPEED_TIME 170
#define     BUF_SIZE     75  

int offset=1,refresh=0;
String decodedMsg;
char curMessage[BUF_SIZE];  
static uint8_t  state = 0;
char timeData[20];
char tempData[20];
uint8_t degC[] = { 6, 3, 3, 56, 68, 68, 68 }; // Deg C

// Turn on debug statements to the serial output
#define  DEBUG  0

#if  DEBUG
#define PRINT(s, x) { Serial.print(F(s)); Serial.print(x); }
#define PRINTS(x) Serial.print(F(x))
#define PRINTX(x) Serial.println(x, HEX)
#else
#define PRINT(s, x)
#define PRINTS(x)
#define PRINTX(x)
#endif

typedef struct
{
  uint8_t spacing;  // character spacing
  char  *msg;   // message to display
} msgDef_t;

msgDef_t  M[] =
{
  { 1, "Ola, gorducha!" },
  { 1, "Meu lugar favorito é dentro do teu abraço!" },
  { 1, "Passando para lembrar que te amo!" },
  { 1, "Te amo!" },
  { 1, "Só de te olhar já estou sorrindo!" },
};

#define MAX_STRINGS  (sizeof(M)/sizeof(M[0]))

int maxIntensity=5;
int countAuxIntensity=5;
boolean highIntensity=true;

void handle_buttonClock() {
  server.send(200, "text/html", form);    // Send same page so they can send another msg
  String buttonClock = server.arg("clock");
  Serial.println(buttonClock);
  if(buttonClock=="Clock"){
   state=1; 
  P.displayText("", PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);
  P.displayReset();
  }
}

void handle_buttonTemp() {
  server.send(200, "text/html", form);    // Send same page so they can send another msg
  String buttonTemp = server.arg("temp");
  Serial.println(buttonTemp);
  if(buttonTemp=="Temperature"){
   state=2; 
   P.displayText("", PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);
  P.displayReset();
  }
}

void handle_msg() {
  char buf[BUF_SIZE];                    
  //matrix.fillScreen(LOW);
  server.send(200, "text/html", form);    // Send same page so they can send another msg
  refresh=1;
  // Display msg on Oled
  String msg = server.arg("msg");
  Serial.println(msg);
  decodedMsg = msg;
  // Restore special characters that are misformed to %char by the client browser
  decodedMsg.replace("+", " ");      
  decodedMsg.replace("%21", "!");  
  decodedMsg.replace("%22", "");  
  decodedMsg.replace("%23", "#");
  decodedMsg.replace("%24", "$");
  decodedMsg.replace("%25", "%");  
  decodedMsg.replace("%26", "&");
  decodedMsg.replace("%27", "'");  
  decodedMsg.replace("%28", "(");
  decodedMsg.replace("%29", ")");
  decodedMsg.replace("%2A", "*");
  decodedMsg.replace("%2B", "+");  
  decodedMsg.replace("%2C", ",");  
  decodedMsg.replace("%2F", "/");   
  decodedMsg.replace("%3A", ":");    
  decodedMsg.replace("%3B", ";");  
  decodedMsg.replace("%3C", "<");  
  decodedMsg.replace("%3D", "=");  
  decodedMsg.replace("%3E", ">");
  decodedMsg.replace("%3F", "?");  
  decodedMsg.replace("%40", "@"); 
  Serial.println(decodedMsg);  // print original string to monitor

  decodedMsg.toCharArray(buf,BUF_SIZE);
  strcpy(curMessage,  buf); 
  M[0].msg = curMessage;
  state=0; 

  P.displayText("", PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);
  P.displayReset();
}

BLYNK_WRITE(V0)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V0 to a variable
  state=1;

  lcd.clear(); //Use it to clear the LCD Widget
  lcd.print(4, 0, "Mostrando"); // use: (position X: 0-15, position Y: 0-1, "Message you want to print")
  lcd.print(5, 1, "a hora");
  P.displayText("", PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);
  P.displayReset();
}

BLYNK_WRITE(V1)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  state=2; 

  lcd.clear(); //Use it to clear the LCD Widget
  lcd.print(4, 0, "Mostrando"); // use: (position X: 0-15, position Y: 0-1, "Message you want to print")
  lcd.print(1, 1, "a temperatura");
  P.displayText("", PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);
  P.displayReset();
}
int pinValue=1;

BLYNK_WRITE(V2)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
 
  if(pinValue ==1){
  M[0].msg = "Bom dia, amor";
  }
 if(pinValue ==2){
  M[0].msg = "Meu favorito eh dentro do teu abraço";
  }
  if(pinValue ==3){
  M[0].msg = "Passando para te lembrar que te amo";
  }
  if(pinValue ==4){
  M[0].msg = "Te amo!";
  }
  if(pinValue ==5){
  M[0].msg = "Feliz Natal!!!";
  }
  state=0; 

  lcd.clear(); //Use it to clear the LCD Widget
  lcd.print(3, 0, "Mostrando a"); // use: (position X: 0-15, position Y: 0-1, "Message you want to print")
  lcd.print(2, 1, "mgs escolhida");

  P.displayText("", PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);
  P.displayReset();
}

BLYNK_WRITE(V3) {
  TimeInputParam t(param);
  RtcDateTime now = Rtc.GetDateTime();
  //sprintf(timeData, "%02d%c%02d", now.Hour(), (f ? ':' : ' '), now.Minute());
  RtcDateTime ajust = RtcDateTime(now.Year(), now.Month(), now.Day(), t.getStartHour(), t.getStartMinute(),now.Second());

  Rtc.SetDateTime(ajust);
  Serial.println("Hora ajustada");

  P.displayText("", PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);
  P.displayReset();
}

BLYNK_WRITE(V5)
{
  char buf[BUF_SIZE];  
  String terminalMsg = param.asStr(); // assigning incoming value from pin V1 to a variable
  
  decodedMsg = terminalMsg;
  // Restore special characters that are misformed to %char by the client browser
  decodedMsg.replace("+", " ");      
  decodedMsg.replace("%21", "!");  
  decodedMsg.replace("%22", "");  
  decodedMsg.replace("%23", "#");
  decodedMsg.replace("%24", "$");
  decodedMsg.replace("%25", "%");  
  decodedMsg.replace("%26", "&");
  decodedMsg.replace("%27", "'");  
  decodedMsg.replace("%28", "(");
  decodedMsg.replace("%29", ")");
  decodedMsg.replace("%2A", "*");
  decodedMsg.replace("%2B", "+");  
  decodedMsg.replace("%2C", ",");  
  decodedMsg.replace("%2F", "/");   
  decodedMsg.replace("%3A", ":");    
  decodedMsg.replace("%3B", ";");  
  decodedMsg.replace("%3C", "<");  
  decodedMsg.replace("%3D", "=");  
  decodedMsg.replace("%3E", ">");
  decodedMsg.replace("%3F", "?");  
  decodedMsg.replace("%40", "@"); 
  Serial.println(decodedMsg);  // print original string to monitor

  decodedMsg.toCharArray(buf,BUF_SIZE);
  strcpy(curMessage,  buf); 
  M[0].msg = curMessage;
  state=0; 

 P.displayText("", PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);
  P.displayReset();
}

void connectAllServices(){
boolean connectedToSomeAP=false;
  // Mac address should be different for each device in your LAN
  byte arduino_mac[] = { 0xC0, 0x15, 0xC0, 0xFF, 0xEE, 0x81 };

  P.displayClear();

//  IPAddress arduino_ip (192, 168, 0, 20);;
//  IPAddress dns_ip     (  8,   8,   8,   8);
//  IPAddress gateway_ip (192, 168, 0, 1);
//  IPAddress subnet_mask(255, 255, 255, 0);

   //-----------------------------------------------------------------
   //Try connection on Naty's home  
   if(connectedToSomeAP == false){  //test if it was already connected
    IPAddress arduino_ip_1 (192, 168, 15, 20);
    IPAddress dns_ip_1     (  8,   8,   8,   8);
    IPAddress gateway_ip_1 (192, 168, 15, 1);
    IPAddress subnet_mask_1 (255, 255, 255, 0);

    WiFi.config(arduino_ip_1,gateway_ip_1,subnet_mask_1);
    WiFi.begin("V.Zani", "v.z41374293");  

  //Check if connection was sucessful
   for (int i=0; i <= 10; i++){
    if(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
      if(i==10){
        Serial.print("Not possible to connected to Naty's home\n");
      }
    }
    else{
    Serial.print("Connected to Naty's home\n");
    connectedToSomeAP = true;
    break;
    }
   }
   }
//-----------------------------------------------------------------

   //Try connection on Plinio's cellphone  
   if(connectedToSomeAP == false){ //test if it was already connected
    IPAddress arduino_ip_2 (192, 168, 43, 20);
    IPAddress dns_ip_2     (  8,   8,   8,   8);
    IPAddress gateway_ip_2 (192, 168, 43, 1);
    IPAddress subnet_mask_2 (255, 255, 0, 0);

    WiFi.config(arduino_ip_2,gateway_ip_2,subnet_mask_2);
    WiFi.begin("MyASUS", "27699659");  

  //Check if connection was sucessful
   for (int i=0; i <= 10; i++){
    if(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
      if(i==10){
        Serial.print("Not possible to connected to Plinio's cellphone\n");
      }
    }
    else{
    Serial.print("Connected to Plinio's cellphone\n");
    connectedToSomeAP = true;
    break;
    }
   }
   }
//-----------------------------------------------------------------

  //Try connection on Plinio's home  
  if(connectedToSomeAP == false){  //test if it was already connected
    IPAddress arduino_ip_3 (192, 168, 0, 20);
    IPAddress dns_ip_3     (  8,   8,   8,   8);
    IPAddress gateway_ip_3 (192, 168, 0, 1);
    IPAddress subnet_mask_3 (255, 255, 255, 0);

    WiFi.config(arduino_ip_3,gateway_ip_3,subnet_mask_3);
    WiFi.begin("Wifi", "fpga1234");  

  //Check if connection was sucessful
   for (int i=0; i <= 10; i++){
    if(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
      if(i==10){
        Serial.print("Not possible to connected to Plinio's home\n");
      }
    }
    else{
    Serial.print("Connected to Plinio's home\n");
    connectedToSomeAP = true;
    break;
    }
   }
  }
//-----------------------------------------------------------------

  if(connectedToSomeAP == true){  
  Blynk.connect(3333); 
  IPAddress blynk_IP(139, 59, 206, 133);
  Blynk.config(auth, blynk_IP, 8442);//8442
  while (Blynk.connect() == false) {
    // Wait until connected
     Serial.print("Blynk trying to connect..");
  }
  
   // Set up the endpoints for HTTP server,  Endpoints can be written as inline functions:
  server.on("/", []() {
    server.send(200, "text/html", form);
  });
  server.on("/msg", handle_msg);                  // And as regular external functions:
  server.on("/clock", handle_buttonClock);
  server.on("/temp", handle_buttonTemp);
  server.begin();                                 // Start the server 


       
 
  char result[16];
  Serial.println();
  Serial.println(result);
  decodedMsg = result;
  Serial.println("WebServer ready!   ");
  Serial.println(WiFi.localIP());                 // Serial monitor prints localIP
  connected = true;

   P.setFont(NULL);
   P.displayText(" ", PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);
   P.displayReset();
   P.displayAnimate();
   P.displayText("Conectado", PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
   P.setCharSpacing(M[0].spacing);
   P.displayReset();

  }

  else{
    Serial.print("Blynk NOT connected\n");
    connected = false;
  }
}

void disconnectAllServices(){
  Blynk.disconnect();
   WiFi.disconnect();
   Serial.println("Disconnected");
   connected = false;

      P.setFont(NULL);
      P.displayText(" ", PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);
      P.displayClear();
      P.displayText("Desconectado", PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
      P.setCharSpacing(M[0].spacing);
      P.displayReset();
}

void setup() {
  Serial.begin(9600);
  pinMode(BUTTON, INPUT);
  
  P.begin();
  P.setInvert(false);
  P.setIntensity(5);
  P.addChar('º', degC);
  //P.addChar('&', degF);
  //P.addChar('~', waveSine);
  //P.addChar('+', waveSqar);
  //P.addChar('^', waveTrng);

   P.setFont(NULL);
   P.displayText(" ", PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);
   P.displayText("Iniciando", PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
   P.setCharSpacing(M[0].spacing);
   P.displayReset();
   P.displayAnimate();

  connectAllServices();

  Serial.print("compiled: ");
    Serial.print(__DATE__);
    Serial.println(__TIME__);
    
    //----------------------------------------------------------------------------------------
    //--------RTC SETUP ----------------------------------------------------------------------
    //----------------------------------------------------------------------------------------
    Rtc.Begin();

    // if you are using ESP-01 then uncomment the line below to reset the pins to
    // the available pins for SDA, SCL
    // Wire.begin(0, 2); // due to limited pins, use pin 0 and 2 for SDA, SCL
    
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    printDateTime(compiled);
    Serial.println();

    if (!Rtc.IsDateTimeValid()) 
    {
        // Common Cuases:
        //    1) first time you ran and the device wasn't running yet
        //    2) the battery on the device is low or even missing

        Serial.println("RTC lost confidence in the DateTime!");

        // following line sets the RTC to the date & time this sketch was compiled
        // it will also reset the valid flag internally unless the Rtc device is
        // having an issue

        Rtc.SetDateTime(compiled);
    }

    if (!Rtc.GetIsRunning())
    {
        Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }

    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled) 
    {
        Serial.println("RTC is older than compile time!  (Updating DateTime)");
        Rtc.SetDateTime(compiled);
    }
    else if (now > compiled) 
    {
        Serial.println("RTC is newer than compile time. (this is expected)");
    }
    else if (now == compiled) 
    {
        Serial.println("RTC is the same as compile time! (not expected but all is fine)");
    }

    // never assume the Rtc was last configured by you, so
    // just clear them to your needed state
    Rtc.Enable32kHzPin(false);
    Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone); 

    // Alarm 1 set to trigger every day when 
    // the hours, minutes, and seconds match
    RtcDateTime alarmTime = now + 88; // into the future
    DS3231AlarmOne alarm1(
            0,
            21,
            0, 
            0,
            DS3231AlarmOneControl_HoursMinutesSecondsMatch);
    Rtc.SetAlarmOne(alarm1);

     // Alarm 2 set to trigger at the top of the minute
    DS3231AlarmTwo alarm2(
            0,
            6,
            30, 
            DS3231AlarmTwoControl_HoursMinutesMatch );
    Rtc.SetAlarmTwo(alarm2);

    // throw away any old alarm state before we ran
    Rtc.LatchAlarmsTriggeredFlags();


    lcd.clear(); //Use it to clear the LCD Widget
    lcd.print(3, 0, "Mostrando a"); // use: (position X: 0-15, position Y: 0-1, "Message you want to print")
    lcd.print(2, 1, "mgs escolhida");
}

RtcDateTime readDateTime(bool f = true){
  if (!Rtc.IsDateTimeValid()) 
    {
        // Common Cuases:
        //    1) the battery on the device is low or even missing and the power line was disconnected
        Serial.println("RTC lost confidence in the DateTime!");
    }

    RtcDateTime now = Rtc.GetDateTime();
    sprintf(timeData, "%02d%c%02d", now.Hour(), (f ? ':' : ' '), now.Minute());
   // printDateTime(now);
    //Serial.println();
return now; 
}

void readTemp(){
  RtcTemperature temp = Rtc.GetTemperature();
  float temp1 = temp.AsFloat();
  int a = temp1; // a will contain 123
   sprintf(tempData, "%dº", a);
  Serial.println(tempData); 
}


#define countof(a) (sizeof(a) / sizeof(a[0]))

void printDateTime(const RtcDateTime& dt)
{
    //sprintf(datestring, "%02d%c%02d", dt.Hour(), (f ? ':' : ' '), dt.Minute());

    //snprintf_P(datestring, 
           // countof(datestring),
            //PSTR("%02u : %02u"),
           // dt.Hour(),
           // dt.Minute());
            
}

void loop() {
  static uint32_t lastTime = 0; // millis() memory
  static uint8_t  display = 0;  // current display mode
  static bool flasher = false;  // seconds passing flasher
  
  buttonPressed();//Verfica se o botão foi pressionado e muda de estado   
   
   if (Blynk.connected()){
    Blynk.run();
    server.handleClient(); 
   }
   else{
    RtcDateTime now = Rtc.GetDateTime();
    int reconnect_Min = now.Minute(); 
      if(reconnect_Min%5==0){
      connectAllServices(); 
      }
   }
    
  if (P.displayAnimate())
  {

    switch(state) 
    {
    case 0: 
    //P.setTextBuffer(M[state].msg);
    P.setFont(NULL);
    P.displayText(M[state].msg, PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    P.setCharSpacing(M[state].spacing);
    P.displayReset();
    break;

    case 1:
    //P.setTextBuffer(M[n].msg);
    if (millis() - lastTime >= 1000)
  {
    lastTime = millis();
    readDateTime(flasher);
    flasher = !flasher;
  }
    P.setFont(numeric7Seg);
    M[1].msg=timeData;
    P.displayText(M[state].msg, PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);
    P.setCharSpacing(M[state].spacing);
    P.displayReset();
    break;
    
    case 2:
    P.setFont(numeric7Seg);
    readTemp();
    M[2].msg=tempData;
    P.displayText(M[state].msg, PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);
    P.setCharSpacing(M[state].spacing);
    P.displayReset();
    break;
    
    }

    Alarmed();//Verifica se o alarme foi acionado
    verifyLight();//Verifica e configura a intensidade dos LEDs de acordo com luminosidade ambiente
 
    }

}

  void Alarmed(){
      
    bool wasAlarmed = false;
        wasAlarmed = true;
        // this gives us which alarms triggered and
        // then allows for others to trigger again
        DS3231AlarmFlag flag = Rtc.LatchAlarmsTriggeredFlags();

        if (flag & DS3231AlarmFlag_Alarm1)
        {
           
           M[0].msg = "Hora do Remedio";
           state=0; 
           Serial.println("Hora do Remedio");
        }
        if (flag & DS3231AlarmFlag_Alarm2)
        {
            M[0].msg = "Bom dia, gorducha!";
            state=0; 
            Serial.println("Bom dia, gorducha!");
        }
         
  }



  void verifyLight(){
   readingLight = analogRead(AnalogIn);

    if(readingLight<50){
      P.setIntensity(0);
      highIntensity = false;
    }
    if(readingLight>51){
      P.setIntensity(maxIntensity); 
      highIntensity = true;
    }
  }

  void modifyIntensity(){
   countAuxIntensity++;
    maxIntensity=countAuxIntensity%6;
  }

const int buttonPin = 16; 
boolean prevPressedButton = false;
int butoonFirstPressed = 0;
int countButtonFalse = 0;
int countRoll=0;

  void buttonPressed() {
    int reading = digitalRead(BUTTON);// read the state of the switch into a local variable:
      
    if(reading==1){
        if(prevPressedButton==false){
        butoonFirstPressed = millis();
         }
      prevPressedButton = true;
    }
    
    if(reading==0 && prevPressedButton == true){
    countButtonFalse++;
    if(countButtonFalse>10){  
        if((millis()- butoonFirstPressed)<400){
        Serial.println("Opção 1");
        countRoll++;
        state=countRoll%3; 
        P.displayText(" ", PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);
        }
        if((millis()- butoonFirstPressed)<5000 && (millis()- butoonFirstPressed)>400){
        modifyIntensity();
         Serial.println(maxIntensity);
        }
        if((millis()- butoonFirstPressed)>5000){
        Serial.println("Opção 3");
            if( connected == true){
              disconnectAllServices();
              Serial.println("desconectado");

              P.setFont(NULL);
              P.displayText(" ", PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);
              P.displayText("Desconectado", PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
              P.setCharSpacing(M[state].spacing);
              P.displayReset();                
            }
            
            else{
              connectAllServices();

            }
        }
     prevPressedButton=false; 
     countButtonFalse=0; 
    }
     
    }
 }
