// This module adds a web get to retrieve the moon phase for the location. The module must be placed before the time module as it requires updating every hour.
enum {
  KEY_LATITUDE = 0,
  KEY_LONGITUDE = 1,
  KEY_ALTITUDE = 2,
  KEY_ALTACC = 3,
  KEY_USEOLDDATA = 4,
  KEY_UTCh = 5,
  KEY_RISE_0 = 6,
  KEY_RISE_1 = 7,
  KEY_RISE_2 = 8,
  KEY_RISE_3 = 9,
  KEY_RISE_4 = 10,
  KEY_RISE_5 = 11,
  KEY_RISE_6 = 12
};    //Keys for JS implementation
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
  LDATE_STORED = 11
};    //Keys for local persistent storage
static TextLayer *moonRiseDispLayer;
static TextLayer *moonPhaseDispLayer;
static TextLayer *sunRiseDispLayer;
static TextLayer *sunSetDispLayer;
static BitmapLayer *moonIconDispLayer;
static GBitmap *moonIcon_bitmap;
static float latitude;
static float longitude;
static float UToffset;
static int lastUpdateTime;
static int altitude;
static int altitudeAcc;
static int localDebugDisp;    //0 or 1 for debug mode, when debug mode, prints values

void dispVal(int val, char valName[]) {
  //Displays the value
  APP_LOG(APP_LOG_LEVEL_DEBUG, "%s %d", valName, val);
}

static int returnHour(int timeCode) {
  return timeCode / 100;
}
static int returnMinTens(int timeCode) {
  return (timeCode/10) % 10;
}
static int returnMinOnes(int timeCode) {
  return timeCode % 10;
}

static void update_GPS() {
  // Begin dictionary
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  // Add a key-value pair
  dict_write_uint8(iter, 0, 0);

  // Send the message!
  app_message_outbox_send();
}

// THIS IS THE START OF THE SUN/MOON PHASE CALCULATIONS------------------------------------------------------------------------------------------------------------------------------------------------------
//Due to the weird trig functions, it is possible that everything is done in degrees! NOTE, moon rise must be calculated by phone (too many errors)
// Adapted from http://www.stjarnhimlen.se/comp/ppcomp.html

// Create some constants
static double PI = 3.14159265359;

static long calc_d_JDate(long dateInCodeFormat) {
  //Converts the date given in code format into a julian date set by the reference website
  long yearVar = dateInCodeFormat / 10000;
  long monthVar = (dateInCodeFormat % 10000) / 100;
  long dayVar = dateInCodeFormat % 100;
  long temp = 367 * yearVar - 7 * (yearVar + (monthVar + 9) / 12) / 4 + 275 * monthVar / 9 + dayVar - 730530;
  
  return temp;
}

static double modDecimal(double x, double divisor) {
  //A modulus operator that results in positive decimal remainders
  double temp = x;
  double d = divisor;
  if (divisor == 0) {
    //Can't divide by 0!
    temp = 0;    //This should be enough to crush multiplications and flag a division by zero
  } else {
    if (divisor <= 0) {
      d = -divisor;
    } else {
      d = divisor;
    }
    //Deal with negative numbers first, positive x values will be exempt from this
    while (temp < 0) {
      temp = temp + d;    //Add divisor until temp is above 0, this is the remainder
    }
    //Now deal with positive numbers above divisor, negative numbers are already within range and will skip this step
    while (temp > d) {
      temp = temp - d;    //Subtract divisor until temp is within divisor range
    }
  }
  
  return temp;
}

static double Abs(double x) {
  if (x < 0) {
    x = -x;
  }
  return x;
}

static double Sin(double xDeg) {
  //Unfortunately sin will not work on doubles so this work around will return the sin as a decimal in degrees!
  int32_t trigAngle = TRIG_MAX_ANGLE * xDeg / 360;
  return (double)sin_lookup(trigAngle) / TRIG_MAX_RATIO;
}

