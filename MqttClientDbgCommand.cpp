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
      TR_PRINTF(m_mqttClient->trPort(), DbgTrace_Level::alert, "MQTT client connecting to broker now.");
    }
  }
}

void DbgCli_Cmd_MqttClientCon::printUsage()
{
  TR_PRINTF(m_mqttClient->trPort(), DbgTrace_Level::alert, "%s", getHelpText());
  TR_PRINTF(m_mqttClient->trPort(), DbgTrace_Level::alert, "Usage: %s %s %s", RootNode()->getNodeName(), getParentNode()->getNodeName(), getNodeName());  
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
      TR_PRINTF(m_mqttClient->trPort(), DbgTrace_Level::alert, "MQTT client disconnecting from broker now.");
    }
  }
}

void DbgCli_Cmd_MqttClientDis::printUsage()
{
  TR_PRINTF(m_mqttClient->trPort(), DbgTrace_Level::alert, "%s", getHelpText());
  TR_PRINTF(m_mqttClient->trPort(), DbgTrace_Level::alert, "Usage: %s %s %s", RootNode()->getNodeName(), getParentNode()->getNodeName(), getNodeName());  
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
        publisher = new MqttTopicPublisher(args[idxToFirstArgToHandle], args[idxToFirstArgToHandle+1], MqttTopicPublisher::DO_AUTO_PUBLISH);
      }
	  else
      {
        publisher->setData(args[idxToFirstArgToHandle+1]);
        publisher->publish();
      }
      TR_PRINTF(m_mqttClient->trPort(), DbgTrace_Level::alert, "MQTT client, published to %s",  publisher->getTopicString(), publisher->getData());
    }
  }
}

void DbgCli_Cmd_MqttClientPub::printUsage()
{
  TR_PRINTF(m_mqttClient->trPort(), DbgTrace_Level::alert, "%s", getHelpText());
  TR_PRINTF(m_mqttClient->trPort(), DbgTrace_Level::alert, "Usage: %s %s %s <topic> <value>", RootNode()->getNodeName(), getParentNode()->getNodeName(), getNodeName());  
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
        subscriber = new DefaultMqttSubscriber(args[idxToFirstArgToHandle]);
        TR_PRINTF(m_mqttClient->trPort(), DbgTrace_Level::alert, "MQTT client, subscribed to %s", subscriber->getTopicString());
      }
      else
      {
        TR_PRINTF(m_mqttClient->trPort(), DbgTrace_Level::alert, "MQTT client, already subscribed to %s", subscriber->getTopicString());
      }
    }
  }
}

void DbgCli_Cmd_MqttClientSub::printUsage()
{
  TR_PRINTF(m_mqttClient->trPort(), DbgTrace_Level::alert, "%s", getHelpText());
  TR_PRINTF(m_mqttClient->trPort(), DbgTrace_Level::alert, "Usage: %s %s %s <topic>", RootNode()->getNodeName(), getParentNode()->getNodeName(), getNodeName());  
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
        TR_PRINTF(m_mqttClient->trPort(), DbgTrace_Level::alert, "MQTT client, unsubscribing from %s", subscriber->getTopicString());
        delete subscriber;
        subscriber = 0;
      }
      else
      {
        TR_PRINTF(m_mqttClient->trPort(), DbgTrace_Level::alert, "MQTT client, unsubscribe not possible, topic %s was not subscribed.", args[idxToFirstArgToHandle]);
      }
    }
  }
}

void DbgCli_Cmd_MqttClientUnsub::printUsage()
{
  TR_PRINTF(m_mqttClient->trPort(), DbgTrace_Level::alert, "%s", getHelpText());
  TR_PRINTF(m_mqttClient->trPort(), DbgTrace_Level::alert, "Usage: %s %s %s <topic>", RootNode()->getNodeName(), getParentNode()->getNodeName(), getNodeName());  
}

//-----------------------------------------------------------------------------

DbgCli_Cmd_MqttClientShow::DbgCli_Cmd_MqttClientShow(DbgCli_Topic* mqttClientTopic, MqttClientController* mqttClient)
: DbgCli_Command(mqttClientTopic, "show", "Show info from the MQTT client.")
, m_mqttClient(mqttClient)
{ }

void DbgCli_Cmd_MqttClientShow::execute(unsigned int argc, const char** args, unsigned int idxToFirstArgToHandle)
{
  TR_PRINTF(m_mqttClient->trPort(), DbgTrace_Level::alert, "Subscriber Topics:");
  MqttTopicSubscriber* subscriber = m_mqttClient->mqttSubscriberChain();
  if (0 == subscriber)
  {
    TR_PRINTF(m_mqttClient->trPort(), DbgTrace_Level::alert, "=x= no subscribers in the list.");
  }
  while (0 != subscriber)
  {
    TR_PRINTF(m_mqttClient->trPort(), DbgTrace_Level::alert, "- %s", subscriber->getTopicString());
    subscriber = subscriber->next();
  }
  TR_PRINTF(m_mqttClient->trPort(), DbgTrace_Level::alert, "Publisher Topics:");
  MqttTopicPublisher* publisher = m_mqttClient->mqttPublisherChain();
  if (0 == publisher)
  {
    TR_PRINTF(m_mqttClient->trPort(), DbgTrace_Level::alert, "=x= no publishers in the list.");
  }
  while (0 != publisher)
  {
    TR_PRINTF(m_mqttClient->trPort(), DbgTrace_Level::alert, "- %s", publisher->getTopicString());
    publisher = publisher->next();
  }
  TR_PRINTF(m_mqttClient->trPort(), DbgTrace_Level::alert, "MQTT Client Status: Shall %s connect", m_mqttClient->getShallConnect() ? "" : "not ");
  TR_PRINTF(m_mqttClient->trPort(), DbgTrace_Level::alert, "Connection Monitor State: %s", m_mqttClient->connMon()->state()->toString());
  m_mqttClient->loop();
}

//-----------------------------------------------------------------------------
