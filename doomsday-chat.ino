#include <Ssd1306.h>
#include <M5stackCardKB.h>
#include <Steelsquid.h>
#include <FeatherRfm9X.h>

#define SSD1306_WIDTH 128
#define SSD1306_HEIGHT 64
#define MAX_PACKAGE_SIZE 19
#define MATRIX_LENGTH 176
#define FREQUENCY 434.0


Ssd1306 ssd1306 = Ssd1306(SSD1306_WIDTH, SSD1306_HEIGHT);
M5stackCardKB cardKB = M5stackCardKB(MAX_PACKAGE_SIZE);
FeatherRfm9X rfm9X = FeatherRfm9X(FREQUENCY);
bool autoPing = false;


//                0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18   19   20   21
char matrix[] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\n',  /* 0   - 21  */
                 ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\n',  /* 22  - 43  */
                 ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\n',  /* 44  - 65  */
                 ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\n',  /* 66  - 87  */
                 ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\n',  /* 88  - 109 */
                 ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\n',  /* 110 - 131 */
                 ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\n',  /* 132 - 153 */
                 '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', NULL}; /* 154 - 175 */


//##########################################################################################
void setup() {
  Steelsquid:init();
  ssd1306.init();
  rfm9X.init();

  ssd1306.printTextToScreen(matrix);
}


//##########################################################################################
void loop() {
  // Read any keystrokes from keyboard
  cardKB.check();

  // Is any key pressed
  if(cardKB.isKeyPressed()){
    // Get message from keyboard and type on screen
    setTypedText(cardKB.getText());
  }
  
  // Is enter pressed
  if(cardKB.isEnterPressed()){
    // Get and clear message from keyboard
    String message = cardKB.getText();
    cardKB.clearText();

    // Set text SENDING to display
    setStatus("SENDING");

    // Send message over radio
    rfm9X.transmitString(message);

    // Clear status and move message to history
    clearStatus();
    typedTextToHistory();
  }

  // Send PING
  if(cardKB.isDownPressed()){
    // Set text SENDING to display
    setStatus("SENDING");

    // Transmit PING
    rfm9X.transmitRepeterPing();

    // Clear status and print R-PING to history
    clearStatus();
    addToHistory('>', "R-PING");
  }

  // Send PING
  if(cardKB.isTabPressed()){
    // Set text SENDING to display
    setStatus("SENDING");

    // Transmit PING
    rfm9X.transmitPing();

    // Clear status and print PING to history
    clearStatus();
    addToHistory('>', "PING");
  }

  // Start to send ping every 8 seconds
  if(cardKB.isUpPressed()){
    if(autoPing){
      autoPing = false;
    }
    else{
      autoPing = true;
    }
  }
  
  // Check if something to receive
  if(rfm9X.available()){
    // Set text RECEIVING to display
    setStatus("RECEIVING");

    // Read the package
    int packageType = rfm9X.receive();

    // Clear status on display
    clearStatus();
        
    // Receive PING
    if(packageType == TYPE_PING){
      // Print PING to history and set rssi
      addToHistory('<', "PING");

      // Set text SENDING to display
      setStatus("SENDING");

      // Transmit PONG
      byte bytes[1];
      bytes[0] = FeatherRfm9X::batteryPercent();
      rfm9X.transmitPong(bytes, 1);

      // Clear status and print PONG to history
      clearStatus();
      addToHistory('>', "PONG");
    }

    // Receive REPETER PONG
    if(packageType == TYPE_REPETER_PONG){
      // Print R-PONG to history and set rssi
      addToHistory('<', "R-PONG (" + String(rfm9X.receivedByte()) + "%)");
    }

    // Receive PONG
    if(packageType == TYPE_PONG){
      // Print PONG to history and set rssi
      addToHistory('<', "PONG (" + String(rfm9X.receivedByte()) + "%)");
    }

    // Receive ACK
    if(packageType == TYPE_ACK){
      // Print ACK to history and set rssi
      addToHistory('<', "ACK");
    }

    // Receive NAK
    if(packageType == TYPE_NAK){
      // Print NAK to history and set rssi
      addToHistory('<', "NAK");
    }

    // Receive message
    if(packageType == TYPE_STRING){
      // Print message to history and set rssi
      addToHistory('<', rfm9X.receivedString());

      // Set text SENDING to display
      setStatus("SENDING");

      // Transmit PONG
      rfm9X.transmitAck();

      // Clear status and print ACK to history
      clearStatus();
      addToHistory('>', "ACK");
    }

    setRssiAndSnr();
  }
  
  // Print battery percent every 2 seconds
  if(Steelsquid::executeEvery(0, 2000)){
    setBatteryPercent(FeatherRfm9X::batteryPercent());  
  }

  // Send ping every 8 seconds
  if(autoPing && Steelsquid::executeEvery(1, 8000)){
    // Set text SENDING to display
    setStatus("SENDING");

    // Transmit PING
    rfm9X.transmitPing();

    // Clear status and print PING to history
    clearStatus();
    addToHistory('>', "PING");
  }

}


