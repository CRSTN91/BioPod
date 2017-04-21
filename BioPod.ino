#include <SoftTimer.h>

#define _ledtimer 200 //Do not modify
#define PIRsensor 2
#define m11 11
#define m12 12
#define trigPin 10  //ultrasuoni
#define echoPin 9   //ultrasuoni
#define DEBUG       //Enable serial messages
//#define COLORDEBUG
#define COLORTIME 5000 //Period of one color in ms
#define TURNOFFDISTANCE 20

void LEDmanager(Task *me); //Declaring prototypes for a bug in arduino IDE
void FSMmanager(Task *me); //https://github.com/prampec/arduino-softtimer/issues/6
Task LEDtask(_ledtimer, LEDmanager);
Task FSMtask(1000, FSMmanager);

enum states {WAITPIR=0, ACTIVATE=1, ACTIVE=2, DEACTIVATE=3};  //States of the FSM
int state = WAITPIR;
int dirpin = 11; // direzione motore
int steppin = 12; //passi motore
int redPin = 3;
int greenPin = 5;
int bluePin = 6;
int speed = 500; //velocità motore
int calibrationTime = 30; //tempo di calibrazione del PIRsensor
int minimumRange = 0; //minimum range needed from ultrasonic sensor
int maximumRange = 100; //maximum range needed from the ultrasonic sensor

// Color handling variables
int colors[][3] = {{ 0,   0,   0 },   //black
                   { 130, 141, 156 }, //blu
                   { 163, 162, 169 }, //lightBlu
                   { 211, 198, 176 }, //pink
                   { 212, 189, 122 }, //beige
                   { 228, 174, 68 },  //orange
                   { 188, 89,  58 },  //red
                   { 199, 169, 132 }};//beigeLight
int nextcolor = 0;
int numcolors = 0;
float currentcolor[3] = {0,0,0};
float colorStep[3] = {0,0,0};

unsigned long colourFadeTime = 0;
unsigned long colourFadeDelay = 3000;  //Delay between fading
int wait = 10;      // 10ms internal crossFade delay; increase for slower fades
int hold = 0;       // Optional hold when a color is complete, before the next crossFade
int loopCount = 60; // How often should DEBUG report?
int repeat = 0;     // How many times should we loop before stopping? (0 for no stop)
int j = 0;          // Loop counter for repeat
int detect = 0;
int PIRcount = 0;
int UScount = 0;

void setup() {
    pinMode(dirpin, OUTPUT);
    pinMode(steppin, OUTPUT);
    pinMode(PIRsensor, INPUT);
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    pinMode (redPin, OUTPUT);
    pinMode (greenPin, OUTPUT);
    pinMode (bluePin, OUTPUT);
    SoftTimer.add(&LEDtask);
    SoftTimer.add(&FSMtask);
    #ifdef DEBUG
    Serial.begin(9600);
    #endif
    /* Temporarily removed annoying calibration delay
    #ifdef DEBUG
    Serial.print("calibrating sensor ");
    #endif
    for(int i = 0; i < calibrationTime; i++){
        #ifdef DEBUG
        Serial.print(".");
        #endif
        delay(1000);
    }*/
    #ifdef DEBUG
    Serial.println(" done");
    Serial.println("SENSOR ACTIVE");
    #endif
    //Color variables initialization
    numcolors = sizeof(colors) / 6;
    #ifdef COLORDEBUG
        Serial.print("Cycling between ");
        Serial.print(numcolors);
        Serial.println(" colors");
    #endif
}

void LEDmanager(Task *me) {
    int delta = 0;
    // Check if reached color target
    bool reached = false;
    for(int i=0; i<3; i++) {
        delta = (currentcolor[i] - colors[nextcolor][i]);
        delta = (delta > 0) ? delta : -delta; //Make the delta variable positive
        if (delta < 1) reached = true;
        else reached = false;
    }
    // If target reached, calculate new steps
    if(reached) {
        #ifdef COLORDEBUG
        Serial.print("Reached color ");
        Serial.println(nextcolor);
        #endif
        int numsteps = COLORTIME/_ledtimer; //Number of times the LED thread will be called
        #ifdef COLORDEBUG
        Serial.print(numsteps);
        Serial.println(" steps until next color");
        #endif
        nextcolor = (nextcolor>(numcolors-2)) ? 0 : nextcolor + 1; //switch to next color
        for(int i=0; i<3; i++) {
            delta = (colors[nextcolor][i] - currentcolor[i]);
            colorStep[i] = (float)delta / (float)numsteps;
        }
        #ifdef COLORDEBUG
        Serial.print("New steps R:");
        Serial.print(colorStep[0]);
        Serial.print(" G:");
        Serial.print(colorStep[1]);
        Serial.print(" B:");
        Serial.println(colorStep[2]);
        #endif
    }
    // If target not reached, update the LED colors
    else {
        for(int i=0; i<3; i++) {
            if(currentcolor[i] != colors[nextcolor][i])
                currentcolor[i] += colorStep[i];
        }
        analogWrite(redPin, (int)currentcolor[0]);
        analogWrite(greenPin, (int)currentcolor[1]);
        analogWrite(bluePin, (int)currentcolor[2]);
    }
}

int readDistance() {
    // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    long duration = pulseIn(echoPin,HIGH);
    // convert the time into a distance
    long distanceCm = duration / 29.1 / 2 ;
return (int)distanceCm;
}

void FSMmanager(Task *me) {
    // Code to execute independently of the state
    #ifdef DEBUG
    Serial.print("state: ");
    Serial.println(state);
    #endif
    // Code for each state
    switch(state)
    {
        case WAITPIR:
            // Activation condition
            detect = digitalRead(PIRsensor);
            #ifdef DEBUG
            Serial.print("PIR: ");
            Serial.println(detect);
            #endif
            if(detect) PIRcount++;
            else PIRcount = 0;
            if(PIRcount > 5) {
                state = ACTIVATE;
                PIRcount = 0;
            }
            break;

        case ACTIVATE:
            digitalWrite(dirpin, LOW);     // Set the direction.
            delay(10);
            for (int i = 0; i<10000; i++) {   // Iterate for 10000 microsteps.
                digitalWrite(steppin, LOW);   // This LOW to HIGH change is what creates the
                digitalWrite(steppin, HIGH);  // "Rising Edge" so the easydriver knows to when to step.
                delayMicroseconds(speed);
            }
            // End motor movement condition
            state = ACTIVE;
            break;

        case ACTIVE:
            // Condition to deactivate
            detect = readDistance() < TURNOFFDISTANCE;
            #ifdef DEBUG
            Serial.print("ultrasonic: ");
            Serial.println(detect);
            #endif
            if(detect) UScount++;
            else UScount = 0;
            if(UScount > 3) state = DEACTIVATE;
            break;

        case DEACTIVATE:
            digitalWrite(dirpin, HIGH);     // Set the direction.
            delay(10);
            for (int i = 0; i<10000; i++) {     // Iterate for 10000 microsteps.
                digitalWrite(steppin, LOW);  // This LOW to HIGH change is what creates the
                digitalWrite(steppin, HIGH); // "Rising Edge" so the easydriver knows to when to step.
                delayMicroseconds(speed);
            }
            // End motor movement condition
            state = WAITPIR;
            break;
    }
}
