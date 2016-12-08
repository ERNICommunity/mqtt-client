/*
 * LanConnectionMonitor.h
 *
 *  Created on: 26.10.2016
 *      Author: dini
 */

#ifndef LIB_MQTT_CLIENT_LANCONNECTIONMONITOR_H_
#define LIB_MQTT_CLIENT_LANCONNECTIONMONITOR_H_

class Timer;
class LanConnMonState;
class DbgTrace_Port;

class LanConnMonAdapter
{
public:
  LanConnMonAdapter();
  virtual ~LanConnMonAdapter();
  virtual bool connectedRaw();
  virtual void notifyLanConnected(bool isLanConnected) { }
  DbgTrace_Port* trPort();

private:
  DbgTrace_Port* m_trPort;

private:
  // forbidden default functions
  LanConnMonAdapter& operator =(const LanConnMonAdapter& src); // assignment operator
  LanConnMonAdapter(const LanConnMonAdapter& src);     // copy constructor
};

class LanConnectionMonitor
{
public:
  LanConnectionMonitor(LanConnMonAdapter* adapter = 0);
  virtual ~LanConnectionMonitor();
  LanConnMonAdapter* adapter();
  bool isConnectedRaw();
  bool isConnected();
  void evaluateState();
  void changeState(LanConnMonState* newState);
  LanConnMonState* state();
  LanConnMonState* prevState();
//  const char* getCurrentStateName();
  void startStableConnCheckTimer();

private:
  Timer* m_statusPollTimer;
  Timer* m_stableConnCheckTimer;
  LanConnMonAdapter* m_adapter;
  LanConnMonState* m_state;
  LanConnMonState* m_prevState;

private:
  // forbidden default functions
  LanConnectionMonitor& operator =(const LanConnectionMonitor& src);  // assignment operator
  LanConnectionMonitor(const LanConnectionMonitor& src);              // copy constructor
};

//-----------------------------------------------------------------------------

class LanConnMonState
{
protected:
  LanConnMonState() { }

public:
  virtual ~LanConnMonState() { }
  virtual void evaluateState(LanConnectionMonitor* monitor) = 0;
  virtual void timeExpired(LanConnectionMonitor* monitor) { }
  virtual void entry(LanConnectionMonitor* monitor);
  virtual const char* toString() = 0;
};

//-----------------------------------------------------------------------------

class LanConnMonState_Unconnected : public LanConnMonState
{
private:
  LanConnMonState_Unconnected() { }

public:
  static LanConnMonState* Instance();
  virtual ~LanConnMonState_Unconnected() { }
  void evaluateState(LanConnectionMonitor* monitor);
  void entry(LanConnectionMonitor* monitor);
  const char* toString();

private:
  static LanConnMonState* s_instance;
};

//-----------------------------------------------------------------------------

class LanConnMonState_Connected : public LanConnMonState
{
private:
  LanConnMonState_Connected() { }

public:
  static LanConnMonState* Instance();
  virtual ~LanConnMonState_Connected() { }
  void evaluateState(LanConnectionMonitor* monitor);
  void timeExpired(LanConnectionMonitor* monitor);
  void entry(LanConnectionMonitor* monitor);
  const char* toString();

private:
  static LanConnMonState* s_instance;
};

//-----------------------------------------------------------------------------

class LanConnMonState_StableConnection : public LanConnMonState
{
private:
  LanConnMonState_StableConnection() { }

public:
  static LanConnMonState* Instance();
  virtual ~LanConnMonState_StableConnection() { }
  void evaluateState(LanConnectionMonitor* monitor);
  void entry(LanConnectionMonitor* monitor);
  const char* toString();

private:
  static LanConnMonState* s_instance;
};

//-----------------------------------------------------------------------------

#endif /* LIB_MQTT_CLIENT_LANCONNECTIONMONITOR_H_ */
