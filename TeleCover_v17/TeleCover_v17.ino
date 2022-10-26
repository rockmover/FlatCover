/*********
  David Kruse
  Telescope Automatic Cover - v1.7
*********/

// Load Wi-Fi library
#include <ESP8266WiFi.h>
#include <Pinger.h>

extern "C"
{
# include <lwip/icmp.h> // needed for icmp packet definitions
}

// Set global to avoid object removing after setup() routine
Pinger pinger;

//Servo Library
#include <Servo.h>

const int lbDeg0 = 8;  //14  is on, so 8 is off but not all the way to zero (safer)
const int lbDeg1 = 14;  
const int lbDeg2 = 15;  //25
const int lbDeg3 = 16;
const int lbDeg4 = 17;
const int lbDeg5 = 18;
const int lbDeg6 = 19;
const int lbDeg7 = 20;
const int lbDeg8 = 21;
const int lbDeg9 = 25;
const int lbDeg10 = 30;
const int lbDeg11 = 35;
const int lbDeg12 = 40;
const int lbDeg13 = 45;
const int lbDeg14 = 50;
const int lbDeg15 = 60;
const int lbDeg16 = 75;
const int lbDeg17 = 90;
const int lbDeg18 = 100;
const int lbDeg19 = 110;


const int lbVolt0 = 0;
const int lbVolt1 = 1;
const int lbVolt2 = 2;
const int lbVolt3 = 4;
const int lbVolt4 = 8;
const int lbVolt5 = 10;
const int lbVolt6 = 15;
const int lbVolt7 = 25;
const int lbVolt8 = 50;
const int lbVolt9 = 75;
const int lbVolt10 = 100;
const int lbVolt11 = 200;
const int lbVolt12 = 300;
const int lbVolt13 = 400;
const int lbVolt14 = 500;
const int lbVolt15 = 600;
const int lbVolt16 = 700;
const int lbVolt17 = 800;
const int lbVolt18 = 900;
const int lbVolt19 = 1000;

const int c1DegOpen = 0;
const int c1DegClose = 180;

const int coverDelay = 50;

// Replace with your network credentials
const char* ssid = "WiFiName";
const char* password = "wiFiPassword";

// Auxiliar variables to store the current output state
String tCover1Position = "Unknown";
String lBoardLevel = "Unknown";
String lBoardSelect = "Unknown";
String roofRestPosition = "Reset2";


// Assign output variables to GPIO pins
const int outputCover1Pin = 5;  // SV130 Servo
const int outputLightboardPin = 14;   //C11 - Light Board

const int outputLevelPin = 12;  //SV130 Brightness
const int outputLBoardRelay1Pin = 0; //Roof Reset
const int outputLBoardRelay2Pin = 4; //Open

//Init Servos
Servo servoCover1;
Servo servoLboard;

// Current time
unsigned long currentTime = millis();
unsigned long PowerOnTime = millis();

// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Set your Static IP address
IPAddress local_IP(192, 168, 1, 36);
IPAddress subnet(255, 255, 255, 0);
IPAddress gateway(192, 168, 115, 1);
IPAddress primaryDNS(192, 168, 1, 1);   //optional
IPAddress secondaryDNS(8, 8, 8, 8); //optional


void setup()
{
    //Serial.begin(9600);

    //Attach Servos
    servoLboard.attach(outputLevelPin, 500, 2500);

    // Initialize the output variables as outputs
    pinMode(outputCover1Pin, OUTPUT);
    pinMode(outputLightboardPin, OUTPUT);
    pinMode(outputLevelPin, OUTPUT);

    pinMode(outputLBoardRelay1Pin, OUTPUT);
    pinMode(outputLBoardRelay2Pin, OUTPUT);

    //  Set outputs to LOW
    digitalWrite(outputLBoardRelay1Pin, LOW);
    digitalWrite(outputLBoardRelay1Pin, LOW);

    // Configures static IP address
    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS))
    {
        //Serial.println("STA Failed to configure");
    }

    // Connect to Wi-Fi network with SSID and password
    //Serial.print("Connecting to ");
    //Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        //Serial.print(".");
    }
    // Print local IP address and start web server
    //Serial.println("");
    //Serial.println("WiFi connected.");
    //Serial.println("IP address: ");
    //Serial.println(WiFi.localIP());
    server.begin();

}


