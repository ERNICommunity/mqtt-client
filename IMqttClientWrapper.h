/*
 * IMqttClientWrapper.h
 *
 *  Created on: 18.10.2016
 *      Author: nid
 */

#ifndef LIB_MQTT_CLIENT_IMQTTCLIENTWRAPPER_H_
#define LIB_MQTT_CLIENT_IMQTTCLIENTWRAPPER_H_

class Client;

class IMqttClientCallbackAdapter
{
protected:
  IMqttClientCallbackAdapter() { }

public:
  virtual ~IMqttClientCallbackAdapter() { }

  virtual void messageReceived(char* topic, byte* payload, unsigned int length) = 0;
};

class IMqttClientWrapper
{
protected:
  IMqttClientWrapper() { }

public:
  virtual ~IMqttClientWrapper() { }

  /**
   * Set callback adapter.
   * @param IMqttClientCallbackAdapter Pointer to an object implementing the interface to the MQTT Client Callback Adapter.
   */
  virtual void setCallbackAdapter(IMqttClientCallbackAdapter* callbackAdapter) = 0;

  virtual IMqttClientCallbackAdapter* callbackAdapter() = 0;

  virtual Client& client() = 0;

  /**
   * Process MQTT messages.
   * To be called in the Arduino loop() function.
   */
  virtual void processMessages() = 0;

  virtual bool connect(const char* id) = 0;
  virtual void disconnect() = 0;
  virtual bool connected() = 0;

  /**
   * Publish data in a message to the mentioned MQTT Topic.
   * @param topic The MQTT topic to publish to
   * @param data  The data to be published
   */
  virtual unsigned char publish(const char* topic, const char* data) = 0;

  /**
   * Subscribe the mentioned MQTT Topic.
   * @param topic The MQTT topic to subscribe from
   */
  virtual unsigned char subscribe(const char* topic) = 0;

  /**
   * Unsubscribe the mentioned MQTT Topic.
   * @param topic The MQTT topic to unsubscribe
   */
  virtual unsigned char unsubscribe(const char* topic) = 0;

  typedef enum
  {
    eIMqttCS_Connected              =  0,
    eIMqttCS_ConnectBadProtocol     =  1,
    eIMqttCS_ConnectBadClientId     =  2,
    eIMqttCS_ConnectUnavailable     =  3,
    eIMqttCS_ConnectBadCredentials  =  4,
    eIMqttCS_ConnectUnauthorized    =  5,
    eIMqttCS_ConnectionTimeout      = 12,
    eIMqttCS_ConnectionLost         = 13,
    eIMqttCS_ConnectFailed          = 14,
    eIMqttCS_Disconnected           = 15
  } eIMqttClientState;

  virtual eIMqttClientState state() = 0;

  virtual const char* stateStr()
  {
    eIMqttClientState st = state();
    return (
      (eIMqttCS_Connected             == st) ? "MqttCS_Connected"             :
      (eIMqttCS_ConnectBadProtocol    == st) ? "MqttCS_ConnectBadProtocol"    :
      (eIMqttCS_ConnectBadClientId    == st) ? "MqttCS_ConnectBadClientId"    :
      (eIMqttCS_ConnectUnavailable    == st) ? "MqttCS_ConnectUnavailable"    :
      (eIMqttCS_ConnectBadCredentials == st) ? "MqttCS_ConnectBadCredentials" :
      (eIMqttCS_ConnectUnauthorized   == st) ? "MqttCS_ConnectUnauthorized"   :
      (eIMqttCS_ConnectionTimeout     == st) ? "MqttCS_ConnectionTimeout"     :
      (eIMqttCS_ConnectionLost        == st) ? "MqttCS_ConnectionLost"        :
      (eIMqttCS_ConnectFailed         == st) ? "MqttCS_ConnectFailed"         :
      (eIMqttCS_Disconnected          == st) ? "MqttCS_Disconnected"          : "MqttCS_UNKNOWN");
  }

private: // forbidden default functions
  IMqttClientWrapper& operator = (const IMqttClientWrapper& src); // assignment operator
  IMqttClientWrapper(const IMqttClientWrapper& src);              // copy constructor
};

#endif /* LIB_MQTT_CLIENT_IMQTTCLIENTWRAPPER_H_ */
