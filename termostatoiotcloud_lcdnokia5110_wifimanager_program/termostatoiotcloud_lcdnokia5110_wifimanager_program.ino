#include <Wire.h>
#include <lm75.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <WiFiManager.h>
#include "thingProperties.h"
#include <time.h>

// Definizione dei pin
const int buttonPin1 = D3;
const int buttonPin2 = D10;
int relayPin = D5;
int light = 10;

// LCD Nokia 5110
Adafruit_PCD8544 lcd(D8, D7, D6, D4, D0);

// Indirizzo sensore
const uint8_t lm75Address = 0x48;

// Variabili
float currentTemperature = 0.0;
float targetTemperature = 25.0;
bool relayState = false;
bool lightState = true;
bool buttonState1 = false;
bool buttonState2 = false;
unsigned long lastButtonCheck = 0;
unsigned long debounceDelay = 50;
unsigned long lastTemperatureUpdate = 0;
const unsigned long temperatureUpdateInterval = 5000;
unsigned long lastButtonPressTime = 0;

// Modalità manuale/automatica
bool manualMode = false;
unsigned long button1PressStart = 0;
unsigned long button2PressStart = 0;
bool button1Held = false;
bool button2Held = false;

char ssidbuf[32];
char passbuf[64];
char timeBuffer[20];

enum MenuState {
  CURRENT_TEMPERATURE_PAGE,
  SET_TEMPERATURE_PAGE
};

MenuState menuState = CURRENT_TEMPERATURE_PAGE;

void setup() {
  WiFiManager wifiManager;
  //wifiManager.resetSettings();
  if (!wifiManager.autoConnect("AutoConnectAP")) {
    ESP.restart();
    delay(1000);
  }

  WiFi.SSID().toCharArray(ssidbuf, (WiFi.SSID().length()) + 1);
  WiFi.psk().toCharArray(passbuf, (WiFi.psk().length()) + 1);

  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);
  pinMode(light, OUTPUT);
  digitalWrite(light, HIGH);
  lightState = HIGH;

  Wire.begin(D2, D1);

  lcd.begin();
  lcd.setContrast(60);
  lcd.clearDisplay();

  if (!lm75Present(lm75Address)) {
    lcd.setTextSize(1);
    lcd.setCursor(0, 0);
    lcd.print("LM75A error");
    lcd.display();
    while (1);
  }

  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  updateLCD();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    ArduinoCloud.update();
  }

  bool currentState1 = digitalRead(buttonPin1);
  bool currentState2 = digitalRead(buttonPin2);

  // BUTTON 1 (manual mode)
  if (currentState1 == LOW) {
    if (!button1Held) {
      button1PressStart = millis();
      button1Held = true;
    } else if (millis() - button1PressStart > 3000) {
      manualMode = true;
      updateLCD();
      button1Held = false;
    }
  } else {
    if (button1Held && (millis() - button1PressStart < 3000)) {
      handleButtonPress();
    }
    button1Held = false;
  }

  // BUTTON 2 (automatic mode)
  if (currentState2 == LOW) {
    if (!button2Held) {
      button2PressStart = millis();
      button2Held = true;
    } else if (millis() - button2PressStart > 3000) {
      manualMode = false;
      updateLCD();
      button2Held = false;
    }
  } else {
    if (button2Held && (millis() - button2PressStart < 3000)) {
      handleButtonPressDecrement();
    }
    button2Held = false;
  }

  if (millis() - lastTemperatureUpdate >= temperatureUpdateInterval) {
    lastTemperatureUpdate = millis();
    currentTemperature =( lm75ReadTempC(lm75Address)-2);
    //Serial.println(currentTemperature);
    updateDateTime();

    if (!manualMode) {
      struct tm *timeinfo;
      time_t now = ArduinoCloud.getLocalTime();
      timeinfo = localtime(&now);

      int day = timeinfo->tm_wday;
      int hour = timeinfo->tm_hour;

      switch (day) {
        case 0: // Domenica
          targetTemperature = 18;
          break;
        case 6: // Sabato
          if (hour >= 6 && hour < 12)
            targetTemperature = 25;
          else
            targetTemperature = 18;
          break;
        default: // Lunedì - Venerdì
          if (hour >= 6 && hour < 20)
            targetTemperature = 25;
          else
            targetTemperature = 18;
          break;
      }
    }

    if (menuState == CURRENT_TEMPERATURE_PAGE) {
      updateLCD();
    }

    if (WiFi.status() == WL_CONNECTED) {
      ArduinoCloud.update();
    }
  }

