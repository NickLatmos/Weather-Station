/*
 * Raspberry address in WAN thermi-weatherstation.org.dyndns.org:5000
 * Posts data to the Raspberry server located at port 5000 with IP 192.168.1.60
 * 
 * ATTENTION! If using in the same place with the server use the local IP address (192.168.1.60) otherwise the WAN
 * 
 * Baud rate 9600
 * 
 */
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h> // make http requests 
#include <stdlib.h>
#include "definitions.h"
#include "debug.h"

extern "C" {
#include "gpio.h"
}

extern "C" {
#include "user_interface.h"
}

enum MESSAGE_TYPE {
  DATA,
  VALVE,
  CONNECT
};

#if USE_STATIC_IP_ADDRESS
IPAddress station_ip(192, 168, 1, 53);        //ESP8266 IP Address
IPAddress gateway(192, 168, 1, 1);    //Gateway IP
IPAddress subnet(255, 255, 255, 0);
#endif

//const String HOST = "http://thermi-weatherstation.dyndns.org:5000";
//const char *HOST = "alatmos.dyndns.org";
const char *HOST = "192.168.1.60";
const int DATA_PORT = 5000;    
const int RC_PORT = 5500;   

//message_array stores the request made from the main controller
char message_array[MAX_BUF_SIZE];                       
char valve_status[20] = "";

unsigned long current_time = 0;
unsigned long previous_time = 0;

static boolean FLAG_DATA_READY = false;              //Indicates whether all the data have been received
int counter = 0;

void setup() 
{
  pinMode(ESP_LED, OUTPUT);
  pinMode(LIGHT_WAKE_PIN, INPUT);
  Serial.begin(9600);
  delay(2000);                              //Give some time to the controller to setup
  
  // 15 seconds to test if there is a WiFi connection and connect to it
  // Our purpose here is to inform MCU about the WiFi network 
  if(connect_to_WiFi(400)){         // 300 * 50ms
    #if DEBUG
      Serial.println(F("WiFi connected"));
      Serial.println(WiFi.localIP());
      Serial.println(getMacAddress());
    #endif
    // Send the message that connection was successful until MCU responds with a "CONNECTED"
    while(1)
    {
      unsigned long now = millis();
      sendResponse("WiFi OK");  

      // Wait 5 seconds to receive a response (if any) from the MCU
      // Otherwise send your message again
      while(millis() - now <= 5000){
        if(Serial.available()) 
          receiveMessageFromConroller();
      }

      // If received data check them
      if(data_received_from_mcu())
      {
        if(check_message() == CONNECT) break;
        reset_message_variables();
      }
    }
  }else
    deactivate_WiFi_Modem(FAILED_TO_CONNECT_ON_START);  
  
  
  toggleLedFast(); // Indicates that the coordinator is ready
  serialFlush();   // Clear input serial buffer
  reset_message_variables();
  
  current_time = millis();
  previous_time = millis();
  delay(500); 
  #if DEBUG
    Serial.println("Going to sleep");
  #endif
  go_to_sleep();
  #if DEBUG
    Serial.println("Woke Up!");
  #endif
}

/*
 * Toggling the blue led on ESP
 * Used mainly for debugging
*/
void toggleLedFast()
{
  for(int i = 0; i < 10; i++){
    digitalWrite(ESP_LED,!digitalRead(2));
    delay(100);
  }
}


//Every time ESP8266 posts data to the server blink the led fast
void togglePostLed()
{
  for(int j = 0; j < 2; j++){
    for(int i = 0; i < 4; i++){
      digitalWrite(ESP_LED,!digitalRead(2));
      delay(50);
    }
    delay(300);
  }
}

//Gets ESP8266 MAC Address
String getMacAddress() {
  byte mac[6];
  WiFi.macAddress(mac);
  String cMac = "";
  for (int i = 0; i < 6; ++i) {
    cMac += String(mac[i],HEX);
    if(i<5)
      cMac += "-";
  }
  cMac.toUpperCase();
  return cMac;
}

// Post data to the server, receive it's response and then send it to the MCU
void start_communication_with_server(const char *host, int port)
{
  WiFiClient client;
  char response[512];
  int index = 0;

  //send request to the server
  if(client.connect(host, port))
  {
    client.print(message_array);  
      
    yield();
    //delay for a while until a response is available
    delay(500);
    if(client.available()){
      while(client.available())
        response[index++] = (char) client.read();
    }
    response[index] = '\0';
    
    //send response to the main controller
    if(port == RC_PORT)
      send_response_without_bell_character(response);  // RC response has already an '\a' at the end
    else sendResponse(response);
    
    serialFlush();
    
    client.stop();
  }
  delay(100);
  // Used for debugging
  /*Serial.print("MCU sent: ");
  sendResponse(message_array);*/
}

