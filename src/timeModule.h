#include <batteryModule.h>    // Battery module requires time module functions
#include <moonModule.h>        // Moon module requires time module functions
// This module adds functionality to display the time and date (one call to TickTimerService)
static TextLayer *timeDisplayLayer;
static TextLayer *dateDisplayLayer;
static TextLayer *dateDisplayLayer2;

static void update_time() {
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
  } else {
    // Use 12 hour format
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }
  strftime(buffer2, sizeof("Day DD mmm yyyy"), "%a %e %b %Y", tick_time);
  strftime(buffer3, sizeof("Day DDD - Week WW"), "Day %j - Week %W", tick_time);
  
  // Display this time on the TextLayer
  text_layer_set_text(timeDisplayLayer, buffer);
  text_layer_set_text(dateDisplayLayer, buffer2);
  text_layer_set_text(dateDisplayLayer2, buffer3);
}

static void time_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();        //Update the time
  
  
  int hourNum = tick_time->tm_hour;
  hourNum = hourNum + 2;    //Adj hour so the mod works nicely
  
  if (hourNum % 4 == 4) {
    // Update every four hours, starting at midnight - adjustment + 4
    update_GPS();
    update_battery();
  }
  if (hourNum % 6 == 4) {
    // Update every six hours, starting at mignight - adjustment + 6
    update_Astronomy();
  }
}

static void timeModule_init(TextLayer *temp, TextLayer *temp2, TextLayer *temp3) {
  // Conform the display layer
  timeDisplayLayer = temp;
  dateDisplayLayer = temp2;
  dateDisplayLayer2 = temp3;
  
  // Run an update on the time to get the latest time
  update_time();
  
  // Register to the TickTimerService for time updates
  tick_timer_service_subscribe(MINUTE_UNIT, time_tick_handler);
}

static void timeModule_deinit() {
  tick_timer_service_unsubscribe();
}