// Controllo relè con isteresi
if (relayState) {
  // Il relè è acceso, lo spengo solo se supera upper threshold
  if (currentTemperature >= targetTemperature + 0.5) {
    digitalWrite(relayPin, LOW);
    relayState = false;
  }
} else {
  // Il relè è spento, lo accendo solo se scende sotto lower threshold
  if (currentTemperature <= targetTemperature - 0.5) {
    digitalWrite(relayPin, HIGH);
    relayState = true;
  }
}

  if (WiFi.status() == WL_CONNECTED) {
    ArduinoCloud.update();
  }

  if (menuState == SET_TEMPERATURE_PAGE && millis() - lastButtonPressTime > 2000) {
    digitalWrite(light, LOW);
    lightState = false;
    menuState = CURRENT_TEMPERATURE_PAGE;
    updateLCD();
  }

  delay(100);
  yield();
}

void updateLCD() {
  lcd.clearDisplay();
  lcd.setTextSize(1);

  // Indicatore modalità "A" o "M"
  lcd.setCursor(75, 0);
  lcd.print(manualMode ? "M" : "A");

  switch (menuState) {
    case CURRENT_TEMPERATURE_PAGE:
      lcd.setCursor(0, 0);
      lcd.print(" Temperatura");
      lcd.setCursor(0, 8);
      lcd.print("   Attuale:");
      lcd.setCursor(0, 20);
      lcd.setTextSize(2);
      lcd.print(currentTemperature, 2);
      lcd.print((char)248);
      lcd.print("C");
      lcd.setTextSize(1);
      lcd.setCursor(0, 40);
      lcd.print(timeBuffer);
      break;

    case SET_TEMPERATURE_PAGE:
      lcd.setCursor(0, 0);
      lcd.print("   Imposta");
      lcd.setCursor(0, 8);
      lcd.print(" Temperatura:");
      lcd.setCursor(0, 20);
      lcd.setTextSize(2);
      lcd.print(targetTemperature, 2);
      lcd.print((char)248);
      lcd.print("C");
      break;
  }

  lcd.display();
}

void handleButtonPress() {
  if (menuState == CURRENT_TEMPERATURE_PAGE) {
    menuState = SET_TEMPERATURE_PAGE;
    lastButtonPressTime = millis();
  } else if (menuState == SET_TEMPERATURE_PAGE) {
    targetTemperature += 0.5;
    if (targetTemperature >= 33)
      targetTemperature = 0;
    lastButtonPressTime = millis();
  }
  updateLCD();
}

void handleButtonPressDecrement() {
  if (menuState == CURRENT_TEMPERATURE_PAGE) {
    menuState = SET_TEMPERATURE_PAGE;
    lastButtonPressTime = millis();
  } else if (menuState == SET_TEMPERATURE_PAGE) {
    targetTemperature -= 0.5;
    if (targetTemperature < 0)
      targetTemperature = 33;
    lastButtonPressTime = millis();
  }
  updateLCD();
}

bool lm75Present(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission() == 0;
}

float lm75ReadTempC(uint8_t address) { 
Wire.beginTransmission(address);
 Wire.write(0); 
 Wire.endTransmission(false); 
 Wire.requestFrom(address, (uint8_t)2);
 int16_t t = Wire.read() << 8 | Wire.read(); 
 return t / 256.0; 
 }



void updateDateTime() {
  time_t now = ArduinoCloud.getLocalTime();
  struct tm *timeinfo = localtime(&now);
  strftime(timeBuffer, sizeof(timeBuffer), "%d/%m/%y %H:%M", timeinfo);
}
