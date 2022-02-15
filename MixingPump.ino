#include <U8g2lib.h>
#include <U8x8lib.h>
#include "FastAccelStepper.h"

// As in StepperDemo for Motor 1 on ESP32
#define dirPinStepper 32//34
#define stepPinStepper 27//25
#define dirPinStepper2 34
#define stepPinStepper2 25
#define RatioPin 4
#define SpeedPin 15
#define stopPin 33


U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);
FastAccelStepperEngine engine = FastAccelStepperEngine();
float prevRatio;
float prevFlow;
bool stopMotors;

FastAccelStepper *stepper = NULL;
FastAccelStepper *stepper2 = NULL;
void setup() {

  pinMode(stopPin, INPUT_PULLUP);
  stopMotors = false;
  engine.init();
  stepper = engine.stepperConnectToPin(stepPinStepper);
  stepper2 = engine.stepperConnectToPin(stepPinStepper2);
  if (stepper && stepper2) {

    stepper->setDirectionPin(dirPinStepper);
    stepper->setAcceleration(1000);

    stepper2->setDirectionPin(dirPinStepper2);
    stepper2->setAcceleration(1000);

  }
  Serial.begin(9600);
  prevRatio = 0;
  prevFlow = 0;


  u8x8.begin();
  u8x8.setPowerSave(0);

  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.drawString(0, 1, "Welcome ");
  u8x8.drawString(0, 10, "To ");
  u8x8.drawString(0, 20, "Kyle se ");
  u8x8.drawString(0, 30, "Brandy Pomp");
  //u8x8.drawString(0,1,"Hello World 2!");
  u8x8.refreshDisplay();    // only required for SSD1606/7
  delay(2000);
}

bool shouldConstrain(float mot)
{
  if (mot >= 6000 || mot <= 0)
  {
    return true;
  }
  else
  {
    return false;
  }
}


float constrainSpeed(float upper, float lower, float val)
{

  float constrainedVal = 0;
  if (val >= upper)
  {
    constrainedVal = upper;
    return constrainedVal;
  }
  else if (val <= lower)
  {
    constrainedVal = lower;
    return constrainedVal;
  }
  else
  {
    constrainedVal = val;
    return constrainedVal;
  }
}

void loop() {

  
  float speedIn = analogRead(SpeedPin);
  float flow = (speedIn / 4096)*8600;

  float ratioIn = analogRead(RatioPin);
  float ratio = (ratioIn) / 4096;

  if(digitalRead(stopPin))
  {
    stopMotors = true;
  }
  else
  {
    stopMotors = false;
  }

  
  float mot1 = ratio * flow;
  float mot2 = (1 - ratio) * flow;


  bool ratioChanged = ((prevRatio > (ratio ) ) || (prevRatio < (ratio )));
  bool flowChanged = ((prevFlow > (flow )  ) || (prevFlow < (flow )));
  bool disp = (flowChanged || ratioChanged);


  prevRatio = ratio;
  prevFlow = flow;

  bool check1 = shouldConstrain(mot1);
  bool check2 = shouldConstrain(mot2);

  if (check1)
  {
    mot1 = constrainSpeed(4300, 0, mot1);
    if (mot1 == 4300)
    {
      mot2 = (mot1 - (ratio * mot1)) / ratio;
      flow = mot1 + mot2;
    }
  }
  if (check2)
  {
    mot2 = constrainSpeed(4300, 0, mot2);
    if (mot2 == 4300)
    {
      mot1 = (ratio * mot2) / (1 - ratio);
      flow = mot1 + mot2;
    }
  }

  if(stopMotors)
  {
    stepper->stopMove();
    stepper2->stopMove();
  }
  else
  {
    stepper->setSpeedInHz(mot1);
    stepper->runForward();
    stepper2->setSpeedInHz(mot2);
    stepper2->runForward();
  }
  

  float printRatio = ratio * 100;
  float printSpeed = flow * 100;
  if (disp)
  {
    u8x8.clear();
    u8x8.drawString(0, 1, "Speed");
    u8x8.setCursor(8, 1);
    u8x8.print(flow, 0);

    u8x8.drawString(0, 5, "Ratio");
    u8x8.setCursor(8, 5);
    u8x8.print(printRatio, 0);
    
  }
  //delay(100);
}
