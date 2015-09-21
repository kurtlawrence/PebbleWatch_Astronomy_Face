// This module adds a web get to retrieve the moon phase for the location. The module must be placed before the time module as it requires updating every set number of hours.
enum {
  KEY_LATITUDE = 1,
  KEY_LONGITUDE = 2,
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
static float latitude;
static float longitude;
static float UToffset;
static int lastUpdateTime;
static int altitude;
static int altitudeAcc;
static int GPScalls;

static int returnHour(int timeCode) {
  return timeCode / 100;
}
static int returnMinTens(int timeCode) {
  return (timeCode/10) % 10;
}
static int returnMinOnes(int timeCode) {
  return timeCode % 10;
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
double powerOfTen (int num) {
	double rst = 1.0;
	if (num >= 0) {
		for(int i = 0; i < num ; i++){
			rst *= 10.0;
		}
	} else {
		for(int i = 0; i < (0 - num ); i++){
			rst *= 0.1;
		}
	}
	return rst;
}
double my_sqrt(double a) {
	// Thanks to http://www.codeproject.com/Articles/570700/SquareplusRootplusalgorithmplusforplusC for a more accurate sqrt method
	// find more detail of this method on wiki methods_of_computing_square_roots
	// Babylonian method cannot get exact zero but approximately value of the square_root
	double z = a; 
	double rst = 0.0;
	int max = 8;     // to define maximum digit 
	int i;
	double j = 1.0;
	for(i = max ; i > 0 ; i--){
		// value must be bigger then 0
		if (z - (( 2 * rst ) + ( j * powerOfTen(i)))*( j * powerOfTen(i)) >= 0) {
			while ( z - (( 2 * rst ) + ( j * powerOfTen(i)))*( j * powerOfTen(i)) >= 0) {
				j++;
				if (j >= 10) {break;}
			}
			j--; //correct the extra value by minus one to j
			z -= (( 2 * rst ) + ( j * powerOfTen(i)))*( j * powerOfTen(i)); //find value of z
			rst += j * powerOfTen(i);     // find sum of a
			j = 1.0;
		}
	}

	for (i = 0 ; i >= 0 - max ; i--) {
		if (z - (( 2 * rst ) + ( j * powerOfTen(i)))*( j * powerOfTen(i)) >= 0) {
			while ( z - (( 2 * rst ) + ( j * powerOfTen(i)))*( j * powerOfTen(i)) >= 0) {
				j++;
			}
			j--;
			z -= (( 2 * rst ) + ( j * powerOfTen(i)))*( j * powerOfTen(i)); //find value of z
			rst += j * powerOfTen(i);     // find sum of a
			j = 1.0;
		}
	}
	// find the number on each digit
	return rst;
}
/*static float my_sqrt(const float num) {
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
}*/
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
  //APP_LOG(APP_LOG_LEVEL_INFO, "Entered SunRiseSetTimes");
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
  double xv_sun = (Cos(E_anomaly_sun) - e_sun) * 1000;		// Sun distance is mulitplied by 1000 for increased sqrt accuracy
  double yv_sun = (my_sqrt(1 - e_sun * e_sun) * Sin(E_anomaly_sun)) * 1000;
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
  //APP_LOG(APP_LOG_LEVEL_INFO, "Entered getMoonPhase");
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
	//APP_LOG(APP_LOG_LEVEL_INFO, "Entered update_Astronomy");
  //This is the main function that runs all the times and phase functions - updates the information on the watch
  //First we get the current date into a format that works with the code long(YYYYMMDD)----------------------------------------------
  // Get a tm structure for localtime and UTC0 time
  time_t localSeconds = time(NULL);
  struct tm *tick_time = localtime(&localSeconds);
  long cCodeDate = (tick_time->tm_year + 1900) * 10000 + (tick_time->tm_mon + 1) * 100 + tick_time->tm_mday;    //Alterations account for time.h functions
  
  // Need to get the rise time that corresponds to the right day, this is called by the difference in julian days
  long lastDateCode = persist_read_int(LDATE_STORED);
  int daysWithoutUpdate = calc_d_JDate(cCodeDate) - calc_d_JDate(lastDateCode);
  
  //Second we calculate moon rise time----------------------------------------------------------------------------------------------
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
  /*sunRiseTime = 617;
  sunSetTime = 1727;
  latitude = -27.47;
  longitude = 153.03;
  riseInt = 732;
  lastUpdateTime = 1808;
	lastDateCode = 20150817;
  moonPhaseNum = 1;*/
  
  //Create the buffers
  static char moonRiseBuffer[16] = "No rise";
  static char moonPhaseBuffer[16] = "##########";
  static char sunRiseBuffer[16] = "##########";
  static char sunSetBuffer[16] = "##########";
  static char lastUpdateBuffer[32] = "Not updated";
  static char GPScoordsBuffer[32] = "No GPS data yet";
  
  //Fill the buffers
  if(riseInt == 4000) {
    //Flags an error
    snprintf(moonRiseBuffer, sizeof(moonRiseBuffer), "No rise");
  } else {
    snprintf(moonRiseBuffer, sizeof(moonRiseBuffer), "%d:%d%d rise", returnHour(riseInt), returnMinTens(riseInt), returnMinOnes(riseInt));
  }
	gbitmap_destroy(s_moonIcon_bitmap);			// First destroy the bitmap that will house the moon icon, this prevents a leaking memory
  switch (moonPhaseNum) {
    case 0:
      snprintf(moonPhaseBuffer, sizeof(moonRiseBuffer), "New moon");
      if (latitude < 0) {
        s_moonIcon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_NewMoonSH_icon);
      } else {
        s_moonIcon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_NewMoonNH_icon);
      }
      break;
    case 1:
      snprintf(moonPhaseBuffer, sizeof(moonRiseBuffer), "Waxing");
      if (latitude < 0) {
        s_moonIcon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MoonCrescentL_icon);
      } else {
        s_moonIcon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MoonCrescentR_icon);
      }
      break;
    case 2:
      snprintf(moonPhaseBuffer, sizeof(moonRiseBuffer), "First qtr");
      if (latitude < 0) {
        s_moonIcon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MoonQtrL_icon);
      } else {
        s_moonIcon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MoonQtrR_icon);
      }
      break;
    case 3:
      snprintf(moonPhaseBuffer, sizeof(moonRiseBuffer), "Waxing");
      if (latitude < 0) {
        s_moonIcon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MoonGibbousL_icon);
      } else {
        s_moonIcon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MoonGibbousR_icon);
      }
      break;
    case 4:
      snprintf(moonPhaseBuffer, sizeof(moonRiseBuffer), "Full moon");
      s_moonIcon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FullMoon_icon);
      break;
    case 5:
      snprintf(moonPhaseBuffer, sizeof(moonRiseBuffer), "Waning");
      if (latitude < 0) {
        s_moonIcon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MoonGibbousR_icon);
      } else {
        s_moonIcon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MoonGibbousL_icon);
      }
      break;
    case 6:
      snprintf(moonPhaseBuffer, sizeof(moonRiseBuffer), "Last qtr");
      if (latitude < 0) {
        s_moonIcon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MoonQtrR_icon);
      } else {
        s_moonIcon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MoonQtrL_icon);
      }
      break;
    case 7:
      snprintf(moonPhaseBuffer, sizeof(moonRiseBuffer), "Waning");
      if (latitude < 0) {
        s_moonIcon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MoonCrescentR_icon);
      } else {
        s_moonIcon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MoonCrescentL_icon);
      }
      break;
    default:
      snprintf(moonPhaseBuffer, sizeof(moonRiseBuffer), "not found");
      s_moonIcon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_NewMoonSH_icon);
      break;
  }
  snprintf(sunRiseBuffer, sizeof(sunRiseBuffer), "%d:%d%d", returnHour(sunRiseTime), returnMinTens(sunRiseTime), returnMinOnes(sunRiseTime));
  snprintf(sunSetBuffer, sizeof(sunSetBuffer), "%d:%d%d", returnHour(sunSetTime), returnMinTens(sunSetTime), returnMinOnes(sunSetTime));
  int yearVar = lastDateCode / 10000;
  int monthVar = (lastDateCode % 10000) / 100;
  int dayVar = lastDateCode % 100;
	if (lastDateCode != 0){
		//Update the location data
  	snprintf(lastUpdateBuffer, sizeof(lastUpdateBuffer), "%d-%d-%d %d:%d%d UTC%d", dayVar, monthVar, yearVar, returnHour(lastUpdateTime), returnMinTens(lastUpdateTime), returnMinOnes(lastUpdateTime), (int)UToffset);
  	snprintf(GPScoordsBuffer, sizeof(GPScoordsBuffer), "Lat %d.%d%d, Long %d.%d%d", (int)latitude, returnMinTens(Abs(latitude) * 100), returnMinOnes(Abs(latitude) * 100), (int)longitude, returnMinTens(Abs(longitude) * 100), returnMinOnes(Abs(longitude) * 100));
	}
		
  //Display the values
  text_layer_set_text(s_moonRise_layer, moonRiseBuffer);
  text_layer_set_text(s_moonPhase_layer, moonPhaseBuffer);
  bitmap_layer_set_bitmap(s_moonIcon_layer, s_moonIcon_bitmap);     //Display the moon phase icon
  text_layer_set_text(s_sunRise_layer, sunRiseBuffer);
  text_layer_set_text(s_sunSet_layer, sunSetBuffer);
  text_layer_set_text(s_UTC_layer, lastUpdateBuffer);
  text_layer_set_text(s_Coords_layer, GPScoordsBuffer);
}
// END OF ASTRONOMY CALCULATIONS...-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static void moonModule_deinit() {
	//APP_LOG(APP_LOG_LEVEL_INFO, "Entered moon module deinit");
  app_message_deregister_callbacks();    //Destroy the callbacks for clean up
}
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
	//APP_LOG(APP_LOG_LEVEL_INFO, "Got data back from Phone");
  // This is the main function that operates when a callback happens for the GPS request
  // Store incoming information
  int newLatitude;
  int newLongitude;
  int useNewData = 0;		//Assume using old data until proven otherwise
  int newUToffset;
  int newRise0, newRise1, newRise2, newRise3, newRise4, newRise5, newRise6;
	
  // Read first item
  Tuple *t;
	t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
			case 0:
				// Key 0 is the first entry in the dictionary, skipped for values as I believe that was the source of problems..
				break;
      case KEY_LATITUDE:
        newLatitude = t->value->int32;
        break;
      case KEY_LONGITUDE:
        newLongitude = (int)t->value->int32;
        break;
      case KEY_USEOLDDATA:
        useNewData = (int)t->value->int32;
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
    }

    // Look for next item
    t = dict_read_next(iterator);
  }		//Transer the incoming information into the stores
	
  // Set the latitude and longitude global variables
  if (useNewData == 0) {
    // No new position data available, use old data
    // Latitude and longitude remain unchanged (along with time zone I believe)
		//APP_LOG(APP_LOG_LEVEL_INFO, "Data invalid from Phone");
		/*
		if (GPScalls < 3) {
			//Flag for a re-ping, dictionary memory will be destroyed, but callbacks are still registered
			repingGPS = 1;
		} else {
			//Run an update on the, as data has been correctly received and processed
			repingGPS = 0;
			moonModule_deinit();		//Call to deregister the callbacks (no longer needed)
			update_Astronomy();			//Refresh calculations
		}
		*/
		
  } else {
    // New position data available, update global variables. Lat, long, and UTC must remain as flexible variables, Rise values can be directly accessed through storage
    // Last update time can be updated to now as well
		//APP_LOG(APP_LOG_LEVEL_INFO, "Data valid from Phone");
    time_t localSeconds = time(NULL);
    struct tm *tick_time = localtime(&localSeconds);
    lastUpdateTime = (tick_time->tm_hour) * 100 + (tick_time->tm_min);
    int32_t cCodeDate = (tick_time->tm_year + 1900) * 10000 + (tick_time->tm_mon + 1) * 100 + tick_time->tm_mday;    //Alterations account for time.h functions
    
    latitude = (float)newLatitude/100;
    longitude = (float)newLongitude/100;
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
	//Run an update on the, as data has been correctly received and processed
	update_Astronomy();			//Refresh calculations
}
static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  //APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
	update_Astronomy();		//Redisplay values
}
static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  //APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
	update_Astronomy();		//Redisplay values
}
static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  //APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void moonModule_init() {
	//APP_LOG(APP_LOG_LEVEL_INFO, "Entered moon module init");
	// Register the callbacks
	app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
	// Open AppMessage
  app_message_open(APP_MESSAGE_INBOX_SIZE_MINIMUM, APP_MESSAGE_OUTBOX_SIZE_MINIMUM);		//Hopefully the guaranteed size is large enough to carry the data across bluetooth
  
  latitude = (float)persist_read_int(LAT_STORED) / 100;
  longitude = (float)persist_read_int(LNG_STORED) / 100;
  UToffset = persist_read_int(UTC_STORED);
  lastUpdateTime = persist_read_int(LAST_STORED);
	
	update_Astronomy();
}
static void update_GPS(int nCalls) {
	//APP_LOG(APP_LOG_LEVEL_INFO, "Entered update_GPS()");
	text_layer_set_text(s_UTC_layer, "Awaiting GPS data");		//Display update message
	
	// First increment the number of calls by setting GPScalls to the seed value passed through
	GPScalls = nCalls + 1;
  // Begin dictionary
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  // Add a key-value pair
  dict_write_uint8(iter, 0, 0);
  // Send the message!
  app_message_outbox_send();
}