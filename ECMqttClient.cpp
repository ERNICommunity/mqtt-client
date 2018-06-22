/*
 * ECMqttClient.cpp
 *
 *  Created on: 23.06.2017
 *      Author: nid
 */

#include <MqttClientController.h>
#include <ECMqttClient.h>

const unsigned short int ECMqttClientClass::defaultMqttPort = 1883;

void ECMqttClientClass::begin(const char* address, unsigned short int port)
{
  MqttClientController::Instance()->setServer(address, port);
}

void ECMqttClientClass::loop()
{
  MqttClientController::Instance()->loop();
}

ECMqttClientClass ECMqttClient;
