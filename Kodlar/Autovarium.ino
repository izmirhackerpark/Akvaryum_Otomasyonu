/* Autovarium: Aquarium atumation project codes.
 - This Arduino sketch converts your arduino uno board into a aquarium 
   automation device with it's integrated components.
 - For more information: https://github.com/izmirhackerpark/Akvaryum_Otomasyonu
 - DEVELOPERS:
   - Coding: Sencer HAMARAT
   - Electronic Design: Şükrü COŞAR
 - V 0.01 Alpha 18/01/2015 */

/*-------------------------( Import needed libraries )------------------------*/
/*--------------------( Declare Constants and Pin Numbers )-------------------*/
/*-----------------------------( Declare objects )----------------------------*/
/*----------------------------( Declare Variables )---------------------------*/

                        /*------( Wireless Modlue )------*/

#define SSID "YOUR WIFI NETWORK NAME" // Wifi SSID
#define PASS "YOUR WIFI NETWOTK PASS" // Wifi Password
#include <OneWire.h>

const int ledPin =  13;     // Communication led pin
#define BUFFER_SIZE 128     // Indicates buffer size
char buffer[BUFFER_SIZE];   // Creates buffer
int ledState = LOW;         // Communication led status on start


                        /*-----( Temperature Sensor )-----*/
                        
int DS18S20_Pin = 2;        // DS18S20 Signal pin on digital 2
OneWire ds(DS18S20_Pin);    // Temperature chip i/o on digital pin 2


                        /*-----( Water level Sensor )-----*/

// Fish tank water level sensor with internal pull-up resistor and detecting 
// transitions and Debouncing without delay
const byte switchPin = 8;
byte oldSwitchState = HIGH;  // assume switch open because of pull-up resistor
const unsigned long debounceTime = 10;  // milliseconds
unsigned long switchPressTime;  // when the switch last changed state


/*----------------------------------( SETUP )---------------------------------*/

void setup()   /****** SETUP: RUNS ONCE ******/
{   // put your setup code here, to run once:
    
    /*-----( Wifi module )-----*/
    Serial.begin(9600);
    Serial3.begin(115200);
    pinMode(2, OUTPUT);
    pinMode(ledPin, OUTPUT);
    Serial.println("Setup Begin!");
    delay(1000);
    digitalWrite(2, HIGH);
    
    sendData("AT+RST\r\n", 2000, false); // reset module
    sendData("AT+CWMODE=1\r\n", 1000, true); // configure as access point
    sendData("AT+CWLAP\r\n", 2000, true);
    connectWiFi(SSID, PASS);
    sendData("AT+CIFSR\r\n", 1000, true); // get ip address
    sendData("AT+CIPMUX=1\r\n", 1000, true); // configure for multiple connections
    sendData("AT+CIPSERVER=1,3000\r\n", 1000, true);

    /*-----( Temperature Sensor )-----*/
    Serial.begin(9600);

    /*-----( Water Level Sensor )-----*/
    Serial.begin (115200);
    pinMode (switchPin, INPUT_PULLUP);
}//---(end setup )---


/*----------------------------------( LOOP )----------------------------------*/

void loop()   /****** LOOP: RUNS CONSTANTLY ******/
{
    /*-----( Wifi Module Loop )-----*/
    if (Serial3.available() > 0)
    {
        int ch_id, packet_len;
        char *pb;
        Serial3.readBytesUntil('\n', buffer, BUFFER_SIZE);  // Buffer read
        if (strncmp(buffer, "+IPD,", 5) == 0)
        {
            sscanf(buffer + 5, "%d,%d", &ch_id, &packet_len);
            if (packet_len > 0)
            {
                pb = buffer + 5;
                while (*pb != ':') pb++;
                pb++;
                if (strncmp(pb, "GET /?led", 8) == 0) {
                    Serial.print(millis());
                    Serial.print(" : ");
                    Serial.println(buffer);
                    Serial.print( "get led from ch :" );
                    Serial.println(ch_id);
                    delay(100);
                    clearSerialBuffer();
                    if (ledState == LOW)
                        ledState = HIGH;
                    else
                        ledState = LOW;
                    digitalWrite(ledPin, ledState);
                    homepage(ch_id);
                } else if (strncmp(pb, "GET / ", 6) == 0) {
                    Serial.print(millis());
                    Serial.print(" : ");
                    Serial.println(buffer);
                    Serial.print( "get Status from ch:" );
                    Serial.println(ch_id);
                    delay(100);
                    clearSerialBuffer();
                    homepage(ch_id);
                }
            }
        }
    }
    
    /*-----( Temperature Sensor Loop )-----*/
    float temperature = getTemp();
    Serial.println(temperature);
    delay(100); //just here to slow down the output so it is easier to read

    /*-----( Water Level Sensor Loop )-----*/
    byte switchState = digitalRead (switchPin); // see if switch is open or closed
    
    if (switchState != oldSwitchState) 
    {
        if (millis () - switchPressTime >= debounceTime) // debounce
        {
            switchPressTime = millis ();  // when we closed the switch
            oldSwitchState =  switchState;  // remember for next time
            if (switchState == LOW)
            {
                Serial.println ("Switch closed.");
            }  // end if switchState is LOW
            else
            {
                Serial.println ("Switch opened.");
            }  // end if switchState is HIGH
        }  // end if debounce time up
    }  // end of state change
}//---( End of main loop )---