//##########################################################################################
void scrollHistory() {
  for(int i=0; i<=MAX_PACKAGE_SIZE; i++){
    matrix[i+22] = matrix[i+44];
    matrix[i+44] = matrix[i+66];
    matrix[i+66] = matrix[i+88];
    matrix[i+88] = matrix[i+110];
    matrix[i+110] = matrix[i+132];
  }  
}


//##########################################################################################
void setTypedText(String text) {
  for(int i=0; i<MAX_PACKAGE_SIZE; i++){
    if(i < text.length()){
      char c = text.charAt(i);
      if(i==0){
        if(c >= 'a' && c <= 'z') c += 'A' - 'a';
      }
      matrix[i+156] = c;
    }
    else{
      matrix[i+156] = ' ';
    }
  }
  ssd1306.printTextToScreen(matrix);
}


//##########################################################################################
void typedTextToHistory() {
  scrollHistory();
  matrix[132] = '>';
  matrix[133] = ' ';
  for(int i=0; i<=MAX_PACKAGE_SIZE-1; i++){
    matrix[i+134] = matrix[i+156];
    matrix[i+156] = ' ';
  }
  ssd1306.printTextToScreen(matrix);  
}


//##########################################################################################
void addToHistory(char inOrOut, String text) {
  scrollHistory();
  matrix[132] = inOrOut;
  matrix[133] = ' ';
  for(int i=0; i<MAX_PACKAGE_SIZE; i++){
    if(i < text.length()){
      char c = text.charAt(i);
      if(i==0){
        if(c >= 'a' && c <= 'z') c += 'A' - 'a';
      }
      matrix[i+134] = c;
    }
    else{
      matrix[i+134] = ' ';
    }
  }
  ssd1306.printTextToScreen(matrix);  
}


//##########################################################################################
void setBatteryPercent(int i) {
  if(i<10){
    char buffer[1];
    dtostrf(i, 1, 0, buffer);  
    matrix[17] = ' ';
    matrix[18] = ' ';
    matrix[19] = 'buffer[0]';
    matrix[20] = '%';
  }
  else if(i<100){
    char buffer[2];
    dtostrf(i, 2, 0, buffer);  
    matrix[17] = ' ';
    matrix[18] = buffer[0];
    matrix[19] = buffer[1];
    matrix[20] = '%';
  }
  else{
    matrix[17] = '1';
    matrix[18] = '0';
    matrix[19] = '0';
    matrix[20] = '%';
  }
  ssd1306.printTextToScreen(matrix);  
}


