/* DHTServer - ESP8266 Webserver with a DHT sensor as an input

   Based on ESP8266Webserver, DHTexample, and BlinkWithoutDelay (thank you)

   Version 1.0  5/3/2014  Version 1.0   Mike Barela for Adafruit Industries
*/
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DHT.h>
#define DHTTYPE DHT11
#define DHTPIN  2

const char* ssid     = "MicroLab";
const char* password = "microp4ss";

ESP8266WebServer server(80);
 
// Initialize DHT sensor 
// NOTE: For working with a faster than ATmega328p 16 MHz Arduino chip, like an ESP8266,
// you need to increase the threshold for cycle counts considered a 1 or 0.
// You can do this by passing a 3rd parameter for this threshold.  It's a bit
// of fiddling to find the right value, but in general the faster the CPU the
// higher the value.  The default for a 16mhz AVR is a value of 6.  For an
// Arduino Due that runs at 84mhz a value of 30 works.
// This is for the ESP8266 processor on ESP-01 
DHT dht(DHTPIN, DHTTYPE, 11); // 11 works fine for ESP8266
 
float data[5];
float humidity, temp_f, temp_c, hif, hic;  // Values read from sensor
String webString="";     // String to display
// Generally, you should use "unsigned long" for variables that hold time
unsigned long previousMillis = 0;        // will store last temp was read
const long interval = 2000;              // interval at which to read sensor

void handle_update() {
  gettemperature();       // read sensor
  char html[400];
  snprintf ( html, 400,
    "{\
    \"f\":\"%.2f\",\
    \"c\":\"%.2f\",\
    \"hif\":\"%.2f\",\
    \"hic":\"%.2f\",\
    \"humidity\":\"%.2f\",\
    }"\
  , temp_f, temp_c, hif, hic, humidity);
}  
 
void handle_root() {
  gettemperature();       // read sensor
  char html[1200];
  snprintf ( html, 1200,
  
  "<html>\
    <head>\
      <title>ESP8266 Temp & Humidity</title>\
        <style>body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }</style>\
    </head>\
    <script>\
    var tempC = document.getElementById(\"c\");\
    setInterval(myTimer, 2000);\
    function myTimer() {\
        getJSON(\"http://10.0.0.38/updateTemp\");\
    }\
    function getJSON(url) {\
      var xhr = new XMLHttpRequest();\
      xhr.open(\'GET\', url);\
      xhr.responseType = \'text\';\
      xhr.send();\
      xhr.onload = function() {\
      let temp = JSON.parse(xhr.response);\
      assignVars(temp);\
      };\
    }\
    function changeHTML(id,val) {\
      document.getElementById(\"\"+id).innerHTML = val;\
    }\
    function assignVars(jsonObj) {\
        changeHTML(\"f\",jsonObj.f);\
        changeHTML(\"c\",jsonObj.c);\
        changeHTML(\"hif\",jsonObj.hif);\
        changeHTML(\"hic\",jsonObj.hic);\
        changeHTML(\"h\",jsonObj.humidity);\
    }\
    </script>\
    <h1>Hello from the weather ESP8266!</h1><br> The current Temperature is <b id=\"c\">%.2fC</b> <b id =\"f\">%.2fF</b> with <b id=\"h\">%.2f%%</b> Humidity <br> It feels like <b id=\"hic\">%.2fC</b> <b id=\"hif\">%.2fF</b>\
    </html>"

  ,temp_c , temp_c, temp_f, humidity, hic, hif);
  
  server.send(200, "text/html", html);
  delay(100);
}
 
void setup(void)
{
  // You can open the Arduino IDE Serial Monitor window to see what the code is doing
  Serial.begin(115200);  // Serial connection from ESP-01 via 3.3v console cable
  dht.begin();           // initialize temperature sensor

  // Connect to WiFi network
  WiFi.begin(ssid, password);
  Serial.print("\n\r \n\rWorking to connect");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("DHT Weather Reading Server");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  gettemperature();       // read sensor
  
  Serial.print("case1 ");
  Serial.print(F("Humidity: "));
  Serial.print(String((int)data[0]));
  Serial.print(F("%  Temperature: "));
  Serial.print(String((int)data[1]));
  Serial.print(F("°C "));
  Serial.print(String((int)data[2]));
  Serial.print(F("°F  Heat index: "));
  Serial.print(String((int)data[3]));
  Serial.print(F("°C "));
  Serial.print(String((int)data[4]));
  Serial.println(F("°F"));

  Serial.print("case2 ");
  Serial.print(F("Humidity: "));
  Serial.print(humidity);
  Serial.print(F("%  Temperature: "));
  Serial.print(temp_c);
  Serial.print(F("°C "));
  Serial.print(temp_f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));
   
  server.on("/uptime", [](){
    int sec = millis() / 1000;
    int min = sec / 60;
    int hr = min / 60;
    char html[400];
    snprintf ( html, 400,

"<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>ESP8266 Temp & Humidity</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from ESP8266!</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
  </body>\
</html>",

      hr, min % 60, sec % 60
    );
    server.send(200, "text/html", html);
  });

  server.on("/", handle_root);

  server.on("/updateTemp", handle_update);
  
  server.on("/temp", [](){  // if you add this subdirectory to your webserver call, you get text below :)
    gettemperature();       // read sensor
    webString="Temperature: "+String((int)temp_f)+" F";   // Arduino has a hard time with float to string
    server.send(200, "text/plain", webString);            // send to someones browser when asked
  });

  server.on("/humidity", [](){  // if you add this subdirectory to your webserver call, you get text below :)
    gettemperature();           // read sensor
    webString="Humidity: "+String((int)humidity)+"%";
    server.send(200, "text/plain", webString);               // send to someones browser when asked
  });
  
  server.begin();
  Serial.println("HTTP server started");
}
 
void loop(void)
{
  server.handleClient();
} 

void gettemperature() {
  // Wait at least 2 seconds seconds between measurements.
  // if the difference between the current time and last time you read
  // the sensor is bigger than the interval you set, read the sensor
  // Works better than delay for things happening elsewhere also
  unsigned long currentMillis = millis();
 
  if(currentMillis - previousMillis >= interval) {
    // save the last time you read the sensor 
    previousMillis = currentMillis;   

    // Reading temperature for humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
    humidity = dht.readHumidity();          // Read humidity (percent)
    temp_f = dht.readTemperature(true);     // Read temperature as Fahrenheit
    temp_c = dht.readTemperature();
    // Compute heat index in Fahrenheit (the default)
    hif = dht.computeHeatIndex(temp_f, humidity);
    // Compute heat index in Celsius (isFahreheit = false)
    hic = dht.computeHeatIndex(temp_c, humidity, false);
    data[0] = humidity;
    //data[] = {humidity,temp_c,temp_f,hic,hif};

    Serial.print("case3 ");
    Serial.print(F("Humidity: "));
    Serial.print(String((int)data[0]));
    Serial.print(F("%  Temperature: "));
    Serial.print(String((int)data[1]));
    Serial.print(F("°C "));
    Serial.print(String((int)data[2]));
    Serial.print(F("°F  Heat index: "));
    Serial.print(String((int)data[3]));
    Serial.print(F("°C "));
    Serial.print(String((int)data[4]));
    Serial.println(F("°F"));
    
    // Check if any reads failed and exit early (to try again).
    if (isnan(humidity) || isnan(temp_f)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
  }
}

