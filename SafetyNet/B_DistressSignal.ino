// This code defines how to SEND distress signal from Distressed Boats
#define DISTRESS_SIGNAL_INTERVAL 30000

unsigned long timeLastSignalSent;
DistressResponse lastResponse;
bool receivedResponse;
// Did the user double click while in this state?
bool didError = false;

void DistressSignal_setup(AlertLevel al) {
  // Initialize state variables
  receivedResponse = false;
  currentAlertLevel = al;
  timeLastSignalSent = 0;
  oled.setTextColor(WHITE);

  // Setup button click code
  DistressSignal_setupButtons();

  oled.clearDisplay();
  oled.setCursor(0, 0);
  oled.setTextSize(2);
  oled.println("Sending");
  oled.print("ALERT LVL ");
  oled.println(currentAlertLevel + 1, DEC);
  oled.display();
}

void handleConfirm() {
  // Trigger error message
  didError = true;
}

//LONG PRESS FUNCTIONS
void handleCancel() {
  cancelmessage();
  isRescuer = false;
  changeProgramState(CANCEL_DISTRESS);
}

void DistressSignal_loop() {
  // When did we last broadcast?
  // If more than 30 seconds passed, we can broadcast again
  unsigned long currentTime = millis();
  if (currentTime - timeLastSignalSent >= DISTRESS_SIGNAL_INTERVAL || timeLastSignalSent == 0)
  {
    // Retrieve data to be sent (e.g., GPS, Alert Level, cancelFlag)
    float gpsLat;
    float gpsLong;
    bool cancelFlag = false;
    // Check if GPS data is valid
    if (gps.location.isValid())
    {
      gpsLat = gps.location.lat();
      gpsLong = gps.location.lng();
      Serial.printf("Valid: GPS (%f, %f)\n", gps.location.lat(), gps.location.lng());
    }
    else
    {
      // Scenario when gps has no valid gps data
      gpsLat = 999.0;
      gpsLong = 999.0;
      Serial.printf("Invalid: GPS (%f, %f)\n", gps.location.lat(), gps.location.lng());
    }

    // Broadcast a distress signal
    if (mesh.sendDistressSignal(gpsLat, gpsLong, currentAlertLevel, cancelFlag))
    {
      // Print debugging
      Serial.printf("Distress signal sent at %lu\n", currentTime);
      timeLastSignalSent = currentTime;
    }
  }

  // While we are also in distress, listen for any distress reponse for us, unless distress has been cancelled
  if (mesh.listenForDistressResponse())
  {
    lastResponse = mesh.getResponse();
    receivedResponse = true;

    Serial.println("Received response");
    Serial.printf("Responder Number: %u\n", lastResponse.address);
    Serial.printf("GPS Lat: %f\n", lastResponse.address);
    Serial.printf("GPS Long: %f\n", lastResponse.address);
  }
  showRescueeMenu();
}

void showRescueeMenu() {
  oled.clearDisplay();

  if (!didError) {
    // No error occurred so we can draw the SOS signal
    // Display the beaming circle
    int phase = (millis() / 1000) % 3;
    drawBeamingCircle(phase);
    
    // Continuously print out response if there is one received
    if (receivedResponse) {
      displayLastResponseInfo(lastResponse);
    }
    oled.display();
  }
  else {
    // Show the error message
    oled.setTextSize(1);          // text size
    oled.setTextColor(WHITE);     // text color
    oled.setCursor(0, 10);        // position to display
    //print menu
    oled.println("Long press any button to cancel current distress signal and back to main menu");
    oled.display();
    delay(2000);
    didError = false;
  }
}

// ACS: Moved here from A_DefaultMenu
void cancelmessage() {
  oled.clearDisplay(); // clear display
  delay(100); // wait for initializing
  oled.setTextSize(1);          // text size
  oled.setTextColor(WHITE);     // text color
  oled.setCursor(0, 10);        // position to display
  oled.println("Stopping the distress signal.");
  oled.println("\nReturning to main menu.");
  oled.display();
}

void displayLastResponseInfo(DistressResponse res) {
  // Assumption: Only data of last responder will appear
  oled.setCursor(40, 30);
  oled.setTextSize(1);
  oled.print("RES: ");
  oled.print(res.address, DEC);

  oled.setCursor(35, 40);
  // If both rescuee and rescuer have valid coordinates, display the distance between them
  if (gps.location.isValid() && isValidGps(res.gpsLat, res.gpsLong)) {
    double distanceInMeters = TinyGPSPlus::distanceBetween(
                                gps.location.lat(),
                                gps.location.lng(),
                                res.gpsLat,
                                res.gpsLong);

    // And then, show the distance
    oled.print("DIS: ");
    oled.print(distanceInMeters, 2);
    oled.print("m");
  }
  else {
    oled.print("DIS: N/A");
  }
}

// Draws the beaming circle
#define CIRCLE_CENTER_X 63
#define CIRCLE_CENTER_Y 31
void drawBeamingCircle(int phase)
{
  switch (phase)
  {
    case 2:
      oled.fillCircle(CIRCLE_CENTER_X, CIRCLE_CENTER_Y, 65, WHITE);
      oled.fillCircle(CIRCLE_CENTER_X, CIRCLE_CENTER_Y, 55, BLACK);
    case 1:
      oled.fillCircle(CIRCLE_CENTER_X, CIRCLE_CENTER_Y, 45, WHITE);
      oled.fillCircle(CIRCLE_CENTER_X, CIRCLE_CENTER_Y, 35, BLACK);
  }

  oled.setCursor(40, 10);
  oled.setTextSize(2);
  oled.print("SOS");
  oled.print(currentAlertLevel + 1, DEC);
}

// Setup the button functions for this state
void DistressSignal_setupButtons() {
  button1.attachClick(doNothing);
  button1.attachDoubleClick(handleConfirm);
  button1.attachLongPressStart(handleCancel);

  button2.attachClick(doNothing);
  button2.attachDoubleClick(handleConfirm);
  button2.attachLongPressStart(handleCancel);
  
  button3.attachClick(doNothing);
  button3.attachDoubleClick(handleConfirm);
  button3.attachLongPressStart(handleCancel);
  
  button4.attachClick(doNothing);
  button4.attachDoubleClick(handleConfirm);
  button4.attachLongPressStart(handleCancel);
  
  button5.attachClick(doNothing);
  button5.attachDoubleClick(handleConfirm);
  button5.attachLongPressStart(handleCancel);
}
