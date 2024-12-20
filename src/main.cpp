#include <Arduino.h>
#include <BluetoothSerial.h>

BluetoothSerial SerialBT;


// LED Pin and PWM setup
const int ledPin = 2; // Onboard LED pin
int ledBrightness = 255; // Default to full brightness
const int pwmChannel = 0;
const int pwmFreq = 5000;
const int pwmResolution = 8; // 8-bit resolution (0-255)

// Blinking sequence variables
unsigned long blinkInterval = 0; // Default no blinking
bool ledState = false; // LED state for blinking
unsigned long previousMillis = 0; // Store the time for blinking

String inputBuffer = ""; // To accumulate input until a full command is received

// Enum for states
enum State {
  MAIN_MENU,
  BRIGHTNESS_MENU,
  BLINKING_MENU,
  EXIT
};

// function prototypes
void processCommand(String command);
void handleMainMenu(char input);
void handleBrightnessMenu(char input);
void handleBlinkingMenu(char input);

State currentState = MAIN_MENU; // Initial state

TaskHandle_t bluetoothTaskHandle = NULL;
TaskHandle_t ledTaskHandle = NULL;

// Function to print the main menu
void printMainMenu() {
  SerialBT.println("\nMain Menu:");
  SerialBT.println("1. Toggle LED");
  SerialBT.println("2. Set LED Brightness");
  SerialBT.println("3. Set LED Blinking Sequence");
  SerialBT.println("4. Exit");
  SerialBT.print("Select an option: ");
}

// Function to print the brightness submenu
void printBrightnessMenu() {
  SerialBT.print("\nCurrent Brightness:");
  SerialBT.println(ledBrightness);
  SerialBT.println("Brightness Menu:");
  SerialBT.println("1. Set Brightness (0-255)");
  SerialBT.println("2. Go back to Main Menu");
  SerialBT.print("Select an option: ");
}

// Function to print the blinking sequence submenu
void printBlinkingMenu() {
  SerialBT.println("\nBlinking Sequence Menu:");
  SerialBT.println("1. Fast Blinking");
  SerialBT.println("2. Medium Blinking");
  SerialBT.println("3. Slow Blinking");
  SerialBT.println("4. Turn Off Blinking");
  SerialBT.println("5. Go back to Main Menu");
  SerialBT.print("Select an option: ");
}

