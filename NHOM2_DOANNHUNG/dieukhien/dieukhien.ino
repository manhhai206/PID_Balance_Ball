#include <ESP32Servo.h>
#include <Wire.h>

volatile float previous_error_x = 0;
volatile float previous_error_y = 0;
volatile int period_x = 1;
volatile int period_y = 1;
volatile int servoAngleX, servoAngleY;

// Khai báo Servo
const int servoPin1 = 16;
const int servoPin2 = 17;
Servo servo1;
Servo servo2;

// Khai báo biến lỗi
volatile float x_error = -1, y_error = -1;

TaskHandle_t Task1; 
TaskHandle_t Task2;
SemaphoreHandle_t xMutex;

void setup() {
  Serial.begin(115200);
  servo1.attach(servoPin1);
  servo2.attach(servoPin2);

  xMutex = xSemaphoreCreateMutex();
  xTaskCreatePinnedToCore(Task1code, "Task1", 10000, NULL, 1, &Task1, 0);    
  xTaskCreatePinnedToCore(Task2code, "Task2", 10000, NULL, 1, &Task2, 1);   
}

void loop() {}

void Task1code(void * parameter) {
  for (;;) {
    if (Serial.available() > 0) {
      String input = Serial.readStringUntil('\n');
      
      if (input == "no") {
        xSemaphoreTake(xMutex, portMAX_DELAY);
        x_error = -1;
        y_error = -1;
        xSemaphoreGive(xMutex);
      } else {
        int separatorIndex = input.indexOf('x');
        if (separatorIndex != -1) { 
          String x_str = input.substring(0, separatorIndex); 
          String y_str = input.substring(separatorIndex + 1); 
    
          xSemaphoreTake(xMutex, portMAX_DELAY);
          x_error = (float)x_str.toInt();
          y_error = (float)y_str.toInt();
          xSemaphoreGive(xMutex);
        }
      }
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}


void Task2code(void * parameter) { 
  for (;;) {
      xSemaphoreTake(xMutex, portMAX_DELAY);
      float x = x_error;
      float y = y_error;
      xSemaphoreGive(xMutex);
      
      if (x >= 0 && x <= 180) {
        servo1.write(x);
      } else {
        servo1.write(90);  
      }

      if (y >= 0 && y <= 180) {
        servo2.write(y);
      } else {
        servo2.write(90);  
      }
    
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}