// Deactivate WiFi modem if no WiFi network available and inform the MCU about that
void deactivate_WiFi_Modem(int seconds)
{
  // Go to sleep for 120 seconds and try again 
  // (deepsleep causes a reset, set to 0 to sleep forever) 
  // Don't forget to connect the GPIO 16 to RST. GPIO outputs a low pulse to wake up the ESP after the specified time.
  sendResponse("WiFi Unavailable");
  delay(10);
  ESP.deepSleep(seconds * 1000000);       
}

// Connect to WIFi. 
// @threshold_counter = max number of trials in order to connect (multiplies by 50ms)
// Using DHCP protocol causes ESP8266 to connect in about ~7seconds
// Using Static IP takes only ~2 seconds
boolean connect_to_WiFi(int threshold_counter)
{
  int wifi_connection_trial_counter = 0;
  
  WiFi.forceSleepWake();
  WiFi.mode(WIFI_STA);      

  #if USE_STATIC_IP_ADDRESS
    WiFi.config(station_ip, gateway, subnet);    
  #endif
  
  if(WPA_PASSWORD != "")
    WiFi.begin(WIFI_SSID, WPA_PASSWORD);    //Network with password
  else
    WiFi.begin(WIFI_SSID);                  //Network without password
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(50);
    yield();
    #if DEBUG
     Serial.print(".");
    #endif
    wifi_connection_trial_counter++;
    if(wifi_connection_trial_counter >= threshold_counter){  // e.g. 100 * 50 = 5 secs seconds to connect
      wifi_connection_trial_counter = 0;
      return false;  
    }
  }
  #if DEBUG
    Serial.print("time: ");
    Serial.println(wifi_connection_trial_counter * 50 / 1000);
    Serial.print("wifi_connection_trial_counter: ");
    Serial.println(wifi_connection_trial_counter);
  #endif
  wifi_connection_trial_counter = 0;
  return true;
}

//1 for valve status
//0 for data (REST)
int check_message()
{
  MESSAGE_TYPE mes_type;
  char *message = message_array;
  
  if(!strncmp(message, "VALVE?", strlen("VALVE?")))
    mes_type = VALVE;
  else if(!strncmp(message, "CONNECTED", strlen("CONNECTED")))
    mes_type = CONNECT;
  else
    mes_type = DATA;
    
  return mes_type;
}


// If the controller has sent any data receive them
void receiveMessageFromConroller()
{
  //While buffer has data read them
  //The controller waits a while before transmitting new data
  while(Serial.available()){
    if(counter < MAX_BUF_SIZE){
      char c = (char) Serial.read();
      //\a to indicate the end of the message
      if(c == '\a'){
        data_from_mcu_ready(true);
        serialFlush();
        break;
      }
      message_array[counter++] = c;
    }else{
      serialFlush();
      counter = 0;
      break;  
    }
  }
}

// Reset the message variables used for UART communication
void reset_message_variables(void)
{
  //reset variables
  memset(message_array, '\0', MAX_BUF_SIZE);  
  data_from_mcu_ready(false);
  counter = 0;
}

// Message from MCU has arrived
void data_from_mcu_ready(boolean status){
  FLAG_DATA_READY = status;
}

boolean data_received_from_mcu(void){
  return FLAG_DATA_READY;
}
  
/*
 *Sends a response back to main controller
 */
void sendResponse(char *response)
{
  Serial.print(response);  
  Serial.write('\a');
}

/*
 *Sends a response without a bell character (there is one in the response) back to main controller
 */
void send_response_without_bell_character(char* response)
{
  Serial.print(response);  
}

/*Clear Serial buffer*/
void serialFlush()
{
  while(Serial.read() > 0) {}
}

// Goes to Light sleep mode and awakes on interrupt HIGH Voltage
void go_to_sleep()
{
  wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);           // Light sleep mode on
  wifi_fpm_open();                                  // Enables force sleep
  gpio_pin_wakeup_enable(GPIO_ID_PIN(LIGHT_WAKE_PIN), GPIO_PIN_INTR_HILEVEL);     // Wake up on digital pin HIGH
  wifi_fpm_do_sleep(0xFFFFFFF);                     // sleep forever
  delay(10);                                       // delay is needed to do some internal stack functions
}

void loop() 
{             
  // Check if MCU has sent any data
  if(Serial.available()) receiveMessageFromConroller();
  
  // If all data have been received send the message to the server or receive the RC status.
  if(data_received_from_mcu())
  {
    // If not connected after 10 seconds deactivate WiFi modem and inform MCU
    if(connect_to_WiFi(200)) 
    {
      //check controller's message
      if(check_message() == VALVE)
        start_communication_with_server(HOST, RC_PORT);
      else if(check_message() == DATA)
        start_communication_with_server(HOST, DATA_PORT);
    }else
      deactivate_WiFi_Modem(FAILED_TO_CONNECT_NEXT_TRIAL_TIME);   //10 minutes
    reset_message_variables();
    // Go to sleep. MCU will wake you up and you will continue from here.
    #if DEBUG
      Serial.println("Going to sleep");
      delay(1000);
    #endif
    go_to_sleep();
    #if DEBUG
      Serial.println("Woke Up!");
    #endif
  }
}
