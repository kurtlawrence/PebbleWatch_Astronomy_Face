#include <batteryModule.h>    // Battery module requires time module functions
#include <moonModule.h>        // Moon module requires time module functions
// This module adds functionality to display the time and date (one call to TickTimerService)
static void update_time() {
	//APP_LOG(APP_LOG_LEVEL_INFO, "Entered update_time(). Memory: %d", heap_bytes_used());
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  // Create a long-lived buffer
  static char buffer[] = "00:00";
  static char buffer2[] = "Day DD mmm yyyy";
  static char buffer3[] = "Day DDD - Week WW";
  
  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
		text_layer_set_text(s_AMorPM_layer, "");			//Empty the AM or PM text layer
  } else {
    // Use 12 hour format
		
    strftime(buffer, sizeof("00:00"), "%l:%M", tick_time);
		if (tick_time->tm_hour >= 12) {
			text_layer_set_text(s_AMorPM_layer, "\nPM");
		} else {
			text_layer_set_text(s_AMorPM_layer, "AM");
		}
  }
  strftime(buffer2, sizeof("Day DD mmm yyyy"), "%a %e %b %Y", tick_time);
  strftime(buffer3, sizeof("Day DDD - Week WW"), "Day %j - Week %W", tick_time);
  
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
  text_layer_set_text(s_date_layer1, buffer2);
  text_layer_set_text(s_date_layer2, buffer3);
}
static void time_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();        //Update the time
	//Reset the tap count if off new version screen
	if (shakeCount < 10) {
		shakeCount = 0;  
	} else {
		shakeCount = 10;		//Reset to the initial new version flag
	}
  
  int hourNum = tick_time->tm_hour;
	int minNum = tick_time->tm_min;
  hourNum = hourNum + 7;    //Adj hour so the mod works nicely
	  
	if (hourNum % 12 == 0 && minNum == 0) {
    // Update every four hours, starting at midnight - adjustment + 4
    update_GPS(0);			//Passed seed value of zero, initilised calling
    update_battery();
  }
}
static void timeModule_init() { 
	//APP_LOG(APP_LOG_LEVEL_INFO, "Entered timeModule_init");
  // Run an update on the time to get the latest time
  update_time();  
  // Register to the TickTimerService for time updates
  tick_timer_service_subscribe(MINUTE_UNIT, time_tick_handler);
}
static void timeModule_deinit() {
  tick_timer_service_unsubscribe();
}