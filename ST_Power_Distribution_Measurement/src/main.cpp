#include <Arduino.h>

// put function declarations here:
void ReadData();

void setup() {
  Serial.begin(9600);
}

void loop() {
  ReadData();
  delay(10);
}

// put function definitions here:
void ReadData() {
  Serial.println(analogRead(A0));
}