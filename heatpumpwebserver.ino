#include <Arduino.h>
#include "config.h"
#include <HardwareSerial.h>
#include <ModbusMaster.h>
#include <SPI.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include "Freenove_WS2812_Lib_for_ESP32.h"

#define LEDS_COUNT 1
#define LEDS_PIN 4
#define CHANNEL 0

Freenove_ESP32_WS2812 strip = Freenove_ESP32_WS2812(LEDS_COUNT, LEDS_PIN, CHANNEL, TYPE_GRB);

AsyncWebServer server(80);

const char* PARAM_INT_TANKhigh = "inputTANKhigh";
const char* PARAM_INT_TANKlow = "inputTANKlow";

const char* PARAM_INT_UpperATheat = "inputUpperATheat";
const char* PARAM_INT_LowerATheat = "inputLowerATheat";
const char* PARAM_INT_UpperWTheat = "inputUpperWTheat";
const char* PARAM_INT_LowerWTheat = "inputLowerWTheat";

//Login gegevens van AP
const char* ssid = "TEST";
const char* password = "TEST2022";

//Login gegevens voor website
const char* http_username = "user";
const char* http_password = "user";

const int output = 2;

HardwareSerial Serial485(2);
ModbusMaster node;

// set pin numbers
const int buttonPin = 18;  // the number of the pushbutton pin

// variable for storing the pushbutton status
int buttonState = 0;

//////// Modbusregisters ////////
int RegisterTANK = 13;
int RegisterUpperATheat = 17;
int RegisterLowerATheat = 18;
int RegisterUpperWTheat = 21;
int RegisterLowerWTheat = 22;
int RegisterWeatherdepend;

// Register 2 = MODE: 1 = Heat, 2 = HotWater, 3 = Cool+HotWater, 4 = Heat+HotWater, 5 = Cool
// Koeling blokkeren: Mode mag 1 2 4 zijn. Mode mag niet 3 of 5 zijn.
int RegisterMODE = 2;

////////////////////////////////

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

// HTML web page
const char index_html[] PROGMEM = R"rawliteral(

<!DOCTYPE HTML>
<html>
    <head>
        <title>Heatpump Input Form</title>
        <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
        <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
        <style>
            html {
                font-family: Helvetica;
                display: inline-block;
                margin: 0px auto;
                text-align: center;
            }

            //button {background-color: #4CAF50; border: none; color: white; padding: 16px 40px; text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}
            //button2 {background-color: #555555;}
            .topnav {
                overflow: hidden;
                background-color: #2f4468;
                color: white;
                font-size: 1rem;
            }

            .content {
                padding: 20px;
            }

            .card {
                background-color: white;
                box-shadow: 2px 2px 12px 1px rgba(140, 140, 140, .5);
            }

            .cards {
                max-width: 700px;
                margin: 0 auto;
                display: grid;
                grid-gap: 2rem;
                grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            }

            .reading {
                font-size: 2.8rem;
            }

            .packet {
                color: #bebebe;
            }

            .card.temperaturehigh {
                color: red;
            }

            .card.temperaturelow {
                color: blue;
            }

            <span class="bi bi-thermometer"></span>.button {
                background-color: #4CAF50;
                /* Green */
                border: none;
                color: white;
                padding: 15px 32px;
                text-align: center;
                text-decoration: none;
                display: inline-block;
                font-size: 16px;
                margin: 4px 2px;
                cursor: pointer;
            }

            .button2 {
                background-color: #008CBA;
            }

            /* Blue */
            .button3 {
                background-color: #f44336;
            }

            /* Red */
            .button4 {
                background-color: #e7e7e7;
                color: black;
            }

            /* Gray */
            .button5 {
                background-color: #555555;
            }

            /* Black */
            input[type='number'] {
                width: 50px;
            }
        </style>
        <script>
            function submitMessage() {
                alert("Instelling word gecontrolleerd en doorgestuurd bij geen fout");
                setTimeout(function() {
                    document.location.reload(false);
                }, 2000);
            }
