#include <M5StickCPlus.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <driver/i2s.h>

#define PIN_CLK  0
#define PIN_DATA 34
#define DATA_POINTS 512
#define SAMPLE_RATE 16000
#define READ_LEN (2 * DATA_POINTS)
#define GAIN_FACTOR 3
uint8_t BUFFER[READ_LEN] = {0};

int16_t *adcBuffer = NULL;

WiFiClient espClient;
PubSubClient client(espClient);

// Configuration
const char* ssid            = "Wi-Fi SSID";                                 // Wi-Fi SSID
const char* password        = "Wi-Fi password";                             // Wi-Fi password
const char* mqtt_server     = "MQTT broker IP address";                     // MQTT broker IP address
const uint16_t mqtt_port    = 12345;                                        // MQTT broker port
const char* mqtt_login      = "MQTT broker username";                       // MQTT broker username
const char* mqtt_password   = "MQTT broker password";                       // MQTT broker password
const char* mqtt_topic      = "MQTT topic for controlling the alarm";       // MQTT topic for controlling the alarm
const char* mqtt_topic_db   = "MQTT topic for reporting noise level";       // MQTT topic for reporting noise level
const char* arm_message     = "MQTT payload for arming the alarm";          // MQTT payload for arming the alarm
const char* disarm_message  = "MQTT payload for disarming the alarm";       // MQTT payload for disarming the alarm
const char* sos_message     = "MQTT payload for sounding the alarm";        // MQTT payload for sounding the alarm
const uint16_t sos_duration = 16000;                                        // Milliseconds on the "ON" button for sounding the alarm
const uint16_t send_every   = 5000;                                         // Milliseconds between sound level reports
const uint16_t time_between_samples = 100;                                  // Milliseconds between each sound measurement

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];

void setupWifi();
void callback(char* topic, byte* payload, unsigned int length);
void reConnect();

////////////////////////////////////////////////////////////////////////////////////////////////
//                                  Loudness
// Based on: https://qiita.com/tomoto335/items/263b23d9ba156de12857
// Source: https://gist.githubusercontent.com/tomoto/6a1b67d9e963f9932a43c984171d80fb/raw/4c27b16745debfc93d39006bb03307d3958a3b28/LoudnessMeter.ino
////////////////////////////////////////////////////////////////////////////////////////////////
void i2sInit()
{
   i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
    .sample_rate =  SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // is fixed at 12bit, stereo, MSB
    .channel_format = I2S_CHANNEL_FMT_ALL_RIGHT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 2,
    .dma_buf_len = 128,
   };

   i2s_pin_config_t pin_config;
   pin_config.bck_io_num   = I2S_PIN_NO_CHANGE;
   pin_config.ws_io_num    = PIN_CLK;
   pin_config.data_out_num = I2S_PIN_NO_CHANGE;
   pin_config.data_in_num  = PIN_DATA;
   
   i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
   i2s_set_pin(I2S_NUM_0, &pin_config);
   i2s_set_clk(I2S_NUM_0, SAMPLE_RATE, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
}

void mic_record_task (void* arg)
{   
  size_t bytesread;
  while(1){
    i2s_read(I2S_NUM_0,(char*) BUFFER, READ_LEN, &bytesread, (100 / portTICK_RATE_MS));
    adcBuffer = (int16_t *)BUFFER;
    processSignal();
    delay(1); // Remove the 100ms wait in the original sample
  }
}

float filteredBase = 0;
float totalPower = 0;
int numPower = 0;

float calcDecibell(float x) {
  // Conversion formula that Excel-like flicked out
  return 11.053 * log(x) + 11.142;

  // [Reference] The theoretical value from the datasheet is 8.6859 * log(x) + 25.6699;
}

void processSignal(){
  // Automatically correct the zero point by taking the average
  float base = 0;
  for (int n = 0; n < DATA_POINTS; n++){
    base += adcBuffer[n];
  }
  base /= DATA_POINTS;

  // Apply a filter to slow down changes
  const float alpha = 0.98;
  filteredBase = filteredBase * alpha + base * (1 - alpha);

  // Serial.printf("fb:%f,b:%f\n", filteredBase, base);

  // take the root mean square
  float power = 0;
  for (int n = 0; n < DATA_POINTS; n++){
    power += sq(adcBuffer[n] - filteredBase);
  }
  power = sqrt(power / DATA_POINTS);

  // Serial.printf("p:%f\n", power);
  // Serial.printf("db:%f\n", calcDecibell(power));

  // In order to take the average at regular intervals later, add it to a global variable
  totalPower += power;
  numPower++;
}

////////////////////////////////////////////////////////////////////////////////////////////////
//                                  MQTT
////////////////////////////////////////////////////////////////////////////////////////////////

void setupWifi() {
    delay(10);
    M5.Axp.ScreenSwitch(true);
    M5.Lcd.printf("Connecting to %s", ssid);
    M5.Lcd.println("");
    WiFi.mode(WIFI_STA);  // Set the mode to WiFi station mode.
    M5.Lcd.println("Station mode set");
    WiFi.begin(ssid, password);  // Start Wifi connection.
    M5.Lcd.println("Begun, attempting connection");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        M5.Lcd.print(".");
    }
    M5.Lcd.println("\nSuccess\n");
    M5.Axp.ScreenSwitch(false);
}

