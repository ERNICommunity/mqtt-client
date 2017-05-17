/*
 * LanConnectionMonitor.cpp
 *
 *  Created on: 26.10.2016
 *      Author: dini
 */

#include <Arduino.h>
#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif
#include <Timer.h>
#include <DbgTracePort.h>
#include <DbgTraceLevel.h>

#include <ConnectionMonitor.h>

class StatusPollTimerAdapter : public TimerAdapter
{
private:
  ConnectionMonitor* m_monitor;

public:
  StatusPollTimerAdapter(ConnectionMonitor* monitor)
  : m_monitor(monitor)
  { }

  void timeExpired()
  {
    if (0 != m_monitor)
    {
      m_monitor->evaluateState();
    }
  }
};

class StableCheckPollTimerAdapter : public TimerAdapter
{
private:
  ConnectionMonitor* m_monitor;

public:
  StableCheckPollTimerAdapter(ConnectionMonitor* monitor)
  : m_monitor(monitor)
  { }

  void timeExpired()
  {
    if ((0 != m_monitor) && (0 != m_monitor->state()))
    {
      m_monitor->state()->timeExpired(m_monitor);
    }
  }
};

//-----------------------------------------------------------------------------

ConnMonAdapter::ConnMonAdapter()
: m_trPort(new DbgTrace_Port("conmon", DbgTrace_Level::info))
{ }

ConnMonAdapter::~ConnMonAdapter()
{
  delete m_trPort;
  m_trPort = 0;
}

DbgTrace_Port* ConnMonAdapter::trPort()
{
  return m_trPort;
}

bool ConnMonAdapter::lanConnectedRaw()
{
  bool isLanConnected = false;
#ifdef ESP8266
  isLanConnected = WiFi.isConnected();
#endif
  TR_PRINTF(trPort(), DbgTrace_Level::debug, "WiFi device is %sconnected", (isLanConnected ? "" : "dis"));
  return isLanConnected;
}

bool ConnMonAdapter::mqttConnectedRaw()
{
  return false;
}

//-----------------------------------------------------------------------------

const unsigned long cStatusPollIntervalMillis  = 1000;
const unsigned long cStableCheckIntervalMillis = 2000;

ConnectionMonitor::ConnectionMonitor(ConnMonAdapter* adapter)
: m_statusPollTimer(new Timer(new StatusPollTimerAdapter(this), Timer::IS_RECURRING, cStatusPollIntervalMillis))
, m_stableConnCheckTimer(new Timer(new StableCheckPollTimerAdapter(this), Timer::IS_NON_RECURRING, cStableCheckIntervalMillis))
, m_adapter(adapter)
, m_state(ConnMonState_Unconnected::Instance())
, m_prevState(ConnMonState_Unconnected::Instance())
{
  if (0 == m_adapter)
  {
    new ConnMonAdapter();
  }
}

ConnectionMonitor::~ConnectionMonitor()
{
  delete m_adapter;
  m_adapter = 0;

  delete m_statusPollTimer->adapter();
  m_statusPollTimer->attachAdapter(0);

  delete m_statusPollTimer;
  m_statusPollTimer = 0;

  delete m_stableConnCheckTimer->adapter();
  m_stableConnCheckTimer->attachAdapter(0);

  delete m_stableConnCheckTimer;
  m_stableConnCheckTimer = 0;
}

ConnMonAdapter* ConnectionMonitor::adapter()
{
  return m_adapter;
}

bool ConnectionMonitor::isLanDeviceConnected()
{
  bool isConn = false;
  if (0 != m_adapter)
  {
    isConn = m_adapter->lanConnectedRaw();
  }
  return isConn;
}

bool ConnectionMonitor::isMqttLibConnected()
{
  bool isConn = false;
  if (0 != m_adapter)
  {
    isConn = m_adapter->mqttConnectedRaw();
  }
  return isConn;
}

bool ConnectionMonitor::isLanConnected()
{
  return (ConnMonState_StableLanConnection::Instance() == state());
}

bool ConnectionMonitor::isMqttConnected()
{
  return (ConnMonState_MqttConnected::Instance() == state());
}

void ConnectionMonitor::evaluateState()
{
  if (0 != m_state)
  {
    m_state->evaluateState(this);
  }
}

void ConnectionMonitor::setMqttState(bool mqttIsConnected)
{
  if (0 != m_state)
  {
    m_state->evaluateState(this, mqttIsConnected);
  }
}

void ConnectionMonitor::startStableLanConnCheckTimer()
{
  m_stableConnCheckTimer->startTimer(cStableCheckIntervalMillis);
}

void ConnectionMonitor::changeState(ConnMonState* newState)
{
  m_prevState = m_state;
  m_state = newState;
  if (0 != newState)
  {
    newState->entry(this);
  }
}

ConnMonState* ConnectionMonitor::state()
{
  return m_state;
}

