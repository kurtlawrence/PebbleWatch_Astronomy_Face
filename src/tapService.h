// This module captures the tap service, taps control different windows and logic for upper screen tap or lower screen tap
static void tap_handler(AccelAxisType axis, int32_t direction) {
	//This logic distinguishes between tap events
	switch (axis) {
		case ACCEL_AXIS_X:
			if (settingsScreenShown == 1) {
				//A tap in the x direction signifies to turn off compass
				settingsScreenShown = 0;								//Settings screen no longer displayed
				window_stack_pop(true);		//Switch to the main screen
				if (compassIsDisplayed == 1) {
					//Compass is displayed, turn off
					compassModule_deinit();
				} else {
					//Compass is not displayed, turn on
					compassModule_init();
				}
			}
			break;
		case ACCEL_AXIS_Y:
			if (settingsScreenShown == 1) {
				//A tap in the y direction signifies to update the GPS
				settingsScreenShown = 0;								//Settings screen no longer displayed
				window_stack_pop(true);		//Switch to the main screen
				update_GPS(0);		//Pass the seed of 0 calls, initilised the calling proceedure
			} else {
				shakeCount++;
			}
			break;
		case ACCEL_AXIS_Z:
			if (settingsScreenShown != 1) {
				shakeCount++;
			}
	}
	
	if (shakeCount == 3) {
		// Taps reached the limit for which to display the settings screen
		//APP_LOG(APP_LOG_LEVEL_INFO, "Shake count equals 2, time to display the settings display");
		shakeCount = 0;
		settingsScreenShown = 1;
		window_stack_push(s_settings_window, true);		//Choose settings window display
	}
	if (shakeCount == 13) {
		// Taps reached the limit for which to exit the initial screen and show the main display
		//APP_LOG(APP_LOG_LEVEL_INFO, "Shake count equals 6, time to display the main display");
		shakeCount = 0;
		persist_write_int(VERSCREEN_STORED, APP_VERSION);		//Keep the fact that the new version screen has been shown in memory
		window_stack_pop(true);			//Switch to the main display			
	}
}
static void tapService_init() {	
	//APP_LOG(APP_LOG_LEVEL_INFO, "Entered tapService_init, about to subscribe to tap handling service");
	//Subscribe to the tap event service
	accel_tap_service_subscribe(tap_handler);
}
static void tapService_deinit() {
	accel_tap_service_unsubscribe();		//Deregister from tap service
}