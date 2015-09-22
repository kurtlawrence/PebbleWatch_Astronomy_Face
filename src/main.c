#include <pebble.h>
enum {
	APP_VERSION = 6
};		//Define the version number
enum {
  LAT_STORED = 0,
  LNG_STORED = 1,
  RISE0_STORED = 2,
  RISE1_STORED = 3,
  RISE2_STORED = 4,
  RISE3_STORED = 5,
  RISE4_STORED = 6,
  RISE5_STORED = 7,
  RISE6_STORED = 8,
  UTC_STORED = 9,
  LAST_STORED = 10,
  LDATE_STORED = 11,
	VERSCREEN_STORED = 12
};    //Keys for local persistent storage
  
static Window *s_main_window;      // Initialize the watch face window pointer
static Window *s_settings_window;		//Declare the settings window
static Window *s_version_window;		//Declare the new version window
static TextLayer *s_time_layer;    // Initialize the time value text layer pointer
static TextLayer *s_date_layer1;    // Initialise the date value text layer pointer
static TextLayer *s_date_layer2;    // Initialise the second date value text layer pointer
static TextLayer *s_compassBearing_layer;    // Initialise the compass bearing layer
static TextLayer *s_compassHeading_layer;    // Initialise the compass heading (N,NW etc) layer
static TextLayer *s_battery_layer;    // Initialise the watch battery layer
static TextLayer *s_moonRise_layer;      // Initialise the moon rise text layer
static TextLayer *s_moonPhase_layer;      // Initialise the moon phase name text layer
static TextLayer *s_sunRise_layer;        //Initialise the sun rise text layer
static TextLayer *s_sunSet_layer;        //Initialise the sun set text layer
static BitmapLayer *s_sunIcon_layer;  //Declare the sunIcon layer
static GBitmap *s_sunIcon_bitmap;      //Declare the sunIcon bitmap
static BitmapLayer *s_moonIcon_layer;    //Declare the moonIcon layer
static GBitmap *s_moonIcon_bitmap;			//Declare the moonIcon bitmap
static TextLayer *s_UTC_layer;        //Declare UTC text layer
static TextLayer *s_Coords_layer;      //Declare the lat and long data text layer
static TextLayer *s_degreeSymbol_layer;  //Declare the degree symbol layer
static BitmapLayer *s_battName_layer;      //Declare the battery logo
static GBitmap *s_battIcon_bitmap;      //Declare the battery icon bitmap
static TextLayer *s_settingsTop_layer;		//Declare the settings top text layer
static TextLayer *s_settingsBottom_layer;		//Declare the settings bottow text layer
static TextLayer *s_version_layer;			//Declare the text layer shown for screen update
static TextLayer *s_AMorPM_layer;				//Declare the text layer showing whether it is AM or PM
static GBitmap *s_btIconOn_bitmap;			// Declare the bitmap that will hold the bluetooth icon in memory
static GBitmap *s_btIconOff_bitmap;
static BitmapLayer *s_btIcon_layer;		// Declare the layer that will house the bluetooth icon image
static BitmapLayer *s_compassType_layer;		// Declare the layer that will house the compass type (true or magnetic north)
static GBitmap *s_compassTrue_bitmap;
static GBitmap *s_compassMag_bitmap;
static GBitmap *s_compassNone_bitmap;

//Load and unload main window (main display)
static void main_window_load(Window *window) {}
static void main_window_unload(Window *window) {}

//Load and unload settings window (compass and gps options)
static void settings_window_load(Window *window) {
	// Create Top settings TextLayer --> Create21
  s_settingsTop_layer = text_layer_create(GRect(0, 21, 144, 63));
  text_layer_set_background_color(s_settingsTop_layer, GColorBlack);
  text_layer_set_text_color(s_settingsTop_layer, GColorWhite);
  text_layer_set_text_alignment(s_settingsTop_layer, GTextAlignmentCenter);
	text_layer_set_overflow_mode(s_settingsTop_layer, GTextOverflowModeWordWrap);
  text_layer_set_font(s_settingsTop_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
	
	// Create Top settings TextLayer --> Create22
  s_settingsBottom_layer = text_layer_create(GRect(0, 95, 144, 63));
  text_layer_set_background_color(s_settingsBottom_layer, GColorBlack);
  text_layer_set_text_color(s_settingsBottom_layer, GColorWhite);
  text_layer_set_text_alignment(s_settingsBottom_layer, GTextAlignmentCenter);
	text_layer_set_overflow_mode(s_settingsBottom_layer, GTextOverflowModeWordWrap);
  text_layer_set_font(s_settingsBottom_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));	
	
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_settingsTop_layer));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_settingsBottom_layer));
	
	//Add the text
	text_layer_set_text(s_settingsTop_layer, "{SHAKE}\nto force update GPS");
	text_layer_set_text(s_settingsBottom_layer, "<-- TAP SIDE -->\nto toggle compass");
}
static void settings_window_unload(Window *window) {
	text_layer_destroy(s_settingsTop_layer);
	text_layer_destroy(s_settingsBottom_layer);
}