static double Cos(double xDeg) {
  //Same as sin, need to function using the lookup value
  int32_t trigAngle = TRIG_MAX_ANGLE * xDeg / 360;
  return (double)cos_lookup(trigAngle) / TRIG_MAX_RATIO;
}

static float my_sqrt(const float num) {
  //Thanks to http://forums.getpebble.com/discussion/5792/sqrt-function for creating an accurate sqrt function
  const uint MAX_STEPS = 40;
  const float MAX_ERROR = 0.0001;
  
  float answer = num;
  float ans_sqr = answer * answer;
  uint step = 0;
  while((ans_sqr - num > MAX_ERROR) && (step++ < MAX_STEPS)) {
    answer = (answer + (num / answer)) / 2;
    ans_sqr = answer * answer;
  }
  return answer;
}

static double arctan2(double ynum, double xnum) {
  //As with the sin and cos functions, arc tan must be altered to keep some form of accuracy along the way
  double xMaxMultiplier = Abs(32767 / xnum);
  double yMaxMultiplier = Abs(32767 / ynum);
  double scaleFactor = 1;      //Initially set scale factor to 1
  if (xMaxMultiplier > yMaxMultiplier) {
    scaleFactor = yMaxMultiplier;    //Can only scale by lowest multiplier
  } else {
    scaleFactor = xMaxMultiplier;
  }
  int16_t yScaled = ynum * scaleFactor;
  int16_t xScaled = xnum * scaleFactor;
  double temp = (double)atan2_lookup(yScaled, xScaled);
  temp = temp / TRIG_MAX_ANGLE * 360;
  return temp;
}

static double arccos(double xnum) {
  return arctan2(my_sqrt((1 + xnum) * (1 - xnum)), xnum);
}

static int SunRiseSetTimes(long dateVar, double localLat_deg, double localLng_deg, int isSunSet) {
  //Returns the sun set or rise in HHMM. Rise is set to the next day
  APP_LOG(APP_LOG_LEVEL_INFO, "Entered SunRiseSetTimes");
  //Set constants
  int localTimeVar = 1200;
  double h_trigger = -0.833;
  
  //First calculate the d and UT
  double d = calc_d_JDate(dateVar);
  double UT = localTimeVar / 100 + (float)(localTimeVar % 100) / 60 - UToffset;    //Have to cast a float when calculating minutes decimals
  d = d + UT/24;
  UT = modDecimal(UT, 24);      //If UT is negative convert back to normal time
  //The rise day needs to be incremented one day for tomorrows rise
  if (isSunSet == 0) {
    d = d + 1;    //getting rise
  }
  
  //Next calculate the sun's pseudo constants
  double ecl = 23.4393 - 0.0000003563 * d;    //Obliquity of ecliptic
  double w_sun = 282.9404 + 0.0000470935 * d;
  double M_sun = modDecimal(356.047 + 0.9856002585 * d, 360);
  double e_sun = 0.016709 - 0.000000001151 * d;
  double E_anomaly_sun = M_sun + e_sun * 180 / PI * Sin(M_sun) * (1 + e_sun * Cos(M_sun));
  double xv_sun = Cos(E_anomaly_sun) - e_sun;
  double yv_sun = my_sqrt(1 - e_sun * e_sun) * Sin(E_anomaly_sun);
  double v_sun = arctan2(yv_sun, xv_sun);
  double r_sun = my_sqrt(xv_sun * xv_sun + yv_sun * yv_sun);
  double Ls = v_sun + w_sun;
  
  //Calculate the ecliptic rectangular geocentric coordinates
  double xs = r_sun * Cos(Ls);
  double ys = r_sun * Sin(Ls);
  
 //Convert to equatorial rectangular geocentric coords
  double xe = xs;
  double ye = ys * Cos(ecl);
  double ze = ys * Sin(ecl);

  //Calculate ra and decl
  double RA = arctan2(ye, xe);
  double decl = arctan2(ze, my_sqrt(xe * xe + ye *ye));

  //Can calculate UT_sunInSouth (all in degs)
  double GMST0 = M_sun + w_sun + 180;
  double UT_sunInSouth = RA - GMST0 - localLng_deg;

  //Calculate sun local hour angle - initially a cos function
  double LHA = (Sin(h_trigger) - Sin(localLat_deg) * Sin(decl)) / (Cos(localLat_deg) * Cos(decl));

  double temp;    //Declare temp variable
  //Now to determine the state
  if (LHA > 1) {
    temp = 3000;    //No rise occurs, specify error code
  } else if (LHA < -1) {
    temp = 4000;    //No set occurs, specify error code
  } else {
    //cos(LHA) is between -1 and 1, calculate LHA
    LHA = arccos(LHA);
    //Calc rise and set values in local time
    double sunSet = UT_sunInSouth + LHA + UToffset * 15;
    double sunRise = UT_sunInSouth - LHA + UToffset * 15;

    //Convert to hour decimal and return the asked for time in local time HHMM
    if (isSunSet == 0) {
        temp = sunRise;      //Asked for rise
    } else {
        temp = sunSet;        //Asked for set
    }
    temp = temp / 15;              //Get to hour decimal
    temp = modDecimal(temp, 24);   //Get to positive 24hr time
    temp = (int)temp * 100 + (int)(temp * 60) % 60 ;   //Get to HHMM
  }

  //Return time
  return temp;
}

