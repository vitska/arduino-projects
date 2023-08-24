#ifndef __CHANNEL_DATA_H__
#define __CHANNEL_DATA_H__

#define IN_CHANNELS_SZ 4

struct t_channel_data{
  uint16_t min;
  uint16_t max;
  uint16_t pp;
  float watt;
  float watt_prev;
  uint64_t runMilis;
  uint64_t last_millis;
  double period_watt_h;
  double cumulative_kwatt_h;
  uint64_t cumulative_milis;
  uint8_t changed;
};

extern t_channel_data channel[IN_CHANNELS_SZ];

#endif