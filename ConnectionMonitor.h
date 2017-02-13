/*
 * LanConnectionMonitor.h
 *
 *  Created on: 26.10.2016
 *      Author: dini
 */

#ifndef LIB_MQTT_CLIENT_CONNECTIONMONITOR_H_
#define LIB_MQTT_CLIENT_CONNECTIONMONITOR_H_

class Timer;
class ConnMonState;
class DbgTrace_Port;

class ConnMonAdapter
{
public:
  ConnMonAdapter();
  virtual ~ConnMonAdapter();
  virtual bool lanConnectedRaw();
  virtual bool mqttConnectedRaw();
  virtual void notifyLanConnected(bool isLanConnected) { }
  virtual void notifyMqttConnected(bool isMqttConnected) { }
  DbgTrace_Port* trPort();

private:
  DbgTrace_Port* m_trPort;

private:
  // forbidden default functions
  ConnMonAdapter& operator =(const ConnMonAdapter& src); // assignment operator
  ConnMonAdapter(const ConnMonAdapter& src);             // copy constructor
};

class ConnectionMonitor
{
public:
  ConnectionMonitor(ConnMonAdapter* adapter = 0);
  virtual ~ConnectionMonitor();
  ConnMonAdapter* adapter();
  bool isLanDeviceConnected();
  bool isMqttLibConnected();
  bool isLanConnected();
  bool isMqttConnected();
  void evaluateState();
  void setMqttState(bool mqttIsConnected);
  void changeState(ConnMonState* newState);
  ConnMonState* state();
  ConnMonState* prevState();
  void startStableLanConnCheckTimer();

private:
  Timer* m_statusPollTimer;
  Timer* m_stableConnCheckTimer;
  ConnMonAdapter* m_adapter;
  ConnMonState* m_state;
  ConnMonState* m_prevState;

private:
  // forbidden default functions
  ConnectionMonitor& operator =(const ConnectionMonitor& src);  // assignment operator
  ConnectionMonitor(const ConnectionMonitor& src);              // copy constructor
};

//-----------------------------------------------------------------------------

class ConnMonState
{
protected:
  ConnMonState() { }

public:
  virtual ~ConnMonState() { }
  virtual void evaluateState(ConnectionMonitor* monitor) = 0;
  virtual void evaluateState(ConnectionMonitor* monitor, bool mqttState) { }
  virtual void timeExpired(ConnectionMonitor* monitor) { }
  virtual void entry(ConnectionMonitor* monitor);
  virtual const char* toString() = 0;
};

//-----------------------------------------------------------------------------

class ConnMonState_Unconnected : public ConnMonState
{
private:
  ConnMonState_Unconnected() { }

public:
  static ConnMonState* Instance();
  virtual ~ConnMonState_Unconnected() { }
  void evaluateState(ConnectionMonitor* monitor);
  void entry(ConnectionMonitor* monitor);
  const char* toString();

private:
  static ConnMonState* s_instance;
};

//-----------------------------------------------------------------------------

class ConnMonState_LanConnected : public ConnMonState
{
private:
  ConnMonState_LanConnected() { }

public:
  static ConnMonState* Instance();
  virtual ~ConnMonState_LanConnected() { }
  void evaluateState(ConnectionMonitor* monitor);
  void timeExpired(ConnectionMonitor* monitor);
  void entry(ConnectionMonitor* monitor);
  const char* toString();

private:
  static ConnMonState* s_instance;
};

//-----------------------------------------------------------------------------

class ConnMonState_StableLanConnection : public ConnMonState
{
private:
  ConnMonState_StableLanConnection() { }

public:
  static ConnMonState* Instance();
  virtual ~ConnMonState_StableLanConnection() { }
  void evaluateState(ConnectionMonitor* monitor);
  void evaluateState(ConnectionMonitor* monitor, bool mqttState);
  void entry(ConnectionMonitor* monitor);
  const char* toString();

private:
  static ConnMonState* s_instance;
};

//-----------------------------------------------------------------------------

class ConnMonState_MqttConnected : public ConnMonState
{
private:
  ConnMonState_MqttConnected() { }

public:
  static ConnMonState* Instance();
  virtual ~ConnMonState_MqttConnected() { }
  void evaluateState(ConnectionMonitor* monitor);
  void evaluateState(ConnectionMonitor* monitor, bool mqttState);
  void entry(ConnectionMonitor* monitor);
  const char* toString();

private:
  static ConnMonState* s_instance;
};

//-----------------------------------------------------------------------------

#endif /* LIB_MQTT_CLIENT_CONNECTIONMONITOR_H_ */
