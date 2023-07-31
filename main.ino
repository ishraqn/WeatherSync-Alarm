/*
Description: This is the main file for the Arduino code. It contains the main loop and the state machine logic.
Author: Ishraq Md Nazrul  
Student ID: 3613766
Course: COMP 444 
Assignment: Final Project
Date: July 31, 2023
*/

// header files
#include <LiquidCrystal.h>

// global variables
String incomingData = "";
String currentTime = "";
String weatherData = "";
String alarmTime = "";

unsigned long approachStart = 0;
unsigned long countdownStart = 0;
unsigned long promptCancelStart = 0;
const unsigned long promptCancelTimeout = 10000;

float countdown = 0;
float originalCountdown = 0;
bool snooze = false;
float initialDistance = 0;
bool alarmSet = false;

// pin definitions
const int photoPin = A0;
const int ledPin = 3;
const int speakerPin = 4;
const int trigPin = 6;
const int echoPin = 5;
const int buttonPin = 7;
LiquidCrystal lcd(13, 12, 11, 10, 9, 8);

// state machine states
enum State
{
  IDLE,
  APPROACHING,
  ALARM,
  ALARM_SET,
  CANCEL_PROMPT,
  CANCEL_ALARM,
  SNOOZE
};

State currentState;

// setup function
void setup()
{
  Serial.begin(9600);
  currentState = IDLE; // Set initial state to IDLE

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  pinMode(speakerPin, OUTPUT);

  lcd.begin(16, 2);
}

// function to check if the room is bright
bool isRoomBright()
{
  int photoVal = analogRead(photoPin);
  return photoVal > 450;
}

// function to get the distance from the ultrasonic sensor
float getDistance()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  float duration = pulseIn(echoPin, HIGH);
  float distance = (duration * 0.0343) / 2;
  return distance;
}

// function to check if the button is pressed
bool checkButtonPressed()
{
  // Read the button pin
  int buttonState = digitalRead(buttonPin);

  // If the button state is LOW, then it's pressed
  if (buttonState == LOW)
  {
    Serial.println("Initial button press detected, applying debounce delay.");

    // Pause to "debounce" the button input (accounting for physical button mechanics)
    delay(50);

    // Check the button state again to confirm the press wasn't noise or a very short press
    buttonState = digitalRead(buttonPin);

    if (buttonState == LOW)
    { // The button is still pressed
      Serial.println("Button press confirmed after debounce delay.");
      return true; // Button was pressed
    }
    else
    {
      Serial.println("Likely a false positive due to noise or a very short press therefore not confirmed.");
    }
  }
  else
  { // Button state is HIGH, so it's not pressed
    Serial.println("No button press detected.");
  }

  // no button press detected
  return false;
}

