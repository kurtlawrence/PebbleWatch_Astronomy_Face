// This module adds functionality to display the compass heading
static TextLayer *compassDisplayLayer;
static TextLayer *compassDisplayLayer2;

static void update_compass(int bearing) {  
  // Create a long-lived buffer with date format listed
  static char buffer[] = "000";    // Compass bearing
  
  // First buffer value
  snprintf(buffer, sizeof(buffer), "%d", bearing);
  // Determine the second buffer value
  // Start from NW and work back to N
  if ((bearing <= 337) && (bearing > 292)) {
    text_layer_set_text(compassDisplayLayer2, "NW");
  } else {
  if ((bearing <= 292) && (bearing > 247)) {
    text_layer_set_text(compassDisplayLayer2, "W");
  } else {
  if ((bearing <= 247) && (bearing > 202)) {
    text_layer_set_text(compassDisplayLayer2, "SW");
  } else {
  if ((bearing <= 202) && (bearing > 157)) {
    text_layer_set_text(compassDisplayLayer2, "S");
  } else {
  if ((bearing <= 157) && (bearing > 112)) {
    text_layer_set_text(compassDisplayLayer2, "SE");
  } else {
  if ((bearing <= 112) && (bearing > 67)) {
    text_layer_set_text(compassDisplayLayer2, "E");
  } else {
  if ((bearing <= 67) && (bearing > 22)) {
    text_layer_set_text(compassDisplayLayer2, "NE");
  } else {
    text_layer_set_text(compassDisplayLayer2, "N");
  }}}}}}}
  
  // Display this date on the TextLayer
  text_layer_set_text(compassDisplayLayer, buffer);
}

static void show_calibration_screen() {
  // Create long-lived buffer
  static char buffer[] = "na";
  
  // Display this on the compass textlayer
  text_layer_set_text(compassDisplayLayer, buffer);
  text_layer_set_text(compassDisplayLayer2, buffer);
}

void compass_callback(CompassHeadingData heading) {
  if (heading.compass_status != CompassStatusDataInvalid) {
    update_compass(TRIGANGLE_TO_DEG((int)heading.true_heading));
  } else {
    // Heading not available yet - Show calibration UI to user
    show_calibration_screen();
  }
}

static void compassModule_init(TextLayer *temp, TextLayer *temp2) {
  // Conform the display layer
  compassDisplayLayer = temp2;
  compassDisplayLayer2 = temp;
  
  // Register to the CompassService for heading updates
  compass_service_subscribe(compass_callback);
  compass_service_set_heading_filter(TRIG_MAX_ANGLE / 72);    //Only update the compass if the heading changes more than 3 degrees
}

static void compassModule_deinit() {
  // Unsubscribe from compass service when not on this watch face
  compass_service_unsubscribe();
}