#include "RF24.h"
#include <Servo.h>

class RECEIVER
{
private:
    uint8_t address[2][6] = {"1Node", "2Node"};
    bool role = 0;
    bool radioNumber = role;
    unsigned long previousMillis = 0;      // last time a command was sent
    unsigned long servoPreviousMillis = 0; // last time a command was sent
    long servoUpdateInterval = 5;          // time constant for timer
    long interval;
    uint8_t controls[4] = {90};
    Servo servo_throttle; // create servo object to control a servo
    Servo servo_steering;
    RF24 radio;

    float throttleSmoothed = 100;
    float throttleSmoothedPrev = throttleSmoothed;

    float steeringSmoothed = 90;
    float steeringSmoothedPrev = steeringSmoothed;

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
            Serial.print(controls[0]);

            Serial.print(F(" \tSteering: "));
            Serial.println(controls[1]);
        }

        if (currentMillis - servoPreviousMillis >= servoUpdateInterval) //update servo interval seconds(200x per second)
        {
            servoPreviousMillis = currentMillis;
            int requested = controls[0];
            requested = map(controls[0], 0, 180, 180, 0); // flip it as our steering servo is backwards
            updateServo(servo_throttle, requested, controls[2], throttleSmoothed, throttleSmoothedPrev,0.98);
            requested = map(controls[1], 0, 180, 180, 0); // flip it as our steering servo is backwards
            updateServo(servo_steering, requested, controls[3], steeringSmoothed, steeringSmoothedPrev);
        }
    }

    void smoothServo(Servo &servo, int &requested, float &smoothed, float &smoothedPrev, float smoothingAmount)
    {
        smoothed = (requested * (1.0-smoothingAmount)) + (smoothedPrev * smoothingAmount);
        smoothedPrev = smoothed;
        servo.write(smoothed);
    }
    void updateServo(Servo &servo, int requested, int middle, float &smoothed, float &smoothedPrev, float smoothingAmount = 0.85)
    {
        int deadzoneCheck = requested - middle;
        if (abs(deadzoneCheck) < 15)
            requested = 90;
        //smoothServo(servo, requested, smoothed, smoothedPrev, smoothingAmount);
        servo.write(requested);
    }
};