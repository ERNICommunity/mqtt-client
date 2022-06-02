/*
 * ECMqttClient.cpp
 *
 *  Created on: 23.06.2017
 *      Author: nid
 */

#include <Client.h>
#include <MqttClientController.h>
#include <ECMqttClient.h>

const unsigned short int ECMqttClientClass::defaultMqttPort = 1883;
const unsigned short int ECMqttClientClass::defaultSecureMqttPort = 8883;

void ECMqttClientClass::begin(const char* address, unsigned short int port, Client& client, const char* id)
{
  MqttClientController::Instance()->begin(address, port, client, id);
}

void ECMqttClientClass::loop()
{
  MqttClientController::Instance()->loop();
}

ECMqttClientClass ECMqttClient;
