#ifndef __SENSORS_HPP__
#define __SENSORS_HPP__

#include "AsyncMqttClient_Generic.hpp"
#include "channel_data.h"

extern int dBmtoPercentage(int dBm);

class SensorValue {
  public:
    SensorValue(const char *_name, const char *_json_property, int _ch_num){
      name = _name;
      json_property = _json_property;
      ch_num = _ch_num;
    }

    const char *name;
    const char *json_property;
    int ch_num;

    virtual const char* getDeviceClass() = 0;

    virtual const char* getAdditionalDiscoveryProps() { return ",\"state_class\":\"measurement\""; }

    virtual const char* getUnit() = 0;

    virtual double getValue() = 0;

    virtual void mqttDiscoveryPublish(AsyncMqttClient* pmqttClient);

  protected:
    void mqtt_discovery_publish(AsyncMqttClient* pmqttClient, const char* value_template, const char* state_topic, const char* topic);
};

class PowerValue : public SensorValue {
  public:
    PowerValue(const char *_name, const char *_json_property, int _ch_num) : SensorValue(_name, _json_property, _ch_num){
    }

    const char* getDeviceClass() { return "power"; }

    const char* getUnit() { return "W"; }

    double getValue() { return channel[ch_num].watt; }
};

class PowerBinaryValue : public PowerValue {
  public:
    PowerBinaryValue(const char *_name, const char *_json_property) : PowerValue(_name, _json_property, -1){
    }

    const char* getUnit() { return NULL; }

    const char* getAdditionalDiscoveryProps() { return ",\"payload_on\":1,\"payload_off\":0"; }

    double getValue() { return 1; }

    void mqttDiscoveryPublish(AsyncMqttClient* pmqttClient);
};

class EnergyValue : public SensorValue {
  public:
    EnergyValue(const char *_name, const char *_json_property, int _ch_num) : SensorValue(_name, _json_property, _ch_num){
    }

    virtual const char* getAdditionalDiscoveryProps() { return ",\"state_class\":\"total\""; }

    const char* getDeviceClass() { return "energy"; }

    const char* getUnit() { return "kWh"; }

    double getValue() { return channel[ch_num].cumulative_kwatt_h; }
};

class SumPowerValue : public PowerValue {
  public:
    SumPowerValue(const char *_name, const char *_json_property) : PowerValue(_name, _json_property, -1){
    }

    double getValue() { return channel[0].watt + channel[1].watt + channel[2].watt; }
};

class SumEnergyValue : public EnergyValue {
  public:
    SumEnergyValue(const char *_name, const char *_json_property) : EnergyValue(_name, _json_property, -1){
    }

    double getValue() { return channel[0].cumulative_kwatt_h + channel[1].cumulative_kwatt_h + channel[2].cumulative_kwatt_h; }
};

class RssiValue : public PowerValue {
  public:
    RssiValue(const char *_name, const char *_json_property) : PowerValue(_name, _json_property, -1){
    }
    const char* getAdditionalDiscoveryProps() { return ""; }
    const char* getDeviceClass() { return "signal_strength"; }
    const char* getUnit() { return "dbm"; }
    double getValue();
    void mqttDiscoveryPublish(AsyncMqttClient* pmqttClient);
};

class RssiPercentValue : public RssiValue {
  public:
    RssiPercentValue(const char *_name, const char *_json_property) : RssiValue(_name, _json_property){
    }
    const char* getUnit() { return "%"; }
    double getValue();
    void mqttDiscoveryPublish(AsyncMqttClient* pmqttClient);
};

#endif