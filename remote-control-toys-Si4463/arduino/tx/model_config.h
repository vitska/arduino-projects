#ifndef __MODEL_CONFIG_H__
#define __MODEL_CONFIG_H__

typedef struct tag_DEVICE_CONFIG
{
  uint8_t mode;       //0 //44332211 - 11 - Throtle channel
                  //      22 - Rudder channel
                  //      33 - Elev channel
                  //      44 - Ail channel
  uint16_t batt_lo;     //1 //10.6V = 106 
  uint16_t batt_hi;     //3 //10.6V = 106 
  uint8_t  sel_model;     //5
  uint16_t lcd_contr;     //6
  uint16_t lcd_contr_min;   //8
  uint16_t lcd_contr_max;   //10
  uint16_t lcd_bk_tmout;    //12
  uint8_t  lcd_duty;      //14
  uint16_t ac_cal_min[6];   //15 //12
  uint16_t ac_cal_max[6];   //27 //12
  int16_t  bat_adc_const;   //39
  int16_t  uart_baud_const; //41
//  uint8_t  filler[256-43];  //43
}DEVICE_CONFIG;

typedef struct tag_SWVAL{
  int16_t on;  //-1024 .. +1024
  int16_t mid; //-1024 .. +1024
  int16_t off; //-1024 .. +1024
}SWVAL;

typedef struct tag_SERVOTRAVEL{
  uint8_t hi;  //0..255
  uint8_t lo; //0..255
}SERVOTRAVEL;

typedef struct{
    unsigned add_val:6;//0..63
    unsigned main_val:6;//0..63
    unsigned activate:4;//0 - always off, 1 - always on, 2..10 - 1-8 SWITCH
    }MUXFLAGS;

// MIX definition structure
typedef struct tag_MUXDEF{
  MUXFLAGS flags; //2
  int8_t  add_val_amount; //-100%..+100%
  int16_t add_val_offset; //-1024 .. +1024
}MUXDEF;

typedef struct tag_MODEL_CONFIG{
  uint8_t   ch_num;      //0   //1   // RRmXXXXX Number of channels(X) / modulation (m)  
  uint8_t   channel_map[16]; //1   //16                        
  uint16_t  Reverse;    //17  //2   // 1-16 channel reverce             
  int16_t   stick_trim[4]; //19  //8                     
  int16_t   sub_trim[16];  //27  //32
  int16_t   pseudo_analog[2];      //59  //4
  SWVAL     sw_val[8];      //63  //48
  SERVOTRAVEL travel[16];   //111 //32
  MUXDEF    mix[6];     //143 //30
  char      model_name[12];  //173 //12
  uint16_t  adc_test;    //185 //2   //8x2 bits
//  uint8_t   filler[69];   //187 //69
                //256
}MODEL_CONFIG; //256

//#define MODELS_IN_EEMEMORY 15

typedef struct tag_STORED_CONFIG{
  DEVICE_CONFIG device; // Configuration values not related to model config but require to store in nonvolatile memory
  MODEL_CONFIG model;   // Model settings
}STORED_CONFIG;
extern STORED_CONFIG config;


//#define EEFL_CH_DEV_CFG    0b1
//#define EEFL_CH_MODEL_CFG 0b10

//typedef struct tag_EepromState{
//  uint8_t flags;
//  uint8_t change_timeout;
//}EepromState_t; //256
//extern EepromState_t eeprom;

#define STICK1_UPDN 0
#define STICK1_LFRT 1
#define STICK2_UPDN 2
#define STICK2_LFRT 3

//#define PPM_OUT_DEBUG 1

void MarkChanged(char item);
#define MODEL_CFG_CHANGED MarkChanged(EEFL_CH_MODEL_CFG)

void init_model_config();
//void CheckToSave();
//void SetModel(uint8_t Model);
//uint8_t FlushModelConfig();
//void ReadModel(MODEL_CONFIG* buf, uint8_t Model);

#endif
