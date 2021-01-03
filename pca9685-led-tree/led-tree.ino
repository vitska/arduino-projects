     

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
 
// called this way, it uses the default address 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
 
void setup() {
  Serial.begin(9600);
  Serial.println("16 channel PWM test!");
  pwm.begin();
  pwm.setPWMFreq(100);  // This is the maximum PWM frequency
 
  // save I2C bitrate
  uint8_t twbrbackup = TWBR;
  // must be changed after calling Wire.begin() (inside pwm.begin())
  TWBR = 12; // upgrade to 400KHz!

  // All channels off
  for(int i=0;i<16;i++){
    pwm.setPWM(i, 0, 0 );
  }
 
}

int GetStep(int step){
  int result = step * step;
  if(result > 4){
    result--;
  }
  return result;
}

#define LGREEN1 0
#define LGREEN2 1
#define LGREEN3 2
#define LGREEN4 3

#define RGREEN1 15
#define RGREEN2 14
#define RGREEN3 13
#define RGREEN4 12

#define RED1 4
#define RED2 5

#define BLUE1 11
#define BLUE2 10

#define STAR 6

#define STEP_MAX 46
#define STEP_STAR_MAX STEP_MAX
#define STEP_GREEN_MAX 16
#define STEP_BLUE_MAX STEP_MAX
#define STEP_RED_MAX STEP_MAX


void SetPwm(byte* channels, int sz, int step, int shift){
    for(int ch = 0;ch<sz;ch++){ 
      pwm.setPWM(channels[ch], 0, GetStep(step) & 0x7FF );
    }
}

void SetPwmSingle(byte channel, int step, int shift){
    pwm.setPWM(channel, 0, GetStep(step) & 0x7FF );
}

byte green_all[] = {LGREEN1,LGREEN2,LGREEN3,LGREEN4,RGREEN1,RGREEN2,RGREEN3,RGREEN4};
byte green_low1[] = {LGREEN1,RGREEN1};
byte green_low2[] = {LGREEN2,RGREEN2};
byte green_low3[] = {LGREEN3,RGREEN3};
byte green_low4[] = {LGREEN4,RGREEN4};
byte blue_all[] = {BLUE1,BLUE2};
byte red_all[] = {RED1,RED2};
byte bluered_all[] = {RED1,RED2,BLUE1,BLUE2};


void FadeUpSingle(byte channel, int _delay = 50, int step_max = 32){
    for(int step = 0;step<step_max;step++){ 
      SetPwmSingle(channel, step, 0 );
      delay(_delay);
    }
}

void FadeUp(byte* channels, int sz, int _delay = 50, int step_max = 32){
    for(int step = 0;step<step_max;step++){ 
      SetPwm(channels, sz, step, 0 );
      delay(_delay);
    }
}


void FadeDownSingle(byte channel, int _delay = 50, int step_max = 32){
    for(int step = step_max-1;step>=0;step--){
      SetPwmSingle(channel, step, 0 );
      delay(_delay);
    }
}

void FadeDown(byte* channels, int sz, int _delay = 50, int step_max = 32){
    for(int step = step_max-1;step>=0;step--){
      SetPwm(channels, sz, step, 0 );
      delay(_delay);
    }
}

