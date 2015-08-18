// This module adds functionality to display battery charge information; along with phone battery information if possible
static void update_battery() {
  BatteryChargeState charge_state = battery_state_service_peek();    //Define and return a charge state
  static char buffer[16] = "N/A";    //Declare a long-lived buffer for charging
  
  if (charge_state.is_charging) {
      snprintf(buffer, sizeof(buffer), "+%d%%", charge_state.charge_percent);    //Add the charging icon
  } else {
      snprintf(buffer, sizeof(buffer), "%d%%", charge_state.charge_percent);    //Space padded battery level
  }
  
  // Display the values
  text_layer_set_text(s_battery_layer, buffer);    //Battery level with charge symbol
}

static void batteryModule_init() {
  // Get latest battery update
  update_battery();
  
  // Register to the BatteryService
  battery_state_service_subscribe(update_battery);
}

static void batteryModule_deinit() {
  // Unsubscribe from BatteryService when not on this watch face
  battery_state_service_unsubscribe();
}