ConnMonState* ConnectionMonitor::prevState()
{
  return m_prevState;
}

//-----------------------------------------------------------------------------


void ConnMonState::entry(ConnectionMonitor* monitor)
{
  TR_PRINTF(monitor->adapter()->trPort(), DbgTrace_Level::info, "FSM, entering state %s [from %s]", monitor->state()->toString(), monitor->prevState()->toString());
}

//-----------------------------------------------------------------------------

ConnMonState* ConnMonState_Unconnected::s_instance = 0;

ConnMonState* ConnMonState_Unconnected::Instance()
{
  if (0 == s_instance)
  {
    s_instance = new ConnMonState_Unconnected();
  }
  return s_instance;
}

void ConnMonState_Unconnected::evaluateState(ConnectionMonitor* monitor)
{
  if (monitor->isLanDeviceConnected())
  {
    monitor->changeState(ConnMonState_LanConnected::Instance());
  }
}

void ConnMonState_Unconnected::entry(ConnectionMonitor* monitor)
{
  ConnMonState::entry(monitor);
  monitor->adapter()->notifyLanConnected(false);
  monitor->adapter()->notifyMqttConnected(false);
}

const char* ConnMonState_Unconnected::toString()
{
  return "Unconnected";
}

//-----------------------------------------------------------------------------

ConnMonState* ConnMonState_LanConnected::s_instance = 0;

ConnMonState* ConnMonState_LanConnected::Instance()
{
  if (0 == s_instance)
  {
    s_instance = new ConnMonState_LanConnected();
  }
  return s_instance;
}

void ConnMonState_LanConnected::evaluateState(ConnectionMonitor* monitor)
{
  if (!monitor->isLanDeviceConnected())
  {
    monitor->changeState(ConnMonState_Unconnected::Instance());
  }
}

void ConnMonState_LanConnected::timeExpired(ConnectionMonitor* monitor)
{
  if (monitor->isLanDeviceConnected())
  {
    monitor->changeState(ConnMonState_StableLanConnection::Instance());
  }
  else
  {
    monitor->changeState(ConnMonState_Unconnected::Instance());
  }
}

void ConnMonState_LanConnected::entry(ConnectionMonitor* monitor)
{
  ConnMonState::entry(monitor);
  monitor->startStableLanConnCheckTimer();
}

const char* ConnMonState_LanConnected::toString()
{
  return "LanConnected";
}

//-----------------------------------------------------------------------------

ConnMonState* ConnMonState_StableLanConnection::s_instance = 0;

ConnMonState* ConnMonState_StableLanConnection::Instance()
{
  if (0 == s_instance)
  {
    s_instance = new ConnMonState_StableLanConnection();
  }
  return s_instance;
}

void ConnMonState_StableLanConnection::evaluateState(ConnectionMonitor* monitor)
{
  if (monitor->isLanDeviceConnected())
  {
    if (monitor->isMqttLibConnected())
    {
      monitor->changeState(ConnMonState_MqttConnected::Instance());
    }
  }
  else
  {
    monitor->changeState(ConnMonState_Unconnected::Instance());
  }
}

void ConnMonState_StableLanConnection::evaluateState(ConnectionMonitor* monitor, bool mqttState)
{
  if (mqttState)
  {
    monitor->changeState(ConnMonState_MqttConnected::Instance());
  }
}

void ConnMonState_StableLanConnection::entry(ConnectionMonitor* monitor)
{
  ConnMonState::entry(monitor);
  monitor->adapter()->notifyLanConnected(true);
}

const char* ConnMonState_StableLanConnection::toString()
{
  return "StableLanConnection";
}

//-----------------------------------------------------------------------------

ConnMonState* ConnMonState_MqttConnected::s_instance = 0;

ConnMonState* ConnMonState_MqttConnected::Instance()
{
  if (0 == s_instance)
  {
    s_instance = new ConnMonState_MqttConnected();
  }
  return s_instance;
}

void ConnMonState_MqttConnected::evaluateState(ConnectionMonitor* monitor)
{
  if (monitor->isLanDeviceConnected())
  {
    if (!monitor->isMqttLibConnected())
    {
      monitor->changeState(ConnMonState_StableLanConnection::Instance());
      monitor->adapter()->notifyMqttConnected(false);
    }
  }
  else
  {
    monitor->changeState(ConnMonState_Unconnected::Instance());
  }
}

void ConnMonState_MqttConnected::evaluateState(ConnectionMonitor* monitor, bool mqttState)
{
  if (!mqttState)
  {
    monitor->changeState(ConnMonState_LanConnected::Instance());
  }
}

void ConnMonState_MqttConnected::entry(ConnectionMonitor* monitor)
{
  ConnMonState::entry(monitor);
  monitor->adapter()->notifyMqttConnected(true);
}

const char* ConnMonState_MqttConnected::toString()
{
  return "MqttConnected";
}