static int getMoonPhase(long dateVar) {
  //Returns the moon's phase nummber, from 0-7 starting from new moon
  APP_LOG(APP_LOG_LEVEL_INFO, "Entered getMoonPhase");
  //Set some constants
  int localTimeVar = 600;
  double modDivisor = 29.530588853;       //Lunar month length
  double phaseLength = modDivisor / 4;    //Major phase length
  
  //First calculate the d and UT
  double d = calc_d_JDate(dateVar);
  double UT = localTimeVar / 100 + (float)(localTimeVar % 100) / 60 - UToffset;    //Have to cast a float when calculating minutes decimals
  d = d + UT/24;
  UT = modDecimal(UT, 24);      //If UT is negative convert back to normal time
  
  //Getting the d from previous calculations, this needs to be adjusted to the last known new moon
  d = d - 5498.1347222;
  
  double phaseDay = modDecimal(d, modDivisor);    //Calculate the phase day
  //Now the logic - the major modes (New, qtrs, Full) require 1 days accuracy, others result in the waxing/waning
  // as the julian day is calculated at 6am, then I give the moon an extra day of approximation (ie +24 hrs)
  int phaseNum = 1;
  for (int i = 1; i <= 4; i++) {
    if (phaseDay > (phaseLength * (i - 1))) {
      if (phaseDay <= (phaseLength * i)) {
        phaseNum = i * 2 - 1;
      }
      if (phaseDay <= (phaseLength * (i - 1) + 1)) {
        phaseNum = i * 2 - 2;
      }
    }
  }
  
  return phaseNum;  
}