void Program1(){
  for(int i=0;i<2;i++){
    FadeUpSingle(STAR, 10, STEP_STAR_MAX);
    FadeUpSingle(LGREEN4, 5, STEP_GREEN_MAX);
    FadeUpSingle(LGREEN3, 5, STEP_GREEN_MAX);
    FadeUpSingle(LGREEN2, 5, STEP_GREEN_MAX);
    FadeUpSingle(LGREEN1, 5, STEP_GREEN_MAX);
    FadeDownSingle(STAR, 10, STEP_STAR_MAX);
    FadeDownSingle(LGREEN4, 5, STEP_GREEN_MAX);
    FadeDownSingle(LGREEN3, 5, STEP_GREEN_MAX);
    FadeDownSingle(LGREEN2, 5, STEP_GREEN_MAX);
    FadeDownSingle(LGREEN1, 5, STEP_GREEN_MAX);
    FadeUpSingle(STAR, 10, STEP_STAR_MAX);
    FadeUpSingle(RGREEN4, 5, STEP_GREEN_MAX);
    FadeUpSingle(RGREEN3, 5, STEP_GREEN_MAX);
    FadeUpSingle(RGREEN2, 5, STEP_GREEN_MAX);
    FadeUpSingle(RGREEN1, 5, STEP_GREEN_MAX);
    FadeDownSingle(STAR, 10, STEP_STAR_MAX);
    FadeDownSingle(RGREEN4, 5, STEP_GREEN_MAX);
    FadeDownSingle(RGREEN3, 5, STEP_GREEN_MAX);
    FadeDownSingle(RGREEN2, 5, STEP_GREEN_MAX);
    FadeDownSingle(RGREEN1, 5, STEP_GREEN_MAX);
  }
  FadeUpSingle(STAR, 25, STEP_STAR_MAX);
  for(int i=0;i<5;i++){
    FadeUp(green_low4, sizeof(green_low4), 5, STEP_GREEN_MAX);
    FadeUp(green_low3, sizeof(green_low4), 5, STEP_GREEN_MAX);
    FadeDown(green_low4, sizeof(green_low4), 5, STEP_GREEN_MAX);
    FadeUp(green_low2, sizeof(green_low4), 5, STEP_GREEN_MAX);
    FadeDown(green_low3, sizeof(green_low3), 5, STEP_GREEN_MAX);
    FadeUp(green_low1, sizeof(green_low4), 5, STEP_GREEN_MAX);
    FadeDown(green_low2, sizeof(green_low2), 5, STEP_GREEN_MAX);
    FadeDown(green_low1, sizeof(green_low1), 5, STEP_GREEN_MAX);
  }
  FadeDownSingle(STAR, 25, STEP_STAR_MAX);
}
 
void Program2(){
  for(int i=0;i<5;i++){
    FadeUp(blue_all, sizeof(blue_all), 50, STEP_BLUE_MAX);
    FadeDown(blue_all, sizeof(blue_all), 50, STEP_BLUE_MAX);
    FadeUp(red_all, sizeof(red_all), 50, STEP_RED_MAX);
    FadeDown(red_all, sizeof(red_all), 50, STEP_RED_MAX);
  }
  delay(1000);
}

void Program3(){
  FadeUpSingle(STAR, 25, STEP_STAR_MAX);
  FadeUp(blue_all, sizeof(blue_all), 50, STEP_BLUE_MAX);
  FadeDown(blue_all, sizeof(blue_all), 50, STEP_BLUE_MAX);
  FadeUp(red_all, sizeof(red_all), 50, STEP_RED_MAX);
  FadeDown(red_all, sizeof(red_all), 50, STEP_RED_MAX);
  delay(1000);
  FadeDownSingle(STAR, 25, STEP_STAR_MAX);
}

void Program4(){
  FadeUp(bluered_all, sizeof(bluered_all), 50, STEP_BLUE_MAX);
  FadeUpSingle(STAR, 25, STEP_STAR_MAX);
  FadeDown(bluered_all, sizeof(bluered_all), 50, STEP_BLUE_MAX);
  delay(1000);
  FadeDownSingle(STAR, 25, STEP_STAR_MAX);
}

void Program5(){
  for(int i=0;i<50;i++){
    FadeUp(blue_all, sizeof(blue_all), 5, STEP_BLUE_MAX);
    FadeDown(blue_all, sizeof(blue_all), 5, STEP_BLUE_MAX);
    FadeUp(red_all, sizeof(red_all), 5, STEP_BLUE_MAX);
    FadeDown(red_all, sizeof(red_all), 5, STEP_BLUE_MAX);
  }
  delay(1000);
}

void Program6(){
  for(int i=0;i<10;i++){
    FadeUp(green_low1, sizeof(green_low1), 10, STEP_GREEN_MAX);
    FadeUp(green_low2, sizeof(green_low2), 10, STEP_GREEN_MAX);
    FadeUp(green_low3, sizeof(green_low3), 10, STEP_GREEN_MAX);
    FadeUp(green_low4, sizeof(green_low4), 10, STEP_GREEN_MAX);
    FadeUpSingle(STAR, 25, STEP_STAR_MAX);
    FadeDownSingle(STAR, 25, STEP_STAR_MAX);
    FadeDown(green_low4, sizeof(green_low4), 10, STEP_GREEN_MAX);
    FadeDown(green_low3, sizeof(green_low3), 10, STEP_GREEN_MAX);
    FadeDown(green_low2, sizeof(green_low2), 10, STEP_GREEN_MAX);
    FadeDown(green_low1, sizeof(green_low1), 10, STEP_GREEN_MAX);
    delay(100);
  }
  delay(1000);
}


void loop() {
  Program1();
  Program2();
  Program3();
  Program4();
  Program5();
  Program6();
}
