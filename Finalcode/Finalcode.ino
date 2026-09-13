#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h> 

#define SEALEVELPRESSURE_HPA (1013.25)

#define AOUT_PIN 33 // ESP32 pin GIOP36 (ADC0) that connects to AOUT pin of moisture sensor

#define FAN_PIN 16 // ESP32 pin for fan control
#define PUMP_PIN 14 // ESP32 pin for water pump control

Adafruit_BME280 bme; // I2C
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
unsigned long delayTime;

const char* ssid     = "------------";
const char* password = "------------";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(9600);
  Serial.println(F("BME280, soil moisture, and control test"));

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("BME280, soil");
  display.println("moisture, and");
  display.println("control test");
  display.display();
  delay(5000);
  display.clearDisplay();
  display.display();

  pinMode(FAN_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);

  bool status;
  status = bme.begin(0x76);
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  delay(5000);

  //bool status;

  // default settings
  // (you can also pass in a Wire library object like &Wire2)
  //status = bme.begin();  
  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop() {
  float temperature = bme.readTemperature();
  float pressure = bme.readPressure() / 100.0F;
  float humidity = bme.readHumidity();
  int soil_moisture = analogRead(AOUT_PIN);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Temp: ");
  display.print(temperature);
  display.println(" *C");
  display.print("Pressure: ");
  display.print(pressure);
  display.println(" hPa");
  display.print("Humidity: ");
  display.print(humidity);
  display.println(" %");
  display.print("Soil moisture: ");
  display.println(soil_moisture);
  display.display();

  if (temperature >= 30) { // if temperature is above 30 degrees Celsius
    digitalWrite(FAN_PIN, HIGH); // turn on the fan
    delay(20000); // wait for 20 seconds
    digitalWrite(FAN_PIN, LOW); // turn off the fan
  }

  if (soil_moisture > 2470) { // if soil moisture is below 500
    digitalWrite(PUMP_PIN, HIGH); // turn on the water pump
    delay(5000); // wait for 5 seconds
    digitalWrite(PUMP_PIN, LOW); // turn off the water pump
  }

  Serial.print("Temperature = "); Serial.print(temperature); Serial.println(" *C");
  Serial.print("Pressure = "); Serial.print(pressure); Serial.println(" hPa");
  Serial.print("Humidity = "); Serial.print(humidity); Serial.println(" %");
  Serial.print("Soil moisture = "); Serial.println(soil_moisture);

  delay(1000);

  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the table 
            client.println("<style>body { text-align: center; font-family: \"Trebuchet MS\", Arial;}");
            client.println("table { border-collapse: collapse; width:35%; margin-left:auto; margin-right:auto; }");
            client.println("th { padding: 12px; background-color: #0043af; color: white; }");
            client.println("tr { border: 1px solid #ddd; padding: 12px; }");
            client.println("tr:hover { background-color: #bcbcbc; }");
            client.println("td { border: none; padding: 12px; }");
            client.println(".sensor { color:white; font-weight: bold; background-color: #bcbcbc; padding: 1px; }");
            
            // Web Page Heading
            client.println("</style></head><body><h1>ESP32 Greenhouse</h1>");
            client.println("<table><tr><th>MEASUREMENT</th><th>VALUE</th></tr>");
            client.println("<tr><td>Temp. Celsius</td><td><span class=\"sensor\">");
            client.println(temperature);
            client.println(" *C</span></td></tr>");         
            client.println("<tr><td>Pressure</td><td><span class=\"sensor\">");
            client.println(pressure);
            client.println(" hPa</span></td></tr>");
            client.println("<tr><td>Soil moisture</td><td><span class=\"sensor\">");
            client.println(soil_moisture);
            client.println(" m</span></td></tr>"); 
            client.println("<tr><td>Humidity</td><td><span class=\"sensor\">");
            client.println(humidity);
            client.println(" %</span></td></tr>"); 
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