static void update_Astronomy() {
  //This is the main function that runs all the times and phase functions - updates the information on the watch
  //First we get the current date into a format that works with the code long(YYYYMMDD)----------------------------------------------
  // Get a tm structure for localtime and UTC0 time
  time_t localSeconds = time(NULL);
  struct tm *tick_time = localtime(&localSeconds);
  long cCodeDate = (tick_time->tm_year + 1900) * 10000 + (tick_time->tm_mon + 1) * 100 + tick_time->tm_mday;    //Alterations account for time.h functions
  
  // Need to get the rise time that corresponds to the right day, this is called by the difference in julian days
  long lastDateCode = persist_read_int(LDATE_STORED);
  int daysWithoutUpdate = calc_d_JDate(cCodeDate) - calc_d_JDate(lastDateCode);
  
  if (daysWithoutUpdate != 0) {
    //See if we can get updated GPS coords if it's been longer than a day without update. This will run in the initlisation as well, only if more than one day without updating!
    update_GPS();
    //Since this may have changed, check the last update date
    lastDateCode = persist_read_int(LDATE_STORED);
    daysWithoutUpdate = calc_d_JDate(cCodeDate) - calc_d_JDate(lastDateCode);
  }
  
  //Manually adjust cCodeDate for testing
  //cCodeDate = 20160213;
  
  //Second we calculate moon rise time--------------------------------------------------------------------------------
  // Moon rise times are already calculated from the phone javascript
  int riseInt = 4000;
  switch(daysWithoutUpdate) {
    case 0:
      riseInt = persist_read_int(RISE0_STORED);
      break;
    case 1:
      riseInt = persist_read_int(RISE1_STORED);
      break;
    case 2:
      riseInt = persist_read_int(RISE2_STORED);
      break;
    case 3:
      riseInt = persist_read_int(RISE3_STORED);
      break;
    case 4:
      riseInt = persist_read_int(RISE4_STORED);
      break;
    case 5:
      riseInt = persist_read_int(RISE5_STORED);
      break;
    case 6:
      riseInt = persist_read_int(RISE6_STORED);
      break;
    default:
      riseInt = 4000;    //Error value
      break;
  }
  
  //Third calculate the sun rise and set time-------------------------------------------------------------------------------------
  int sunRiseTime = SunRiseSetTimes(cCodeDate, latitude, longitude, 0);
  int sunSetTime = SunRiseSetTimes(cCodeDate, latitude, longitude, 1);
  
  //Fourth calculate the moon phase-----------------------------------------------------------------------------------------------
  int moonPhaseNum = getMoonPhase(cCodeDate);
  
  
  //Fifth; Display the data!------------------------------------------------------------------------------------------------------
  //Note: Moonphases are flipped for hemispheres -> http://resources.woodlands-junior.kent.sch.uk/time/moon/hemispheres.html
  //Create a test bed for...testing
  /*sunRiseTime = 2207;
  sunSetTime = 2307;
  latitude = -27.07;
  longitude = -120.04;
  riseInt = 2207;
  lastUpdateTime = 1808;
  moonPhaseNum = 4;*/
  
  //Create the buffers
  static char moonRiseBuffer[16] = "No rise/set";
  static char moonPhaseBuffer[16] = "##########";
  static char sunRiseBuffer[16] = "##########";
  static char sunSetBuffer[16] = "##########";
  static char lastUpdateBuffer[32] = "Not updated";
  static char GPScoordsBuffer[32] = "No GPS data yet";
  
  //Fill the buffers
  if(riseInt == 4000) {
    //Flags an error
    snprintf(moonRiseBuffer, sizeof(moonRiseBuffer), "No rise/set");
  } else {
    snprintf(moonRiseBuffer, sizeof(moonRiseBuffer), "%d:%d%d rise", returnHour(riseInt), returnMinTens(riseInt), returnMinOnes(riseInt));
  }
  switch (moonPhaseNum) {
    case 0:
      snprintf(moonPhaseBuffer, sizeof(moonRiseBuffer), "New moon");
      if (latitude < 0) {
        moonIcon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_NewMoonSH_icon);
      } else {
        moonIcon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_NewMoonNH_icon);
      }
      break;
    case 1:
      snprintf(moonPhaseBuffer, sizeof(moonRiseBuffer), "Waxing");
      if (latitude < 0) {
        moonIcon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MoonCrescentL_icon);
      } else {
        moonIcon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MoonCrescentR_icon);
      }
      break;
    case 2:
      snprintf(moonPhaseBuffer, sizeof(moonRiseBuffer), "First qtr");
      if (latitude < 0) {
        moonIcon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MoonQtrL_icon);
      } else {
        moonIcon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MoonQtrR_icon);
      }
      break;
    case 3:
      snprintf(moonPhaseBuffer, sizeof(moonRiseBuffer), "Waxing");
      if (latitude < 0) {
        moonIcon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MoonGibbousL_icon);
      } else {
        moonIcon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MoonGibbousR_icon);
      }
      break;
    case 4:
      snprintf(moonPhaseBuffer, sizeof(moonRiseBuffer), "Full moon");
      moonIcon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FullMoon_icon);
      break;
    case 5:
      snprintf(moonPhaseBuffer, sizeof(moonRiseBuffer), "Waning");
      if (latitude < 0) {
        moonIcon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MoonGibbousR_icon);
      } else {
        moonIcon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MoonGibbousL_icon);
      }
      break;
    case 6:
      snprintf(moonPhaseBuffer, sizeof(moonRiseBuffer), "Last qtr");
      if (latitude < 0) {
        moonIcon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MoonQtrR_icon);
      } else {
        moonIcon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MoonQtrL_icon);
      }
      break;
    case 7:
      snprintf(moonPhaseBuffer, sizeof(moonRiseBuffer), "Waning");
      if (latitude < 0) {
        moonIcon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MoonCrescentR_icon);
      } else {
        moonIcon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MoonCrescentL_icon);
      }
      break;
    default:
      snprintf(moonPhaseBuffer, sizeof(moonRiseBuffer), "not found");
      moonIcon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_NewMoonSH_icon);
      break;
  }
  snprintf(sunRiseBuffer, sizeof(sunRiseBuffer), "%d:%d%d", returnHour(sunRiseTime), returnMinTens(sunRiseTime), returnMinOnes(sunRiseTime));
  snprintf(sunSetBuffer, sizeof(sunSetBuffer), "%d:%d%d", returnHour(sunSetTime), returnMinTens(sunSetTime), returnMinOnes(sunSetTime));
  int yearVar = lastDateCode / 10000;
  int monthVar = (lastDateCode % 10000) / 100;
  int dayVar = lastDateCode % 100;
  snprintf(lastUpdateBuffer, sizeof(lastUpdateBuffer), "%d-%d-%d %d:%d%d UTC%d", dayVar, monthVar, yearVar, returnHour(lastUpdateTime), returnMinTens(lastUpdateTime), returnMinOnes(lastUpdateTime), (int)UToffset);
  snprintf(GPScoordsBuffer, sizeof(GPScoordsBuffer), "Lat %d.%d%d, Long %d.%d%d", (int)latitude, returnMinTens(Abs(latitude) * 100), returnMinOnes(Abs(latitude) * 100), (int)longitude, returnMinTens(Abs(longitude) * 100), returnMinOnes(Abs(longitude) * 100));
  
  //Display the values
  text_layer_set_text(moonRiseDispLayer, moonRiseBuffer);
  text_layer_set_text(moonPhaseDispLayer, moonPhaseBuffer);
  bitmap_layer_set_bitmap(moonIconDispLayer, moonIcon_bitmap);     //Display the moon phase icon
  text_layer_set_text(sunRiseDispLayer, sunRiseBuffer);
  text_layer_set_text(sunSetDispLayer, sunSetBuffer);
  text_layer_set_text(s_UTC_layer, lastUpdateBuffer);
  text_layer_set_text(s_Coords_layer, GPScoordsBuffer);
}