function logoutButton() {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/logout", true);
  xhr.send();
  setTimeout(function(){ window.open("/logged-out","_self"); }, 1000);
}
        </script>
    </head>
    <body>
    
               <div class="topnav">
            <h1>HEATPUMP</h1>
        </div>
        <div class="content">
            <div class="cards">
                <div class="card temperature">
                    <h4>&#127777; Smart-grid SWW</h4>
                    <form action="/get" target="hidden-form"> closed contact: <input type="number" name="inputTANKhigh" min="30" max="60" value="%inputTANKhigh%"> °C <input type="submit" value="Opslaan" onclick="submitMessage()">
                    </form>
                    <br>
                    <form action="/get" target="hidden-form"> open contact: <input type="number" name="inputTANKlow" min="30" max="60" value="%inputTANKlow%"> °C <input type="submit" value="Opslaan" onclick="submitMessage()">
                    </form>
                    <br>
                </div>

                <div class="card temperature">
                    <h4>&#127777; Weather depend</h4>
                    <form action="/get" target="hidden-form"> Upper AT-heat: <input type="number" name="inputUpperATheat" min="10" max="37" value="%inputUpperATheat%"> °C <input type="submit" value="Opslaan" onclick="submitMessage()">
                    </form>
                    <br>
                    <form action="/get" target="hidden-form"> Lower AT-heat: <input type="number" name="inputLowerATheat" min="-20" max="9" value="%inputLowerATheat%"> °C <input type="submit" value="Opslaan" onclick="submitMessage()">
                    </form>
                    <br>
                    <form action="/get" target="hidden-form"> Upper WT-heat: <input type="number" name="inputUpperWTheat" min="20" max="60" value="%inputUpperWTheat%"> °C <input type="submit" value="Opslaan" onclick="submitMessage()">
                    </form>
                    <br>
                    <form action="/get" target="hidden-form"> Lower WT-heat: <input type="number" name="inputLowerWTheat" min="20" max="60" value="%inputLowerWTheat%"> °C <input type="submit" value="Opslaan" onclick="submitMessage()">
                    </form>
                    <br>
                </div>
            </div>
            <br><button onclick="logoutButton()">Logout</button>
        </div>
        <iframe style="display:none" name="hidden-form"></iframe>

</body></html>)rawliteral";

void notFound(AsyncWebServerRequest* request) {
  request->send(404, "text/plain", "Not found");
}

String readFile(fs::FS& fs, const char* path) {
  //  Serial.printf("Reading file: %s\r\n", path);
  File file = fs.open(path, "r");
  if (!file || file.isDirectory()) {
    Serial.println("- empty file or failed to open file");
    return String();
  }
  //  Serial.println("- read from file:");
  String fileContent;
  while (file.available()) {
    fileContent += String((char)file.read());
  }
  file.close();
  //  Serial.println(fileContent);
  return fileContent;
}

void writeFile(fs::FS& fs, const char* path, const char* message) {
  Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, "w");
  if (!file) {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
  file.close();
}

// Replaces placeholder with stored values
String processor(const String& var) {
  //Serial.println(var);

  // Files voor tank
  if (var == "inputTANKhigh") {
    return readFile(SPIFFS, "/inputTANKhigh.txt");
  } else if (var == "inputTANKlow") {
    return readFile(SPIFFS, "/inputTANKlow.txt");

    // Files voor AT en WT HEAT
  } else if (var == "inputUpperATheat") {
    return readFile(SPIFFS, "/inputUpperATheat.txt");
  } else if (var == "inputLowerATheat") {
    return readFile(SPIFFS, "/inputLowerATheat.txt");
  } else if (var == "inputUpperWTheat") {
    return readFile(SPIFFS, "/inputUpperWTheat.txt");
  } else if (var == "inputLowerWTheat") {
    return readFile(SPIFFS, "/inputLowerWTheat.txt");

  } else {
    return "OFF";
  }


  return String();
}

const char logout_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body>
  <p>Logged out or <a href="/">return to homepage</a>.</p>
  <p><strong>Note:</strong> close all web browser tabs to complete the logout process.</p>
</body>
</html>
)rawliteral";

void setup() {
  pinMode(RS485_EN_PIN, OUTPUT);
  digitalWrite(RS485_EN_PIN, HIGH);

  pinMode(RS485_SE_PIN, OUTPUT);
  digitalWrite(RS485_SE_PIN, HIGH);

  pinMode(PIN_5V_EN, OUTPUT);
  digitalWrite(PIN_5V_EN, HIGH);

  Serial.begin(9600);
  Serial485.begin(9600, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);
  delay(5);
  Serial.println("test");
  //Slave id = 2
  node.begin(2, Serial485);
  // initialize the pushbutton pin as an input
  pinMode(buttonPin, INPUT);

  // Initialize SPIFFS
#ifdef ESP32
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
#else
  if (!SPIFFS.begin()) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
#endif

  // Connect to Wi-Fi network with SSID and password
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    return;
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP()); */

