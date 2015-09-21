// This module adds functionality to display the compass heading
static void update_compass(int bearing, bool isTrueNorth) {  
  // Create a long-lived buffer with date format listed
  static char buffer[] = "000";    // Compass bearing
  
  // First buffer value
  snprintf(buffer, sizeof(buffer), "%d", bearing);
  // Determine the second buffer value
  // Start from NW and work back to N
  if ((bearing <= 337) && (bearing > 292)) {
    text_layer_set_text(s_compassHeading_layer, "NW");
  } else {
  if ((bearing <= 292) && (bearing > 247)) {
    text_layer_set_text(s_compassHeading_layer, "W");
  } else {
  if ((bearing <= 247) && (bearing > 202)) {
    text_layer_set_text(s_compassHeading_layer, "SW");
  } else {
  if ((bearing <= 202) && (bearing > 157)) {
    text_layer_set_text(s_compassHeading_layer, "S");
  } else {
  if ((bearing <= 157) && (bearing > 112)) {
    text_layer_set_text(s_compassHeading_layer, "SE");
  } else {
  if ((bearing <= 112) && (bearing > 67)) {
    text_layer_set_text(s_compassHeading_layer, "E");
  } else {
  if ((bearing <= 67) && (bearing > 22)) {
    text_layer_set_text(s_compassHeading_layer, "NE");
  } else {
    text_layer_set_text(s_compassHeading_layer, "N");
  }}}}}}}
  
  // Display this date on the TextLayer
  text_layer_set_text(s_compassBearing_layer, buffer);
	
	if (isTrueNorth) {
		bitmap_layer_set_bitmap(s_compassType_layer, s_compassTrue_bitmap);		// Set to display true north
	} else {
		bitmap_layer_set_bitmap(s_compassType_layer, s_compassMag_bitmap);		// Set to display magnetic north
	}
}
static void show_calibration_screen() {
  // Create long-lived buffer
  static char buffer[] = "na";
  
  // Display this on the compass textlayer
  text_layer_set_text(s_compassBearing_layer, buffer);
  text_layer_set_text(s_compassHeading_layer, buffer);
}
void compass_callback(CompassHeadingData heading) {
  if (heading.compass_status != CompassStatusDataInvalid) {
    update_compass(360 - TRIGANGLE_TO_DEG((int)heading.true_heading), heading.is_declination_valid);
  } else {
    // Heading not available yet - Show calibration UI to user
    show_calibration_screen();
  }
}
static void compassModule_init() { 
	//APP_LOG(APP_LOG_LEVEL_INFO, "Entered compass module init");
  // Register to the CompassService for heading updates
	compassIsDisplayed = 1;
  compass_service_subscribe(compass_callback);
  compass_service_set_heading_filter(TRIG_MAX_ANGLE / 180);    //Only update the compass if the heading changes more than 2 degrees
}
static void compassModule_deinit() {
  // Unsubscribe from compass service when not on this watch face
	//APP_LOG(APP_LOG_LEVEL_INFO, "Entered compass module deinit");
	compassIsDisplayed = 0;
  compass_service_unsubscribe();
	show_calibration_screen();
	bitmap_layer_set_bitmap(s_compassType_layer, s_compassNone_bitmap);		// Set to display no true or magnetic detail
}