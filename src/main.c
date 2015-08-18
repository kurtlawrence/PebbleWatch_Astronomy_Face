#include <pebble.h>
  
static Window *s_main_window;      // Initialize the watch face window pointer
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
static TextLayer *s_background_layer;    //Declare the background layer
static TextLayer *s_degreeSymbol_layer;  //Declare the degree symbol layer
static BitmapLayer *s_battName_layer;      //Declare the battery logo
static GBitmap *s_battIcon_bitmap;      //Declare the battery icon bitmap


static void main_window_load(Window *window) {
  // Create the background colour --> Create16
  s_background_layer = text_layer_create(GRect(0,0,144,168));
  text_layer_set_background_color(s_background_layer, GColorBlack);
  
  // Create time TextLayer --> Create2
  s_time_layer = text_layer_create(GRect(0, 0, 144, 44));
  text_layer_set_background_color(s_time_layer, GColorBlack);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  
  // Create date1 TextLayer --> Create3
  s_date_layer1 = text_layer_create(GRect(0, 44, 144, 21));
  text_layer_set_background_color(s_date_layer1, GColorBlack);
  text_layer_set_text_color(s_date_layer1, GColorWhite);
  text_layer_set_text_alignment(s_date_layer1, GTextAlignmentCenter);
  text_layer_set_font(s_date_layer1, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  
  // Create date2 TextLayer --> Create4
  s_date_layer2 = text_layer_create(GRect(0, 65, 144, 16));
  text_layer_set_background_color(s_date_layer2, GColorBlack);
  text_layer_set_text_color(s_date_layer2, GColorWhite);
  text_layer_set_text_alignment(s_date_layer2, GTextAlignmentCenter);
  text_layer_set_font(s_date_layer2, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  
  // Create compass1 TextLayer --> Create5
  s_compassBearing_layer = text_layer_create(GRect(0, 110, 40, 28));
  text_layer_set_background_color(s_compassBearing_layer, GColorBlack);
  text_layer_set_text_color(s_compassBearing_layer, GColorWhite);
  text_layer_set_text_alignment(s_compassBearing_layer, GTextAlignmentRight);
  text_layer_set_font(s_compassBearing_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  
  // Create compass2 TextLayer --> Create6
  s_compassHeading_layer = text_layer_create(GRect(48, 110, 32, 28));
  text_layer_set_background_color(s_compassHeading_layer, GColorBlack);
  text_layer_set_text_color(s_compassHeading_layer, GColorWhite);
  text_layer_set_text_alignment(s_compassHeading_layer, GTextAlignmentLeft);
  text_layer_set_font(s_compassHeading_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  
  // Create battery1 TextLayer --> Create7
  s_battery_layer = text_layer_create(GRect(90, 110, 54, 24));
  text_layer_set_background_color(s_battery_layer, GColorBlack);
  text_layer_set_text_color(s_battery_layer, GColorWhite);
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentLeft);
  text_layer_set_font(s_battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  
  // Create moonRise TextLayer --> Create8
  s_moonRise_layer = text_layer_create(GRect(92, 96, 52, 14));
  text_layer_set_background_color(s_moonRise_layer, GColorClear);
  text_layer_set_text_color(s_moonRise_layer, GColorWhite);
  text_layer_set_text_alignment(s_moonRise_layer, GTextAlignmentCenter);
  text_layer_set_font(s_moonRise_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  
  // Create moonPhase TextLayer --> Create9
  s_moonPhase_layer = text_layer_create(GRect(92, 81, 52, 16));
  text_layer_set_background_color(s_moonPhase_layer, GColorBlack);
  text_layer_set_text_color(s_moonPhase_layer, GColorWhite);
  text_layer_set_text_alignment(s_moonPhase_layer, GTextAlignmentCenter);
  text_layer_set_font(s_moonPhase_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  
  // Create sunRise TextLayer --> Create10
  s_sunRise_layer = text_layer_create(GRect(30, 96, 28, 14));
  text_layer_set_background_color(s_sunRise_layer, GColorBlack);
  text_layer_set_text_color(s_sunRise_layer, GColorWhite);
  text_layer_set_text_alignment(s_sunRise_layer, GTextAlignmentCenter);
  text_layer_set_font(s_sunRise_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  
  // Create sunSet TextLayer --> Create11
  s_sunSet_layer = text_layer_create(GRect(30, 81, 28, 14));
  text_layer_set_background_color(s_sunSet_layer, GColorBlack);
  text_layer_set_text_color(s_sunSet_layer, GColorWhite);
  text_layer_set_text_alignment(s_sunSet_layer, GTextAlignmentCenter);
  text_layer_set_font(s_sunSet_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  
  // Create sunIcon bitmap layer --> Create 12
  s_sunIcon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SunRiseSet_icon);
  s_sunIcon_layer = bitmap_layer_create(GRect(0, 82, 28, 28));
  bitmap_layer_set_bitmap(s_sunIcon_layer, s_sunIcon_bitmap);
  
  // Create moonIcon bitmap layer --> Create 13
  s_moonIcon_layer = bitmap_layer_create(GRect(62, 82, 28, 28));      //Only create layer, the bitmap gets created layer
  
  // Create UTC TextLayer --> Create14
  s_UTC_layer = text_layer_create(GRect(0, 138, 144, 14));
  text_layer_set_background_color(s_UTC_layer, GColorBlack);
  text_layer_set_text_color(s_UTC_layer, GColorWhite);
  text_layer_set_text_alignment(s_UTC_layer, GTextAlignmentCenter);
  text_layer_set_font(s_UTC_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  
  // Create GPS TextLayer --> Create15
  s_Coords_layer = text_layer_create(GRect(0, 152, 144, 16));
  text_layer_set_background_color(s_Coords_layer, GColorBlack);
  text_layer_set_text_color(s_Coords_layer, GColorWhite);
  text_layer_set_text_alignment(s_Coords_layer, GTextAlignmentCenter);
  text_layer_set_font(s_Coords_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  
  //Create the degree symbol text layer --> Create17
  s_degreeSymbol_layer = text_layer_create(GRect(40,110,8,16));
  text_layer_set_background_color(s_degreeSymbol_layer, GColorBlack);
  text_layer_set_text_color(s_degreeSymbol_layer, GColorWhite);
  text_layer_set_text_alignment(s_degreeSymbol_layer, GTextAlignmentLeft);
  text_layer_set_font(s_degreeSymbol_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  
  //Create the battery name text layer --> Create18
  s_battIcon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_Battery_icon);
  s_battName_layer = bitmap_layer_create(GRect(80, 118, 10, 20));
  bitmap_layer_set_bitmap(s_battName_layer, s_battIcon_bitmap);
  
  //Set the degree symbol now
  static char degSymbol[] = "o";
  text_layer_set_text(s_degreeSymbol_layer, degSymbol);
  

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_background_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer1));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer2));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_compassBearing_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_compassHeading_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_battery_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_moonRise_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_moonPhase_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_sunRise_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_sunSet_layer));
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_sunIcon_layer));
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_moonIcon_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_UTC_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_Coords_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_degreeSymbol_layer));
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_battName_layer));
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_time_layer);          // Destroy the text layer showing the time --> Create2
  text_layer_destroy(s_date_layer1);        //Destroy the date1 text layer --> Create3
  text_layer_destroy(s_date_layer2);        //Destroy the date2 text layer --> Create4
  text_layer_destroy(s_compassBearing_layer);      //Destroy the compass1 text layer --> Create5
  text_layer_destroy(s_compassHeading_layer);      //Destroy the compass2 text layer --> Create6
  text_layer_destroy(s_battery_layer);      //Destroy the battery1 text layer --> Create7
  text_layer_destroy(s_moonRise_layer);      //Destroy the battery1 text layer --> Create8
  text_layer_destroy(s_moonPhase_layer);      //Destroy the battery1 text layer --> Create9
  text_layer_destroy(s_sunRise_layer);    //Destroy the sunRise text layer --> Create10
  text_layer_destroy(s_sunSet_layer);      //Destroy the sunSet text layer --> Create11
  bitmap_layer_destroy(s_sunIcon_layer);      //Destroy the sunIcon bitmap layer --> Create12
    gbitmap_destroy(s_sunIcon_bitmap);        //Destroy the sunIcon bitmap
  bitmap_layer_destroy(s_moonIcon_layer);      //Destroy the moonIcon bitmap layer --> Create13    (Bitmap is destroyed in moon module deinit)
		gbitmap_destroy(s_moonIcon_bitmap);				//Destroy the moonIcon bitmap
  text_layer_destroy(s_UTC_layer);        //Destroy the UTC text layer --> Create14
  text_layer_destroy(s_Coords_layer);      //Destroy the GPS layer --> Create15
  text_layer_destroy(s_background_layer);    //Destroy the background color --> Create16
  text_layer_destroy(s_degreeSymbol_layer);    //Destroy the background color --> Create17
  bitmap_layer_destroy(s_battName_layer);    //Destroy the background color --> Create18
    gbitmap_destroy(s_battIcon_bitmap);    //Destroy the battery icon bitmap
}

#include <timeModule.h>    // Time module installed
#include <compassModule.h>  //Compass module installed
static void init() {
  // Create main Window element and assign to pointer --> Create1
  s_main_window = window_create();
  
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  // Show the Window on the watch, with anitmated=true
  window_stack_push(s_main_window, true);
  
  // Initialise the add-on modules
  timeModule_init();    //Initialise timeModule
  compassModule_init();        //Initialise compassModule
  batteryModule_init();                          //Initialise batteryModule
  moonModule_init();                  //Initialise moonModule
}

static void deinit() {
  // Call the add-on deinits if available, do this before destroying the main window
  moonModule_deinit();        //Deinit the moonModule
  batteryModule_deinit();    // Deinit the battery module
  compassModule_deinit();    // Deinit the compass module
  timeModule_deinit();        //Deinit the time module
  
  
  // Destroy the window create --> Create1
  window_destroy(s_main_window);  
}


int main(void) {
  // Main function loop
  init();    // First run the init function
  app_event_loop();    // Loop
  deinit();  // Close off the watch face
}