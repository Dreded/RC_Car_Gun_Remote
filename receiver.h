#include "RF24.h"
#include <Servo.h>

class RECEIVER
{
private:
    uint8_t address[2][6] = {"1Node", "2Node"};
    bool role = 0;
    bool radioNumber = role;
    unsigned long previousMillis = 0; // last time a command was sent
    long interval;
    uint8_t controls[4];
    Servo servo_throttle; // create servo object to control a servo
    Servo servo_steering;
    RF24 radio;

    int throttleSmoothed = 512;
    int throttleSmoothedPrev = throttleSmoothed;

    int steeringSmoothed = 512;
    int steeringSmoothedPrev = steeringSmoothed;

public:
    void begin(size_t numberOfAuxServos, const byte *auxServoPinArray, byte throttleServoPin = 2, byte steeringServoPin = 3, long timeoutInterval = 1000)
    //void begin(byte throttleServoPin = 2, byte steeringServoPin = 3, long timeoutInterval = 1000)
    {
        radio.begin(7, 8);      // using pin 7 for the CE pin, and pin 8 for the CSN pin
        radio.startListening(); // put radio in RX mode
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
        interval = timeoutInterval;

        Serial.println(F("Arduino Wireless Car Radio Receiver"));

        servo_throttle.attach(2, 1000, 2000); // attach servo to indicated pin
        servo_steering.attach(3, 1000, 2000); // attach servo to indicated pin
        Servo servo_aux[numberOfAuxServos];
        Serial.println(auxServoPinArray[3]);
        for (size_t i = 0; i < numberOfAuxServos; i++)
        {
            Serial.print("Initiating Servo: ");
            Serial.println(i);
            servo_aux[i].attach(auxServoPinArray[i], 1000, 2000); // attach servo to indicated pin
        }
    }
    void loop()
    {
        unsigned long currentMillis = millis();
        // This device is a RX node
        if (currentMillis - previousMillis >= interval)
        {
            previousMillis = currentMillis; // Reset the recv timer so we dont just flood the monitor
            Serial.println("\t-- Lost Connection to Transmitter --");
        }
        uint8_t pipe;
        if (radio.available(&pipe))
        {                                           // is there a payload? get the pipe number that recieved it
            previousMillis = currentMillis;         // Reset the recv timer.
            uint8_t bytes = radio.getPayloadSize(); // get the size of the payload
            radio.read(&controls, bytes);           // fetch payload from FIFO

            Serial.print(F("Throttle: "));
            Serial.print(char(controls[2]));
            Serial.print(F(" - "));
            Serial.print(controls[0]);
            Serial.print("%");

            Serial.print(F(" \tSteering: "));
            Serial.print(char(controls[3]));
            Serial.print(F(" - "));
            Serial.print(controls[1]);
            Serial.println("%");
        }
        int output = 90;
        if (char(controls[2]) == 'S')
        {
            servo_throttle.write(90);
        }
        else if (char(controls[2]) == 'F')
        {
            output = map(controls[0], 0, 100, 90, 180);
            servo_throttle.write(output);
        }
        else if (char(controls[2]) == 'R')
        {
            output = map(controls[0], 0, 100, 90, 0);
            servo_throttle.write(output);
        }
        if (char(controls[3]) == 'S')
        {
            //servo_steering.write(90);
        }
        else if (char(controls[3]) == 'L')
        {
            output = map(controls[1], 0, 100, 90, 180);
            servo_steering.write(output);
        }
        else if (char(controls[3]) == 'R')
        {
            output = map(controls[1], 0, 100, 90, 0);
            //servo_steering.write(output);

            smoothServo(servo_steering, output, steeringSmoothed, steeringSmoothedPrev);
        }
    }

    void smoothServo(Servo &servo, int &output, int &smoothed, int &smoothedPrev)
    {
        smoothed = (output * 0.02) + (smoothedPrev * 0.98);
        smoothedPrev = smoothed;
        servo.write(steeringSmoothed);

        // if (char(controls[3]) == 'S')
        // {
        //     servo_steering.write(90);
        // }
        // else if (char(controls[3]) == 'L')
        // {
        //     output = map(controls[1], 0, 100, 90, 180);
        //     steeringSmoothed = (output * 0.02) + (steeringSmoothedPrev * 0.98);
        //     steeringSmoothedPrev = steeringSmoothed;
        //     servo_steering.write(steeringSmoothed);
        // }
        // else if (char(controls[3]) == 'R')
        // {
        //     output = map(controls[1], 0, 100, 90, 0);
        //     steeringSmoothed = (output * 0.02) + (steeringSmoothedPrev * 0.98);
        //     steeringSmoothedPrev = steeringSmoothed;
        //     servo_steering.write(steeringSmoothed);
        // }
    }
};