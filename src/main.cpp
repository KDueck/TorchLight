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

// Enum for states
enum State {
  MAIN_MENU,
  BRIGHTNESS_MENU,
  BLINKING_MENU,
  EXIT
};

State currentState = MAIN_MENU; // Initial state

// Function to print the main menu
void printMainMenu() {
  SerialBT.println("\nMain Menu:");
  SerialBT.println("1. Toggle LED");
  SerialBT.println("2. Show LED Brightness");
  SerialBT.println("3. Set LED Brightness");
  SerialBT.println("4. Set LED Blinking Sequence");
  SerialBT.println("5. Exit");
  SerialBT.print("Select an option: ");
}

// Function to print the brightness submenu
void printBrightnessMenu() {
  SerialBT.println("\nBrightness Menu:");
  SerialBT.println("1. Increase Brightness");
  SerialBT.println("2. Decrease Brightness");
  SerialBT.println("3. Set Brightness (0-255)");
  SerialBT.println("4. Go back to Main Menu");
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

void setup() {
  // Initialize Serial Monitor and Bluetooth Serial
  Serial.begin(115200);
  SerialBT.begin("ESP32_LED_Control"); // Start Bluetooth Serial

  // Set the LED pin as output
  pinMode(ledPin, OUTPUT);

  // Initialize PWM for LED control
  ledcSetup(pwmChannel, pwmFreq, pwmResolution);
  ledcAttachPin(ledPin, pwmChannel);

  SerialBT.println("ESP32 Bluetooth LED Control Ready.");
  printMainMenu(); // Print the main menu on Bluetooth
}

void handleMainMenu(char input) {
  switch (input) {
    case '1': // Toggle LED
      ledState = !ledState;
      ledcWrite(pwmChannel, ledState ? ledBrightness : 0);
      SerialBT.println(ledState ? "LED ON" : "LED OFF");
      break;

    case '2': // Show LED Brightness
      SerialBT.print("LED Brightness: ");
      SerialBT.println(ledBrightness);
      break;

    case '3': // Set LED Brightness
      currentState = BRIGHTNESS_MENU; // Transition to brightness menu
      return; // Early return to avoid fall-through

    case '4': // Set LED Blinking Sequence
      currentState = BLINKING_MENU; // Transition to blinking menu
      return; // Early return to avoid fall-through

    case '5': // Exit
      currentState = EXIT; // Transition to exit
      return; // Early return to avoid fall-through

    default:
      SerialBT.println("Invalid selection.");
      break;
  }
  printMainMenu(); // Print the main menu again
}

void handleBrightnessMenu(char input) {
  // Declare brightness variable outside of the switch statement
  int brightness = 0;

  switch (input) {
    case '1': // Increase Brightness
      ledBrightness = min(ledBrightness + 10, 255); // Increment brightness, max 255
      ledcWrite(pwmChannel, ledBrightness);
      SerialBT.print("Brightness Increased to: ");
      SerialBT.println(ledBrightness);
      break;

    case '2': // Decrease Brightness
      ledBrightness = max(ledBrightness - 10, 0); // Decrement brightness, min 0
      ledcWrite(pwmChannel, ledBrightness);
      SerialBT.print("Brightness Decreased to: ");
      SerialBT.println(ledBrightness);
      break;

    case '3': // Set Brightness manually
      SerialBT.print("Enter brightness (0-255): ");
      while (!SerialBT.available()); // Wait for user input
      brightness = SerialBT.parseInt();  // Get the user input here
      brightness = constrain(brightness, 0, 255); // Make sure it's in range
      ledBrightness = brightness;
      ledcWrite(pwmChannel, ledBrightness);
      SerialBT.print("Brightness set to: ");
      SerialBT.println(ledBrightness);
      break;

    case '4': // Go back to Main Menu
      currentState = MAIN_MENU; // Transition back to main menu
      return; // Early return to avoid fall-through

    default:
      SerialBT.println("Invalid selection.");
      break;
  }

  // Print the brightness menu again after handling the input
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
      return; // Early return to avoid fall-through

    default:
      SerialBT.println("Invalid selection.");
      break;
  }
  printBlinkingMenu(); // Print the blinking menu again
}

void loop() {
  if (SerialBT.available()) {
    char receivedChar = SerialBT.read(); // Read the received character

    // Handle input based on the current state
    switch (currentState) {
      case MAIN_MENU:
        handleMainMenu(receivedChar);
        break;

      case BRIGHTNESS_MENU:
        handleBrightnessMenu(receivedChar);
        break;

      case BLINKING_MENU:
        handleBlinkingMenu(receivedChar);
        break;

      case EXIT:
        SerialBT.println("Exiting...");
        return; // Exit the program

      default:
        break;
    }
  }

  // Handle blinking if enabled
  if (blinkInterval > 0) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= blinkInterval) {
      previousMillis = currentMillis; // Save the last time LED was updated
      ledState = !ledState; // Toggle LED state
      ledcWrite(pwmChannel, ledState ? ledBrightness : 0); // Apply LED state
    }
  }
}