// END OF ASTRONOMY CALCULATIONS...-------------------------------------------------------------------------------------------------------------------------------------------------------------------------


static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // This is the main function that operates when a callback happens for the GPS request
  // Store incoming information
  static int newLatitude;
  static int newLongitude;
  static int newAltitude;
  static int newAltitudeAcc;
  static int useOldData;
  static int newUToffset;
  static int newRise0, newRise1, newRise2, newRise3, newRise4, newRise5, newRise6;
  
  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
      case KEY_LATITUDE:
        newLatitude = (int)t->value->int32;
        break;
      case KEY_LONGITUDE:
        newLongitude = (int)t->value->int32;
        break;
      case KEY_ALTITUDE:
        newAltitude = (int)t->value->int32;
        break;
      case KEY_ALTACC:
        newAltitudeAcc = (int)t->value->int32;
        break;
      case KEY_USEOLDDATA:
        useOldData = (int)t->value->int32;
        break;
      case KEY_UTCh:
        newUToffset = (int)t->value->int32;
        break;
      case KEY_RISE_0:
        newRise0 = (int)t->value->int32;
        break;
      case KEY_RISE_1:
        newRise1 = (int)t->value->int32;
        break;
      case KEY_RISE_2:
        newRise2 = (int)t->value->int32;
        break;
      case KEY_RISE_3:
        newRise3 = (int)t->value->int32;
        break;
      case KEY_RISE_4:
        newRise4 = (int)t->value->int32;
        break;
      case KEY_RISE_5:
        newRise5 = (int)t->value->int32;
        break;
      case KEY_RISE_6:
        newRise6 = (int)t->value->int32;
        break;
      default:
        APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
        break;
    }

    // Look for next item
    t = dict_read_next(iterator);
  }

  // Set the latitude and longitude global variables
  if (useOldData == 1) {
    // No new position data available, use old data
    // Latitude and longitude remain unchanged (along with time zone I believe)
  } else {
    // New position data available, update global variables. Lat, long, and UTC must remain as flexible variables, Rise values can be directly accessed through storage
    // Last update time can be updated to now as well
    time_t localSeconds = time(NULL);
    struct tm *tick_time = localtime(&localSeconds);
    lastUpdateTime = (tick_time->tm_hour) * 100 + (tick_time->tm_min);
    int32_t cCodeDate = (tick_time->tm_year + 1900) * 10000 + (tick_time->tm_mon + 1) * 100 + tick_time->tm_mday;    //Alterations account for time.h functions
    
    latitude = (float)newLatitude/100;
    longitude = (float)newLongitude/100;
    altitude = newAltitude;
    altitudeAcc = newAltitudeAcc;
    UToffset = newUToffset;    //Already in hrs
    persist_write_int(LAT_STORED, newLatitude);    //Already as an integer (must / 100 when retrieving)
    persist_write_int(LNG_STORED, newLongitude);
    persist_write_int(UTC_STORED, newUToffset);
    persist_write_int(RISE0_STORED, newRise0);
    persist_write_int(RISE1_STORED, newRise1);
    persist_write_int(RISE2_STORED, newRise2);
    persist_write_int(RISE3_STORED, newRise3);
    persist_write_int(RISE4_STORED, newRise4);
    persist_write_int(RISE5_STORED, newRise5);
    persist_write_int(RISE6_STORED, newRise6);
    persist_write_int(LAST_STORED, lastUpdateTime);
    persist_write_int(LDATE_STORED, cCodeDate);
  }
  
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}




static void moonModule_init(TextLayer *temp, TextLayer *temp2, TextLayer *temp3, TextLayer *temp4, BitmapLayer *temp5) {
  //Set dbug mode
  localDebugDisp = 2;
  //dispVal(localDebugDisp, "Debug mode:");
  
  // Conform the display layers
  moonRiseDispLayer = temp;
  moonPhaseDispLayer = temp2;
  sunRiseDispLayer = temp3;
  sunSetDispLayer = temp4;
  moonIconDispLayer = temp5;      //The bitmap layer that the moon icon display layer is done on.
  
  latitude = (float)persist_read_int(LAT_STORED) / 100;
  longitude = (float)persist_read_int(LNG_STORED) / 100;
  UToffset = persist_read_int(UTC_STORED);
  lastUpdateTime = persist_read_int(LAST_STORED);
  
  // Register the callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
  
  // Update the GPS data
  //update_GPS();
  
  //Update the astronomy data
  update_Astronomy();
  
}

static void moonModule_deinit() {
  app_message_deregister_callbacks();    //Destroy the callbacks for clean up
  gbitmap_destroy(moonIcon_bitmap);      //Destroy the bitmap filled with moon icon
}