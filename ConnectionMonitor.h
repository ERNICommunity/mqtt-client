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
  virtual bool appProtocolConnectedRaw();
  virtual bool shallAppProtocolConnect();
  virtual void actionConnectAppProtocol() { }
  virtual void notifyLanConnected(bool isLanConnected) { }
  virtual void notifyAppProtocolConnected(bool isMqttConnected) { }
  DbgTrace_Port* trPort();

private:
  DbgTrace_Port* m_trPort;

private:
  // forbidden default functions
  ConnMonAdapter& operator =(const ConnMonAdapter& src); // assignment operator
  ConnMonAdapter(const ConnMonAdapter& src);             // copy constructor
};

class ConnMon
{
public:
  ConnMon(ConnMonAdapter* adapter = 0);
  virtual ~ConnMon();
  ConnMonAdapter* adapter();
  bool isLanDeviceConnected();
  bool isAppProtocolLibConnected();
  bool shallAppProtocolConnect();
  bool isLanConnected();
  bool isAppProtocolConnected();
  void evaluateState();
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
  ConnMon& operator =(const ConnMon& src);  // assignment operator
  ConnMon(const ConnMon& src);              // copy constructor
};

//-----------------------------------------------------------------------------

class ConnMonState
{
protected:
  ConnMonState() { }

public:
  virtual ~ConnMonState() { }
  virtual void evaluateState(ConnMon* monitor) = 0;
//  virtual void evaluateState(ConnMon* monitor, bool mqttState) { }
  virtual void timeExpired(ConnMon* monitor) { }
  virtual void entry(ConnMon* monitor);
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
  void evaluateState(ConnMon* monitor);
  void entry(ConnMon* monitor);
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
  void evaluateState(ConnMon* monitor);
  void timeExpired(ConnMon* monitor);
  void entry(ConnMon* monitor);
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
  void evaluateState(ConnMon* monitor);
//  void evaluateState(ConnMon* monitor, bool mqttState);
  void entry(ConnMon* monitor);
  const char* toString();

private:
  static ConnMonState* s_instance;
};

//-----------------------------------------------------------------------------

class ConnMonState_AppProtocolConnected : public ConnMonState
{
private:
  ConnMonState_AppProtocolConnected() { }

public:
  static ConnMonState* Instance();
  virtual ~ConnMonState_AppProtocolConnected() { }
  void evaluateState(ConnMon* monitor);
//  void evaluateState(ConnMon* monitor, bool mqttState);
  void entry(ConnMon* monitor);
  const char* toString();

private:
  static ConnMonState* s_instance;
};

//-----------------------------------------------------------------------------

#endif /* LIB_MQTT_CLIENT_CONNECTIONMONITOR_H_ */
