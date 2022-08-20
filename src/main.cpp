#include <Arduino.h>
#include <DFRobot_SIM7000.h>
#include <gnssHandler.h>

#define PIN_TX 27
#define PIN_RX 26
#define PWR_PIN 4

#define LED_PIN 12

#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 30 * 1   /* Time ESP32 will go to sleep (in seconds) */

SoftwareSerial mySerial(PIN_RX, PIN_TX);
DFRobot_SIM7000 sim7000(&mySerial);

void setup()
{

  Serial.begin(115200);
  mySerial.begin(19200);
  // Set LED_PIN AS OUPUT
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  turnOnSIM7000(&sim7000);
}

void loop()
{
  /*
First we configure the wake up source
We set our ESP32 to wake up every 5 seconds
*/
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");

  // stay alive for 10 sec to sync SMS.
  // delay(10000);
  // getLastSms
  SmsType lastReceivedSms = getLastSms(&sim7000);

  Serial.println("GOT:index-");
  Serial.println(lastReceivedSms.index);
  Serial.println("GOT:date-");
  Serial.println(lastReceivedSms.date);
  Serial.println("GOT:sender-");
  Serial.println(lastReceivedSms.sender);
  Serial.println("GOT:stat-");
  Serial.println(lastReceivedSms.stat);
  Serial.println("GOT:time-");
  Serial.println(lastReceivedSms.time);
  Serial.println("GOT:text-");
  Serial.println(lastReceivedSms.text);

  int i =0;
  // to lower text
  while (lastReceivedSms.text[i])
  {
    lastReceivedSms.text[i] = (tolower(lastReceivedSms.text[i]));
    i++;
  };

  if (strstr(lastReceivedSms.text, "where") != NULL)
  {
    Serial.println("YES, prepare response");
  }
  // START and sync GNSS module
  // bool gnss_module = sync_position(&sim7000);

  Serial.println("Going to sleep now");
  delay(1000);
  Serial.flush();
  esp_deep_sleep_start();
}