//Load and unload for new version window
static void version_window_load(Window *window) {
	// Create Top settings TextLayer --> Create20
  s_version_layer = text_layer_create(GRect(0, 0, 144, 168));
  text_layer_set_background_color(s_version_layer, GColorBlack);
  text_layer_set_text_color(s_version_layer, GColorWhite);
  text_layer_set_text_alignment(s_version_layer, GTextAlignmentCenter);
	text_layer_set_overflow_mode(s_version_layer, GTextOverflowModeWordWrap);
  text_layer_set_font(s_version_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
	
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_version_layer));
	
	text_layer_set_text(s_version_layer, "Welcome to v2.2\nBuild 6\nRefer to changelog for updates\nShake 3 times within a minute to active settings screen\nPlease be patient with gesture recognition");
}
static void version_window_unload(Window *window) {
	text_layer_destroy(s_version_layer);		//Destroy text layer -->Create23
}

static int settingsScreenShown;
static int compassIsDisplayed;
static int shakeCount;
#include <timeModule.h>    // Time module installed
#include <compassModule.h>  //Compass module installed
#include <tapService.h>			//Tap service handling module installed
#include <btModule.h>				//Bluetooth service handling module installed
static void init() {
	//Create the three windows
  s_main_window = window_create();																			// Create main Window element and assign to pointer --> Create1
	window_set_background_color(s_main_window, GColorBlack);							// Set the background color to black
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });			// Set handlers to manage the elements inside the Window
	s_settings_window = window_create();																	// Create the settings window element --> Create2
	window_set_background_color(s_settings_window, GColorBlack);					// Set the background color to black
	window_set_window_handlers(s_settings_window, (WindowHandlers) {
    .load = settings_window_load,
    .unload = settings_window_unload
  });	// Set handlers for settings window
	s_version_window = window_create();																		// Create the new version window element --> Create3
	window_set_window_handlers(s_version_window, (WindowHandlers) {
    .load = version_window_load,
    .unload = version_window_unload
  });	// Set handlers for new version window
	
	// Create the main display text layers, these need to persist across windows (essentially runnning in background)
	// Create time TextLayer --> Create4
  s_time_layer = text_layer_create(GRect(0, 0, 144, 44));
  text_layer_set_background_color(s_time_layer, GColorBlack);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  // Create date1 TextLayer --> Create5
  s_date_layer1 = text_layer_create(GRect(0, 44, 144, 21));
  text_layer_set_background_color(s_date_layer1, GColorBlack);
  text_layer_set_text_color(s_date_layer1, GColorWhite);
  text_layer_set_text_alignment(s_date_layer1, GTextAlignmentCenter);
  text_layer_set_font(s_date_layer1, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  // Create date2 TextLayer --> Create6
  s_date_layer2 = text_layer_create(GRect(0, 65, 144, 16));
  text_layer_set_background_color(s_date_layer2, GColorBlack);
  text_layer_set_text_color(s_date_layer2, GColorWhite);
  text_layer_set_text_alignment(s_date_layer2, GTextAlignmentCenter);
  text_layer_set_font(s_date_layer2, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  // Create compass1 TextLayer --> Create7
  s_compassBearing_layer = text_layer_create(GRect(0, 110, 40, 28));
  text_layer_set_background_color(s_compassBearing_layer, GColorBlack);
  text_layer_set_text_color(s_compassBearing_layer, GColorWhite);
  text_layer_set_text_alignment(s_compassBearing_layer, GTextAlignmentRight);
  text_layer_set_font(s_compassBearing_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  // Create compass heading TextLayer --> Create8
  s_compassHeading_layer = text_layer_create(GRect(47, 110, 32, 28));
  text_layer_set_background_color(s_compassHeading_layer, GColorBlack);
  text_layer_set_text_color(s_compassHeading_layer, GColorWhite);
  text_layer_set_text_alignment(s_compassHeading_layer, GTextAlignmentLeft);
  text_layer_set_font(s_compassHeading_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  // Create battery TextLayer --> Create9
  s_battery_layer = text_layer_create(GRect(96, 110, 54, 24));
  text_layer_set_background_color(s_battery_layer, GColorBlack);
  text_layer_set_text_color(s_battery_layer, GColorWhite);
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentLeft);
  text_layer_set_font(s_battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  // Create moonRise TextLayer --> Create10
  s_moonRise_layer = text_layer_create(GRect(92, 99, 52, 14));
  text_layer_set_background_color(s_moonRise_layer, GColorClear);
  text_layer_set_text_color(s_moonRise_layer, GColorWhite);
  text_layer_set_text_alignment(s_moonRise_layer, GTextAlignmentCenter);
  text_layer_set_font(s_moonRise_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  // Create moonPhase TextLayer --> Create11
  s_moonPhase_layer = text_layer_create(GRect(92, 83, 52, 16));
  text_layer_set_background_color(s_moonPhase_layer, GColorClear);
  text_layer_set_text_color(s_moonPhase_layer, GColorWhite);
  text_layer_set_text_alignment(s_moonPhase_layer, GTextAlignmentCenter);
  text_layer_set_font(s_moonPhase_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  // Create sunRise TextLayer --> Create12
  s_sunRise_layer = text_layer_create(GRect(30, 99, 28, 14));
  text_layer_set_background_color(s_sunRise_layer, GColorBlack);
  text_layer_set_text_color(s_sunRise_layer, GColorWhite);
  text_layer_set_text_alignment(s_sunRise_layer, GTextAlignmentCenter);
  text_layer_set_font(s_sunRise_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  // Create sunSet TextLayer --> Create13
  s_sunSet_layer = text_layer_create(GRect(30, 83, 28, 14));
  text_layer_set_background_color(s_sunSet_layer, GColorClear);
  text_layer_set_text_color(s_sunSet_layer, GColorWhite);
  text_layer_set_text_alignment(s_sunSet_layer, GTextAlignmentCenter);
  text_layer_set_font(s_sunSet_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  // Create sunIcon bitmap layer --> Create 14
  s_sunIcon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SunRiseSet_icon);
  s_sunIcon_layer = bitmap_layer_create(GRect(0, 85, 28, 28));
  bitmap_layer_set_bitmap(s_sunIcon_layer, s_sunIcon_bitmap);
  // Create moonIcon bitmap layer --> Create 15
  s_moonIcon_layer = bitmap_layer_create(GRect(62, 85, 28, 28));      //Only create layer, the bitmap gets created later
	s_moonIcon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_NewMoonSH_icon);		// Create a default bitmap such that it can be destroyed upon bitmap update
  // Create UTC TextLayer --> Create16
  s_UTC_layer = text_layer_create(GRect(0, 138, 144, 16));
  text_layer_set_background_color(s_UTC_layer, GColorClear);
  text_layer_set_text_color(s_UTC_layer, GColorWhite);
  text_layer_set_text_alignment(s_UTC_layer, GTextAlignmentCenter);
  text_layer_set_font(s_UTC_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  // Create GPS TextLayer --> Create17
  s_Coords_layer = text_layer_create(GRect(0, 152, 144, 16));
  text_layer_set_background_color(s_Coords_layer, GColorClear);
  text_layer_set_text_color(s_Coords_layer, GColorWhite);
  text_layer_set_text_alignment(s_Coords_layer, GTextAlignmentCenter);
  text_layer_set_font(s_Coords_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  //Create the degree symbol text layer --> Create18
  s_degreeSymbol_layer = text_layer_create(GRect(40,110,8,16));
  text_layer_set_background_color(s_degreeSymbol_layer, GColorClear);
  text_layer_set_text_color(s_degreeSymbol_layer, GColorWhite);
  text_layer_set_text_alignment(s_degreeSymbol_layer, GTextAlignmentLeft);
  text_layer_set_font(s_degreeSymbol_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  //Create the battery name text layer --> Create19
  s_battIcon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_Battery_icon);
  s_battName_layer = bitmap_layer_create(GRect(84, 118, 10, 20));
  bitmap_layer_set_bitmap(s_battName_layer, s_battIcon_bitmap);
	//Create the am or pm text layer --> Create23
  s_AMorPM_layer = text_layer_create(GRect(126,10,18,40));
  text_layer_set_background_color(s_AMorPM_layer, GColorClear);
  text_layer_set_text_color(s_AMorPM_layer, GColorWhite);
  text_layer_set_text_alignment(s_AMorPM_layer, GTextAlignmentLeft);
	text_layer_set_overflow_mode(s_AMorPM_layer, GTextOverflowModeWordWrap);
  text_layer_set_font(s_AMorPM_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
	//Create the bluetooth layer --> Create24
	s_btIcon_layer = bitmap_layer_create(GRect(125, 69, 12, 11));
	s_btIconOn_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BT_ICON_ON);
	s_btIconOff_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BT_ICON_OFF);
	if (bluetooth_connection_service_peek()) {		// Peek the current bluetooth status and set the icon to what it is!
		bitmap_layer_set_bitmap(s_btIcon_layer, s_btIconOn_bitmap);
	} else {
		bitmap_layer_set_bitmap(s_btIcon_layer, s_btIconOff_bitmap);
	}
	
	//Create the compass type layer layer --> Create25
	s_compassType_layer = bitmap_layer_create(GRect(0, 119, 3, 20));
	s_compassNone_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CompassOffIcon);
	s_compassTrue_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CompassTrueIcon);
	s_compassMag_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CompassMagIcon);
	bitmap_layer_set_bitmap(s_compassType_layer, s_compassNone_bitmap);
	
  //Set the degree symbol now
  static char degSymbol[] = "o";
  text_layer_set_text(s_degreeSymbol_layer, degSymbol);
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(s_main_window), text_layer_get_layer(s_time_layer));
  layer_add_child(window_get_root_layer(s_main_window), text_layer_get_layer(s_date_layer1));
  layer_add_child(window_get_root_layer(s_main_window), text_layer_get_layer(s_date_layer2));
  layer_add_child(window_get_root_layer(s_main_window), text_layer_get_layer(s_compassBearing_layer));
  layer_add_child(window_get_root_layer(s_main_window), text_layer_get_layer(s_compassHeading_layer));
  layer_add_child(window_get_root_layer(s_main_window), text_layer_get_layer(s_battery_layer));
  layer_add_child(window_get_root_layer(s_main_window), text_layer_get_layer(s_moonRise_layer));
  layer_add_child(window_get_root_layer(s_main_window), text_layer_get_layer(s_moonPhase_layer));
  layer_add_child(window_get_root_layer(s_main_window), text_layer_get_layer(s_sunRise_layer));
  layer_add_child(window_get_root_layer(s_main_window), text_layer_get_layer(s_sunSet_layer));
  layer_add_child(window_get_root_layer(s_main_window), bitmap_layer_get_layer(s_sunIcon_layer));
  layer_add_child(window_get_root_layer(s_main_window), bitmap_layer_get_layer(s_moonIcon_layer));
  layer_add_child(window_get_root_layer(s_main_window), text_layer_get_layer(s_UTC_layer));
  layer_add_child(window_get_root_layer(s_main_window), text_layer_get_layer(s_Coords_layer));
  layer_add_child(window_get_root_layer(s_main_window), text_layer_get_layer(s_degreeSymbol_layer));
  layer_add_child(window_get_root_layer(s_main_window), bitmap_layer_get_layer(s_battName_layer));
	layer_add_child(window_get_root_layer(s_main_window), text_layer_get_layer(s_AMorPM_layer));
	layer_add_child(window_get_root_layer(s_main_window), bitmap_layer_get_layer(s_btIcon_layer));
	layer_add_child(window_get_root_layer(s_main_window), bitmap_layer_get_layer(s_compassType_layer));
	
	window_stack_push(s_main_window, true);		//Put the main display at the bottom of the stack
	
	settingsScreenShown = 0;
	compassIsDisplayed = 0;		//Turn compass off when initialised
	
	compassModule_deinit();       //Initialise compassModule
	btModule_init();					//Initialise btModule
  batteryModule_init();       //Initialise batteryModule
  moonModule_init();          //Initialise moonModule
	tapService_init();			//Initialise the tap service module, could possibly call compassModule functions
  timeModule_init();    	//Initialise timeModule, now can possibly call moonModule and batteryModule functions
  
	
	//Display the appropiate window (if new version has not been seen yet show that screen)
	if (persist_read_int(VERSCREEN_STORED) != APP_VERSION) {
		//APP_LOG(APP_LOG_LEVEL_INFO, "Verscreen persist does not equal version number");
		shakeCount = 10;		//High value for flagging purposes
		window_stack_push(s_version_window, true);			//Display the welcome screen to the new version
	} else {
		//APP_LOG(APP_LOG_LEVEL_INFO, "Verscreen persist is equal to the current app version");
		shakeCount = 0;
	}
}
static void deinit() {
  // Call the add-on deinits if available, do this before destroying the main window
	tapService_deinit();				//Deinit tap service, stops possible calls to moonModule, compassModule
	timeModule_deinit();        //Deinit the time module, stops possible calls to moonModule, batteryModule
  moonModule_deinit();        //Deinit the moonModule
  batteryModule_deinit();    // Deinit the battery module
	btModule_deinit();					// Deinit the bluetooth module
  compassModule_deinit();    // Deinit the compass module
	// After all the deinits for each module, there should be no interruptions from services that can interfere with the destruction of text layers/windows
	
	// Destroy the persistent text layers
	bitmap_layer_destroy(s_compassType_layer);		// Destroy the compass type layer --> Create25
		gbitmap_destroy(s_compassNone_bitmap);
		gbitmap_destroy(s_compassTrue_bitmap);
		gbitmap_destroy(s_compassMag_bitmap);
	bitmap_layer_destroy(s_btIcon_layer);			//Destroy the bt icon layer --> Create24
		gbitmap_destroy(s_btIconOn_bitmap);
		gbitmap_destroy(s_btIconOff_bitmap);
	text_layer_destroy(s_AMorPM_layer);				//Destroy the am or pm text layer --> Create23
		gbitmap_destroy(s_battIcon_bitmap);    //Destroy the battery icon bitmap
	bitmap_layer_destroy(s_battName_layer);    //Destroy the battery icon layer --> Create19
	text_layer_destroy(s_degreeSymbol_layer);    //Destroy the degree symbol layer --> Create18
	text_layer_destroy(s_Coords_layer);      //Destroy the GPS layer --> Create17
	text_layer_destroy(s_UTC_layer);        //Destroy the UTC text layer --> Create16
		gbitmap_destroy(s_moonIcon_bitmap);				//Destroy the moonIcon bitmap
	bitmap_layer_destroy(s_moonIcon_layer);      //Destroy the moonIcon bitmap layer --> Create15
	  gbitmap_destroy(s_sunIcon_bitmap);        //Destroy the sunIcon bitmap
	bitmap_layer_destroy(s_sunIcon_layer);      //Destroy the sunIcon bitmap layer --> Create14
	text_layer_destroy(s_sunSet_layer);      //Destroy the sunSet text layer --> Create13
	text_layer_destroy(s_sunRise_layer);    //Destroy the sunRise text layer --> Create12
	text_layer_destroy(s_moonPhase_layer);      //Destroy the moon phase text layer --> Create11
	text_layer_destroy(s_moonRise_layer);      //Destroy the moon rise text layer --> Create10
	text_layer_destroy(s_battery_layer);      //Destroy the battery1 text layer --> Create9
	text_layer_destroy(s_compassHeading_layer);      //Destroy the compass2 text layer --> Create8
	text_layer_destroy(s_compassBearing_layer);      //Destroy the compass1 text layer --> Create7
	text_layer_destroy(s_date_layer2);        //Destroy the date2 text layer --> Create6
	text_layer_destroy(s_date_layer1);        //Destroy the date1 text layer --> Create5
	text_layer_destroy(s_time_layer);          // Destroy the text layer showing the time --> Create4
	
  // Destroy the windows
	window_destroy(s_version_window);		//Destroy the version window --> Create3
  window_destroy(s_settings_window);	//Destroy the settings window --> Create2
  window_destroy(s_main_window);  		// Destroy the  main window create --> Create1
}
int main(void) {
  // Main function loop
  init();    // First run the init function
  app_event_loop();    // Loop
  deinit();  // Close off the watch face
}