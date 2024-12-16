// Include necessary libraries
#include <Arduino.h>
#include "BluetoothSerial.h"

// Function prototypes

void showMainMenu();
void handleUserInput(char input);
void showBrightnessMenu();
void handleBrightnessInput(char input);

// Set up initial variables
int ledPin = 2; // Example pin (LED) to control
bool ledState = false; // Current state of the LED
int brightness = 0; // Brightness of LED

String device_name = "ESP32-BT-Slave";

// Check if Bluetooth is available
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

// Check Serial Port Profile
#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Port Profile for Bluetooth is not available or not enabled. It is only available for the ESP32 chip.
#endif

BluetoothSerial SerialBT;

void setup() {
  // Start serial communication at 9600 baud
  Serial.begin(115200);
  SerialBT.begin(device_name);  //Bluetooth device name
  //SerialBT.deleteAllBondedDevices(); // Uncomment this to delete paired devices; Must be called after begin
  Serial.printf("The device with name \"%s\" is started.\nNow you can pair it with Bluetooth!\n", device_name.c_str());

  // Set up the LED pin
  pinMode(ledPin, OUTPUT);
  
  // Display the initial menu
  showMainMenu();
}

void loop() {
  // Check if data is available in the Serial Monitor
  if (SerialBT.available()) {
    char userInput = SerialBT.read(); // Read the user's input

    // If the user pressed Enter (newline or carriage return), ignore it
    if (userInput == '\n' || userInput == '\r') {
      return;
    }

    // Handle the user's input
    handleUserInput(userInput);
  }


  // Additional logic can go here (e.g., sensor readings, other functionality)
}


void showMainMenu() {
  // Display the menu options to the Serial Monitor
  SerialBT.println("====================================");
  SerialBT.println("Welcome to the Arduino Project Menu");
  SerialBT.println("====================================");
  SerialBT.println("1. Toggle ON/OFF");
  SerialBT.println("2. Set Brightness");
  SerialBT.println("3. Exit");
  SerialBT.println("====================================");
  SerialBT.print("Enter an option (1-3): ");
}

void handleUserInput(char input) {
  switch (input) {
    case '1': // Toggle LED
      ledState = !ledState; // Toggle the LED state
      digitalWrite(ledPin, ledState ? HIGH : LOW); // Set LED state
      //Serial.println(ledState ? "LED is ON" : "LED is OFF");
      SerialBT.println(ledState ? "LED is ON" : "LED is OFF");
      break;

    case '2': // Get LED Status
      //Serial.println(ledState ? "LED is ON" : "LED is OFF");
      showBrightnessMenu();
      delay(50);
      if (SerialBT.available())
      {
        SerialBT.println("Waiting");
        char brightnessInput = SerialBT.read(); // Read the user's input
            // If the user pressed Enter (newline or carriage return), ignore it
            SerialBT.print("brightnessInput: ");
        SerialBT.println(brightnessInput);
        if (brightnessInput == '\n' || brightnessInput == '\r')
        {
          return;
        }
        SerialBT.print("brightnessInput: ");
        SerialBT.println(brightnessInput);
        // If the user pressed Enter (newline or carriage return), ignore it

        // Handle the user's input
        handleUserInput(brightnessInput);
      }
      SerialBT.println("Exited");
      //char brightnessInput = SerialBT.read(); // Read the user's brightness input
      //handleBrightnessInput(brightnessInput);
      break;

    case '3': // Exit
      //Serial.println("Exiting menu...");
      SerialBT.println("Exiting menu...");
      break;

    default:
      //Serial.println("Invalid option. Please try again.");
      SerialBT.println("Invalid option. Please try again.");
      break;
  }

  // Show the menu again after an action
  if (input != '3') {
    showMainMenu();
  }
}

void showBrightnessMenu() {
  // Display the menu options to the Serial Monitor
  SerialBT.println("====================================");
  SerialBT.print("Current Brightness is: ");
  SerialBT.println(brightness);
  SerialBT.println("====================================");
  SerialBT.println("Enter a new value between 0 and 255");
  SerialBT.println("====================================");
}

void handleBrightnessInput(char input){
  SerialBT.println(input);
  brightness = input;
  if (0 <= input && input <= 10){
    SerialBT.print("New brightness is: ");
    SerialBT.println(brightness);
  }
  else SerialBT.println("Invalid input.");
}