function sendBackToPebble(lat, long, hasData, offsetHrs, riseTimes) {
  // Assemble dictionary using our keys, Minutes need to be flipped in sign
  var dictionary = {
    'KEY_LATITUDE': lat,
    'KEY_LONGITUDE': long,
    'KEY_USEOLDDATA': hasData,
    'KEY_UTCh': offsetHrs,
    'KEY_RISE_0': riseTimes[0],
    'KEY_RISE_1': riseTimes[1],
    'KEY_RISE_2': riseTimes[2],
    'KEY_RISE_3': riseTimes[3],
    'KEY_RISE_4': riseTimes[4],
    'KEY_RISE_5': riseTimes[5],
    'KEY_RISE_6': riseTimes[6]
  };
	
	//console.log("Data is about to be sent back to phone; lat is " + dictionary.KEY_LATITUDE);
	
  // Send to Pebble
  Pebble.sendAppMessage(dictionary,
    function(e) {
      console.log('Pos info sent to Pebble successfully!');
    },
    function(e) {
      console.log('Error sending pos info to Pebble!');
    }
  );
}
// Function for when location gathering is successful
function locationSuccess(pos) {
    // Return the position data to the watch
	//console.log("Location succesfully found; latitude is " + pos.coords.latitude);
    var latitude = parseInt(pos.coords.latitude * 100);
    var longitude = parseInt(pos.coords.longitude * 100);
    var useOldData = 1;		//Have to set new data to 1 and no data as 0, so the fallback is no data (empty = 0) -> This is for the watch side of things
	
	//console.log("Latitude transformed to integer " + latitude);
    
    // Get the number of hours to add to convert localtime to utc
    var offsetHrs = new Date().getTimezoneOffset() / -60;
  
  var tempDate = new Date();
  tempDate.setDate(tempDate.getDate() - 1);
  var riseTimes = [4000,4000,4000,4000,4000,4000,4000];    //Error value of 4000h to indicate no rise
  for (var i = 0; i <= 6; i++) {
    tempDate.setDate(tempDate.getDate() + 1);
    riseTimes[i] = getMoonRiseTime(tempDate, latitude / 100, longitude / 100, offsetHrs);
    //console.log('Date' + i + ' ' + tempDate.getFullYear() + '/' + (tempDate.getMonth() + 1) + '/' + tempDate.getDate() + " Moon rise " + riseTimes[i]);
  }
  
  sendBackToPebble(latitude, longitude, useOldData, offsetHrs, riseTimes);
}
// Function for when the location gathering is unsuccessful
function locationError(err) {
  //console.log("Error requesting location!");
  // Return a failure --> Use last know position
  var latitude = 0;
  var longitude = 0;
  var useNewData = 0;
  var riseTime1 = 0;
  
  // Get the number of minutes to add to convert localtime to utc
  var offsetHrs = new Date().getTimezoneOffset();
  sendBackToPebble(latitude, longitude, useNewData, offsetHrs, riseTime1);
}
// Function to get the current position
function getLocation() {
  navigator.geolocation.getCurrentPosition(locationSuccess, locationError, {enableHighAccuracy: false, timeout: 5000, maximumAge: 900000});
}
// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log('PebbleKit JS ready!');
  }
);
// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log('AppMessage received!');
    
    //Get Location
    getLocation();
  }                     
);

