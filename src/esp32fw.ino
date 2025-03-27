#include <Arduino.h>
#include <WiFi.h>

#include <ESPAsyncWebServer.h>
#include <AccelStepper.h>
#include <string>
#include <Preferences.h>
#include <WiFiMulti.h>


// Initialize Wi-Fi connection
const char* ssid1 = "pupetos";
const char* password1 = "nnueasee";
const char* ssid2 = "pupetos_EXT";
const char* password2 = "nnueasee";
WiFiMulti wifiMulti;
const uint32_t connectTimeoutMs = 10000;
String old_ssid = "";

#define HOSTNAME "ESP32FW"

// Stepper control definitions
#define HALFSTEP 8        // Unipolar half-step in AccelStepper
#define MOTOR_PIN_1  5    // IN1 on ULN2003 ==> Blue   on 28BYJ-48
#define MOTOR_PIN_2  6    // IN2 on ULN2004 ==> Pink   on 28BYJ-48
#define MOTOR_PIN_3  7    // IN3 on ULN2003 ==> Yellow on 28BYJ-48
#define MOTOR_PIN_4  8    // IN4 on ULN2003 ==> Orange on 28BYJ-48
#define HALL_SENSOR  4    // 49E analog Hall sensor

#define MOTOR_IDLE_TIME_MS 10000  // Time before power off (10 seconds)

#define MOTOR_MAX_SPEED 800
#define MOTOR_ACCELERATION 200
#define MOTOR_STOP_ACCELERATION 800

// Filter wheel/stepper motor definitions
#define NUMBER_OF_FILTERS 5
#define STEPS_PER_FILTER 4025 //3950
#define HOME_OFFSET -200

// Calculations 
#define MOVING_AVG_COUNT 5
#define ADC_COUNT 20
#define HOME_RANGE 20000
#define HOME_TEST 1000

int adc_buffer[MOVING_AVG_COUNT] = {0};
int adc_buffer_index = 0;
int adc_max_v[ADC_COUNT] = {0};
int adc_max = 0;
int adc_min_v[ADC_COUNT] = {};
int adc_min = 0;
int adc_min_p[ADC_COUNT] = {0};
int adc_initial_pos = 0;

// Stepper motor setup
AccelStepper motor(HALFSTEP, MOTOR_PIN_1, MOTOR_PIN_3, MOTOR_PIN_2, MOTOR_PIN_4);
volatile bool motorIsRunning = false;
volatile bool motorIsOff = false;
volatile bool motorIsHoming = false;
volatile bool motorIsGoingHome = false;
unsigned long lastMovementTime = 0;
volatile bool motorStop = false;
volatile bool homeOk = false;
volatile bool homeSuccess = false;
volatile int filterPosition = -1;

// Create server objects: A real HTTP server and a simple websocket server to support INDI
AsyncWebServer webServer(80);
WiFiServer INDIServer(8080); // We need a second server because ESPAsyncWebServer does not support persistent connections
WiFiClient INDIClient;
String INDIRequest="";

// Task Handles
TaskHandle_t webServerTaskHandle = NULL;
TaskHandle_t indiTaskHandle = NULL;
TaskHandle_t stepperControlTaskHandle = NULL;
TaskHandle_t homePositionTaskHandle = NULL;
TaskHandle_t motorIdleTaskHandle = NULL;
TaskHandle_t multiWifiTaskHandle = NULL;

// Common functions

int calcFilterPosition()
 {
  int motorPosition = motor.currentPosition();
  return (motorPosition % STEPS_PER_FILTER) == 0 ? motorPosition / STEPS_PER_FILTER + 1 : -1;
 }

int read_adc(int raw) 
 {
  int avg = 0;
  int i = 0;
  
  adc_buffer_index++;

  if (adc_buffer_index < MOVING_AVG_COUNT)
   {
    return 0;
   }

  for (i=MOVING_AVG_COUNT-2;i>=0;i--)
   {
    adc_buffer[i+1] = adc_buffer[i];
    avg += adc_buffer[i];
   }
  adc_buffer[0] = raw;

  return ((avg+raw)/MOVING_AVG_COUNT);
 }

int adc_max_check(int measurement) 
 {
  int min = adc_max_v[0];
  int pos = 0;
  int max = 0;

  for (int i=0;i<ADC_COUNT;i++)
   {
    if (adc_max_v[i] <= min)
     {
      min = adc_max_v[i];
      pos = i;
     }
    max += adc_max_v[i]; 
   }
  if (measurement > min)
   {
    adc_max_v[pos] = measurement;
   }
  return max/ADC_COUNT; 
 }

