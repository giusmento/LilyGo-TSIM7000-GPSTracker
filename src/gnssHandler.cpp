#include <gnssHandler.h>
#include <DFRobot_SIM7000.h>

#ifndef LED_PIN
#define LED_PIN 12
#endif

#ifndef PWR_PIN
#define PWR_PIN 4
#endif

#define MAX_LOOP 30
#define WAITING_TIME 500

bool turnOnSIM7000(DFRobot_SIM7000 *sim7000)
{
    Serial.println("Turn ON SIM7000......");
    sim7000->turnON(PWR_PIN);

    Serial.println("Set baud rate......");
    int i = 0;
    while (i<10)
    {
        if (sim7000->setBaudRate(19200))
        { // Set SIM7000 baud rate from 115200 to 19200 reduce the baud rate to avoid distortion
            Serial.println("Set baud rate: 19200");
            break;
        }
        else
        {
            Serial.println("Failed to set baud rate");
            delay(1000);
        }
        i++;
    }
    return true;
};

bool sync_position(DFRobot_SIM7000 *sim7000)
{
    // activate GNSS module by setting GPIO 4 high
    while (not sim7000->checkSendCmd("AT+SGPIO=0,4,1,1\r\n", "OK", 3000))
    {
        Serial.println("AT+SGPIO=0,4,1,1\r\n");
        delay(3000);
    }
    delay(1000);
    // starting GNSS module
    while (not sim7000->initPos())
        delay(3000);

    int gnss_cnt = 0;
    char response[150];
    while (gnss_cnt < MAX_LOOP)
    {
        // AT+CGNSINF
        sim7000->getPositionRaw(response);
        Serial.println(response);
        if (sim7000->getPosition())
        {
            Serial.println("GNSS got position");
            const char *latitude = sim7000->getLatitude();
            const char *longitude = sim7000->getLongitude();
            Serial.println(latitude);
            Serial.println(longitude);
            return true;
        }

        Serial.println("Trying to lock the GPS");
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        gnss_cnt++;
        delay(WAITING_TIME);
    }
    return false;
};

bool getLastSms(DFRobot_SIM7000 *sim7000, SmsType *receivedSms)
{
    char response[500];
    sim7000->getLastSMSRaw(response);
    Serial.println(response);
    char *chars_array = strtok(response, "\n");
    char payload[255];
    char parse[100];
    int line = 0;
    while (chars_array != NULL)
    {
        // Serial.print("ROW:");
        // Serial.println(chars_array);
        strncpy(parse, chars_array, sizeof(parse));
        parse[strlen(parse) - 1] = '\0';
        // Serial.print("Comp:");
        // Serial.print(parse);
        // Serial.println(strcmp(parse, "OK"));
        if (strcmp(parse, "OK") == 0)
        {
            Serial.println("No more SMS: Exiting");
            return false;
        }
        else if (line == 1)
        {
            strncpy(payload, chars_array, sizeof(payload));
            Serial.print("Header:");
            Serial.println(payload);
        }
        else if (line == 2)
        {
            strncpy(receivedSms->text, chars_array, sizeof(receivedSms->text));
            Serial.print("Message:");
            Serial.println(receivedSms->text);
        }
        else if (line == 3)
            break;
        line++;
        chars_array = strtok(NULL, "\n");
    }
    // parse header
    chars_array = strtok(payload, ",");
    // INDEX remove first 7 chars
    memmove(chars_array, chars_array + 7, strlen(chars_array) - 7);
    chars_array[strlen(chars_array) - 7] = '\0';
    strncpy(receivedSms->index, chars_array, sizeof(receivedSms->index));
    chars_array = strtok(NULL, ",");
    // STAT remove first and last char
    memmove(chars_array, chars_array + 1, strlen(chars_array) - 2);
    chars_array[strlen(chars_array) - 2] = '\0';
    strncpy(receivedSms->stat, chars_array, sizeof(receivedSms->stat));
    chars_array = strtok(NULL, ",");
    // SENDER remove first and last char
    memmove(chars_array, chars_array + 1, strlen(chars_array) - 2);
    chars_array[strlen(chars_array) - 2] = '\0';
    strncpy(receivedSms->sender, chars_array, sizeof(receivedSms->sender));
    chars_array = strtok(NULL, ",");
    // DATE remove first and last char
    memmove(chars_array, chars_array + 1, strlen(chars_array));
    strncpy(receivedSms->date, chars_array, sizeof(receivedSms->date));
    chars_array = strtok(NULL, ",");
    // TIME remove last char
    chars_array[strlen(chars_array) - 2] = '\0';
    strncpy(receivedSms->time, chars_array, sizeof(receivedSms->time));
    return true;
};