#include "RF24.h"

class TRANSMITTER
{
private:
// Define Joystick Connections
#define THROTTLE A1
#define STEERING A3

    uint8_t address[2][6] = {"1Node", "2Node"};
    bool role = 1;
    bool radioNumber = role;
    unsigned long previousMillis = 0; // last time a command was sent
    bool throttle_change = false;
    bool steering_change = false;
    bool resend = false;
    uint8_t controls[4];
    long interval = 0;

    int throttle_dead_zone = 10; //10
    int steering_dead_zone = 25; //20
    int throttle_middle = 108;   //127
    int steering_middle = 134;   //127
    int throttle_min = 70;       //255
    int throttle_max = 140;      //0
    int steering_min = 75;       //255
    int steering_max = 180;      //0
    int training_mode = 0;       //Set via jumper for now on D2 but moving to screen later
    int training_mode_throttle_max_percent = 50;
    int throttle_max_percent = 100;
    RF24 radio;

    int throttle_last = 1023;
    int steering_last = 1023;

    void readEEPROM()
    {
        throttle_dead_zone = map(EEPROM.read(1),0,255,0,1024);
        steering_dead_zone = map(EEPROM.read(2),0,255,0,1024);
        throttle_middle = map(EEPROM.read(3),0,255,0,180);
        steering_middle = map(EEPROM.read(4),0,255,0,180);
        throttle_min = map(EEPROM.read(5),0,255,0,1024);
        throttle_max = map(EEPROM.read(6),0,255,0,1024);
        steering_min = map(EEPROM.read(7),0,255,0,1024);
        steering_max = map(EEPROM.read(8),0,255,0,1024);
        Serial.println(F("Read Data from EEPROM."));
    }

public:
    long get_interval()
    {
        return interval;
    }
    void begin(long minimumSendInterval = 750)
    {
        readEEPROM();
        radio.begin(7, 8);     // using pin 7 for the CE pin, and pin 8 for the CSN pin
        radio.stopListening(); // put radio in TX mode
            // Set the PA Level low to try preventing power supply related problems
        // because these examples are likely run with nodes in close proximity to
        // each other.
        radio.setPALevel(RF24_PA_LOW); // RF24_PA_MAX is default.

        // save on transmission time by setting the radio to only transmit the
        // number of bytes we need to transmit
        radio.setPayloadSize(sizeof(controls));

        // set the TX address of the RX node into the TX pipe
        radio.openWritingPipe(address[radioNumber]); // always uses pipe 0

        // set the RX address of the TX node into a RX pipe
        radio.openReadingPipe(1, address[!radioNumber]); // using pipe 1
        interval = minimumSendInterval;

        pinMode(THROTTLE, INPUT);
        pinMode(STEERING, INPUT);
        pinMode(2, INPUT_PULLUP);
        Serial.println(F("Ardunio Wireless Car Radio Controller"));

        training_mode = !digitalRead(2);
        if (training_mode)
        {
            throttle_max_percent = training_mode_throttle_max_percent;
            Serial.print(F("Training Mode Enabled Throttle Limited to:"));
            Serial.println(throttle_max_percent);
        }
    }
    void loop()
    {
        unsigned long currentMillis = millis();
        if (steering_change || throttle_change || resend || (currentMillis - previousMillis >= interval))
        {
            previousMillis = currentMillis;
            // Serial.print(controls[0]);
            // Serial.print(F(" / "));
            // Serial.println(controls[1]);
            throttle_change = false;
            steering_change = false;
            resend = false;
            // This device is a TX node
            unsigned long start_timer = micros();                   // start the timer
            bool report = radio.write(&controls, sizeof(controls)); // transmit & save the report
            unsigned long end_timer = micros();                     // end the timer
            if (report)
            {
                Serial.print(F("Transmission successful! ")); // payload was delivered
                Serial.print(F("Time to transmit = "));
                Serial.print(end_timer - start_timer); // print the timer result
                Serial.print(F(" us. Throttle: "));
                Serial.print(controls[0]); // print payload sent
                Serial.print(F("\tSteering: "));
                Serial.println(controls[1]); // print payload sent
            }
            else
            {
                resend = true;
                Serial.println(F("Transmission failed or timed out")); // payload was not delivered
            }
        }

        int throttleValue = analogRead(THROTTLE);
        int steeringValue = analogRead(STEERING);

        throttleValue = constrain(throttleValue,throttle_min,throttle_max);
        steeringValue = constrain(steeringValue,steering_min,steering_max);
        
        controls[2] = throttle_middle;
        controls[0] = map(throttleValue,throttle_min,throttle_max,0,180);

        controls[3] = steering_middle;
        controls[1] = map(steeringValue,steering_min,steering_max,0,180);

        //
        // Should We Send?
        //
        if (throttle_last != controls[0])
        {
            throttle_last = controls[0];
            throttle_change = true;
        }
        if (steering_last != controls[1])
        {
            steering_last = controls[1];
            steering_change = true;
        }
    }
};