#if defined(ESP32)
  #include <WiFiMulti.h>
  WiFiMulti wifiMulti;
  #define DEVICE "ESP32"
  #elif defined(ESP8266)
  #include <ESP8266WiFiMulti.h>
  ESP8266WiFiMulti wifiMulti;
  #define DEVICE "ESP8266"
  #endif
  
  #include <InfluxDbClient.h>
  #include <InfluxDbCloud.h>
  

  #include <math.h>

  #include <ESP8266WiFi.h>
  #include <WiFiClient.h>
  #include <ESP8266WebServer.h>

  #include <LiquidCrystal_I2C.h>
  #include <DHT.h>


  // WiFi AP SSID
  #define WIFI_SSID "TP_link d610"
  // WiFi password
  #define WIFI_PASSWORD "2023&mk11088"
  
  #define INFLUXDB_URL "https://us-east-1-1.aws.cloud2.influxdata.com"
  #define INFLUXDB_TOKEN "vQDAGM-nxxWYj-dS9f3AQ4mMZ6mUT_XbdWjXUEwlUL7h4iYLUIjv-P6p3vpcmjzCQXXGi_snVRcDD8YtCDZyKQ=="
  #define INFLUXDB_ORG "1a7aeb1c7d4c8f80"
  #define INFLUXDB_BUCKET "ESP8266"
  
  // Time zone info
  #define TZ_INFO "UTC1"
  
  // Declare InfluxDB client instance with preconfigured InfluxCloud certificate
  InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);
  
  // Declare Data point
  Point sensor("wifi_status");
  Point dht11sensor("DHT11");

  //// DHT 11           *********************
DHT dht = DHT(2, DHT11, 6); // DHT(uint8_t pin, uint8_t type, uint8_t count=6);

/// LCD_I2C           *********************      

// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;

// set LCD address, number of columns and rows
// if you don't know your display address, run an I2C scanner sketch
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  

///  3 dots 
static uint8_t counter = 0;


 



void lcdsetup(){
  /// LCD_I2C           *********************  
    // initialize LCD
  lcd.init();
  // turn on LCD backlight                      
  lcd.backlight();

  // print message   Hello, World! //////////////
  lcd.setCursor(0, 0);

  lcd.print("Hello, World!");
  lcd.setCursor(0, 0);

  lcd.setCursor(7, 1);

  /////////////////////////////////////////
  // 3 dots 
  delay(500);
  for (counter = 0; counter < 3; counter++) {
      lcd.print(".");
      delay(500);
  }
  delay(3000);
  lcd.clear();
}



void dht11setup(){
  //// DHT 11           *********************

  Serial.println();
  Serial.println("Status\tHumidity (%)\tTemperature (C)");
  dht.begin();
  //dht.setup(D1); /* This function is no longer available in library */


}



  void wifisetup() {
  
    // Setup wifi
    WiFi.mode(WIFI_STA);
    wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  
    Serial.print("Connecting to wifi");
    while (wifiMulti.run() != WL_CONNECTED) {
      Serial.print(".");
      delay(100);
    }

    // Add tags to the data point
    sensor.addTag("device", DEVICE);
    sensor.addTag("SSID", WiFi.SSID());

    Serial.println();

        // Add tags to the data point
    dht11sensor.addTag("device", DEVICE);
    dht11sensor.addTag("SSID", WiFi.SSID());
  
    Serial.println();

  }



  void influxdbsetup() {
    
      // Accurate time is necessary for certificate validation and writing in batches
      // We use the NTP servers in your area as provided by: https://www.pool.ntp.org/zone/
      // Syncing progress and the time will be printed to Serial.
      timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");
    
    
      // Check server connection
      if (client.validateConnection()) {
        Serial.print("Connected to InfluxDB: ");
        Serial.println(client.getServerUrl());
      } else {
        Serial.print("InfluxDB connection failed: ");
        Serial.println(client.getLastErrorMessage());
      }


  }


  

 void setup() {
      Serial.begin(115200);

      wifisetup();

      influxdbsetup() ;

      lcdsetup();
      dht11setup();

    
  }




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void influxdbloop() {

  
    // Print what are we exactly writing
    Serial.print("Writing: ");
    Serial.println(sensor.toLineProtocol());


    
      // Print what are we exactly writing    DHT11
    Serial.print("Writing: ");
    Serial.println(dht11sensor.toLineProtocol());



    // Check WiFi connection and reconnect if needed
    if (wifiMulti.run() != WL_CONNECTED) {
      Serial.println("Wifi connection lost");
    }
  



    // Write point
    if (!client.writePoint(sensor)) {
      Serial.print("InfluxDB write failed: ");
      Serial.println(client.getLastErrorMessage());
    }

    // Write point    DHT11
    if (!client.writePoint(dht11sensor)) {
      Serial.print("InfluxDB write failed: ");
      Serial.println(client.getLastErrorMessage());
    }
  
    Serial.println("Waiting 1 second");
    delay(1*1e3);
  }


  float humidity = dht.readHumidity(); /* Get humidity value */
  float temperature = dht.readTemperature(); /* Get temperature value */

  void dht11read(float& humidity,float& temperature ){
    humidity = dht.readHumidity(); /* Get humidity value */
    temperature = dht.readTemperature(); /* Get temperature value */

  }


  void dht11SerialLoop(){

      //// DHT 11           *********************
      // delay(dht.getMinimumSamplingPeriod()); /* This function is no longer available in library */

      dht11read(humidity,temperature );

      Serial.println();
      Serial.println();
      Serial.println("Status\tHumidity (%)\tTemperature (C)");
      // Serial.print(dht.getStatusString());/* This function is no longer available in library */
      Serial.print("\t");
      Serial.print(humidity, 1);
      Serial.print("\t\t");
      Serial.print(temperature, 1);
      Serial.println("\t\t");
      //Serial.println(dht.convertFtoC(temperature), 1);/* Convert temperature to Fahrenheit units */
      Serial.println();

  }


  void lcdloop(){

      /// LCD_I2C           *********************  
      // set cursor to first column, first row
      lcd.clear();
      dht11read(humidity,temperature );

      /////////////////////////////////////////

      /// Print Temperature & humidity on the lcd
      lcd.setCursor(0,0);
      lcd.print("Humidity (%) ");

      lcd.setCursor(6,1);
      lcd.print(humidity, 1);

      delay(5*1e3);
      lcd.clear();

      lcd.setCursor(0,0);
      lcd.print("Temperature (C)");

      lcd.setCursor(6,1);
      lcd.print(temperature, 1);
      //delay(5*1e3);
      

  }



  void loop() {
      dht11read(humidity,temperature );
    // Clear fields for reusing the point. Tags will remain the same as set above.
    sensor.clearFields();
    dht11sensor.clearFields();
    // Store measured value into point
    // Report RSSI of currently connected network
    sensor.addField("rssi", WiFi.RSSI());

    // Report DHT11 data to the server
    dht11sensor.addField("Temperature", temperature);
    dht11sensor.addField("Humidity", humidity);
    
    lcdloop();

    influxdbloop();

    dht11SerialLoop();



  }
