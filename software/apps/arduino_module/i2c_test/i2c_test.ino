#include <Wire.h>
//#include <SoftwareSerial.h>
#include <Arduino.h>



uint8_t g_buf[300];
int g_buf_size = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(A1, OUTPUT);
  digitalWrite(A1, HIGH);
  Wire.begin(0x20);
  Wire.setClock(400000);
  Wire.onReceive(print_receive);
  Serial.begin(9600);
}

void loop() {
  uint8_t print_buf[300];
  int print_buf_size = 0;
  for (int i = 0; i < g_buf_size; ++i) {
    print_buf[i] = g_buf[i];
    ++print_buf_size;
  }
  if (print_buf_size > 0) {
    g_buf_size = 0;
    Serial.print("Msg: ");   
  
    for (int i = 0; i < print_buf_size; ++i) {
      Serial.print(print_buf[i], HEX);
      Serial.print(", ");
    }
    if (print_buf_size > 0) {
      Serial.print('\n'); 
    }
  }
}

void print_receive(int numBytes) {
  //digitalWrite(A1, LOW);
  Serial.print("Num recevied: ");
  Serial.print(numBytes);
  while (Wire.available()) {
    g_buf[g_buf_size++] = Wire.read();
  }
  //digitalWrite(A1, HIGH);
}

