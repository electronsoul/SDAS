#include <HardwareSerial.h>

HardwareSerial DisplayUart(PC11, PC10);
byte Forward[10] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5A, 0x01, 0x00, 0x04};

void setup() {
  Serial.setTx(PB6);
  Serial.setRx(PB7);
  Serial.begin(115200);

  DisplayUart.begin(115200);
  delay(1000);

  for (int i = 0; i < 4; i++) {
    Serial.write(Forward, sizeof(Forward));
    DisplayUart.write(Forward, sizeof(Forward));
    Forward[9]++;
    delay(100); // Delay between each value for better readability
  }
}

void loop() {
  // Check if a line of text is available to read from serial port
  if (DisplayUart.available()) {
    // Read the line of text
    String input = DisplayUart.readString();

    // Print the received line back to the serial monitor
    Serial.print("Received: ");
    Serial.println(input);
  }
}