int adc_min_check(int measurement, int position) 
 {
  int max = adc_min_v[0];
  int pos = 0;
  int min = 0;

  for (int i=0;i<ADC_COUNT;i++)
   {
    if (adc_min_v[i] >= max)
     {
      max = adc_min_v[i];
      pos = i;
     }
    min += adc_min_v[i]; 
   }
  if (measurement < max)
   {
    adc_min_v[pos] = measurement;
    adc_min_p[pos] = position;
   }
  return min/ADC_COUNT; 
 }

// Web server task
void webServerTask(void *pvParameters) 
 {
  webServer.on("/move", HTTP_GET, [](AsyncWebServerRequest *request)
   {
    if(request->hasParam("p") && !motorIsRunning && !motorIsHoming)
     {
      String position = request->getParam("p")->value();
      int targetPosition = position.toInt();
      motor.setAcceleration(MOTOR_ACCELERATION);
      motor.moveTo(targetPosition);
      motorIsRunning = true;
      request->send(200, "text/plain", "Moving wheel to position " + String(targetPosition));
     }
    else
     {
      if(request->hasParam("f") && !motorIsRunning && !motorIsHoming && homeSuccess)
       {
        String filter = request->getParam("f")->value();
        int filterPosition = filter.toInt();
        if (filterPosition > NUMBER_OF_FILTERS || filterPosition < 1)
         {
          request->send(400, "text/plain", "That filter does not exist in the wheel");
         }
        else
         {
          motor.setAcceleration(MOTOR_ACCELERATION);
          motor.moveTo((filterPosition-1)*STEPS_PER_FILTER);
          motorIsRunning = true;
          request->send(200, "text/plain", "Moving wheel to filter " + String(filterPosition));
         } 
       }
      else
       {
        request->send(400, "text/plain", "Cannot move without a valid position or to a filter before homing or when the motor is already moving");
       }
     }
   }
  );

  webServer.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request)
   {
    if(request->hasParam("s"))
     {
      String stop = request->getParam("s")->value();
      int stopWheel = stop.toInt();
      if (stopWheel == 1 && motorIsRunning)
       {
        Serial.println("Vamos a parar un poco");
        motor.setAcceleration(MOTOR_STOP_ACCELERATION);
        motor.stop();
        motor.runToPosition(); 
        request->send(200, "text/plain", "Stopping wheel");
       }
      else
       {
        request->send(400, "text/plain", "Wheel can only be stopped with s=1 and while it is moving");
       } 
     } 
    else
     {
      request->send(400, "text/plain", "Wheel can only be halted with s=1");
     } 
   }
  );

  webServer.on("/home", HTTP_GET, [](AsyncWebServerRequest *request)
   {
    if(request->hasParam("h"))
     {
      String home = request->getParam("h")->value();
      int homeWheel = home.toInt();
      if (homeWheel == 1 && !motorIsRunning)
       {
        Serial.println("Vamos a casa");
        motorIsHoming = true;
        motorIsGoingHome = false;
        motor.setAcceleration(MOTOR_ACCELERATION);
        adc_initial_pos = motor.currentPosition();
        motor.moveTo(motor.currentPosition()+HOME_RANGE);
        motorIsRunning = true;
        homeSuccess = false;
        request->send(200, "text/plain", "Homing");
       }
      else
       {
        request->send(400, "text/plain", "Will go to home only if h=1 and the motor is not moving");
       } 
     } 
    else
     {
      request->send(400, "text/plain", "Will go to home only if h=1");
     } 
   }
  );

  webServer.on("/where", HTTP_GET, [](AsyncWebServerRequest *request)
   {
    String text = "Wheel is at = "+String(motor.currentPosition())+"(filter# = "+String(calcFilterPosition())+")";
    request->send(200, "text/plain", text);
   } 
  );
  // Set up the server to handle incoming requests
  webServer.begin();
  
  // Loop in task (optional, since server will run in background)
  while (true) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

