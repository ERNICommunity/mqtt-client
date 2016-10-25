/*
 * MqttClientDbgCommand.cpp
 *
 *  Created on: 18.10.2016
 *      Author: nid
 */

#include <DbgCliNode.h>
#include <DbgCliTopic.h>
#include <DbgTraceContext.h>
#include <DbgTracePort.h>
#include <DbgTraceLevel.h>
#include <DbgPrintConsole.h>
#include <DbgTraceOut.h>

#include <MqttClientController.h>
#include <MqttClientDbgCommand.h>

//-----------------------------------------------------------------------------

DbgCli_Cmd_MqttClientCon::DbgCli_Cmd_MqttClientCon(DbgCli_Topic* mqttClientTopic, MqttClientController* mqttClient)
: DbgCli_Command(mqttClientTopic, "con", "Connect MQTT client to broker.")
, m_mqttClient(mqttClient)
{ }

void DbgCli_Cmd_MqttClientCon::execute(unsigned int argc, const char** args, unsigned int idxToFirstArgToHandle)
{
  if (argc - idxToFirstArgToHandle > 0)
  {
    printUsage();
  }
  else
  {
    if (0 != m_mqttClient)
    {
      m_mqttClient->setShallConnect(true);
      Serial.println("MQTT client connecting to broker now.");
    }
  }
}

void DbgCli_Cmd_MqttClientCon::printUsage()
{
  Serial.println(getHelpText());
  Serial.println("Usage: dbg mqtt con");
}

//-----------------------------------------------------------------------------

DbgCli_Cmd_MqttClientDis::DbgCli_Cmd_MqttClientDis(DbgCli_Topic* mqttClientTopic, MqttClientController* mqttClient)
: DbgCli_Command(mqttClientTopic, "dis", "Disconnect MQTT client from broker.")
, m_mqttClient(mqttClient)
{ }

void DbgCli_Cmd_MqttClientDis::execute(unsigned int argc, const char** args, unsigned int idxToFirstArgToHandle)
{
  if (argc - idxToFirstArgToHandle > 0)
  {
    printUsage();
  }
  else
  {
    if (0 != m_mqttClient)
    {
      m_mqttClient->setShallConnect(false);
      Serial.println("MQTT client disconnecting from broker now.");
    }
  }
}

void DbgCli_Cmd_MqttClientDis::printUsage()
{
  Serial.println(getHelpText());
  Serial.println("Usage: dbg mqtt dis");
}

//-----------------------------------------------------------------------------

DbgCli_Cmd_MqttClientPub::DbgCli_Cmd_MqttClientPub(DbgCli_Topic* mqttClientTopic, MqttClientController* mqttClient)
: DbgCli_Command(mqttClientTopic, "pub", "Publish a value to a topic using MQTT client.")
, m_mqttClient(mqttClient)
{ }

void DbgCli_Cmd_MqttClientPub::execute(unsigned int argc, const char** args, unsigned int idxToFirstArgToHandle)
{
  if (argc - idxToFirstArgToHandle != 2)
  {
    printUsage();
  }
  else
  {
    if (0 != m_mqttClient)
    {
      int retVal = m_mqttClient->publish(args[idxToFirstArgToHandle], args[idxToFirstArgToHandle+1]);
      Serial.print("MQTT client, publish ");
      Serial.println(retVal == 1 ? "successful" : "failed");
    }
  }
}

void DbgCli_Cmd_MqttClientPub::printUsage()
{
  Serial.println(getHelpText());
  Serial.println("Usage: dbg mqtt pub <topic> <value>");
}

//-----------------------------------------------------------------------------

DbgCli_Cmd_MqttClientSub::DbgCli_Cmd_MqttClientSub(DbgCli_Topic* mqttClientTopic, MqttClientController* mqttClient)
: DbgCli_Command(mqttClientTopic, "sub", "Subscribe to a topic using MQTT client.")
, m_mqttClient(mqttClient)
{ }

void DbgCli_Cmd_MqttClientSub::execute(unsigned int argc, const char** args, unsigned int idxToFirstArgToHandle)
{
  if (argc - idxToFirstArgToHandle != 1)
  {
    printUsage();
  }
  else
  {
    if (0 != m_mqttClient)
    {
      int retVal = m_mqttClient->subscribe(args[idxToFirstArgToHandle]);
      Serial.print("MQTT client, subscribe ");
      Serial.println(retVal == 1 ? "successful" : "failed");
    }
  }
}

void DbgCli_Cmd_MqttClientSub::printUsage()
{
  Serial.println(getHelpText());
  Serial.println("Usage: dbg mqtt sub <topic>");
}

//-----------------------------------------------------------------------------

DbgCli_Cmd_MqttClientUnsub::DbgCli_Cmd_MqttClientUnsub(DbgCli_Topic* mqttClientTopic, MqttClientController* mqttClient)
: DbgCli_Command(mqttClientTopic, "unsub", "Unsubscribe a topic using MQTT client.")
, m_mqttClient(mqttClient)
{ }

void DbgCli_Cmd_MqttClientUnsub::execute(unsigned int argc, const char** args, unsigned int idxToFirstArgToHandle)
{
  if (argc - idxToFirstArgToHandle != 1)
  {
    printUsage();
  }
  else
  {
    if (0 != m_mqttClient)
    {
      int retVal = m_mqttClient->unsubscribe(args[idxToFirstArgToHandle]);
      Serial.print("MQTT client, unsubscribe ");
      Serial.println(retVal == 1 ? "successful" : "failed");
    }
  }
}

void DbgCli_Cmd_MqttClientUnsub::printUsage()
{
  Serial.println(getHelpText());
  Serial.println("Usage: dbg mqtt unsub <topic>");
}

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