void loop()
{
    delay(1000);

    WiFiClient client = server.available();   // Listen for incoming clients

    if (client)
    {                             // If a new client connects,
                                  //Serial.println("New Client.");          // print a message out in the serial port
        String currentLine = "";                // make a String to hold incoming data from the client
        currentTime = millis();
        previousTime = currentTime;

        while (client.connected() && currentTime - previousTime <= timeoutTime)
        { // loop while the client's connected
            currentTime = millis();

            if (client.available())
            {             // if there's bytes to read from the client,
                char c = client.read();             // read a byte, then
                                                    //Serial.write(c);                    // print it out the serial monitor
                header += c;
                if (c == '\n')
                {                    // if the byte is a newline character
                                     // if the current line is blank, you got two newline characters in a row.
                                     // that's the end of the client HTTP request, so send a response:
                    if (currentLine.length() == 0)
                    {
                        // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                        // and a content-type so the client knows what's coming, then a blank line:
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println("Connection: close");
                        client.println();

                        ///////////////////////////////////////////////////////////////////////////////////////////////////
                        // Lightboard Selection
                        ///////////////////////////////////////////////////////////////////////////////////////////////////
                        if (header.indexOf("GET /c3a/Off") >= 0)
                        {
                            lBoardSelect = "Off";

                            //Turn off C-11 Lightboard (just in case it was on)
                            analogWrite(outputLightboardPin,0);
                        }
                        else if (header.indexOf("GET /c3b/SV-130") >= 0)
                        {
                            lBoardSelect = "SV-130";

                            //Turn off C-11 Lightboard (just in case it was on)
                            analogWrite(outputLightboardPin,0);
                        }
                        else if (header.indexOf("GET /c3c/C-11") >= 0)
                        {
                            lBoardSelect = "C-11";
                        }
                        ///////////////////////////////////////////////////////////////////////////////////////////////////
                        ///////////////////////////////////////////////////////////////////////////////////////////////////


                        ///////////////////////////////////////////////////////////////////////////////////////////////////
                        // LIGHT LEVEL - Servo Control
                        ///////////////////////////////////////////////////////////////////////////////////////////////////
                        if (lBoardSelect == "SV-130")
                        {

                            if (header.indexOf("GET /0/on") >= 0)
                            {
                                lBoardLevel = "0";
                                servoLboard.write(lbDeg0);
                            }
                            else if (header.indexOf("GET /1/on") >= 0)
                            {
                                lBoardLevel = "1";
                                servoLboard.write(lbDeg1);
                            }
                            else if (header.indexOf("GET /2/on") >= 0)
                            {
                                lBoardLevel = "2";
                                servoLboard.write(lbDeg2);
                            }
                            else if (header.indexOf("GET /3/on") >= 0)
                            {
                                lBoardLevel = "3";
                                servoLboard.write(lbDeg3);
                            }
                            else if (header.indexOf("GET /4/on") >= 0)
                            {
                                lBoardLevel = "4";
                                servoLboard.write(lbDeg4);
                            }
                            else if (header.indexOf("GET /5/on") >= 0)
                            {
                                lBoardLevel = "5";
                                servoLboard.write(lbDeg5);
                            }
                            else if (header.indexOf("GET /6/on") >= 0)
                            {
                                lBoardLevel = "6";
                                servoLboard.write(lbDeg6);
                            }
                            else if (header.indexOf("GET /7/on") >= 0)
                            {
                                lBoardLevel = "7";
                                servoLboard.write(lbDeg7);
                            }
                            else if (header.indexOf("GET /8/on") >= 0)
                            {
                                lBoardLevel = "8";
                                servoLboard.write(lbDeg8);
                            }
                            else if (header.indexOf("GET /9/on") >= 0)
                            {
                                lBoardLevel = "9";
                                servoLboard.write(lbDeg9);
                            }

                            if (header.indexOf("GET /10/on") >= 0)
                            {
                                lBoardLevel = "10";
                                servoLboard.write(lbDeg10);
                            }
                            else if (header.indexOf("GET /11/on") >= 0)
                            {
                                lBoardLevel = "11";
                                servoLboard.write(lbDeg11);
                            }
                            else if (header.indexOf("GET /12/on") >= 0)
                            {
                                lBoardLevel = "12";
                                servoLboard.write(lbDeg12);
                            }
                            else if (header.indexOf("GET /13/on") >= 0)
                            {
                                lBoardLevel = "13";
                                servoLboard.write(lbDeg13);
                            }
                            else if (header.indexOf("GET /14/on") >= 0)
                            {
                                lBoardLevel = "14";
                                servoLboard.write(lbDeg14);
                            }
                            else if (header.indexOf("GET /15/on") >= 0)
                            {
                                lBoardLevel = "15";
                                servoLboard.write(lbDeg15);
                            }
                            else if (header.indexOf("GET /16/on") >= 0)
                            {
                                lBoardLevel = "16";
                                servoLboard.write(lbDeg16);
                            }
                            else if (header.indexOf("GET /17/on") >= 0)
                            {
                                lBoardLevel = "17";
                                servoLboard.write(lbDeg17);
                            }
                            else if (header.indexOf("GET /18/on") >= 0)
                            {
                                lBoardLevel = "18";
                                servoLboard.write(lbDeg18);
                            }
                            else if (header.indexOf("GET /19/on") >= 0)
                            {
                                lBoardLevel = "19";
                                servoLboard.write(lbDeg19);
                            }
                        }
                        ///////////////////////////////////////////////////////////////////////////////////////////////////
                        ///////////////////////////////////////////////////////////////////////////////////////////////////



                        ///////////////////////////////////////////////////////////////////////////////////////////////////
                        // LIGHT LEVEL - Voltage Control
                        ///////////////////////////////////////////////////////////////////////////////////////////////////
                        if (lBoardSelect == "C-11")
                        {

                            if (header.indexOf("GET /0/on") >= 0)
                            {
                                lBoardLevel = "0";
                                analogWrite(outputLightboardPin,lbVolt0);
                            }
                            else if (header.indexOf("GET /1/on") >= 0)
                            {
                                lBoardLevel = "1";
                                analogWrite(outputLightboardPin,lbVolt1);
                            }
                            else if (header.indexOf("GET /2/on") >= 0)
                            {
                                lBoardLevel = "2";
                                analogWrite(outputLightboardPin,lbVolt2);
                            }
                            else if (header.indexOf("GET /3/on") >= 0)
                            {
                                lBoardLevel = "3";
                                analogWrite(outputLightboardPin,lbVolt3);
                            }
                            else if (header.indexOf("GET /4/on") >= 0)
                            {
                                lBoardLevel = "4";
                                analogWrite(outputLightboardPin,lbVolt4);
                            }
                            else if (header.indexOf("GET /5/on") >= 0)
                            {
                                lBoardLevel = "5";
                                analogWrite(outputLightboardPin,lbVolt5);
                            }
                            else if (header.indexOf("GET /6/on") >= 0)
                            {
                                lBoardLevel = "6";
                                analogWrite(outputLightboardPin,lbVolt6);
                            }
                            else if (header.indexOf("GET /7/on") >= 0)
                            {
                                lBoardLevel = "7";
                                analogWrite(outputLightboardPin,lbVolt7);
                            }
                            else if (header.indexOf("GET /8/on") >= 0)
                            {
                                lBoardLevel = "8";
                                analogWrite(outputLightboardPin,lbVolt8);
                            }
                            else if (header.indexOf("GET /9/on") >= 0)
                            {
                                lBoardLevel = "9";
                                analogWrite(outputLightboardPin,lbVolt9);
                            }

                            if (header.indexOf("GET /10/on") >= 0)
                            {
                                lBoardLevel = "10";
                                analogWrite(outputLightboardPin,lbVolt10);
                            }
                            else if (header.indexOf("GET /11/on") >= 0)
                            {
                                lBoardLevel = "11";
                                analogWrite(outputLightboardPin,lbVolt11);
                            }
                            else if (header.indexOf("GET /12/on") >= 0)
                            {
                                lBoardLevel = "12";
                                analogWrite(outputLightboardPin,lbVolt12);
                            }
                            else if (header.indexOf("GET /13/on") >= 0)
                            {
                                lBoardLevel = "13";
                                analogWrite(outputLightboardPin,lbVolt13);
                            }
                            else if (header.indexOf("GET /14/on") >= 0)
                            {
                                lBoardLevel = "14";
                                analogWrite(outputLightboardPin,lbVolt14);
                            }
                            else if (header.indexOf("GET /15/on") >= 0)
                            {
                                lBoardLevel = "15";
                                analogWrite(outputLightboardPin,lbVolt15);
                            }
                            else if (header.indexOf("GET /16/on") >= 0)
                            {
                                lBoardLevel = "16";
                                analogWrite(outputLightboardPin,lbVolt16);
                            }
                            else if (header.indexOf("GET /17/on") >= 0)
                            {
                                lBoardLevel = "17";
                                analogWrite(outputLightboardPin,lbVolt17);
                            }
                            else if (header.indexOf("GET /18/on") >= 0)
                            {
                                lBoardLevel = "18";
                                analogWrite(outputLightboardPin,lbVolt18);
                            }
                            else if (header.indexOf("GET /19/on") >= 0)
                            {
                                lBoardLevel = "19";
                                analogWrite(outputLightboardPin,lbVolt19);
                            }
                        }
                        ///////////////////////////////////////////////////////////////////////////////////////////////////
                        ///////////////////////////////////////////////////////////////////////////////////////////////////


                        ///////////////////////////////////////////////////////////////////////////////////////////////////
                        // SV-130 Cover
                        ///////////////////////////////////////////////////////////////////////////////////////////////////
                        if (header.indexOf("GET /c1a/Open") >= 0)
                        {
                            tCover1Position = "Open";

                            servoCover1.write(c1DegClose);
                            servoCover1.attach(outputCover1Pin, 500, 2500);

                            for (int n = c1DegClose; n > c1DegOpen; n = n - 1)
                            {
                                servoCover1.write(n);
                                delay(coverDelay);
                            }

                            servoCover1.detach();
                        }
                        else if (header.indexOf("GET /c1b/Close") >= 0)
                        {
                            tCover1Position = "Close";

                            servoCover1.write(c1DegOpen);
                            servoCover1.attach(outputCover1Pin, 500, 2500);

                            servoCover1.write(c1DegClose);
                            for (int n = c1DegOpen; n < c1DegClose; n++)
                            {
                                servoCover1.write(n);
                                delay(coverDelay);
                            }

                            servoCover1.detach();
                        }
                        ///////////////////////////////////////////////////////////////////////////////////////////////////
                        ///////////////////////////////////////////////////////////////////////////////////////////////////


                        ///////////////////////////////////////////////////////////////////////////////////////////////////
                        // Roof Reset
                        ///////////////////////////////////////////////////////////////////////////////////////////////////
                        if (header.indexOf("GET /c2a/Reset1") >= 0)
                        {
                            roofRestPosition = "Reset1";
                            digitalWrite(outputLBoardRelay1Pin, LOW);

                            delay(100);
                            digitalWrite(outputLBoardRelay1Pin, HIGH);
                            delay(200);

                            digitalWrite(outputLBoardRelay1Pin, LOW);
                        }
                        else if (header.indexOf("GET /c2a/Reset2") >= 0)
                        {
                            roofRestPosition = "Reset2";
                            digitalWrite(outputLBoardRelay1Pin, LOW);

                            delay(100);
                            digitalWrite(outputLBoardRelay1Pin, HIGH);
                            delay(200);

                            digitalWrite(outputLBoardRelay1Pin, LOW);
                        }

                        ///////////////////////////////////////////////////////////////////////////////////////////////////
                        ///////////////////////////////////////////////////////////////////////////////////////////////////


                        // Display the HTML web page
                        client.println("<!DOCTYPE html><html>");
                        client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
                        client.println("<link rel=\"icon\" href=\"data:,\">");
                        // CSS to style the on/off buttons 

                        // Feel free to change the background-color and font-size attributes to fit your preferences
                        client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");

                        client.println(".button { background-color: #800080; border: none; color: white; padding: 10px 25px;");
                        client.println("text-decoration: none; font-size: 20px; margin: 2px; cursor: pointer;}");

                        client.println(".button2{ background-color: #008000; border: none; color: white; padding: 10px 25px;");
                        client.println("text-decoration: none; font-size: 20px; margin: 2px; cursor: pointer;}");



                        client.println(".button3{ background-color: #FF0000; border: none; color: white; padding: 16px 40px;");
                        client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");

                        client.println(".button4{ background-color: #008000; border: none; color: white; padding: 16px 40px;");
                        client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");



                        client.println(".button5{ background-color: #77878A; border: none; color: white; padding: 8px 10px;");
                        client.println("text-decoration: none; font-size: 10px; margin: 2px; cursor: pointer;}");
                        
                        client.println(".button6{ background-color: #808000; border: none; color: white; padding: 8px 10px;");
                        client.println("text-decoration: none; font-size: 10px; margin: 2px; cursor: pointer;}");



                        client.println(".button7 {background-color: #77878A;}</style></head>");


                        // Web Page Heading
                        client.println("<body><h1>Telescope Cover & Lightboard System</h1>");
                        client.println("<h2>Pier #1 (v1.7)</h2>");
                        client.println("<p>WDAstro - www.wdastro.com</p>");

                        //System Information
                        unsigned long powerTime = (currentTime - PowerOnTime) / 1000 / 60 / 60;
                        client.println("<p>Arduino Uptime: " + String(powerTime) + " hours</p>");

                        client.println("<p>---------------</p>");
                        client.println("<p>LightBoard Selection</p>");

                        if (lBoardSelect == "SV-130")
                        {
                            analogWrite(outputLightboardPin,0);
                            
                            client.println("<a href=\"/c3a/Off\"><button class=\"button3\">Off</button></a>");
                            client.println("<a href=\"/c3b/SV-130\"><button class=\"button4\">SV-130</button></a>");
                            client.println("<a href=\"/c3c/C-11\"><button class=\"button3\">C-11</button></a>");
                        }
                        else if (lBoardSelect == "C-11")
                        {
                            servoLboard.write(0);
                            
                            client.println("<a href=\"/c3a/Off\"><button class=\"button3\">Off</button></a>");
                            client.println("<a href=\"/c3b/SV-130\"><button class=\"button3\">SV-130</button></a>");
                            client.println("<a href=\"/c3c/C-11\"><button class=\"button4\">C-11</button></a>");
                        }
                        else
                        {
                            servoLboard.write(0);
                            analogWrite(outputLightboardPin,0);
                            lBoardLevel = "0";
                            
                            client.println("<a href=\"/c3a/Off\"><button class=\"button4\">Off</button></a>");
                            client.println("<a href=\"/c3b/SV-130\"><button class=\"button3\">SV-130</button></a>");
                            client.println("<a href=\"/c3c/C-11\"><button class=\"button3\">C-11</button></a>");
                        }


                        if (lBoardSelect == "C-11")
{
                        // Display current Lightboard Status
                        client.println("<p>C-11 LightBoard Brightness " + lBoardLevel + "</p>");
                        client.println("<p>");

  
                        if (lBoardLevel == "0")
                        {
                            client.println("<a href=\"/0/off\"><button class=\"button2\">0</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/0/on\"><button class=\"button\">0</button></a>");
                        }

                        if (lBoardLevel == "1")
                        {
                            client.println("<a href=\"/1/off\"><button class=\"button2\">1</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/1/on\"><button class=\"button\">1</button></a>");
                        }

                        if (lBoardLevel == "2")
                        {
                            client.println("<a href=\"/2/off\"><button class=\"button2\">2</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/2/on\"><button class=\"button\">2</button></a>");
                        }

                        if (lBoardLevel == "3")
                        {
                            client.println("<a href=\"/3/off\"><button class=\"button2\">4</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/3/on\"><button class=\"button\">4</button></a>");
                        }


                        if (lBoardLevel == "4")
                        {
                            client.println("<a href=\"/4/off\"><button class=\"button2\">8</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/4/on\"><button class=\"button\">8</button></a>");
                        }

                        if (lBoardLevel == "5")
                        {
                            client.println("<a href=\"/5/off\"><button class=\"button2\">10</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/5/on\"><button class=\"button\">10</button></a>");
                        }

                        if (lBoardLevel == "6")
                        {
                            client.println("<a href=\"/6/off\"><button class=\"button2\">15</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/6/on\"><button class=\"button\">15</button></a>");
                        }

                        if (lBoardLevel == "7")
                        {
                            client.println("<a href=\"/7/off\"><button class=\"button2\">25</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/7/on\"><button class=\"button\">25</button></a>");
                        }

                        if (lBoardLevel == "8")
                        {
                            client.println("<a href=\"/8/off\"><button class=\"button2\">50</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/8/on\"><button class=\"button\">50</button></a>");
                        }

                        if (lBoardLevel == "9")
                        {
                            client.println("<a href=\"/9/off\"><button class=\"button2\">75</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/9/on\"><button class=\"button\">75</button></a>");
                        }

                        if (lBoardLevel == "10")
                        {
                            client.println("<a href=\"/10/off\"><button class=\"button2\">100</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/10/on\"><button class=\"button\">100</button></a>");
                        }

                        if (lBoardLevel == "11")
                        {
                            client.println("<a href=\"/11/off\"><button class=\"button2\">200</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/11/on\"><button class=\"button\">200</button></a>");
                        }

                        if (lBoardLevel == "12")
                        {
                            client.println("<a href=\"/12/off\"><button class=\"button2\">300</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/12/on\"><button class=\"button\">300</button></a>");
                        }

                        if (lBoardLevel == "13")
                        {
                            client.println("<a href=\"/13/off\"><button class=\"button2\">400</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/13/on\"><button class=\"button\">400</button></a>");
                        }


                        if (lBoardLevel == "14")
                        {
                            client.println("<a href=\"/14/off\"><button class=\"button2\">500</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/14/on\"><button class=\"button\">500</button></a>");
                        }

                        if (lBoardLevel == "15")
                        {
                            client.println("<a href=\"/15/off\"><button class=\"button2\">600</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/15/on\"><button class=\"button\">600</button></a>");
                        }

                        if (lBoardLevel == "16")
                        {
                            client.println("<a href=\"/16/off\"><button class=\"button2\">700</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/16/on\"><button class=\"button\">700</button></a>");
                        }

                        if (lBoardLevel == "17")
                        {
                            client.println("<a href=\"/17/off\"><button class=\"button2\">800</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/17/on\"><button class=\"button\">800</button></a>");
                        }

                        if (lBoardLevel == "18")
                        {
                            client.println("<a href=\"/18/off\"><button class=\"button2\">900</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/18/on\"><button class=\"button\">900</button></a>");
                        }

                        if (lBoardLevel == "19")
                        {
                            client.println("<a href=\"/19/off\"><button class=\"button2\">1000</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/19/on\"><button class=\"button\">1000</button></a>");
                        }
}
if (lBoardSelect == "SV-130")
{
                          // Display current Lightboard Status
                        client.println("<p>SV-130 LightBoard Brightness " + lBoardLevel + "</p>");
                        client.println("<p>");

                        if (lBoardLevel == "0")
                        {
                            client.println("<a href=\"/0/off\"><button class=\"button2\">0</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/0/on\"><button class=\"button\">0</button></a>");
                        }

                        if (lBoardLevel == "1")
                        {
                            client.println("<a href=\"/1/off\"><button class=\"button2\">14</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/1/on\"><button class=\"button\">14</button></a>");
                        }

                        if (lBoardLevel == "2")
                        {
                            client.println("<a href=\"/2/off\"><button class=\"button2\">15</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/2/on\"><button class=\"button\">15</button></a>");
                        }

                        if (lBoardLevel == "3")
                        {
                            client.println("<a href=\"/3/off\"><button class=\"button2\">16</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/3/on\"><button class=\"button\">16</button></a>");
                        }


                        if (lBoardLevel == "4")
                        {
                            client.println("<a href=\"/4/off\"><button class=\"button2\">17</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/4/on\"><button class=\"button\">17</button></a>");
                        }

                        if (lBoardLevel == "5")
                        {
                            client.println("<a href=\"/5/off\"><button class=\"button2\">18</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/5/on\"><button class=\"button\">18</button></a>");
                        }

                        if (lBoardLevel == "6")
                        {
                            client.println("<a href=\"/6/off\"><button class=\"button2\">19</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/6/on\"><button class=\"button\">19</button></a>");
                        }

                        if (lBoardLevel == "7")
                        {
                            client.println("<a href=\"/7/off\"><button class=\"button2\">20</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/7/on\"><button class=\"button\">20</button></a>");
                        }

                        if (lBoardLevel == "8")
                        {
                            client.println("<a href=\"/8/off\"><button class=\"button2\">21</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/8/on\"><button class=\"button\">21</button></a>");
                        }

                        if (lBoardLevel == "9")
                        {
                            client.println("<a href=\"/9/off\"><button class=\"button2\">25</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/9/on\"><button class=\"button\">25</button></a>");
                        }

                        if (lBoardLevel == "10")
                        {
                            client.println("<a href=\"/10/off\"><button class=\"button2\">30</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/10/on\"><button class=\"button\">30</button></a>");
                        }

                        if (lBoardLevel == "11")
                        {
                            client.println("<a href=\"/11/off\"><button class=\"button2\">35</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/11/on\"><button class=\"button\">35</button></a>");
                        }

                        if (lBoardLevel == "12")
                        {
                            client.println("<a href=\"/12/off\"><button class=\"button2\">40</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/12/on\"><button class=\"button\">40</button></a>");
                        }

                        if (lBoardLevel == "13")
                        {
                            client.println("<a href=\"/13/off\"><button class=\"button2\">45</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/13/on\"><button class=\"button\">45</button></a>");
                        }


                        if (lBoardLevel == "14")
                        {
                            client.println("<a href=\"/14/off\"><button class=\"button2\">50</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/14/on\"><button class=\"button\">50</button></a>");
                        }

                        if (lBoardLevel == "15")
                        {
                            client.println("<a href=\"/15/off\"><button class=\"button2\">60</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/15/on\"><button class=\"button\">60</button></a>");
                        }

                        if (lBoardLevel == "16")
                        {
                            client.println("<a href=\"/16/off\"><button class=\"button2\">75</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/16/on\"><button class=\"button\">75</button></a>");
                        }

                        if (lBoardLevel == "17")
                        {
                            client.println("<a href=\"/17/off\"><button class=\"button2\">90</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/17/on\"><button class=\"button\">90</button></a>");
                        }

                        if (lBoardLevel == "18")
                        {
                            client.println("<a href=\"/18/off\"><button class=\"button2\">100</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/18/on\"><button class=\"button\">100</button></a>");
                        }

                        if (lBoardLevel == "19")
                        {
                            client.println("<a href=\"/19/off\"><button class=\"button2\">110</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/19/on\"><button class=\"button\">110</button></a>");
                        }
}

                        client.println("<p>");




                        client.println("<p>---------------</p>");

                        client.println("<p>AP1200 -- SV130 - Cover - " + tCover1Position + "</p>");


                        if (tCover1Position == "Open")
                        {
                            client.println("<a href=\"/c1a/Open\"><button class=\"button4\">Open</button></a>");
                            client.println("<a href=\"/c1b/Close\"><button class=\"button3\">Close</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/c1a/Open\"><button class=\"button3\">Open</button></a>");
                            client.println("<a href=\"/c1b/Close\"><button class=\"button4\">Close</button></a>");
                        }



                        client.println("<p>---------------</p>");

                        client.println("<p>Roof Arduino Reset</p>");


                        if (roofRestPosition == "Reset1")
                        {
                            client.println("<a href=\"/c2a/Reset2\"><button class=\"button5\">Reset</button></a>");
                        }
                        else
                        {
                            client.println("<a href=\"/c2a/Reset1\"><button class=\"button6\">Reset</button></a>");
                        }

                        client.println("</body></html>");

                        // The HTTP response ends with another blank line
                        client.println();
                        // Break out of the while loop
                        break;
                    }
                    else
                    { // if you got a newline, then clear currentLine
                        currentLine = "";
                    }
                }
                else if (c != '\r')
                {  // if you got anything else but a carriage return character,
                    currentLine += c;      // add it to the end of the currentLine
                }
            }
        }
        // Clear the header variable
        header = "";
        // Close the connection
        client.stop();
        //Serial.println("Client disconnected.");
        //Serial.println("");
    }
}