// INDI interface
void indiTask(void *pvParameters) 
 {
  while (true) 
   {
    INDIClient = INDIServer.available();
    while (INDIClient.connected())
     {
      if (INDIClient.available())
       {
        char c = INDIClient.read();
        INDIRequest += c;
        if (c == '\n')
         {
          int pos = INDIRequest.indexOf("SF*"); // Select filter
          if (pos != -1) 
           {
            String filter = INDIRequest.substring(pos+3,pos+5);
            int filterPosition = filter.toInt();
/*            
            Serial.print("request = ");
            Serial.print(INDIRequest);
            Serial.print(", pos = ");
            Serial.print(pos);
            Serial.print(", filter = ");
            Serial.print(filter);
            Serial.print(", filterPosition = ");
            Serial.println(filterPosition);
*/
            motor.setAcceleration(MOTOR_ACCELERATION);
            motor.moveTo((filterPosition-1)*STEPS_PER_FILTER);
            motorIsRunning = true;
            INDIRequest="";
           }
          pos = INDIRequest.indexOf("HM*"); // Go to home position
          if (pos != -1) 
           {
            Serial.println("Home");
            motorIsHoming = true;
            motorIsGoingHome = false;
            motor.setAcceleration(MOTOR_ACCELERATION);
            adc_initial_pos = motor.currentPosition();
            motor.moveTo(motor.currentPosition()+HOME_RANGE);
            motorIsRunning = true;
            homeSuccess = false;
            INDIRequest="";
           }
          pos = INDIRequest.indexOf("WH*"); // Query filter position
          if (pos != -1) 
           {
            String response = "#";
            if (motorIsRunning)
             {
              INDIClient.print("WH*00#"); // Filter wheel is moving
             }
            else
             { 
              int f = calcFilterPosition(); // If answer is -1, this means the filter wheel is stopped at a non-filter position
              response = String(f) + response;
              if (f > 0 && f < 10)
               {
                response = "0" + response; // Filter wheel is stopped at a valid filter position
               }
              INDIClient.print("WH*" + response);
             }
           }           
         }
       }
     }

    //delay(1);
    INDIRequest="";
    INDIClient.stop();
   }
   vTaskDelay(1000 / portTICK_PERIOD_MS);
 }

