#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Servo.h>
#include "DRV8833ESC.h"

//#define REMOTE_DEBUG 1

//TODO: use https://github.com/MaiaVictor/Bitspeak to generate pronounciable
// expected AP and let the receiver connect to it! Static password for now!

// NOTE: Copy the wifi_credentials_template.h as wifi_credentials.h
// and fill in your wifi details there. It is gitingored for your safety.
#include "wifi_credentials.h"

// Wifi operating mode
//#define USE_AP 1 

// NOTE: Configure your Servos or H-Bridge motors

// Usually GPIO-0 (pin 0 or TX pin 1)
//#define CH1_PIN 2
#define HBRIDGE_CH1_PIN1 0
#define HBRIDGE_CH1_PIN2 2

// Usually GPIO-2 (pin 2 or RX pin 3)
// #define CH2_PIN 2
#define HBRIDGE_CH2_PIN1 1
#define HBRIDGE_CH2_PIN2 3

// NOTE: Newer ESP-01S may have LED on GPIO2, b/c it is used for a servo, do not use it at all
//#define ACTIVITY_LED LED_BUILTIN

#define LOCAL_UDP_PORT 4242
#define DEAD_MAN_TIMEOUT 2500

#define CH1_COAST_ZONE 5
#define CH2_COAST_ZONE 5

// Enable or disable debug
#ifdef REMOTE_DEBUG
  #include <RemoteDebug.h> //https://github.com/JoaoLopesF/RemoteDebug
  RemoteDebug Debug;
  #define DEBUG 1
#endif

#ifdef CH1_PIN
  Servo ch1;
#elif defined(HBRIDGE_CH1_PIN1) && defined(HBRIDGE_CH1_PIN2)
  DRV8833ESC ch1;
#endif

#ifdef CH2_PIN
  Servo ch2;
#elif defined(HBRIDGE_CH2_PIN1) && defined(HBRIDGE_CH2_PIN2)
  DRV8833ESC ch2;
#endif

WiFiUDP Udp;
char incoming_packet[12];  // buffer for incoming packets
int ch1Pos = 90; // 0-180
int ch2Pos = 90; // 0-180
unsigned long prev_packet_millis = 0;
unsigned long prev_ch2_update_millis = 0;

void setup()
{
  #ifdef CH1_PIN
    ch1.attach(CH1_PIN); 
  #elif defined(HBRIDGE_CH1_PIN1) && defined(HBRIDGE_CH1_PIN2)
    ch1.attach(HBRIDGE_CH1_PIN1, HBRIDGE_CH1_PIN2, CH1_COAST_ZONE);
  #endif
  #ifdef CH2_PIN
    ch2.attach(CH1_PIN); 
  #elif defined(HBRIDGE_CH2_PIN1) && defined(HBRIDGE_CH2_PIN2)
    ch2.attach(HBRIDGE_CH2_PIN1, HBRIDGE_CH2_PIN2, CH2_COAST_ZONE);
  #endif
  ch1.write(90);
  ch2.write(90);
  
  prev_packet_millis = millis();
  prev_ch2_update_millis = millis();
  
  // Initialize the ACTIVITY_LED pin as an output (if defined)
  #ifdef ACTIVITY_LED
    pinMode(ACTIVITY_LED, OUTPUT);
    digitalWrite(ACTIVITY_LED, HIGH); // OFF
  #endif
    
  #ifdef USE_AP
    WiFi.mode(WIFI_AP);  //need both to serve the webpage and take commands via tcp
    IPAddress ip(192,168,4,2);
    IPAddress gateway(192,168,4,1);
    IPAddress subnet(255,255,255,0); 
    WiFi.softAPConfig(ip, gateway, subnet);
    
    char buffer[50] = "ESP01RCRX XX:XX:XX:XX:XX";
    WiFi.softAPmacAddress().toCharArray(buffer+10, 15);
    buffer[24]='\0';
    if (!WiFi.softAP(buffer, 0, 1+buffer[23]%13, false, 1))
      return
    #ifdef ACTIVITY_LED
      digitalWrite(ACTIVITY_LED, LOW); // LED ON -> AP active!
    #endif
    IPAddress myIP = WiFi.softAPIP();
  #else  
    WiFi.begin(WIFI_SSID, WIFI_PWD);
    while (WiFi.status() != WL_CONNECTED)
    {
      // Fast blinking -> waiting for the connection to come up
      delay(250);
      #ifdef ACTIVITY_LED
        digitalWrite(ACTIVITY_LED, LOW); 
        delay(250);
        digitalWrite(ACTIVITY_LED, HIGH); 
      #endif
    }

    #ifdef ACTIVITY_LED
      digitalWrite(ACTIVITY_LED, LOW); // LED ON -> Connected to wifi!
    #endif
    IPAddress myIP = WiFi.localIP();
  #endif

  #ifdef REMOTE_DEBUG
    // Set up the TELNET debugger
    Debug.begin(WiFi.localIP().toString());
    Debug.setResetCmdEnabled(true);
    Debug.showProfiler(true);
    Debug.showColors(true);
    Debug.println("AP up, setting up UDP server.");
  #endif

  Udp.begin(LOCAL_UDP_PORT);

  #ifdef REMOTE_DEBUG
    Debug.printf("Now listening at IP %s, UDP port %d\n", myIP.toString().c_str(), 23);
    Debug.handle();
  #endif
}

