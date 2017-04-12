#define PIRsensor 2
#define m11 11
#define m12 12
#define trigPin 10 //ultrasuoni
#define echoPin 9 //ultrasuoni
#define DELAY 500 //uso per valutare la lentezza del ciclo
#define DEBUG // Print distance over serial


enum states {WAITPIR=0, ACTIVATE=1, ACTIVE=2, DEACTIVATE=3};  //States of the FSM
int state = WAITPIR;
int PIRPin = 2;
int dirpin = 11; // direzione motore
int steppin = 12; //passi motore
int redPin = 3;
int greenPin = 5;
int bluePin = 6;
int speed = 500; //velocità motore
int calibrationTime = 30; //tempo di calibrazione del PIRsensor
int minimumRange = 0; //minimum range needed from ultrasonic sensor
int maximumRange = 100; //maximum range needed from the ultrasonic sensor

// Color arrays
int black [3]      = { 0,     0,   0 };
int blu [3]        = { 130, 141, 156 };
int lightBlu [3]   = { 163, 162, 169 };
int pink  [3]      = { 211, 198, 176 };
int beige [3]      = { 212, 189, 122 };
int orange [3]     = { 228, 174, 68 };
int Red [3]        = { 188, 89, 58 };
int beigeLight[3]  = { 199, 169, 132 };

// Set initial color
int redVal = black[0];
int greenVal = black[1];
int blueVal = black[2];
unsigned long colourFadeTime = 0;  //
unsigned long colourFadeDelay = 3000;  //Delay between fading
int idletime = 100;
int wait = 10;      // 10ms internal crossFade delay; increase for slower fades
int hold = 0;       // Optional hold when a color is complete, before the next crossFade
int loopCount = 60; // How often should DEBUG report?
int repeat = 0;     // How many times should we loop before stopping? (0 for no stop)
int j = 0;          // Loop counter for repeat
int PIRpresence = 0;
int startPIR = 0;
int prevR = redVal;
int prevG = greenVal;
int prevB = blueVal;

int calculateStep(int prevValue, int endValue) {
  int step = endValue - prevValue; // What's the overall gap?
  if (step) {                      // If its non-zero,
    step = 1020/step;              // divide by 1020
  }
  return step;
}

/* The next function is calculateVal. When the loop value, i,
*  reaches the step size appropriate for one of the
*  colors, it increases or decreases the value of that color by 1.
*  (R, G, and B are each calculated separately.)
*/

int calculateVal(int step, int val, int i) {

  if ((step) && i % step == 0) { // If step is non-zero and its time to change a value,
    if (step > 0) {              //   increment the value if step is positive...
      val += 1;
    }
    else if (step < 0) {         //   ...or decrement it if step is negative
      val -= 1;
    }
  }
  // Defensive driving: make sure val stays in the range 0-255
  if (val > 255) {
    val = 255;
  }
  else if (val < 0) {
    val = 0;
  }
  return val;
}

/* crossFade() converts the percentage colors to a
*  0-255 range, then loops 1020 times, checking to see if
*  the value needs to be updated each time, then writing
*  the color values to the correct pins.
*/

void crossFade(int color[3]) {
  // Convert to 0-255
  int R = (color[0] * 255) / 100;
  int G = (color[1] * 255) / 100;
  int B = (color[2] * 255) / 100;

  int stepR = calculateStep(prevR, R);
  int stepG = calculateStep(prevG, G);
  int stepB = calculateStep(prevB, B);

  for (int i = 0; i <= 1020; i++) {
    redVal = calculateVal(stepR, redVal, i);
    greenVal = calculateVal(stepG, greenVal, i);
    blueVal = calculateVal(stepB, blueVal, i);

    analogWrite(redPin, redVal);   // Write current values to LED pins
    analogWrite(greenPin, greenVal);
    analogWrite(bluePin, blueVal);

    delay(wait); // Pause for 'wait' milliseconds before resuming the loop
  }
  // Update current values for next loop
  prevR = redVal;
  prevG = greenVal;
  prevB = blueVal;
  delay(hold); // Pause for optional 'wait' milliseconds before resuming the loop
}

void setup() {

    pinMode(dirpin, OUTPUT);
    pinMode(steppin, OUTPUT);
    pinMode(PIRsensor, INPUT);
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    pinMode (redPin, OUTPUT);
    pinMode (greenPin, OUTPUT);
    pinMode (bluePin, OUTPUT);
    Serial.print("calibrating sensor ");
   for(int i = 0; i < calibrationTime; i++){
     Serial.print(".");
     delay(1000);
     }
   Serial.println(" done");
   Serial.println("SENSOR ACTIVE");
   delay(50);
   #ifdef DEBUG
   Serial.begin(9600);
   #endif
}

void loop() {

    // Code to execute independently of the state
     crossFade(blu);  // Main program: list the order of crossfades
     crossFade(lightBlu);
     crossFade(pink);
     crossFade(beige);
     crossFade(orange);
     crossFade(Red);
     crossFade(beigeLight);
    #ifdef DEBUG
    Serial.print("state: ");
    Serial.println(state);
    #endif

    // Code for each state
    /*
      switch(state)
    {
        case WAITPIR:
            // Activation condition
            if(!PIRpresence){
              if(digitalRead(PIRsensor)){
                PIRpresence = 1;
                startPIR = millis();
              }
            else{
              if ((millis() - startPIR>5000) && digitalRead(PIRsensor)){
                state=ACTIVATE;
              }
            }
            // Code for the state
            sleep(idletime);
            break;

        case ACTIVATE:
            // End motor movement condition

            int i;
            digitalWrite(dirpin, LOW);     // Set the direction.
            delay(10);
            for (i = 0; i<10000; i++) {   // Iterate for 10000 microsteps.
            digitalWrite(steppin, LOW);   // This LOW to HIGH change is what creates the
            digitalWrite(steppin, HIGH);  // "Rising Edge" so the easydriver knows to when to step.
            delayMicroseconds(speed);
            break;
        case ACTIVE:
            // Condition to deactivate
            sleep(idletime);
            break;

        case DEACTIVATE:
            // End motor movement condition
            digitalWrite(trigPin, LOW);
            delayMicroseconds(2);
            digitalWrite(trigPin, HIGH);
            delay(200);
            digitalWrite(trigPin, LOW);
            duration = pulseIn(echoPin, HIGH);
            int i;
            digitalWrite(dirpin, HIGH);     // Set the direction.
            delay(10);
            for (i = 0; i=10000; i++) {     // Iterate for 10000 microsteps.
            digitalWrite(steppin, LOW);  // This LOW to HIGH change is what creates the
            digitalWrite(steppin, HIGH); // "Rising Edge" so the easydriver knows to when to step.
            delayMicroseconds(speed);
            break;
    }
    delay(DELAY);
}
*/
}
