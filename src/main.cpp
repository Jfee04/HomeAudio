#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define ADC_MAX 1023
#define VOLT_TO_ADC (1023.0/5.0)
#define SAMPLE_PEAK 1
#define ADJUSTED_ADC_MAX (ADC_MAX*SAMPLE_PEAK/5)
#define FPS 24
#define DEBUG 1
#define MEAURED_ADC_MAX 1023
#define SAMPLE_SIZE 50


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int bassPin = A0;
const int midPin = A1;
const int treblePin = A2;

const int maxBarHeight = 60; // Maximum height for bars on OLED
const int barWidth = 30;     // Width for each bar
const int samplingInterval = (1/FPS)*1000; // Sampling interval in milliseconds


//measurment variables
uint8_t bassPeak = 0, midPeak = 0, treblePeak = 0;
uint8_t smoothedBass = 0, smoothedMid = 0, smoothedTreble = 0;
float alpha = 0.2; // Smoothing factor
int8_t bassBuffer[SAMPLE_SIZE] = {0};
int8_t midBuffer[SAMPLE_SIZE] = {0};
int8_t trebleBuffer[SAMPLE_SIZE] = {0};
uint8_t sampleIndex = 0;

unsigned long previousMillis = 0;
uint8_t printCounter = 0;

void takeSample();
void updateDisplay();
int8_t calculateAmplitude(int8_t *buffer);
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
  while(!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)){
    Serial.println(F("OLED allocation failed"));
  }
  display.clearDisplay();
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= samplingInterval) {
    previousMillis = currentMillis;
    takeSample();
    updateDisplay();
    printCounter++;
  }
}


void takeSample(){
  // Read peak values from each frequency band (simulate or real-time values)
  bassBuffer[sampleIndex] = analogRead(bassPin) * (1023/127);
  midBuffer[sampleIndex] = analogRead(midPin)* (1023/127);
  trebleBuffer[sampleIndex] = analogRead(treblePin)* (1023/127);

  //circluar increment
  sampleIndex = (sampleIndex + 1) % SAMPLE_SIZE;



  //get amplitude
  bassPeak = calculateAmplitude(bassBuffer);
  midPeak = calculateAmplitude(midBuffer);
  treblePeak = calculateAmplitude(trebleBuffer);

 

  // Scale the ADC value (0 to 1023) to a height on the OLED
  int bassHeight = map(bassPeak, 0, 127, 0, maxBarHeight);
  int midHeight = map(midPeak, 2.5 * VOLT_TO_ADC, ADC_MAX, 0, maxBarHeight);
  int trebleHeight = map(treblePeak, 2.5 * VOLT_TO_ADC, ADC_MAX, 0, maxBarHeight);

  // Smooth transitions using exponential smoothing
  smoothedBass = alpha * bassHeight + (1 - alpha) * smoothedBass;
  smoothedMid = alpha * midHeight + (1 - alpha) * smoothedMid;
  smoothedTreble = alpha * trebleHeight + (1 - alpha) * smoothedTreble;
   //changed the debug define to 1 to print
  if(DEBUG && printCounter >= 10){
    Serial.println(smoothedBass);
  }
}


  // display.fillRect(10, 0, barWidth, SCREEN_HEIGHT, SSD1306_WHITE);
  // display.display();
void updateDisplay(){
  display.clearDisplay();

  display.fillRect(10, SCREEN_HEIGHT - smoothedBass, barWidth, smoothedBass, SSD1306_WHITE);
  display.fillRect(50, SCREEN_HEIGHT - smoothedMid, barWidth, smoothedMid, SSD1306_WHITE);
  display.fillRect(90, SCREEN_HEIGHT - smoothedTreble, barWidth, smoothedTreble, SSD1306_WHITE);

  display.display();
}

int8_t calculateAmplitude(int8_t *buffer) {
  // Calculate the DC offset
  // long sum = 0;
  // for (int i = 0; i < SAMPLE_SIZE; i++) {
  //   sum += buffer[i];
  // }
  // float mean = sum / (float)SAMPLE_SIZE;

  // Calculate peak-to-peak
  int8_t maxVal = 0, minVal = ADC_MAX;
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



//test 
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