void loop()
{
  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    // receive incoming UDP packets
    // can be "r 123\n" or "t 123\n" or "b 123 123\n"
    #ifdef REMOTE_DEBUG
    Debug.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
    #endif
    int len = Udp.read(incoming_packet, 10);
    incoming_packet[len] = '\0';
    if (len >= 4 && len<=6) // unsure if the \n and \0 is counted
    {
      // the package is "100"-"999"
      int pos = (int)((max(0, min(9, incoming_packet[2]-'0'))*100+
                       max(0, min(9, incoming_packet[3]-'0'))*10+
                       max(0, min(9, incoming_packet[4]-'0'))-100)/899.0*180.0);
                  
      // ch1
      if (incoming_packet[0]=='s')
      {
        ch1Pos = pos;
        ch1.write( ch1Pos );
        prev_packet_millis = millis();
      }
      // ch2
      else if (incoming_packet[0]=='t')
      {
        ch2Pos = pos;
        ch2.write( ch2Pos );
        prev_packet_millis = millis();
      }
      #ifdef DEBUG
        // unknown ch
        else Debug.printf("WARNING: Invalid channel code: %c\n", incoming_packet[0]);
      #endif 
    }
    else if (len >=9 && len<=11 and incoming_packet[0]=='b')
    {
      ch1Pos = (int)((max(0, min(9, incoming_packet[2]-'0'))*100+
                      max(0, min(9, incoming_packet[3]-'0'))*10+
                      max(0, min(9, incoming_packet[4]-'0'))-100)/899.0*180.0);
      ch2Pos = (int)((max(0, min(9, incoming_packet[6]-'0'))*100+
                      max(0, min(9, incoming_packet[7]-'0'))*10+
                      max(0, min(9, incoming_packet[8]-'0'))-100)/899.0*180.0);

      ch1.write( ch1Pos );
      ch2.write( ch2Pos );
      prev_packet_millis = millis();
    }
    #ifdef DEBUG    
      else
      {
        Debug.printf("WARNING: package is too short!");
      }
      Debug.printf("UDP packet contents: %s\n", incoming_packet);
      Debug.printf("ch1 pos: %.2f\n", ch1Pos);
      Debug.printf("ch2 pos: %.2f\n", ch2Pos);
      Debug.printf("\n");
    #endif
  }
  else if (millis()-prev_packet_millis>DEAD_MAN_TIMEOUT)
  {
    ch1.write( 90 );
    ch2.write( 90 );
    prev_packet_millis = millis(); // Reset again in DEAD_MAN_TIMEOUT if no packages.
  }

  /*unsigned long delta_ms = millis()-prev_ch2_update_millis
  if (delta_ms>CH2_UPDATE_INTERVAL_MS)
  {
    
  }*/
  
  #ifdef DEBUG
    Debug.handle();
  #endif

  // TODO: try if delay takes care of the jitter?
  //delay(5);
  yield();
}
