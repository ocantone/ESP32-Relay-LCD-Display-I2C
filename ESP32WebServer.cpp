/*********
  ESP32 Web server
  Implementa un servidor web al que se accede mediante una conección WiFi.
  Incorpora un lcd H44780 conectado via I2C bus. La dirección IP obtenida via 
  protocolo DHCP se muestra en el display. 
  También muestra el estado de los relé.
  Este sketch está basado en el de Rui Santos https://randomnerdtutorials.com 
  Editado por mí en Ramos Mejía, ARGENTINA, Agosto 2020.
  - Osvaldo Cantone  correo@cantone.com.ar
  PINOUT
  -reles:
    Relé1 GPIO.15
    Relé2 GPIO.2
  -display:
    SDA GPIO.21
    SCL GPIO.22
*********/

#include <Arduino.h>
#include <WiFi.h>
#include <LiquidCrystal_PCF8574.h>
#include <Wire.h>

LiquidCrystal_PCF8574 lcd(0x27); // set the LCD address to 0x27 for a 16 chars and 2 line display


// Reemplazar con los datos de tu WiFi
const char* ssid = "AQUÍ VA EL NOMBRE (SSID) DE LA WIFI";
const char* password = "AQUÍ VA TU CLAVE DE WIFI";

// Setea el web server para que escuche el puerto 80
WiFiServer server(80);

// Esta variable almacena la HTTP request
String header;

// Estado de las saldas digitales
String output26State = "off";
String output27State = "off";

// Numero de gpio de las salidas usadas.
const int output26 = 15;
const int output27 = 2;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;
int show = -1;

void setup() {

  int error;

  Serial.begin(9600);
  // Inicializa las salidas.
  pinMode(output26, OUTPUT);
  pinMode(output27, OUTPUT);
  // PONE SALIDAS EN HIGH
  digitalWrite(output26, HIGH);
  digitalWrite(output27, HIGH);
  
  //Initializing the display
Wire.begin();
  Wire.beginTransmission(0x27);
  error = Wire.endTransmission();
  Serial.print("Error: ");
  Serial.print(error);

  if (error == 0) {
    Serial.println(": LCD found.");
    show = 0;
    lcd.begin(16, 2); // initialize the lcd
 } else {
    Serial.println(": LCD not found.");
  } // if
    lcd.setBacklight(255);
    lcd.home();
    lcd.clear();
    lcd.setCursor(0, 0);
//  lcd.print("0123456789ABCDEF");
    lcd.print(" ESP32 WebSerer ");
    

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

    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
delay(2000);
    lcd.setCursor(0, 0);
    lcd.print("ESP32 ");
    lcd.print(WiFi.localIP());
    lcd.setCursor(0, 1);
    lcd.print("R1: ON   R2: ON ");


  server.begin();
}

void loop(){
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
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /26/on") >= 0) {
              Serial.println("GPIO 26 on");
              output26State = "on";
              digitalWrite(output26, HIGH);
                lcd.setCursor(4, 1);
                lcd.print("OFF");

            } else if (header.indexOf("GET /26/off") >= 0) {
              Serial.println("GPIO 26 off");
              output26State = "off";
              digitalWrite(output26, LOW);
                lcd.setCursor(4, 1);
                lcd.print("ON ");

            } else if (header.indexOf("GET /27/on") >= 0) {
              Serial.println("GPIO 27 on");
              output27State = "on";
              digitalWrite(output27, HIGH);
                lcd.setCursor(13, 1);
                lcd.print("OFF");
            } else if (header.indexOf("GET /27/off") >= 0) {
              Serial.println("GPIO 27 off");
              output27State = "off";
              digitalWrite(output27, LOW);
                lcd.setCursor(13, 1);
                lcd.print("ON ");
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
                        // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            


            // Web Page Heading
            client.println("<body><h1>ESP32 Web Server</h1>");
            client.println("<p>Adapted by Osvaldo Cantone <a href=https://github.com/ocantone/ESP32-Relay-LCD-Display-I2C> GItHub/ocantone </a> </p>");
            
            // Display current state, and ON/OFF buttons for GPIO 15 
            client.println("<p>GPIO 26 - State " + output26State + "</p>");
            // If the output26State is off, it displays the ON button       
            if (output26State=="off") {
              client.println("<p><a href=\"/26/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/26/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
               
            // Display current state, and ON/OFF buttons for GPIO 2 
            client.println("<p>GPIO 27 - State " + output27State + "</p>");
            // If the output27State is off, it displays the ON button       
            if (output27State=="off") {
              client.println("<p><a href=\"/27/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/27/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
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
