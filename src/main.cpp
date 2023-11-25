#include <Arduino.h>
#include <myIOT2.h>

#include <TimeLib.h>
#include <MD_Parola.h>  /* Display */
#include <MD_MAX72xx.h> /* Display */
#include <SPI.h>        /* Display */

#define VER "LEDClock_V0.5"

#define CS_PIN 15
#define MAX_DEVICES 4
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW

myIOT2 iot;
MD_Parola dotMatrix = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

char txt_dotMatrix[10];

void extMQTT(char *incoming_msg, char *_topic)
{
    Serial.println(incoming_msg);
    char msg[150];
    if (strcmp(incoming_msg, "status") == 0)
    {
        sprintf(msg, "I\'m Good");
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "help2") == 0)
    {
        sprintf(msg, "help #2:No other functions");
        iot.pub_msg(msg);
    }
}
void start_iot2()
{
    iot.useSerial = true;
    iot.useFlashP = false;
    iot.noNetwork_reset = 2;
    iot.ignore_boot_msg = false;

    iot.add_gen_pubTopic("myHome/Messages");
    iot.add_gen_pubTopic("myHome/log");
    iot.add_gen_pubTopic("myHome/debug");
    iot.add_subTopic("myHome/matrixClock");
    iot.add_pubTopic("myHome/matrixClock/Avail");
    iot.add_subTopic("myHome/matrixClock/State");

    iot.start_services(extMQTT);
}

void dotMatrix_init()
{
  dotMatrix.begin();
  dotMatrix.setIntensity(2);
}
void update_clk_newMinute(time_t &t)
{
  struct tm *timeinfo;
  timeinfo = localtime(&t);

  sprintf(txt_dotMatrix, "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
  dotMatrix.displayText(txt_dotMatrix, PA_CENTER, 30, 0, PA_SCROLL_LEFT);
  while (!dotMatrix.displayAnimate())
  {
  }
}
void display_date(time_t &t)
{
  sprintf(txt_dotMatrix, "%02d/%02d", day(t), month(t));
  dotMatrix.displayText(txt_dotMatrix, PA_CENTER, 30, 0, PA_SCROLL_RIGHT);
  dotMatrix.displayText(txt_dotMatrix, PA_CENTER, 20, 0, PA_OPENING);
  while (!dotMatrix.displayAnimate())
  {
  }
}
void update_clk_blink(time_t &t)
{
  static bool blink = false;
  struct tm *timeinfo;
  timeinfo = localtime(&t);

  sprintf(txt_dotMatrix, "%02d%c%02d", timeinfo->tm_hour, blink ? ':' : ' ', timeinfo->tm_min);
  dotMatrix.displayText(txt_dotMatrix, PA_CENTER, 0, 0, PA_PRINT);
  dotMatrix.displayAnimate();
  blink = !blink;
}
void updateDisplay()
{
  const int blink_delay = 500;
  static uint8_t lastMin = 0;
  static bool dateshown = false;
  static unsigned long blink_clock = 0;

  time_t t;
  struct tm *timeinfo;

  time(&t);
  timeinfo = localtime(&t);

  if (lastMin != timeinfo->tm_min) /* update clk every minute with animation */
  {
    update_clk_newMinute(t);
    lastMin = timeinfo->tm_min;
    dateshown = false;
  }
  else if (timeinfo->tm_sec > 5 && timeinfo->tm_sec <= 10) /* display date at first 5sec every minute */
  {
    if (dateshown == false)
    {
      display_date(t);
      dateshown = true;
    }
  }
  else if (millis() >= blink_clock + blink_delay) /* update blinks */
  {
    update_clk_blink(t);
    blink_clock = millis();
  }
}

void setup()
{
  start_iot2();
  dotMatrix_init();
}
void loop()
{
  iot.looper();
  updateDisplay();
}