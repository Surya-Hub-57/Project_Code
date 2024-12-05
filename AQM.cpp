// Blynk credentials
#define BLYNK_TEMPLATE_ID "TMPL3j3zd2zov"
#define BLYNK_TEMPLATE_NAME "Air Quality Monitoring"
#define BLYNK_AUTH_TOKEN "o7BvpkZ9bnWMkOFELX0HJQPRce8GIMVH"

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// Pin Definitions

#define BUZZER_PIN 12        // Buzzer pin

// DHT11 sensor setup
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address for 16x2 LCD


// WiFi credentials
char ssid[] = "SURYA-HOTSPOT";
char pass[] = "SURYA123";

// Methane threshold (in ppm)
int methaneThreshold = 400;

// Variable to store sensor readings
int methaneLevel = 0;

void setup() {
  // Start serial communication for debugging
  Serial.begin(115200);
  
  // Initialize LCD display
  lcd.begin(16, 2);
  lcd.setBacklight(1);
  lcd.clear();
  
  // Display initial message on LCD
  lcd.setCursor(0, 0);
  lcd.print("Starting...");
  
  // Initialize WiFi and Blynk
  Serial.println("Connecting to WiFi...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  // Wait until WiFi is connected
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  
  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Confirm successful connection to Blynk
  if (Blynk.connected()) {
    Serial.println("Blynk Connected!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("System OK");
    lcd.setCursor(0, 1);
    lcd.print("All set!");
  } else {
    Serial.println("Blynk Not Connected!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Blynk Error");
    lcd.setCursor(0, 1);
    lcd.print("Check Conn.");
  }

  // Initialize DHT11 sensor
  dht.begin();

  // Initialize buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  
  // Wait before proceeding
  delay(3000);
}

void loop() {
  Blynk.run();

  // Read sensors
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  methaneLevel = analogRead(MQ4_PIN);

  // Map sensor readings to ppm (example mapping)
  methaneLevel = map(methaneLevel, 0, 4095, 0, 1000);

  // Check for sensor errors
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Display data on Serial Monitor
  Serial.print("Temp: ");
  Serial.print(temperature);
  Serial.print("°C, Humidity: ");
  Serial.print(humidity);
  Serial.print("%, Methane: ");
  Serial.print(methaneLevel);
  Serial.println(" ppm");

  // Display data on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp:");
  lcd.print(temperature);
  lcd.print("C");
  
  lcd.setCursor(0, 1);
  lcd.print("Methane:");
  lcd.print(methaneLevel);
  lcd.print("ppm");

  // Send data to Blynk
  Blynk.virtualWrite(V0, temperature);
  Blynk.virtualWrite(V1, humidity);
  Blynk.virtualWrite(V2, methaneLevel);

  // Check threshold conditions and send alert
  if (methaneLevel > methaneThreshold) {
    Serial.println("Warning: Dangerous methane levels detected!");
    digitalWrite(BUZZER_PIN, HIGH); // Activate buzzer

    // Send notification using Blynk.logEvent()
    Blynk.logEvent("alert_event", "Dangerous methane levels detected!");
  } else {
    digitalWrite(BUZZER_PIN, LOW); // Deactivate buzzer
  }

  delay(2000); // Wait for 2 seconds before the next reading
}