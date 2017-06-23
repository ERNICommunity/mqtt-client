#include <MqttClientController.h>

#include <MqttClient.h>

const unsigned short int MqttClientClass::defaultMqttPort = 1883;

void MqttClientClass::begin(const char* address, unsigned short int port)
{
  MqttClientController::Instance()->setServer(address, port);
}

void MqttClientClass::loop()
{
  MqttClientController::Instance()->loop();
}

MqttClientClass MqttClient;