void callback(char* topic, byte* payload, unsigned int length) {
    M5.Lcd.setTextColor(WHITE, BLACK);

    String msg = "";
    for (int i = 0; i < length; i++) {
        //M5.Lcd.print((char)payload[i]);
        msg += (char)payload[i];
    }
    if(msg == arm_message){
      digitalWrite(33, HIGH); // Bouton 1
      delay(250);
      digitalWrite(33, LOW);
      M5.Axp.ScreenSwitch(true);
      M5.Lcd.fillScreen(RED);
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.setTextColor(BLACK, RED);
      M5.Lcd.setTextSize(10);
      M5.Lcd.println("Alarm");
      M5.Lcd.println("ON");
      M5.Lcd.setTextSize(1);
      delay(5000);
      M5.Axp.ScreenSwitch(false);
    }
    if(msg == disarm_message){
      digitalWrite(32, HIGH); // Bouton 2
      delay(250);
      digitalWrite(32, LOW);
      M5.Axp.ScreenSwitch(true);
      M5.Lcd.fillScreen(GREEN);
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.setTextColor(BLACK, GREEN);
      M5.Lcd.setTextSize(10);
      M5.Lcd.println("Alarm");
      M5.Lcd.println("OFF");
      M5.Lcd.setTextSize(1);
      delay(5000);
      M5.Axp.ScreenSwitch(false);
    }
    if(msg == sos_message){
      M5.Axp.ScreenSwitch(true);
      digitalWrite(33, HIGH); // Bouton 1
      M5.Lcd.fillScreen(RED);
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.setTextColor(BLACK, RED);
      M5.Lcd.setTextSize(20);
      M5.Lcd.println("SOS");
      M5.Lcd.setTextSize(1);
      delay(sos_duration);
      digitalWrite(33, LOW);
      M5.Axp.ScreenSwitch(false);
    }
    M5.Lcd.println();
}

void reConnect() {
  M5.Axp.ScreenSwitch(true);
  while (!client.connected()) {
      M5.Lcd.setTextColor(WHITE, BLACK);
      M5.Lcd.println("Attempting MQTT connection...");
      // Create a random client ID.
      String clientId = "M5StickC PLUS - ";
      clientId += String(random(0xffff), HEX);
      M5.Lcd.println(clientId);
      // Attempt to connect.
      if (client.connect(clientId.c_str(), mqtt_login, mqtt_password)) {
          M5.Lcd.printf("\nSuccess\n");
          // Once connected, publish an announcement to the topic.

          client.publish(mqtt_topic, "hello world");
          // ... and resubscribe.
          client.subscribe(mqtt_topic);
      } else {
          M5.Lcd.print("failed, rc=");
          M5.Lcd.print(client.state());
          M5.Lcd.println("try again in 5 seconds");
          delay(5000);
      }
  }
  M5.Axp.ScreenSwitch(false);
}

////////////////////////////////////////////////////////////////////////////////////////////////
//                                  Setup and Loop
////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  M5.begin();
  setupWifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);  // Sets the message callback function
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE, BLACK);

  pinMode(32, OUTPUT); // Relai 2
  pinMode(33, OUTPUT); // Relai 1

  Serial.begin(115200);

  i2sInit();
  xTaskCreate(mic_record_task, "mic_record_task", 2048, NULL, 1, NULL);
}

float maximumDb = 0.0;
float sumDb = 0.0;
int nbDb = 0;
float moyDb = 0.0;
float minimumDb = 999.0;
bool screenOn = true; // The display is on by default at boot

void loop() {
  if (numPower > 0) {
    // Take the average of the accumulated data and convert it to dB
    float p = totalPower / numPower;
    float db = calcDecibell(p);
    if(db > maximumDb) maximumDb = db;
    if(db < minimumDb) minimumDb = db;
    sumDb += db;
    nbDb ++;
    moyDb = sumDb / nbDb;

    // Manage display only if it's actually on - this saves 25ms
    if(screenOn){
      uint16_t bgColor, fgColor;
      if (db < 55) {
        // calm color
        bgColor = TFT_BLACK;
        fgColor = TFT_NAVY;
      } else if (db < 70) {
        // A color that feels like a normal conversation
        bgColor = TFT_BLACK;
        fgColor = TFT_DARKCYAN;
      } else if (db < 80) {
        // Color that feels a little noisy
        bgColor = TFT_MAROON;
        fgColor = TFT_YELLOW;
      } else {
        // very annoying color
        bgColor = TFT_PURPLE;
        fgColor = TFT_RED;
      }
      M5.Lcd.fillScreen(bgColor);
      M5.Lcd.setTextColor(fgColor);
      M5.Lcd.setTextDatum(CC_DATUM);
      String str(db);
      M5.Lcd.drawString(str, 80, 40, 7);
    }

    totalPower = 0;
    numPower = 0;
  }

  M5.update();  // Read the press state of the key
  if (M5.BtnA.wasReleased()) {  // If the button A is pressed
    if(screenOn){
      M5.Axp.ScreenSwitch(false);
      screenOn = false;
    } else {
      M5.Axp.ScreenSwitch(true);
      screenOn = true;
    }
  }

  // Wait a little to not over-compute
  delay(time_between_samples);

  if (!client.connected()) {
    reConnect();
  }
  client.loop();  // This function is called periodically to allow clients to process incoming messages and maintain connections to the server.

  unsigned long now = millis();
  
  if (now - lastMsg > send_every) {
      lastMsg = now;
      
      snprintf(msg, MSG_BUFFER_SIZE, "{\"min\":%.2f, \"avg\":%.2f, \"max\":%.2f, \"N\":%d}", minimumDb, moyDb, maximumDb, nbDb);  // Format to the specified string and store it in MSG.
      client.publish(mqtt_topic_db, msg);  // Publishes a message to the specified topic
      
      maximumDb = 0.0;
      sumDb = 0.0;
      nbDb = 0;
      moyDb = 0.0;
      minimumDb = 999.0;
  }
}

/*
 * screen on, no delay     --> 0,74W
 * screen on, 500ms delay  --> 0,71W
 * screen off, no delay    --> 0,68W
 * screen off, 500ms delay --> 0,54W
 * screen off, 100ms delay --> 0,54W
 */