// Stel een AP op
  Serial.println("Configuring access point...");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  Serial.println("Wait 100 ms for AP_START...");
  delay(100);

  Serial.println("Set softAPConfig");
  IPAddress Ip(192, 168, 1, 1);
  IPAddress NMask(255, 255, 255, 0);
  WiFi.softAPConfig(Ip, Ip, NMask);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  ////////////////////////////////////////////////////////

  // Send web page with input fields to client
  /*  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html, processor);
  });
*/
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/logout", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(401);
  });

  server.on("/logged-out", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", logout_html, processor);
  });

  // Send a GET request to <ESP_IP>/get?inputTANKhigh=<inputMessage>
  server.on("/get", HTTP_GET, [](AsyncWebServerRequest* request) {
    String inputMessage;
    if (request->hasParam(PARAM_INT_TANKhigh)) {
      inputMessage = request->getParam(PARAM_INT_TANKhigh)->value();
      writeFile(SPIFFS, "/inputTANKhigh.txt", inputMessage.c_str());
    } else if (request->hasParam(PARAM_INT_TANKlow)) {
      inputMessage = request->getParam(PARAM_INT_TANKlow)->value();
      writeFile(SPIFFS, "/inputTANKlow.txt", inputMessage.c_str());
    } else if (request->hasParam(PARAM_INT_UpperATheat)) {
      inputMessage = request->getParam(PARAM_INT_UpperATheat)->value();
      writeFile(SPIFFS, "/inputUpperATheat.txt", inputMessage.c_str());
    } else if (request->hasParam(PARAM_INT_LowerATheat)) {
      inputMessage = request->getParam(PARAM_INT_LowerATheat)->value();
      writeFile(SPIFFS, "/inputLowerATheat.txt", inputMessage.c_str());
    } else if (request->hasParam(PARAM_INT_UpperWTheat)) {
      inputMessage = request->getParam(PARAM_INT_UpperWTheat)->value();
      writeFile(SPIFFS, "/inputUpperWTheat.txt", inputMessage.c_str());
    } else if (request->hasParam(PARAM_INT_LowerWTheat)) {
      inputMessage = request->getParam(PARAM_INT_LowerWTheat)->value();
      writeFile(SPIFFS, "/inputLowerWTheat.txt", inputMessage.c_str());
    } else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    request->send(200, "text/text", inputMessage);
  });
  server.onNotFound(notFound);
  server.begin();
  strip.begin();
  strip.setBrightness(5);
}

void loop() {

  //Lees status uit opgeslagen files
  int HighTemp = readFile(SPIFFS, "/inputTANKhigh.txt").toInt();
  Serial.print("*** Your inputTANKhigh: ");
  Serial.println(HighTemp);

  int LowTemp = readFile(SPIFFS, "/inputTANKlow.txt").toInt();
  Serial.print("*** Your inputTANKlow: ");
  Serial.println(LowTemp);

  int UpperATheat = readFile(SPIFFS, "/inputUpperATheat.txt").toInt();
  Serial.print("*** Your inputUpperATheat: ");
  Serial.println(UpperATheat);

  int LowerATheat = readFile(SPIFFS, "/inputLowerATheat.txt").toInt();
  Serial.print("*** Your inputLowerATheat: ");
  Serial.println(LowerATheat);

  int UpperWTheat = readFile(SPIFFS, "/inputUpperWTheat.txt").toInt();
  Serial.print("*** Your inputUpperWTheat: ");
  Serial.println(UpperWTheat);

  int LowerWTheat = readFile(SPIFFS, "/inputLowerWTheat.txt").toInt();
  Serial.print("*** Your LowerWTheat: ");
  Serial.println(LowerWTheat);

  //Functie zonnepanelen AAN of UIT
  // read the state of the pushbutton value
  buttonState = digitalRead(buttonPin);
  // check if the pushbutton is pressed.
  // if it is, the buttonState is HIGH
  if (buttonState == HIGH) {
    // turn LED GREEN
    strip.setLedColor(0, 0, 255, 0);
    Serial.println("Contact gesloten");
    node.writeSingleRegister(RegisterTANK, HighTemp);
  } else {
    // turn LED RED
    strip.setLedColor(0, 255, 0, 0);
    Serial.println("Contact open");
    node.writeSingleRegister(RegisterTANK, LowTemp);
  }
  
  node.writeSingleRegister(RegisterUpperATheat, UpperATheat);
  node.writeSingleRegister(RegisterLowerATheat, LowerATheat);
  node.writeSingleRegister(RegisterUpperWTheat, UpperWTheat);
  node.writeSingleRegister(RegisterLowerWTheat, LowerWTheat);

  delay(5000);
}