// Stepper motor control task
void stepperControlTask(void *pvParameters) {
  while (true) {
    if (motorIsRunning && motor.distanceToGo() == 0) {
      motorIsRunning = false;
      lastMovementTime = millis();
      motorIsOff = false;
    }
    motor.run();
//    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

// Home position task (find home position)
void homePositionTask(void *pvParameters) 
 {
  int hall = 0;
  int position = 0;
  int new_home = 0;
  int min = 0;
  int max = 0;
  
  for (int i=0;i<ADC_COUNT;i++)
   {
    adc_min_v[i] = 10000;
    adc_max_v[i] = 0;
    adc_min_p[i] = 0;
  }

  while (true)
   {
    if (motorIsHoming) 
     {
      if (motorIsRunning)
       {
        hall = read_adc(analogRead(HALL_SENSOR));
        position = motor.currentPosition();
        max = adc_max_check(hall);
        min = adc_min_check(hall,position);
/*
        Serial.print("adc_initial_pos = ");
        Serial.print(adc_initial_pos);
        Serial.print(", position = ");
        Serial.print(position);
        Serial.print(", max = ");
        Serial.print(max);
        Serial.print(", min = ");
        Serial.print(min);
        if (homeOk)
         {
          Serial.println(", homeOk = true");
         }
        else
         {
          Serial.println(", homeOk = false");
         } 
*/

        if (abs(adc_initial_pos - position) > 1000 && !homeOk)
         {
          if ((max-min) > 200)
           {
            adc_initial_pos = motor.currentPosition();
            motor.moveTo(motor.currentPosition()+HOME_RANGE);
            hall = 0;
            position = 0;
            new_home = 0;
            for (int i=0;i<ADC_COUNT;i++)
             {
              adc_min_v[i] = 10000;
              adc_max_v[i] = 0;
              adc_min_p[i] = 0;
             }
           }
          else
           {
            homeOk = true;
           }  
         }
       }
      else
       {
        for (int i=0;i<ADC_COUNT;i++)
         {
          new_home += adc_min_p[i];
/*
          Serial.print("adc_max_v[");
          Serial.print(i);
          Serial.print("] = ");
          Serial.print(adc_max_v[i]);
          Serial.print(", adc_min_v = [");
          Serial.print(i);
          Serial.print("] = ");
          Serial.print(adc_min_v[i]);
          Serial.print(", adc_min_p = [");
          Serial.print(i);
          Serial.print("] = ");
          Serial.print(adc_min_p[i]);
          Serial.print(", new_home = ");
          Serial.print(new_home);
          Serial.print(", serCP = ");
          Serial.println(new_home/ADC_COUNT+HOME_OFFSET);
*/
         }
         motor.setCurrentPosition(HOME_RANGE-new_home/ADC_COUNT-HOME_OFFSET+adc_initial_pos);
         motor.moveTo(0);
         homeOk = false;
         homeSuccess = true;
         motorIsRunning = true;
         motorIsHoming = false;
         hall = 0;
         position = 0;
         new_home = 0;
         for (int i=0;i<ADC_COUNT;i++)
          {
           adc_min_v[i] = 10000;
           adc_max_v[i] = 0;
           adc_min_p[i] = 0;
          }
       }
     }
    vTaskDelay(5 / portTICK_PERIOD_MS);
  }
 }

// Motor idle power-off task
void motorIdleTask(void *pvParameters) {
 while (true) 
  {
   if (!motorIsRunning && (millis() - lastMovementTime >= MOTOR_IDLE_TIME_MS) && motorIsOff == false) {
    // Logic to cut off power to the motor
    motor.disableOutputs();
    motorIsOff = true;
    Serial.println("Motor power removed due to idle time.");
  }
  vTaskDelay(1000 / portTICK_PERIOD_MS);
 }
}

// MultiWifi task (multiple wifi AP's)
void multiWifiTask(void *pvParameters) {
 while(true)
  {
   if ((wifiMulti.run(connectTimeoutMs) == WL_CONNECTED) && (old_ssid != WiFi.SSID())) 
    {
     Serial.print("\nWiFi connected = ");
     Serial.println(WiFi.SSID());
     old_ssid = WiFi.SSID();
    }
  }  
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}

void setup() {
  Serial.begin(115200);

  // Initialize GPIO pins for stepper motor
  pinMode(MOTOR_PIN_1, OUTPUT);
  pinMode(MOTOR_PIN_2, OUTPUT);
  pinMode(MOTOR_PIN_3, OUTPUT);
  pinMode(MOTOR_PIN_4, OUTPUT);

  // Stepper motor setup
  motor.setSpeed(MOTOR_MAX_SPEED);
  motor.setMaxSpeed(MOTOR_MAX_SPEED);
  motor.setAcceleration(MOTOR_ACCELERATION);

  //Initialize WiFi
  wifiMulti.addAP(ssid1,password1);
  wifiMulti.addAP(ssid2,password2);
  WiFi.setHostname(HOSTNAME);
//  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  if(wifiMulti.run() == WL_CONNECTED)
   {
    Serial.println("\nWiFi connected");
    Serial.print("IP address = ");
    Serial.println(WiFi.localIP());
    old_ssid=WiFi.SSID();
  }
  
  INDIServer.begin();

  // Create FreeRTOS tasks
  xTaskCreate(webServerTask, "WebServerTask", 4096, NULL, 1, &webServerTaskHandle);
  xTaskCreate(indiTask, "INDITask", 2048, NULL, 1, &indiTaskHandle);
  xTaskCreate(stepperControlTask, "StepperControlTask", 2048, NULL, 1, &stepperControlTaskHandle);
  xTaskCreate(homePositionTask, "HomePositionTask", 2048, NULL, 1, &homePositionTaskHandle);
  xTaskCreate(motorIdleTask, "MotorIdleTask", 2048, NULL, 1, &motorIdleTaskHandle);
  xTaskCreate(multiWifiTask, "MultiWifiTask", 2048, NULL, 1, &multiWifiTaskHandle);
}

void loop() 
 {


/*


  hall_raw = analogRead(HALL_SENSOR);
  hall_mov_avg = read_adc(hall_raw);

//  Serial.print("MovAvg = ");
//  Serial.print(hall_mov_avg);
//  Serial.print(", Raw = ");
//  Serial.print(hall_raw);
//  Serial.print(", Position = ");
//  Serial.println(motor.currentPosition());
//  delay(10);
  //int pos = 1000;
  //stepper.moveTo(pos);
  //delay(pos/MOTOR_MAX_SPEED * 3000);
  //stepper.moveTo(0);
  //delay(pos/MOTOR_MAX_SPEED * 3000);
*/
}
