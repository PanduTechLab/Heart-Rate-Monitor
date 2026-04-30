#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define PULSE_PIN 1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Graph variables
int lastX = 0;
int lastY = 26; 

// BPM variables
int threshold = 2200; 
unsigned long lastBeatTime = 0;
int BPM = 0;
bool beatDetected = false;

// Signal filtering variables
int signalMax = 0;
int signalMin = 4095;
int amplitude = 0;
const int minAmplitudeRequired = 200; // MUST have this much signal swing to count as a beat

void setup() {
  Serial.begin(115200);
  Wire.begin(8, 9);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) for(;;);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.display();
}

void loop() {
  int rawSignal = analogRead(PULSE_PIN);
  
  // --- SIGNAL ANALYSIS (To filter noise) ---
  if(rawSignal < signalMin) signalMin = rawSignal;
  if(rawSignal > signalMax) signalMax = rawSignal;
  amplitude = signalMax - signalMin;

  // --- GRAPHING ---
  int currentY = map(rawSignal, 0, 4095, 45, 7);
  currentY = constrain(currentY, 7, 45); 

  int clearX = lastX + 3;
  if (clearX >= 128) clearX = 0; 
  display.drawLine(clearX, 7, clearX, 45, BLACK); 
  display.drawLine(clearX + 1, 7, clearX + 1, 45, BLACK);
  display.drawLine(lastX, lastY, lastX + 2, currentY, WHITE);
  
  // --- BPM CALCULATION (With Amplitude Filter) ---
  // Only count a beat if it crosses threshold AND has enough amplitude (it's a real pulse)
  if (rawSignal > threshold && !beatDetected && amplitude > minAmplitudeRequired) {
    unsigned long currentTime = millis();
    unsigned long interval = currentTime - lastBeatTime;
    
    if (interval > 300) { 
      BPM = 60000 / interval;
      lastBeatTime = currentTime;
      beatDetected = true;
      
      display.fillRect(0, 46, 128, 18, BLACK); 
      display.setTextSize(2);
      display.setCursor(0, 46);
      display.print("BPM: ");
      display.print(BPM);
    }
  }
  
  if (rawSignal < threshold) beatDetected = false;

  // Reset Min/Max periodically to keep tracking real-time
  if (millis() % 1000 == 0) {
    signalMin = 2048; 
    signalMax = 2048;
  }

  // Timeout Logic
  if (millis() - lastBeatTime > 3000) {
    if (BPM != 0) {
      display.fillRect(0, 46, 128, 18, BLACK);
      display.setCursor(0, 46);
      display.print("BPM: 0");
      BPM = 0;
    }
  }
  
  display.display();
  lastX += 2;
  lastY = currentY;
  if (lastX >= 128) lastX = 0;
  delay(20);
}