/*
 * MqttClientDbgCommand.h
 *
 *  Created on: 18.10.2016
 *      Author: nid
 */

#ifndef LIB_MQTT_CLIENT_MQTTCLIENTDBGCOMMAND_H_
#define LIB_MQTT_CLIENT_MQTTCLIENTDBGCOMMAND_H_

#include <DbgCliCommand.h>

class DbgCli_Topic;
class MqttClientController;


//-----------------------------------------------------------------------------

class DbgCli_Cmd_MqttClientCon : public DbgCli_Command
{
private:
  MqttClientController* m_mqttClient;
public:
  DbgCli_Cmd_MqttClientCon(DbgCli_Topic* mqttClientTopic, MqttClientController* mqttClient);
  void execute(unsigned int argc, const char** args, unsigned int idxToFirstArgToHandle);
  void printUsage();
};

//-----------------------------------------------------------------------------

class DbgCli_Cmd_MqttClientDis : public DbgCli_Command
{
private:
  MqttClientController* m_mqttClient;
public:
  DbgCli_Cmd_MqttClientDis(DbgCli_Topic* mqttClientTopic, MqttClientController* mqttClient);
  void execute(unsigned int argc, const char** args, unsigned int idxToFirstArgToHandle);
  void printUsage();
};

//-----------------------------------------------------------------------------

class DbgCli_Cmd_MqttClientPub : public DbgCli_Command
{
private:
  MqttClientController* m_mqttClient;
public:
  DbgCli_Cmd_MqttClientPub(DbgCli_Topic* mqttClientTopic, MqttClientController* mqttClient);
  void execute(unsigned int argc, const char** args, unsigned int idxToFirstArgToHandle);
  void printUsage();
};

//-----------------------------------------------------------------------------

class DbgCli_Cmd_MqttClientSub : public DbgCli_Command
{
private:
  MqttClientController* m_mqttClient;
public:
  DbgCli_Cmd_MqttClientSub(DbgCli_Topic* mqttClientTopic, MqttClientController* mqttClient);
  void execute(unsigned int argc, const char** args, unsigned int idxToFirstArgToHandle);
  void printUsage();
};

//-----------------------------------------------------------------------------

class DbgCli_Cmd_MqttClientUnsub : public DbgCli_Command
{
private:
  MqttClientController* m_mqttClient;
public:
  DbgCli_Cmd_MqttClientUnsub(DbgCli_Topic* mqttClientTopic, MqttClientController* mqttClient);
  void execute(unsigned int argc, const char** args, unsigned int idxToFirstArgToHandle);
  void printUsage();
};

//-----------------------------------------------------------------------------

class DbgCli_Cmd_MqttClientShow : public DbgCli_Command
{
private:
  MqttClientController* m_mqttClient;
public:
  DbgCli_Cmd_MqttClientShow(DbgCli_Topic* mqttClientTopic, MqttClientController* mqttClient);
  void execute(unsigned int argc, const char** args, unsigned int idxToFirstArgToHandle);
  void printUsage();
};

//-----------------------------------------------------------------------------

#endif /* LIB_MQTT_CLIENT_MQTTCLIENTDBGCOMMAND_H_ */
