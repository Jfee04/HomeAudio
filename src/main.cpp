#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define ADC_MAX (1023)
#define VOLT_TO_ADC (1023.0/5.0)
#define SAMPLE_PEAK 1
#define ADJUSTED_ADC_MAX (ADC_MAX*SAMPLE_PEAK/5)
#define SPS (10000)  //the screen should update 24 times per second and it will update after all samples have been taken
#define DEBUG 1
#define MEAURED_ADC_MAX 1023
#define SAMPLE_SIZE 50


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int bassPin = A0;
const int midPin = A1;
const int treblePin = A2;

const int maxBarHeight = 60; // Maximum height for bars on OLED
const int barWidth = 30;     // Width for each bar
const int samplingInterval = (1/SPS)*1000; // Sampling interval in milliseconds


//measurment variables
uint8_t bassAmplitude = 0, midAmplitude = 0, trebleAmplitude = 0;
uint8_t smoothedBass = 0, smoothedMid = 0, smoothedTreble = 0;
uint8_t mean = 0, maxVal = 0, minVal;
float alpha = 0.2; // Smoothing factor
uint8_t bassBuffer[SAMPLE_SIZE] = {0};
uint8_t midBuffer[SAMPLE_SIZE] = {0};
uint8_t trebleBuffer[SAMPLE_SIZE] = {0};
uint8_t sampleIndex = 0;

unsigned long previousMillis = 0;
uint8_t printCounter = 0;

void takeSample();
void updateDisplay();
uint8_t calculateAmplitude(uint8_t *buffer);
void displayx(char x);

int freeMemory() {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void setup() {
  Serial.begin(9600);
  Serial.print("Free Memory: ");
  Serial.println(freeMemory());
  while(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c)){
    Serial.println(F("OLED allocation failed"));
  }
  display.clearDisplay();
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= samplingInterval) {
    previousMillis = currentMillis;
    takeSample();
    printCounter++;
  }
  if(sampleIndex == 0){
    updateDisplay();
  }

}


void takeSample(){
  // Read peak values from each frequency band (simulate or real-time values)
  bassBuffer[sampleIndex] = map(analogRead(bassPin), 0, 1023, 0, 225);
  midBuffer[sampleIndex] = map(analogRead(midPin), 0, 1023, 0, 225);
  trebleBuffer[sampleIndex] = map(analogRead(treblePin), 0, 1023, 0, 225);

  //circluar increment
  sampleIndex = (sampleIndex + 1) % SAMPLE_SIZE;

  //get amplitude
  bassAmplitude = calculateAmplitude(bassBuffer);
  midAmplitude = calculateAmplitude(midBuffer);
  trebleAmplitude = calculateAmplitude(trebleBuffer);

  // Scale the ADC value (0 to 1023) to a height on the OLED
  int bassHeight = map(bassAmplitude, 0, 125, 0, maxBarHeight);
  int midHeight = map(midAmplitude, 0, 125, 0, maxBarHeight);
  int trebleHeight =map(trebleAmplitude, 0, 125, 0, maxBarHeight);

  // Smooth transitions using exponential smoothing
  smoothedBass = alpha * bassHeight + (1 - alpha) * smoothedBass;
  smoothedMid = alpha * midHeight + (1 - alpha) * smoothedMid;
  smoothedTreble = alpha * trebleHeight + (1 - alpha) * smoothedTreble;
  //  changed the debug define to 1 to print
  if(DEBUG && printCounter >= 50 && 0){
    printCounter = 0;

    // Serial.print("bassBuffer: ");
    // for (int i = 0; i < SAMPLE_SIZE; i++) {
    //   Serial.print(bassBuffer[i]);
    //   Serial.print(", ");
    // }
    // Serial.println();

    Serial.print("bassPeak: ");
    Serial.println(bassAmplitude);

    Serial.print("bassHeight: ");
    Serial.println(bassHeight);

    Serial.print("smoothedBass: ");
    Serial.println(smoothedBass);
    Serial.println();
  }
}


void updateDisplay(){
  display.clearDisplay();
  if(DEBUG && printCounter >= 50 && 0){
    printCounter = 0;

    Serial.println("display");
    Serial.print("SCREEN_HEIGHT - smoothedBass: ");
    Serial.println(SCREEN_HEIGHT - smoothedBass);

    Serial.print("smoothedBass: ");
    Serial.println(smoothedBass);
  }

  if(DEBUG){
    Serial.print("Buffer: ");
    for (int i = 0; i < SAMPLE_SIZE; i++) {
      Serial.print(trebleBuffer[i]);
      Serial.print(", ");
    }
    Serial.println();

    Serial.print("treblePeak: ");
    Serial.println(trebleAmplitude);

    Serial.print("smoothedtreble: ");
    Serial.println(smoothedTreble);
    Serial.println();
  }


  display.fillRect(10, SCREEN_HEIGHT - smoothedBass, barWidth, smoothedBass, SSD1306_WHITE);
  display.fillRect(50, SCREEN_HEIGHT - smoothedMid, barWidth, smoothedMid, SSD1306_WHITE);
  display.fillRect(90, SCREEN_HEIGHT - smoothedTreble, barWidth, smoothedTreble, SSD1306_WHITE);

  display.display();
}

uint8_t calculateAmplitude(uint8_t *buffer) {
  //Calculate the DC offset
  long sum = 0;
  for (int i = 0; i < SAMPLE_SIZE; i++) {
    sum += buffer[i];
  }
  mean = sum / SAMPLE_SIZE;

  // Calculate peak-to-peak
  maxVal = 0, minVal = ADC_MAX;
  for (int i = 0; i < SAMPLE_SIZE; i++) {
    if (buffer[i] > maxVal) maxVal = buffer[i];
    if (buffer[i] < minVal) minVal = buffer[i];
  }
  return (maxVal - minVal) / 2; // Return half peak-to-peak as amplitude
}


void displayx(char x){
  display.clearDisplay();             // Clear the buffer
  display.setTextSize(2);             // Set text size to 2 for larger text
  display.setTextColor(SSD1306_WHITE);// Set text color to white
  display.setCursor(0, 0);            // Set cursor position at top-left corner
  display.print(x);             // Print "Hello" on the display
  display.display();                  // Display the text
}



// // test 
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>

// #define SCREEN_WIDTH 128
// #define SCREEN_HEIGHT 64
// #define OLED_RESET -1
// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// void setup() {
//   // Initialize the OLED display
//   if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // 0x3C is the I2C address for most OLEDs
//     Serial.println(F("OLED allocation failed"));
//     for (;;); // Stop if the display cannot be initialized
//   }

//   display.clearDisplay();             // Clear the buffer
//   display.setTextSize(2);             // Set text size to 2 for larger text
//   display.setTextColor(SSD1306_WHITE);// Set text color to white
//   display.setCursor(0, 0);            // Set cursor position at top-left corner
//   display.print("Hello");             // Print "Hello" on the display
//   display.display();                  // Display the text
// }

// void loop() {
//   // Nothing needed here for static display
// }