// main loop
void loop()
{
  float distance = getDistance();

  // Read incoming data from serial
  while (Serial.available())
  {
    incomingData = Serial.readStringUntil('\n');
  }

  switch (currentState)
  {

  // IDLE state
  case IDLE:
    Serial.println("Current State: IDLE");

    // parse the incoming data into current time and weather data
    if (incomingData.indexOf("|") > 0)
    {
      int delimiterIndex = incomingData.indexOf("|");
      currentTime = incomingData.substring(0, delimiterIndex);

      String cityAndWeather = incomingData.substring(delimiterIndex + 1);
      if (cityAndWeather.indexOf(",") > 0)
      {
        delimiterIndex = cityAndWeather.indexOf(",");
        String city = cityAndWeather.substring(0, delimiterIndex);
        String temperature = cityAndWeather.substring(delimiterIndex + 2);
        weatherData = city + ", " + temperature;
      }
    }
    // check if button is pressed to change state to ALARM
    if (checkButtonPressed())
    {
      currentState = ALARM;
      lcd.clear();
      break;
    }

    // check if the user is approaching the device or closeby
    Serial.print("The closest object is found at " + String(distance));
    if (distance < 25)
    {
      if (approachStart == 0)
      {
        approachStart = millis();
      }
      else if (millis() - approachStart >= 2000)
      {
        currentState = APPROACHING;
        lcd.clear();
        countdownStart = millis();
        countdown = 8; // Reset countdown each time we enter APPROACHING state
      }
    }

    // remain in the IDLE state
    else
    {
      approachStart = 0;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("T:" + currentTime);
      lcd.setCursor(0, 1);
      lcd.print("W:" + weatherData);
    }
    break;

  // APPROACHING state
  case APPROACHING:
    Serial.println("Current State: APPROACHING");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Set Alarm?");

    // countdown for the approaching state to go back to IDLE
    if (millis() - countdownStart >= 1000)
    {
      countdownStart = millis();
      countdown--;
      Serial.println("APPROACHING state, decreasing countdown: " + String(countdown));
    }

    // countdown logic
    lcd.setCursor(0, 1);
    lcd.print("Countdown: ");
    lcd.print(countdown);

    // If button is pressed, go to ALARM
    if (checkButtonPressed())
    {
      currentState = ALARM;
      countdown = 0;
      lcd.clear();
      break;
    }

    // If countdown reached 0 without button press, go back to IDLE
    if (countdown <= 0)
    {
      currentState = IDLE;
      lcd.clear();
      break;
    }
    break;

  // ALARM state
  case ALARM:
    Serial.println("Current State: ALARM");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SET ALARM:");

    // If button is pressed, go back to IDLE or when snooze is confirmed by swipe
    if (checkButtonPressed() || snooze)
    {
      Serial.println(String("snooze ? ") + (snooze ? "Yes" : "No"));

      // timer is being set
      if (countdown > 0)
      {
        Serial.println("ALARM state, countdown starting at: " + String(countdown));
        originalCountdown = countdown;

        // Get current time in minutes
        int currentHours = currentTime.substring(0, 2).toInt();
        int currentMinutes = currentTime.substring(3, 5).toInt();
        int totalCurrentMinutes = currentHours * 60 + currentMinutes;

        // Calculate the alarm time
        int totalAlarmMinutes = totalCurrentMinutes + countdown;

        // Convert it back to HH:MM format since Arduino wasn't able to support second hand
        alarmTime = String(totalAlarmMinutes / 60) + ":" + String(totalAlarmMinutes % 60);

        // go to ALARM_SET state and start the countdown timer for the alarm
        alarmSet = true;
        currentState = ALARM_SET;
        countdownStart = millis();
        lcd.clear();
        initialDistance = 0;
        break;
      }

      // timer is not being set, go back to IDLE state
      else
      {
        currentState = IDLE;
        countdown = 0;
        lcd.clear();
        initialDistance = 0;
        break;
      }
    }

    // logic for the motion sensor to increment and decrement the countdown timer for alarm
    else
    {
      float currentDistance = getDistance(); // get the current distance
      float maxDistance = 30.0;              // Set a maximum distance beyond which no action is taken

      // If the current distance is greater than the maximum distance, do not update the countdown
      if (currentDistance > maxDistance)
      {
        lcd.setCursor(0, 1);
        lcd.print("Time: ");
        lcd.print(countdown);
        lcd.print(" mins");
        break;
      }

      // check if initial distance is not set
      if (initialDistance == 0)
      {
        initialDistance = currentDistance; // set the initial distance
      }

      float leeway = 3.0; // this leeway is used to prevent the countdown from changing too quickly and prevent false positives by hand movements

      // if current distance is less than initial distance (with leeway) and countdown is less than 60
      if (currentDistance < initialDistance - leeway && countdown < 60)
      {
        countdown++; //   increase countdown time
        Serial.println("ALARM state, increasing countdown: " + String(countdown));
      }

      // if current distance is more than initial distance (with leeway) and countdown is more than 0
      else if (currentDistance > initialDistance + leeway && countdown > 0)
      {
        countdown--; // decrease countdown time
        Serial.println("ALARM state, decreasing countdown: " + String(countdown));
      }
      lcd.setCursor(0, 1);
      lcd.print("Time: ");
      lcd.print(countdown);
      lcd.print(" mins");
    }
    break;

  // ALARM_SET state
  case ALARM_SET:
    Serial.println("Current State: ALARM_SET");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("END: ");

    // Display the real time when the alarm will go off
    lcd.print(alarmTime);

    // Display the countdown timer
    lcd.setCursor(0, 1);
    lcd.print("COUNTDOWN: " + String(countdown) + " mins");

    // Update countdown every minute
    if (millis() - countdownStart >= 60000) // 60000 milliseconds = 1 minute
    {
      countdownStart = millis();
      countdown--;
      Serial.println("ALARM_SET state, decreasing countdown: " + String(countdown));
    }

    // If countdown reached 0, go back to IDLE
    if (countdown <= 0)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Time's UP");
      lcd.setCursor(0, 1);
      lcd.print("Swipe to Snooze!");

      // check for motion to snooze the alarm
      for (int i = 0; i < 10; i++)
      {
        distance = getDistance();
        if (distance < 10)
        {
          snooze = true;
          currentState = ALARM;

          // Set the snooze time to half of the original countdown time unless the original countdown time is less than 2 minutes
          if (originalCountdown < 2)
          {
            countdown = originalCountdown;
          }
          else
          {
            countdown = originalCountdown / 2;
          }

          // Set the snooze
          countdownStart = millis();
          lcd.clear();
          initialDistance = 0;
          Serial.println("Setting Snooze");
          break;
        }
        else
        {
          snooze = false;
        }

        tone(speakerPin, 1000);
        digitalWrite(ledPin, HIGH);
        delay(300);
        noTone(speakerPin);
        digitalWrite(ledPin, LOW);
        delay(300);

        // If the loop completes without detecting any motion to snooze
        if (i == 9)
        {
          currentState = IDLE;
          break;
        }
      }
      break;
    }

    Serial.println("Checking if button is pressed from ALARM_SET...");

    //  If button is pressed, transition to CANCEL_PROMPT state
    if (checkButtonPressed())
    {
      currentState = CANCEL_PROMPT;
      Serial.println("Button was pressed! Transitioning to CANCEL_PROMPT state.");
      Serial.println(currentState);
      break;
    }
    Serial.println("Button was not pressed. Staying in ALARM_SET state.");

    break;

    // CANCEL_PROMPT state
  case CANCEL_PROMPT:

    Serial.println("Current State: CANCEL_PROMPT");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Swipe to cancel");

    // Record the start time when entering CANCEL_PROMPT
    if (promptCancelStart == 0)
    {
      promptCancelStart = millis();
    }

    // Check for motion sensor input to cancel the alarm
    if (getDistance() < 10)
    {
      currentState = CANCEL_ALARM;
      promptCancelStart = 0; // Reset start time for next use
      break;
    }

    // If button is pressed again, go back to ALARM_SET
    if (checkButtonPressed())
    {
      currentState = ALARM_SET;
      promptCancelStart = 0; // Reset start time for next use
      break;
    }

    // If no action taken within timeout period, go back to ALARM_SET
    if (millis() - promptCancelStart >= promptCancelTimeout)
    {
      currentState = ALARM_SET;
      promptCancelStart = 0; // Reset start time for next use
    }

    break;

    // CANCEL_ALARM state
  case CANCEL_ALARM:
    Serial.println("Current State: CANCEL_ALARM");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Alarm cancelled");

    // reset the alarm and go back to IDLE
    countdown = 0;
    alarmSet = false;
    alarmTime = "";
    delay(2000);
    currentState = IDLE;
    break;
  }
  delay(500);
}