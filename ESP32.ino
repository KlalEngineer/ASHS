/**********************************************************************************************
<<< Required libraries >>>
https://github.com/me-no-dev/AsyncTCP
https://github.com/me-no-dev/ESPAsyncWebServer/archive/master.zip
https://github.com/adafruit/Adafruit_SSD1306
https://github.com/adafruit/Adafruit-GFX-Library
https://downloads.arduino.cc/libraries/github.com/adafruit/DHT_sensor_library-1.4.4.zip
**********************************************************************************************/

#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT22.h>

// set this to 'true' when testing all the peripherals on the PCB to ensure that hardware works
// set this to 'false' to enable the web server
#define HARDWARE_TEST true 

// Place your network credentials
const char* ssid = "place-your-network-name-here";
const char* password = "place-your-network-password-here";

































/* Ultrasonic sensor variables */
#define TRIG_PIN 19 // ultrasonic sensor - trig pin
#define ECHO_PIN 18 // ultrasonic sensor - echo pin
//define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701
long duration = 0;
float distance = 0.0;

/* DHT22 sensor variables */
#define DHT_PIN 5 // data pin of DHT22 sensor
DHT22 dht22(DHT_PIN); 
float t_prev = 0.0;
float h_prev = 0.0;

/* OLED size in pixels */
#define SCREEN_WIDTH 128 // OLED display width,  in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define BUTTON_PIN 12
#define POT 34 // Potentiometer
#define LED_RED 27
#define LED_GREEN 26
#define LED_YELLOW 14

// declare an SSD1306 display object connected to I2C
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

String readUltrasonic(){
  // Clears the TRIG_PIN
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  // Sets the TRIG_PIN on HIGH state for 10 micro seconds
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(ECHO_PIN, HIGH);
  
  // Calculate the distance
  distance = duration * SOUND_SPEED/2;
  return String(distance);
}

String readDHTTemperature() {
  // Sensor readings may also be up to 3 seconds  (its a very slow sensor)
  // Read temperature as Celsius (the default)
  float t = dht22.getTemperature();
  if (isnan(t) || t <= 0) {
    return String(t_prev);
  }
  else {
    // Serial.println(t);
    return String(t);
  }
  t_prev = t;
}

String readDHTHumidity() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht22.getHumidity();
  if (isnan(h)) {
    return String(h_prev);
  }
  else {
    // Serial.println(h);
    return String(h);
  }
  h_prev = h;
}

String readPotentiometer(){
  int val = analogRead(POT);
  val = map(val, 0, 4095, 0, 99); // map 12-bit analog value to percentage
  return String(val);
}

String readButton(){
  int val = digitalRead(BUTTON_PIN);
  return String(val);
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>ESP32 Web Server</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="dht-labels">Temperature</span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="dht-labels">Humidity</span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">&percnt;</sup>
  </p>
  <p>
    <i class="fas fa-solid fa-stroopwafel" style="color:#af0d84;"></i> 
    <span class="dht-labels">Ultrasonic</span>
    <span id="ultrasonic">%ULTRASONIC%</span>
    <sup class="units">cm;</sup>
  </p>
  <p>
    <i class="fas fa-screwdriver" style="color:#000000;"></i> 
    <span class="dht-labels">Potentiometer</span>
    <span id="potentiometer">%POTENTIOMETER%</span>
    <sup class="units">&percnt;</sup>
  </p>
  <p>
    <i class="fas fa-solid fa-star" style="color: #fff01a;"></i> 
    <span class="dht-labels">ButtonPress</span>
    <span id="button">%BUTTON%</span>
  </p>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 3000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 3000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("ultrasonic").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/ultrasonic", true);
  xhttp.send();
}, 1000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("potentiometer").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/potentiometer", true);
  xhttp.send();
}, 100 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("button").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/button", true);
  xhttp.send();
}, 10 ) ;

</script>
</html>
)rawliteral";

