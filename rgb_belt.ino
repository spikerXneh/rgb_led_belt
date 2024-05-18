#include <Scheduler.h>

#include <Adafruit_NeoPixel.h>
#include <Wire.h>
//
// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN 28
#define MODE_SWITCH 0
const int sensorPin = 27;  // the piezo is connected to analog pin 0
const int threshold = 10;   // threshold value to decide when the detected sound is a knock or not
// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 144
volatile bool fast = false;
long currentColor = 0;
boolean val =0;

int lastState = HIGH; // the previous state from the input pin
int currentState;    // the current reading from the input pin
volatile int currentThreshold = 50;

const int sampleWindow = 50;
unsigned int sample;

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(9600);
  Serial.println("Started");
  // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
  pinMode(sensorPin, INPUT);
  pinMode(MODE_SWITCH, INPUT_PULLUP);

  strip.begin();             // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();              // Turn OFF all pixels ASAP
  strip.setBrightness(10);  // Set BRIGHTNESS to about 1/5 (max = 255)
  Scheduler.startLoop(showLights);
  Scheduler.startLoop(readSensor2);
  Scheduler.startLoop(readModeButton);
}

void loop(){}

void readModeButton(){
    currentState = digitalRead(MODE_SWITCH);
    if (lastState == LOW && currentState == HIGH){
      switch(currentThreshold){
        case 50:
          currentThreshold = 100;
          break;
        case 100:
          currentThreshold = 200;
          break;
        case 200:
          currentThreshold = 400;
          break;
        case 400:
          currentThreshold = 800;
          break;
        case 800:
          currentThreshold = 50;
          break;
      }      
    }
    
  if(lastState == HIGH && currentState == LOW)
    Serial.println("The button is pressed");
  else if(lastState == LOW && currentState == HIGH)
    Serial.println("The button is released");

          lastState = currentState;

}

void readSensor2(){
unsigned long startMillis = millis(); // Start of sample window
  unsigned int peakToPeak = 0;   // peak-to-peak level

  unsigned int signalMax = 0;
  unsigned int signalMin = 1024;

  // collect data for 50 mS and then plot data
  while (millis() - startMillis < sampleWindow)
  {
    sample = analogRead(sensorPin);
    if (sample < 1024)  // toss out spurious readings
    {
      if (sample > signalMax)
      {
        signalMax = sample;  // save just the max levels
      }
      else if (sample < signalMin)
      {
        signalMin = sample;  // save just the min levels
      }
    }
  }
  peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
  Serial.print("Value ");
  Serial.print(peakToPeak);
  Serial.print(" Current Theshold " );
  Serial.print(currentThreshold);
  Serial.println();
  fast = peakToPeak > currentThreshold;
  yield();
}

void showLights() {
  int speed = 25;
  if (fast == true) {
    // colorWipe(strip.Color(255, 0, 255), speed);  // Red
    rainbow2(5);//5
      strip.setBrightness(100);
    //theaterChaseRainbow(speed);
  } else {
      strip.setBrightness(10);
    rainbow(speed);  // Flowing rainbow cycle along the whole strip
  }
  yield();
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow2(int wait) {
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this loop:
  for (long firstPixelHue = currentColor; firstPixelHue <65536; firstPixelHue += 1024) {
    if (currentColor != 0) {
     // firstPixelHue = currentColor;
      currentColor = 0;
    }
    if (fast == false) {
      if (firstPixelHue != 0) {
        currentColor = firstPixelHue;
      }
      break;
    }

    if (wait ==5){
      strip.setBrightness(100);
    }else{
      strip.setBrightness(10);
    }
    // strip.rainbow() can take a single argument (first pixel hue) or
    // optionally a few extras: number of rainbow repetitions (default 1),
    // saturation and value (brightness) (both 0-255, similar to the
    // ColorHSV() function, default 255), and a true/false flag for whether
    // to apply gamma correction to provide 'truer' colors (default true).
    strip.rainbow(firstPixelHue);
    // Above line is equivalent to:
    // strip.rainbow(firstPixelHue, 1, 255, 255, true);
    strip.show();  // Update strip with new contents
    delay(wait);   // Pause for a moment
  }
}

void rainbow(int wait) {
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this loop:
  for (long firstPixelHue = currentColor; firstPixelHue < 65536; firstPixelHue += 256) {
    if (currentColor != 0) {
   //   firstPixelHue = currentColor;
      currentColor = 0;
    }
    if (fast) {
      if (firstPixelHue != 0) {
        currentColor = firstPixelHue;
      }
      break;
    }
    // strip.rainbow() can take a single argument (first pixel hue) or
    // optionally a few extras: number of rainbow repetitions (default 1),
    // saturation and value (brightness) (both 0-255, similar to the
    // ColorHSV() function, default 255), and a true/false flag for whether
    // to apply gamma correction to provide 'truer' colors (default true).
    strip.rainbow(firstPixelHue);
    // Above line is equivalent to:
    // strip.rainbow(firstPixelHue, 1, 255, 255, true);
    strip.show();  // Update strip with new contents
    delay(wait);   // Pause for a moment
  }
}

// // Rainbow-enhanced theater marquee. Pass delay time (in ms) between frames.
void theaterChaseRainbow(int wait) {
  int firstPixelHue = 0;           // First pixel starts at red (hue 0)
  for (int a = 0; a < 30; a++) {   // Repeat 30 times...
    for (int b = 0; b < 3; b++) {  //  'b' counts from 0 to 2...
      if (false == fast) {
        break;
      }
      strip.clear();  //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in increments of 3...
      for (int c = b; c < strip.numPixels(); c += 3) {
        // hue of pixel 'c' is offset by an amount to make one full
        // revolution of the color wheel (range 65536) along the length
        // of the strip (strip.numPixels() steps):
        int hue = firstPixelHue + c * 65536L / strip.numPixels();
        uint32_t color = strip.gamma32(strip.ColorHSV(hue));  // hue -> RGB
        strip.setPixelColor(c, color);                        // Set pixel 'c' to value 'color'
      }
      strip.show();                 // Update strip with new contents
      delay(wait);                  // Pause for a moment
      firstPixelHue += 65536 / 90;  // One cycle of color wheel over 90 frames
    }
  }
}