/*---------------------( Declare User-written Functions )---------------------*/

/*-----( Wifi Module Fuctions )-----*/

void homepage(int ch_id) {
    String Header;  // Building Http header

    Header =  "HTTP/1.1 200 OK\r\n";
    Header += "Content-Type: text/html\r\n";
    Header += "Connection: close\r\n";
    //Header += "Refresh: 5\r\n";

    String Content;  // Page Content
    Content = "<center><h1>Hello World!</h1></center><hr><center><h2>Autovarium</h2></center><hr>Led State: D";
    Content += String(ledState);
    Content += "<br><a href=\"/?led\"><input type=\"button\" value=\"Led State\"></a>";

    Header += "Content-Length: ";
    Header += (int)(Content.length());
    Header += "\r\n\r\n";

    Serial3.print("AT+CIPSEND=");  // AT command and CH_ID content length
    Serial3.print(ch_id);
    Serial3.print(",");
    Serial3.println(Header.length() + Content.length());
    delay(10);

    // Sending Content
    if (Serial3.find(">")) {
        Serial3.print(Header);
        Serial3.print(Content);
        delay(10);
    }
}

void clearSerialBuffer(void) {
    while ( Serial3.available() > 0 ) {
        Serial3.read();
    }
}

String sendData(String command, const int timeout, boolean debug)
{
    String response = "";  // Server response

    Serial3.print(command);  // Command sent to Server

    long int time = millis();  // First value of the timer

    while ( (time + timeout) > millis())
    {
        while (Serial3.available())
        {
            // The esp has data so display its output to the serial window
            char c = Serial3.read(); // read the next character.
            response += c;
        }
    }
    if (debug)      // Debug line
    {
        Serial.print(response);
    }
    return response;
}

void connectWiFi(String NetworkSSID, String NetworkPASS) {
    String cmd = "AT+CWJAP=\"";
    cmd += NetworkSSID;
    cmd += "\",\"";
    cmd += NetworkPASS;
    cmd += "\"";

    sendData(cmd, 2000, true);
}



/*-----( Temperature Sensor Funcions )-----*/

float getTemp(){
    //returns the temperature from one DS18S20 in DEG Celsius
    byte data[12];
    byte addr[8];
    if ( !ds.search(addr) ) {
        //no more sensors on chain, reset search
        ds.reset_search();
        return -1000;
    }
    if ( OneWire::crc8( addr, 7) != addr[7]) {
        Serial.println("CRC is not valid!");
        return -1000;
    }
    if ( addr[0] != 0x10 && addr[0] != 0x28) {
        Serial.print("Device is not recognized");
        return -1000;
    }
    
    ds.reset();
    ds.select(addr);
    ds.write(0x44,1); // start conversion, with parasite power on at the end
    
    byte present = ds.reset();
    ds.select(addr);
    ds.write(0xBE); // Read Scratchpad
    
    for (int i = 0; i < 9; i++) { // we need 9 bytes
        data[i] = ds.read();
    }
    
    ds.reset_search();
    
    byte MSB = data[1];
    byte LSB = data[0];
    
    float tempRead = ((MSB << 8) | LSB); //using two's compliment
    float TemperatureSum = tempRead / 16;
    
    return TemperatureSum;
}

/*-----( Water Level Sensor Functions )-----*/

// No function needed. It's all happening in loop.

//*********( THE END )***********