void bluetoothTask(void *pvParameters) {
  while (true) {
    while (SerialBT.available()) {
      char receivedChar = SerialBT.read(); // Read one character

      // Accumulate input in buffer
      if (receivedChar == '\r' || receivedChar == '\n') {
        // End of input detected
        if (inputBuffer.length() > 0) {
          // Process the full command
          processCommand(inputBuffer);
          inputBuffer = ""; // Clear the buffer after processing
        }
      } else {
        inputBuffer += receivedChar; // Add the character to the buffer
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void ledTask(void *pvParameters) {
  while (true) {
    if (blinkInterval > 0) {
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis >= blinkInterval) {
        previousMillis = currentMillis; // Save the last time LED was updated
        ledState = !ledState; // Toggle LED state
        ledcWrite(pwmChannel, ledState ? ledBrightness : 0); // Apply LED state
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void setup() {
  // Initialize Serial Monitor and Bluetooth Serial
  Serial.begin(115200);
  SerialBT.begin("ESP32_LED_Control"); // Start Bluetooth Serial
  SerialBT.setTimeout(10000);

  // Set the LED pin as output
  pinMode(ledPin, OUTPUT);

  // Initialize PWM for LED control
  ledcSetup(pwmChannel, pwmFreq, pwmResolution);
  ledcAttachPin(ledPin, pwmChannel);

  SerialBT.println("ESP32 Bluetooth LED Control Ready.");
  printMainMenu(); // Print the main menu on Bluetooth

  // Create FreeRTOS tasks
  xTaskCreatePinnedToCore(
    bluetoothTask,   // Task function
    "BluetoothTask", // Task name
    2048,            // Stack size
    NULL,            // Parameters
    1,               // Priority
    &bluetoothTaskHandle, // Task handle
    0                // Core 0
  );

  xTaskCreatePinnedToCore(
    ledTask,         // Task function
    "LedTask",      // Task name
    2048,            // Stack size
    NULL,            // Parameters
    1,               // Priority
    &ledTaskHandle,  // Task handle
    0                // Core 0
  );
}

void processCommand(String command) {
  command.trim(); // Remove any leading/trailing whitespace or newlines

  if (command.length() == 1) {
    char input = command[0]; // Get the first character of the command

    // Handle input based on the current state
    switch (currentState) {
      case MAIN_MENU:
        handleMainMenu(input);
        break;

      case BRIGHTNESS_MENU:
        handleBrightnessMenu(input);
        break;

      case BLINKING_MENU:
        handleBlinkingMenu(input);
        break;

      case EXIT:
        SerialBT.println("Exiting...");
        return; // Exit the program

      default:
        break;
    }
  } else {
    SerialBT.println("Invalid input. Please enter a valid option.");
  }
}

void handleMainMenu(char input) {
  switch (input) {
    case '1': // Toggle LED
      ledState = !ledState;
      ledcWrite(pwmChannel, ledState ? ledBrightness : 0);
      SerialBT.println(ledState ? "LED ON" : "LED OFF");
      break;

    case '2': // Set LED Brightness
      currentState = BRIGHTNESS_MENU; // Transition to brightness menu
      printBrightnessMenu(); // Display the brightness menu
      return; // Early return to avoid fall-through

    case '3': // Set LED Blinking Sequence
      currentState = BLINKING_MENU; // Transition to blinking menu
      printBlinkingMenu(); // Display the blinking menu
      return; // Early return to avoid fall-through

    case '4': // Exit
      currentState = EXIT; // Transition to exit
      return; // Early return to avoid fall-through

    default:
      SerialBT.println("Invalid selection.");
      break;
  }
  printMainMenu(); // Print the main menu again
}

void handleBrightnessMenu(char input) {
  int brightness = 0;

  switch (input) {
    case '1': // Set Brightness manually
      SerialBT.print("Enter brightness (0-255): ");
      while (!SerialBT.available()); // Wait for user input
      brightness = SerialBT.parseInt();  // Get the user input here
      brightness = constrain(brightness, 0, 255); // Make sure it's in range
      ledBrightness = brightness;
      ledcWrite(pwmChannel, ledBrightness);
      SerialBT.print("Brightness set to: ");
      SerialBT.println(ledBrightness);
      break;

    case '2': // Go back to Main Menu
      currentState = MAIN_MENU; // Transition back to main menu
      printMainMenu(); // Display the main menu
      return; // Early return to avoid fall-through

    default:
      SerialBT.println("Invalid selection.");
      break;
  }

  printBrightnessMenu();
}

void handleBlinkingMenu(char input) {
  switch (input) {
    case '1': // Fast Blinking
      blinkInterval = 200; // 200 ms (fast)
      SerialBT.println("Fast Blinking selected.");
      break;

    case '2': // Medium Blinking
      blinkInterval = 500; // 500 ms (medium)
      SerialBT.println("Medium Blinking selected.");
      break;

    case '3': // Slow Blinking
      blinkInterval = 1000; // 1000 ms (slow)
      SerialBT.println("Slow Blinking selected.");
      break;

    case '4': // Turn off blinking
      blinkInterval = 0; // Stop blinking
      ledState = false;  // Ensure the LED is off
      ledcWrite(pwmChannel, ledState ? ledBrightness : 0);
      SerialBT.println("Blinking turned off.");
      break;

    case '5': // Go back to Main Menu
      currentState = MAIN_MENU; // Transition back to main menu
      printMainMenu(); // Display the main menu
      return; // Early return to avoid fall-through

    default:
      SerialBT.println("Invalid selection.");
      break;
  }
  printBlinkingMenu();
}

void loop() {
  // Unused in FreeRTOS setup
}
