#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>


const char THING_ID[]           = "XXXX";  //Enter THING ID   296dcf32-7dca-42d7-b55a-80808075725a
const char DEVICE_LOGIN_NAME[]  = "XXXX"; //Enter DEVICE ID  708e52cc-4654-4bb4-a575-99b4a35861f4
const char DEVICE_KEY[]  = "XXXX";    // Secret device password
// Dichiarazione delle variabili di cloud
extern float currentTemperature;
extern float targetTemperature;
extern bool relayState;
extern bool manualMode;
extern int relayPin;
extern int light;
extern bool lightState;
extern char ssidbuf[32];
extern char passbuf[64];
// Funzioni callback
// Dichiara la funzione updateLCD definita nello sketch principale
void updateLCD();

void onTargetTemperatureChange() {
  // Codice da eseguire quando la temperatura target cambia
  Serial.print("Target Temperature changed to: ");
  Serial.println(targetTemperature);
}

void onRelayStateChange() {
  // Codice da eseguire quando lo stato del relè cambia
  Serial.print("Relay State changed to: ");
  Serial.println(relayState);
  digitalWrite(relayPin, relayState ? HIGH : LOW);
}
void onStateModeChange() {
 Serial.print("State Mode changed to: ");
  Serial.println(manualMode ? "Manual (M)" : "Automatic (A)");
  updateLCD();  // Se esiste nel tuo sketch
}

void onLightStateChange() {
  // Codice da eseguire quando lo stato del backlight cambia
  Serial.print("Light State changed to: ");
  Serial.println(lightState);
  digitalWrite(light, lightState ? HIGH : LOW);
}



void initProperties() {
 
    ArduinoCloud.setBoardId(DEVICE_LOGIN_NAME);
  ArduinoCloud.setSecretDeviceKey(DEVICE_KEY);
  ArduinoCloud.setThingId(THING_ID);
  ArduinoCloud.addProperty(manualMode, READWRITE, ON_CHANGE, onStateModeChange);
 ArduinoCloud.addProperty(targetTemperature, READWRITE, ON_CHANGE, onTargetTemperatureChange);
  ArduinoCloud.addProperty(currentTemperature, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(lightState, READWRITE, ON_CHANGE, onLightStateChange);
  ArduinoCloud.addProperty(relayState, READ, ON_CHANGE, NULL);
}


WiFiConnectionHandler ArduinoIoTPreferredConnection(ssidbuf,passbuf);
