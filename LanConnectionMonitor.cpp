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

#include <LanConnectionMonitor.h>

class StatusPollTimerAdapter : public TimerAdapter
{
private:
  LanConnectionMonitor* m_monitor;

public:
  StatusPollTimerAdapter(LanConnectionMonitor* monitor)
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
  LanConnectionMonitor* m_monitor;

public:
  StableCheckPollTimerAdapter(LanConnectionMonitor* monitor)
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

bool LanConnMonAdapter::connectedRaw()
{
  bool isConnected = false;
#ifdef ESP8266
  isConnected = WiFi.isConnected();
#endif
  Serial.print("LanConnMonAdapter: ");
  Serial.println(isConnected ? "connected" : "disconnected");
  return isConnected;
}

const unsigned long cStatusPollIntervalMillis  = 1000;
const unsigned long cStableCheckIntervalMillis = 6000;

LanConnectionMonitor::LanConnectionMonitor(LanConnMonAdapter* adapter)
: m_statusPollTimer(new Timer(new StatusPollTimerAdapter(this), Timer::IS_RECURRING, cStatusPollIntervalMillis))
, m_stableConnCheckTimer(new Timer(new StableCheckPollTimerAdapter(this), Timer::IS_NON_RECURRING, cStableCheckIntervalMillis))
, m_adapter(adapter)
, m_state(LanConnMonState_Unconnected::Instance())
, m_prevState(LanConnMonState_Unconnected::Instance())
{
  if (0 == m_adapter)
  {
    new LanConnMonAdapter();
  }
}

LanConnectionMonitor::~LanConnectionMonitor()
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

LanConnMonAdapter* LanConnectionMonitor::adapter()
{
  return m_adapter;
}

bool LanConnectionMonitor::isConnectedRaw()
{
  bool isConn = false;
  if (0 != m_adapter)
  {
    isConn = m_adapter->connectedRaw();
  }
  return isConn;
}

bool LanConnectionMonitor::isConnected()
{
  return (LanConnMonState_StableConnection::Instance() == state());
}

void LanConnectionMonitor::evaluateState()
{
  if (0 != m_state)
  {
    Serial.println("LanConnectionMonitor::evaluateState()");
    Serial.print("State: ");
    Serial.println(m_state == LanConnMonState_Unconnected::Instance()      ? "LanConnMonState_Unconnected"      :
                   m_state == LanConnMonState_Connected::Instance()        ? "LanConnMonState_Connected"        :
                   m_state == LanConnMonState_StableConnection::Instance() ? "LanConnMonState_StableConnection" : "Unknown");
    m_state->evaluateState(this);
  }
}

void LanConnectionMonitor::startStableConnCheckTimer()
{
  m_stableConnCheckTimer->startTimer(cStableCheckIntervalMillis);
}

void LanConnectionMonitor::changeState(LanConnMonState* newState)
{
  m_prevState = m_state;
  m_state = newState;
  if (0 != newState)
  {
    newState->entry(this);
  }
}

LanConnMonState* LanConnectionMonitor::state()
{
  return m_state;
}

const char* LanConnectionMonitor::getCurrentStateName()
{
  if (0 == m_state)
  {
    return "LanConnectionMonitor::m_state, null pointer exception";
  }
  return m_state->toString();
}


//-----------------------------------------------------------------------------

LanConnMonState* LanConnMonState_Unconnected::s_instance = 0;

LanConnMonState* LanConnMonState_Unconnected::Instance()
{
  if (0 == s_instance)
  {
    s_instance = new LanConnMonState_Unconnected();
  }
  return s_instance;
}

void LanConnMonState_Unconnected::evaluateState(LanConnectionMonitor* monitor)
{
  if (monitor->isConnectedRaw())
  {
    monitor->changeState(LanConnMonState_Connected::Instance());
  }
}

void LanConnMonState_Unconnected::entry(LanConnectionMonitor* monitor)
{
  Serial.println("LAN Conn FSM: entered Unconnected state");
}

const char* LanConnMonState_Unconnected::toString()
{
  return "Unconnected";
}

//-----------------------------------------------------------------------------

LanConnMonState* LanConnMonState_Connected::s_instance = 0;

LanConnMonState* LanConnMonState_Connected::Instance()
{
  if (0 == s_instance)
  {
    s_instance = new LanConnMonState_Connected();
  }
  return s_instance;
}

void LanConnMonState_Connected::evaluateState(LanConnectionMonitor* monitor)
{
  if (!monitor->isConnectedRaw())
  {
    monitor->changeState(LanConnMonState_Unconnected::Instance());
  }
//  else
//  {
//    monitor->startStableConnCheckTimer();
//  }
}

void LanConnMonState_Connected::timeExpired(LanConnectionMonitor* monitor)
{
  if (monitor->isConnectedRaw())
  {
    monitor->changeState(LanConnMonState_StableConnection::Instance());
  }
  else
  {
    monitor->changeState(LanConnMonState_Unconnected::Instance());
  }
}

void LanConnMonState_Connected::entry(LanConnectionMonitor* monitor)
{
  Serial.println("LAN Conn FSM: entered Connected state");
  monitor->startStableConnCheckTimer();
}

const char* LanConnMonState_Connected::toString()
{
  return "Connected";
}

//-----------------------------------------------------------------------------

LanConnMonState* LanConnMonState_StableConnection::s_instance = 0;

LanConnMonState* LanConnMonState_StableConnection::Instance()
{
  if (0 == s_instance)
  {
    s_instance = new LanConnMonState_StableConnection();
  }
  return s_instance;
}

void LanConnMonState_StableConnection::evaluateState(LanConnectionMonitor* monitor)
{
  if (!monitor->isConnectedRaw())
  {
    monitor->changeState(LanConnMonState_Unconnected::Instance());
  }
}

void LanConnMonState_StableConnection::entry(LanConnectionMonitor* monitor)
{
  Serial.println("LAN Conn FSM: entered StableConnection state");
}

const char* LanConnMonState_StableConnection::toString()
{
  return "StableConnection";
}