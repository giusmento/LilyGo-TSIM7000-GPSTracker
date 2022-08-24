#include <Arduino.h>
#include <DFRobot_SIM7000.h>
#include <gnssHandler.h>

#define PIN_TX 27
#define PIN_RX 26
#define PWR_PIN 4

#define LED_PIN 12

#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 60 * 15  /* Time ESP32 will go to sleep (in seconds) */

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
  Serial.println("Wait 5 secs");
  delay(5000);

  Serial.println("Loading Last received SMS");
  SmsType lastReceivedSms;
  while (getLastSms(&sim7000, &lastReceivedSms))
  {

    // Serial.println("GOT:index-");
    // Serial.println(lastReceivedSms.index);
    // Serial.println("GOT:date-");
    // Serial.println(lastReceivedSms.date);
    // Serial.println("GOT:sender-");
    // Serial.println(lastReceivedSms.sender);
    // Serial.println("GOT:stat-");
    // Serial.println(lastReceivedSms.stat);
    // Serial.println("GOT:time-");
    // Serial.println(lastReceivedSms.time);
    // Serial.println("GOT:text-");
    // Serial.println(lastReceivedSms.text);

    int i = 0;
    // to lower text
    while (lastReceivedSms.text[i])
    {
      lastReceivedSms.text[i] = (tolower(lastReceivedSms.text[i]));
      i++;
    };

    if (strstr(lastReceivedSms.text, "where") != NULL)
    {
      Serial.println("YES, preparing response");
      char response[255];
      // START and sync GNSS module
      if (sync_position(&sim7000))
      {
        const char *latitude = sim7000.getLatitude();
        const char *longitude = sim7000.getLongitude();
        strcpy(response, "Hey I am here: https://maps.google.com/?q=");
        strcat(response, latitude);
        strcat(response, ",");
        strcat(response, longitude);
        Serial.println(response);
      }
      else
      {
        strcpy(response, "No GPS data");
      }
      // Get battery status
      int power = sim7000.batteryPower();
      char power_char[5];
      strcat(response, ". Battery:");
      strcat(response, itoa(power, power_char, 10));
      // send SMS
      Serial.print("sending response to");
      Serial.println(lastReceivedSms.sender);
      Serial.println(response);
      sim7000.sendSMS(lastReceivedSms.sender, response);
    }
    // delete SMS
    delay(5000);
    Serial.print("Deleting sms index: ");
    Serial.println(lastReceivedSms.index);
    sim7000.deleteSMS(lastReceivedSms.index);
  }
  Serial.println("Going to sleep now");
  delay(1000);
  Serial.flush();
  esp_deep_sleep_start();
}