//##########################################################################################
void setRssiAndSnr() {
  int rssi = rfm9X.lastRssi();
  if(rssi < -99){
    char buffer[4];
    dtostrf(rssi, 4, 0, buffer);  
    matrix[0] = buffer[0];
    matrix[1] = buffer[1];
    matrix[2] = buffer[2];
    matrix[3] = buffer[3];
  }
  else if(rssi<-9){
    char buffer[3];
    dtostrf(rssi, 3, 0, buffer);  
    matrix[0] = buffer[0];
    matrix[1] = buffer[1];
    matrix[2] = buffer[2];
    matrix[3] = ' ';
  }
  else if(rssi<0){
    char buffer[2];
    dtostrf(rssi, 2, 0, buffer);  
    matrix[0] = buffer[0];
    matrix[1] = buffer[1];
    matrix[2] = ' ';
    matrix[3] = ' ';
  }
  else if(rssi<10){
    char buffer[1];
    dtostrf(rssi, 1, 0, buffer);  
    matrix[0] = buffer[0];
    matrix[1] = ' ';
    matrix[2] = ' ';
    matrix[3] = ' ';
  }
  else{
    char buffer[2];
    dtostrf(rssi, 2, 0, buffer);  
    matrix[0] = buffer[0];
    matrix[1] = buffer[1];
    matrix[2] = ' ';
    matrix[3] = ' ';
  }

  int snr = rfm9X.lastSnr();
  if(snr < -99){
    char buffer[4];
    dtostrf(snr, 4, 0, buffer);  
    matrix[9] = buffer[0];
    matrix[10] = buffer[1];
    matrix[11] = buffer[2];
    matrix[12] = buffer[3];
  }
  else if(snr<-9){
    char buffer[3];
    dtostrf(snr, 3, 0, buffer);  
    matrix[9] = buffer[0];
    matrix[10] = buffer[1];
    matrix[11] = buffer[2];
    matrix[12] = ' ';
  }
  else if(snr<0){
    char buffer[2];
    dtostrf(snr, 2, 0, buffer);  
    matrix[9] = buffer[0];
    matrix[10] = buffer[1];
    matrix[11] = ' ';
    matrix[12] = ' ';
  }
  else if(snr<10){
    char buffer[1];
    dtostrf(snr, 1, 0, buffer);  
    matrix[9] = buffer[0];
    matrix[10] = ' ';
    matrix[11] = ' ';
    matrix[12] = ' ';
  }
  else{
    char buffer[2];
    dtostrf(snr, 2, 0, buffer);  
    matrix[9] = buffer[0];
    matrix[10] = buffer[1];
    matrix[11] = ' ';
    matrix[12] = ' ';
  }

  ssd1306.printTextToScreen(matrix);
}


//##########################################################################################
void clearStatus() {
  matrix[6] = ' ';
  matrix[7] = ' ';
  matrix[8] = ' ';
  matrix[9] = ' ';
  matrix[10] = ' ';
  matrix[11] = ' ';
  matrix[12] = ' ';
  matrix[13] = ' ';
  matrix[14] = ' ';
  ssd1306.printTextToScreen(matrix);  
  setRssiAndSnr();  
}


//##########################################################################################
void setStatus(String status) {
  clearStatus();
  if(status.length() >= 9){
    matrix[6] = status.charAt(0);
    matrix[7] = status.charAt(1);
    matrix[8] = status.charAt(2);
    matrix[9] = status.charAt(3);
    matrix[10] = status.charAt(4);
    matrix[11] = status.charAt(5);
    matrix[12] = status.charAt(6);
    matrix[13] = status.charAt(7);
    matrix[14] = status.charAt(8);
  }
  else if(status.length() == 8){
    matrix[6] = status.charAt(0);
    matrix[7] = status.charAt(1);
    matrix[8] = status.charAt(2);
    matrix[9] = status.charAt(3);
    matrix[10] = status.charAt(4);
    matrix[11] = status.charAt(5);
    matrix[12] = status.charAt(6);
    matrix[13] = status.charAt(7);
  }
  else if(status.length() == 7){
    matrix[7] = status.charAt(0);
    matrix[8] = status.charAt(1);
    matrix[9] = status.charAt(2);
    matrix[10] = status.charAt(3);
    matrix[11] = status.charAt(4);
    matrix[12] = status.charAt(5);
    matrix[13] = status.charAt(6);
  }
  else if(status.length() == 6){
    matrix[7] = status.charAt(0);
    matrix[8] = status.charAt(1);
    matrix[9] = status.charAt(2);
    matrix[10] = status.charAt(3);
    matrix[11] = status.charAt(4);
    matrix[12] = status.charAt(5);
  }
  else if(status.length() == 5){
    matrix[8] = status.charAt(0);
    matrix[9] = status.charAt(1);
    matrix[10] = status.charAt(2);
    matrix[11] = status.charAt(3);
    matrix[12] = status.charAt(4);
  }
  else if(status.length() == 4){
    matrix[8] = status.charAt(0);
    matrix[9] = status.charAt(1);
    matrix[10] = status.charAt(2);
    matrix[11] = status.charAt(3);
  }
  else if(status.length() == 3){
    matrix[9] = status.charAt(0);
    matrix[10] = status.charAt(1);
    matrix[11] = status.charAt(2);
  }
  else if(status.length() == 2){
    matrix[9] = status.charAt(0);
    matrix[10] = status.charAt(1);
  }
  else if(status.length() == 1){
    matrix[10] = status.charAt(0);
  }
  ssd1306.printTextToScreen(matrix);  
}
