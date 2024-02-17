#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>
#include <SoftwareSerial.h>

#define SS_PIN 10
#define RST_PIN 9
#define LED A0

MFRC522 mfrc522(SS_PIN, RST_PIN);

// Keypad setup
const byte ROW_NUM = 4;    // four rows
const byte COLUMN_NUM = 4; // four columns

char keys[ROW_NUM][COLUMN_NUM] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

byte pin_rows[ROW_NUM] = {
    8,
    7,
    6,
    5}; // connect to the row pinouts of the keypad
byte pin_column[COLUMN_NUM] = {
    4,
    3,
    2,
    A1}; // connect to the column pinouts of the keypad

Keypad myKeypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);

// Password setup
const char *correctPassword = "1234";
char enteredPassword[5]; // Maximum password length

// GSM setup
SoftwareSerial gsmSerial(1, 0);    // RX, TX
#define PHONE_NUMBER "+1234567890" // Change to your phone number

bool checkRFID();
bool checkPassword();
void checkKeypad();
void unlockDoor();
void sendSMS(const char *message);

void setup()
{
  Serial.begin(9600);
  gsmSerial.begin(9600);

  SPI.begin();
  mfrc522.PCD_Init();
  pinMode(LED, OUTPUT);
  Serial.println("System ready. Please present RFID card or enter password.");
}

void loop()
{
  if (checkRFID())
  {
    unlockDoor();
    digitalWrite(LED, HIGH);
  }
  else if (checkPassword())
  {
    unlockDoor();
    digitalWrite(LED, HIGH);
  }
  else
  {
    digitalWrite(LED, LOW);
  }
  checkKeypad();

  delay(1000); // Add a delay to prevent continuous attempts
}

bool checkRFID()
{
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial())
  {
    Serial.println("RFID Card Detected");
    return true;
  }
  return false;
}

bool checkPassword()
{
  static byte passwordIndex = 0;
  while (Serial.available() > 0)
  {
    char key = Serial.read();

    if (key == '\n' || key == '\r')
    {
      enteredPassword[passwordIndex] = '\0'; // Null-terminate the password
      Serial.print("Entered Password: ");
      Serial.println(enteredPassword);

      if (strcmp(enteredPassword, correctPassword) == 0)
      {
        Serial.println("Password Correct");
        passwordIndex = 0; // Reset password index
        return true;
      }
      else
      {
        Serial.println("Incorrect Password");
        sendSMS("Unauthorized access attempt!");
        passwordIndex = 0; // Reset password index
        return false;
      }
    }
    else
    {
      enteredPassword[passwordIndex++] = key;
      if (passwordIndex >= sizeof(enteredPassword))
      {
        // Password too long, reset index
        passwordIndex = 0;
        Serial.println("Password too long. Try again.");
      }
    }
  }
  return false;
}

void checkKeypad()
{
  char key = myKeypad.getKey();
  if (key)
  {
    Serial.println("Keypad Key Pressed: " + String(key));
    // Add your keypad logic here
  }
}

void unlockDoor()
{
  Serial.println("Door Unlocked!");
  sendSMS("Door Unlocked!");
  // Add code to control the solenoidal lock or any other mechanism
  delay(5000); // Lock the door after 5 seconds
}

void sendSMS(const char *message)
{
  gsmSerial.println("AT+CMGF=1"); // Set the SMS mode to text
  delay(1000);
  gsmSerial.print("AT+CMGS=\"");
  gsmSerial.print(PHONE_NUMBER);
  gsmSerial.println("\"");
  delay(1000);
  gsmSerial.println(message);
  delay(1000);
  gsmSerial.write(26); // Ctrl+Z to send SMS
  delay(1000);
}