// Replaces placeholder with DHT values
String processor(const String& var){
  //Serial.println(var);
  if(var == "TEMPERATURE"){
    return readDHTTemperature();
  } else if(var == "HUMIDITY"){
    return readDHTHumidity();
  } else if(var == "POTENTIOMETER"){
    return readPotentiometer();
  } else if(var == "ULTRASONIC"){
    return readUltrasonic();
  } else if(var == "BUTTON"){
    return readButton();
  }
  return String();
}

void hardwareTest(){  
  oled.clearDisplay();  
  float t = dht22.getTemperature();
  float h = dht22.getHumidity();
  oled.setCursor(0, 0);       // set position to display
  oled.print("Humidity: "); // Humidity
  oled.print(h); oled.println("%");
  oled.print("Temperature: "); // set text
  oled.print(t);
  oled.println("*C");

  int currentPotVal = analogRead(POT);
  oled.setCursor(0, 20);
  oled.print("POT value: ");
  oled.println(currentPotVal);

  // Clears the TRIG_PIN
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  // Sets the TRIG_PIN on HIGH state for 10 micro seconds
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  long _duration = pulseIn(ECHO_PIN, HIGH);
  
  // Calculate the distance
  long _distance = _duration * SOUND_SPEED/2.0;
  oled.setCursor(0, 30);
  oled.print("Distance: ");
  oled.print(_distance); oled.print(" cm");

  oled.setCursor(0, 45);
  oled.println("Press Button to test LEDs");

  if (digitalRead(BUTTON_PIN)){
    digitalWrite(18, LOW);
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_YELLOW, LOW);
  } else {
    digitalWrite(18, HIGH);
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_YELLOW, HIGH);
  }
  oled.display();
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(9600);

  pinMode(TRIG_PIN, OUTPUT); // Sets the TRIG_PIN as an Output
  pinMode(ECHO_PIN, INPUT); // Sets the ECHO_PIN as an Input
  pinMode(LED_RED, OUTPUT); // Sets the LED_RED as an Output
  pinMode(LED_GREEN, OUTPUT); // Sets the LED_GREEN as an Output
  pinMode(LED_YELLOW, OUTPUT); // Sets the LED_YELLOW as an Output
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Sets the BUTTON as an Input with internal pullup enabled

  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);

  // initialize OLED display with address 0x3C for 128x64
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  } 
  delay(500);
  oled.setTextSize(1);         // set text size
  oled.setTextColor(WHITE);
  oled.clearDisplay(); // clear display
  oled.setCursor(0, 0);       // set position to display
  oled.println("Albany Senior"); // set text
  oled.println("   High School");
  oled.setCursor(0, 40);       // set position to display
  oled.println("<< ESP32 Activity >>"); // set text
  oled.display();
  delay(3000);

  if (!HARDWARE_TEST){
    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    oled.clearDisplay();
    oled.setCursor(0, 0); oled.println("Connecting to WiFi...");
    oled.display();
    while (WiFi.status() != WL_CONNECTED) {
      oled.clearDisplay();
      delay(100);
      oled.setCursor(0, 0); oled.println("Connected!");
    }

    // Print ESP32 Local IP Address
    // Serial.println(WiFi.localIP());
    // Serial.println(WiFi.macAddress());
    oled.setCursor(0, 10);       // set position to display
    oled.print("IP: "); // set text
    oled.println(WiFi.localIP());
    oled.display();
    delay(2000);

    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", index_html, processor);
    });
    server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", readDHTTemperature().c_str());
    });
    server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", readDHTHumidity().c_str());
    });
    server.on("/ultrasonic", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", readUltrasonic().c_str());
    });
    server.on("/potentiometer", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", readPotentiometer().c_str());
    });
    server.on("/button", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", readButton().c_str());
    });
    // Start server
    server.begin();
  }
}
 
void loop(){
  if (HARDWARE_TEST){
    hardwareTest();
  }
}
