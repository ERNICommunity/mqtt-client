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
#include <ConnectionMonitor.h>

#include <MqttTopic.h>

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
      MqttTopicPublisher* publisher = m_mqttClient->findPublisherByTopic(args[idxToFirstArgToHandle]);
      if (0 == publisher)
      {
        publisher = new MqttTopicPublisher(args[idxToFirstArgToHandle], args[idxToFirstArgToHandle+1]);
      }
      publisher->publish();
      Serial.print("MQTT client, published to ");
      Serial.println(args[idxToFirstArgToHandle+1]);
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
      MqttTopicSubscriber* subscriber = m_mqttClient->findSubscriberByTopic(args[idxToFirstArgToHandle]);
      if (0 == subscriber)
      {
        new DefaultMqttSubscriber(args[idxToFirstArgToHandle]);
        Serial.print("MQTT client, subscribed to ");
        Serial.println(args[idxToFirstArgToHandle]);
      }
      else
      {
        Serial.print("MQTT client, already subscribed to ");
        Serial.println(args[idxToFirstArgToHandle]);
      }
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
      MqttTopicSubscriber* subscriber = m_mqttClient->findSubscriberByTopic(args[idxToFirstArgToHandle]);
      if (0 != subscriber)
      {
        delete subscriber;
        Serial.print("MQTT client, unsubscribed from ");
        Serial.println(args[idxToFirstArgToHandle]);
      }
      else
      {
        Serial.print("MQTT client, unsubscribe not possible, topic was not subscribed: ");
        Serial.println(args[idxToFirstArgToHandle]);
      }
    }
  }
}

void DbgCli_Cmd_MqttClientUnsub::printUsage()
{
  Serial.println(getHelpText());
  Serial.println("Usage: dbg mqtt unsub <topic>");
}

//-----------------------------------------------------------------------------

DbgCli_Cmd_MqttClientShow::DbgCli_Cmd_MqttClientShow(DbgCli_Topic* mqttClientTopic, MqttClientController* mqttClient)
: DbgCli_Command(mqttClientTopic, "show", "Show info from the MQTT client.")
, m_mqttClient(mqttClient)
{ }

void DbgCli_Cmd_MqttClientShow::execute(unsigned int argc, const char** args, unsigned int idxToFirstArgToHandle)
{
  Serial.println("Subscriber Topics:");
  MqttTopicSubscriber* subscriber = m_mqttClient->mqttSubscriberChain();
  if (0 == subscriber)
  {
    Serial.println("no subscribers in the list.");
  }
  while (0 != subscriber)
  {
    Serial.println(subscriber->getTopicString());
    subscriber = subscriber->next();
  }
  Serial.println("Publisher Topics:");
  MqttTopicPublisher* publisher = m_mqttClient->mqttPublisherChain();
  if (0 == publisher)
  {
    Serial.println("no publishers in the list.");
  }
  while (0 != publisher)
  {
    Serial.println(publisher->getTopicString());
    publisher = publisher->next();
  }
  Serial.print("MQTT Client Status: Shall ");
  Serial.print(m_mqttClient->getShallConnect() ? "" : "not ");
  Serial.println("connect");
  Serial.print("Connection Monitor State: ");
  Serial.println(m_mqttClient->connMon()->state()->toString());
  m_mqttClient->loop();
}

//-----------------------------------------------------------------------------