//Moon rise time calculations
function dispVal(val, valName) {
  console.log(valName + " " + val);
}
function calc_d_JDate(dateInCodeFormat) {
  //Converts the date given in code format into a julian date set by the reference website
  var yearVar = parseInt(dateInCodeFormat / 10000);
  var monthVar = parseInt((dateInCodeFormat % 10000) / 100);
  var dayVar = dateInCodeFormat % 100;
  var temp = 367 * yearVar - parseInt(7 * (yearVar + parseInt((monthVar + 9) / 12)) / 4) + parseInt(275 * monthVar / 9) + dayVar - 730530;
  
  return temp;
}
function modDecimal(x, divisor) {
  //A modulus operator that results in positive decimal remainders
  var temp = x;
  var d = divisor;
  if (divisor === 0) {
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
// Function to get the moon altitude
function getMoonAltitude(localTimeVar, dateVar, localLat_deg, localLng_deg, UToffset) {
  //Calculates the moon's altitude in radians at the given local time
  //console.log('Entered getMoonAltitude');
  var localDebugDisp = 0;
  
  //Setup a few constants
  var RAD = Math.PI / 180,
      Sin  = Math.sin,
      Cos  = Math.cos,
      arcsin = Math.asin,
      arctan2 = Math.atan2,
      my_sqrt = Math.sqrt,
      localLat_rad = localLat_deg * RAD;
  
  //First calculate the d and UT
  var d = calc_d_JDate(dateVar);
  var UT = parseInt(localTimeVar / 100) + (localTimeVar % 100) / 60 - UToffset;    //Have to cast a float when calculating minutes decimals
  d = d + UT/24;
  UT = modDecimal(UT, 24);      //If UT is negative convert back to normal time
  
  if (localDebugDisp == 1) {
    dispVal(localTimeVar, "Local time input:");
    dispVal(UT, "UT:");
    dispVal(d, "Julian date:");
  }
  
  //Next calculate the sun's pseudo constants
  var ecl = (23.4393 - 0.0000003563 * d) * RAD;    //Obliquity of ecliptic
  var w_sun = (282.9404 + 0.0000470935 * d) * RAD;
  var M_sun = modDecimal(356.047 + 0.9856002585 * d, 360) * RAD;
  var e = 0.016709 - 0.000000001151 * d;
  var E_anomaly = M_sun + e * Sin(M_sun) * (1 + e * Cos(M_sun));
  var xv = Cos(E_anomaly) - e;
  var yv = my_sqrt(1 - e * e) * Sin(E_anomaly);
  var v_sun = arctan2(yv, xv);
  var Ls = v_sun + w_sun;
  
  if (localDebugDisp == 1) {
    dispVal(ecl, "ecl");
    dispVal(w_sun, "w_sun");
    dispVal(M_sun, "M_sun");
    dispVal(e, "e_sun");
    dispVal(E_anomaly, "E_sun");
    dispVal(xv, "xv_sun");
    dispVal(yv, "yv_sun");
    dispVal(v_sun, "v_sun");
    dispVal(Ls, "Ls");
  }
  // 'e' is no longer of use --> now implies e_moon
  // 'E_anomaly' is no longer of use --> now implies E_anomaly_moon
  // 'xv' is no longer of use --> now implies xv_moon
  // 'yv' is no longer of use --> now implies yv_moon
  
  //Setup some more pseudo constants for the moon
  e = 0.0549;
  var M_moon = modDecimal(115.3654 + 13.0649929509 * d, 360) * RAD;
  E_anomaly = M_moon + e * Sin(M_moon) * (1 + e * Cos(M_moon));
  var N_moon = modDecimal(125.1228 - 0.0529538083 * d, 360) * RAD;
  var w_moon = modDecimal(318.0634 + 0.1643573223 * d, 360) * RAD;
  var i_moon = 5.1454 * RAD;
  var a_moon = 60.2666;
  xv = a_moon * (Cos(E_anomaly) - e);
  yv = a_moon * (my_sqrt(1 - e * e) * Sin(E_anomaly));
  var v_moon = arctan2(yv, xv);
  var r_moon = my_sqrt(xv * xv + yv * yv);
  
  //Calculate the local sidereal
  var LST = modDecimal((Ls / RAD) + 180 + UT * 15 + localLng_deg, 360) * RAD;
  
  if (localDebugDisp == 1) {
    dispVal(LST, "LST");
    dispVal(e, "e_moon");
    dispVal(M_moon, "M_moon");
    dispVal(E_anomaly, "E_moon");
    dispVal(N_moon, "N_moon");
    dispVal(w_moon, "w_moon");
    dispVal(i_moon, "i_moon");
    dispVal(a_moon, "a_moon");
    dispVal(xv, "xv_moon");
    dispVal(yv, "yv_moon");
    dispVal(v_moon, "v_moon");
    dispVal(r_moon, "r_moon_unP");
  }
  // 'd' is no longer of use --> now implies xh
  // 'UT' is no longer of use --> now impies yh
  // 'a_moon' is no longer of use
  // 'Ls' is no longer of use --> now implies pertF (further down)
  // 'xv' is no longer of use
  // 'yv' is no longer of use
  // 'E_anomaly' is no longer of use
  // 'e' is no longer of use --> now implies zh  
  
  //Calculate the heliocentric rectangular coordinates of the moon
  d = r_moon * (Cos(N_moon) * Cos(v_moon + w_moon) - Sin(N_moon) * Sin(v_moon + w_moon) * Cos(i_moon));
  UT = r_moon * (Sin(N_moon) * Cos(v_moon + w_moon) + Cos(N_moon) * Sin(v_moon + w_moon) * Cos(i_moon));
  e = r_moon * (Sin(v_moon + w_moon) * Sin(i_moon));
  //Convert to ecliptic long and lat
  var lonecl = arctan2(UT, d);
  var latecl = arctan2(e, my_sqrt(d * d + UT * UT));
  
  if (localDebugDisp == 1) {
    dispVal(d, "xh");
    dispVal(UT, "yh");
    dispVal(e, "zh");
    dispVal(lonecl, "lonecl_unP");
    dispVal(latecl, "latecl_unP");
  }
  // 'd' is no longer of use --> now implies L_sun
  // 'UT' is no longer of use --> now implies L_moon
  // 'e' is no longer of use --> now implies pertD
  // 'v_moon' is no longer of use
  // 'i_moon' is no longer of use
  
  //Setup and apply moon pertubations
  d = modDecimal(M_sun + w_sun, 360 * RAD);
  UT = modDecimal(M_moon + w_moon + N_moon, 360 * RAD);
  e = UT - d;
  Ls = UT - N_moon;
  
  if (localDebugDisp == 1) {
    dispVal(d, "L_sun");
    dispVal(UT, "L_moon");
    dispVal(e, "pertD");
    dispVal(Ls, "pertF");
  }
  // 'd' is no longer of use --> now implies lonAdj
  // 'UT' is no longer of use --> now implies latAdj
  // 'w_sun' is no longer of use
  // 'N_moon' is no longer of use
  // 'w_moon' is no longer of use
  
  //Calculate the adjustments
  d = -1.274 * RAD * Sin(M_moon - 2 * e) + 0.658 * RAD * Sin(2 * e) - 0.186 * RAD * Sin(M_sun);
  UT = -0.173 * RAD * Sin(Ls - 2 * e);
  var rAdj = -0.58 * Cos(M_moon - 2 * e) - 0.46 * Cos(2 * e);
  //Apply the adjustments
  lonecl = lonecl + d;
  latecl = latecl + UT;
  r_moon = r_moon + rAdj;
  
  if (localDebugDisp == 1) {
    dispVal(d, "lonAdj");
    dispVal(UT, "latAdj");
    dispVal(rAdj, "rAdj");
    dispVal(lonecl, "lonecl_P");
    dispVal(latecl, "latecl_P");
    dispVal(r_moon, "r_moon_P");
  }
  // 'd' is no longer of use --> now implies xg
  // 'UT' is no longer of use --> now implies yg
  // 'rAdj' is no longer of use
  // 'M_moon' is no longer of use
  // 'e' is no longer of use --> now implies zg
  // 'M_sun' is no longer of use
  // 'Ls' is no longer of use --> now implies ye
  
  //Calculate geocentric rectangular coordinates
  d = r_moon * Cos(lonecl) * Cos(latecl);
  UT = r_moon * Sin(lonecl) * Cos(latecl);
  e = r_moon * Sin(latecl);
  
  // 'lonecl' is no longer of use
  // 'latecl' is no longer of use

  //Calculate the equitorial retangular coordinates (xe = xg = d)
  Ls = UT * Cos(ecl) - e * Sin(ecl);
  var ze = UT * Sin(ecl) + e * Cos(ecl);
  
  if (localDebugDisp == 1) {
    dispVal(d, "xg");
    dispVal(UT, "yg");
    dispVal(e, "zg");
    dispVal(d, "xe");
    dispVal(Ls, "ye");
    dispVal(ze, "ze");
  }  
  // 'UT' is no longer of use --> now implies RA
  // 'ecl' is no longer of use
  // 'e' is no longer of use --> now implies decl

  //Compute major RA and Decl, and hour angle
  UT = arctan2(Ls, d);
  e = arctan2(ze, my_sqrt(d * d + Ls * Ls));
  var HA = LST - UT;
  
  if (localDebugDisp == 1) {
    dispVal(UT, "RA");
    dispVal(e, "Decl");
    dispVal(HA, "HA");
  }
  // 'd' is no longer of use --> now implies x
  // 'UT' is no longer of use --> now implies z
  // 'Ls' is no longer of use --> now implies zhor
  // 'ze' is no longer of use
  // 'LST' is no longer of use

  //Calculate azimuthal coordinates
  d = Cos(HA) * Cos(e);
  UT = Sin(e);
  Ls = d * Cos(localLat_rad) + UT * Sin(localLat_rad);
  
  if (localDebugDisp == 1) {
    dispVal(d, "x");
    dispVal(UT, "z");
    dispVal(Ls, "zhor");
  }
  // 'd' is no longer of use --> now implies alt_geoc
  // 'UT' is no longer of use --> now implies mpar
  // 'HA' is no longer of use
  // 'e' is no longer of use
  
  //Calculate the geocentric altitude
  d = arcsin(Ls);
  UT = arcsin(1 / r_moon);     //Get a parallax value
  
  // 'Ls' is no longer of use
  // 'r_moon' is no longer of use
  
  //Adjust the geocentric altitude to the topocentric altitude
  var alt_topoc = d - UT * Cos(d);
  
  //Display the values for checking if debug mode on
  if (localDebugDisp == 1) {
    dispVal(d, "alt_geoc");
    dispVal(UT, "mpar");
    dispVal(alt_topoc, "alt_topoc");
  }
  
  
  //Set a debug mode to stop crashing
  localDebugDisp = 0;
  
  
  //That's it! return the topocentric altitude in radians, along with converting to a +- 180deg
  return alt_topoc;
}
function getMoonRiseTime(dateVar, latitude, longitude, UToffset) {
  //This is the main function that runs all the times and phase functions - updates the information on the watch
  //First we get the current date into a format that works with the code long(YYYYMMDD)----------------------------------------------
  // Get a tm structure for localtime and UTC0 time
  var cCodeDate = dateVar.getFullYear() * 10000 + (dateVar.getMonth() + 1) * 100 + dateVar.getDate();    //Alterations account for time.h functions
  
  //Manually adjust cCodeDate for testing
  //cCodeDate = 20160213;
  
  //Second we calculate moon rise time--------------------------------------------------------------------------------
  var hc = 0.00232129;    //Horizon trigger
  var h0 = getMoonAltitude(0, cCodeDate, latitude, longitude, UToffset) - hc;      //First call of getMoonAltitude! gets the local time 0 moon altitude
  var h1 = 0;    //Define h1 now
  var h2 = 0;    //Define h2 now, both in rads
  var a, b, xe_time, ye_time, d_time, roots, dx, x1, x2, rise;
  rise = 40;          //Declare just in case, this value will result if no rise/set so can be used as error code
  
  for (var i = 1; i <= 24; i = i + 2) {
    h1 = getMoonAltitude(i * 100, cCodeDate, latitude, longitude, UToffset) - hc;
    h2 = getMoonAltitude((i + 1) * 100, cCodeDate, latitude, longitude, UToffset) - hc;
    
    a = (h0 + h2) / 2 - h1;
    b = (h2 - h0) / 2;
    xe_time = -b / (2 * a);
    ye_time = (a * xe_time + b) * xe_time + h1;
    d_time = b * b - 4 * a * h1;
    roots = 0;
    
    if (d_time >= 0) {
        dx = Math.sqrt(d_time) / (Math.abs(a) * 2);
        x1 = xe_time - dx;
        x2 = xe_time + dx;
        
        if (Math.abs(x1) <= 1) {
          roots = roots + 1;
        }
        if (Math.abs(x2) <= 1) {
          roots = roots + 1;
        }
        if (x1 < -1) {
          x1 = x2;
        }
    }
    
    if (roots == 1) {
      if (h0 < 0) {
          rise = i + x1;
      } 
    } else if (roots == 2) {
        if (ye_time < 0) {
          rise = i + x2;
        } else {
          rise = i + x1;
        }
    }
    
    h0 = h2;
  }
  
  var minVar = parseInt(rise * 60) - parseInt(rise) * 60;
  rise = parseInt(rise) * 100 + minVar;    //Got the rise time in HHMM
  return rise;
}