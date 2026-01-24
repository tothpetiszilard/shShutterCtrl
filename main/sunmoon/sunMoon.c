#include "sunMoon.h"
//#include <stdio.h>

typedef _Bool bool;

#define false (0)
#define true (!false)

#define SECS_PER_MIN  ((time_t)(60UL))
#define SECS_PER_HOUR ((time_t)(3600UL))
#define SECS_PER_DAY  ((time_t)(SECS_PER_HOUR * 24UL))

// leap year calulator expects year argument as years offset from 1970
#define LEAP_YEAR(Y)     ( ((1970+(Y))>0) && !((1970+(Y))%4) && ( ((1970+(Y))%100) || !((1970+(Y))%400) ) )

typedef struct
{
  uint8_t Second;
  uint8_t Minute;
  uint8_t Hour;
  uint8_t Wday;   // day of week, sunday is day 1
  uint8_t Day;
  uint8_t Month;
  uint8_t Year;   // offset from 1970;
} 	tmElements_t;

const float toRad = M_PI/180.0;
const float toDeg = 180.0/M_PI;
const float twoPi = 2 * M_PI;
const float zenith = 90.83 * toRad;
/* zenith:                Sun's zenith for sunrise/sunset
    offical      = 90 degrees 50'
    civil        = 96 degrees
    nautical     = 102 degrees
    astronomical = 108 degrees
*/
static const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; // API starts months from 1, this array starts from 0

static int tz; // GMP offset in minutes
static float longitude, latitude;
static time_t sunTime(bool sunRise, time_t date);
static float normalize(float v);
static time_t makeTime(const tmElements_t * tm);  // convert time elements into time_t
static void breakTime(time_t timeInput, tmElements_t * tm);

StdReturnType SunMoon_Init(int Timezone, float Latitude, float Longitude)
{

  if ((Timezone < - 720) || (Timezone > 720))  return E_NOT_OK;
  if ((Longitude < -180) || (Longitude > 180)) return E_NOT_OK;
  if ((Latitude < -90) || (Latitude > 90))     return E_NOT_OK;
  tz = Timezone;
  longitude = Longitude;
  latitude = Latitude;
  return E_OK;
}

uint32_t SunMoon_JulianDay(time_t date)
{

  if (date == 0) date = now();
  date -= tz*60;
  long y = year(date);
  long m = month(date);
  if (m > 2)
  {
    m = m - 3;
  }
  else
  {
    m = m + 9;
    y--;
  }
  long c = y / 100L;          // Compute century
  y -= 100L * c;
  return ((uint32_t)day(date) + (c * 146097L) / 4 + (y * 1461L) / 4 + (m * 153L + 2) / 5 + 1721119L);
}

uint8_t SunMoon_MoonDay(time_t date)
{
	float IP = normalize((SunMoon_JulianDay(date) - 2451550.1) / 29.530588853);
	IP *= 29.530588853;
	uint8_t age = (uint8_t)IP;
	return age + 1;
}

forecast_t SunMoon_DayForecast(int8_t mDay)
{
  static forecast_t f[30] =
  {
    day_unhappy, day_happy, day_dangerous, day_happy, day_unhappy,
    day_happiest, day_happy, day_happy, day_normal, day_happiest,
    day_happy, day_unhappy, day_dangerous, day_happy, day_normal,
    day_happiest, day_dangerous, day_happy, day_dangerous, day_happiest,
    day_happy, day_unhappy, day_happy, day_normal, day_unhappy,
    day_unhappy, day_happiest, day_happy, day_unhappy, day_happy
  };

  if (mDay < 0) mDay =  SunMoon_MoonDay(0);
  if ((mDay > 29) || mDay < 0) return day_normal;
  return f[mDay];

}

time_t SunMoon_SunRise(time_t date)
{
  return sunTime(true, date);
}

time_t SunMoon_SunSet(time_t date)
{
  return sunTime(false, date);
}

static time_t sunTime(bool sunRise, time_t date)
{

  if (date == 0) date = now();
  // Calculate the sunrise and sunset times for date and 'noon time'
  tmElements_t tm;
  breakTime(date, &tm);
  tm.Hour   = 12;
  tm.Minute = 0;
  tm.Second = 0;
  date = makeTime(&tm);
  date -= tz*60;

  // first calculate the day of the year
  int N1 = 275 * (month(date)) / 9;
  int N2 = (month(date)+9)/12;
  int N3 = 1 + (year(date) - 4 * (year(date) / 4) + 2) / 3;
  int N = N1 - (N2 * N3) + day(date) - 30;
  
  // convert the longitude to hour value and calculate an approximate time
  float lngHour = longitude / 15.0;
  float t = 0;
  if (sunRise) 
    t = N + ((6 - lngHour) / 24);
  else
    t = N + ((18 - lngHour) / 24);

  // Sun's mean anomaly
  float M = (0.9856 * t) - 3.289;
  M *= toRad;

  // the Sun's true longitude
  float L = M + (1.916*toRad * sin(M)) + (0.020*toRad * sin(2 * M)) + 282.634*toRad;
  if (L < 0)      L += twoPi;
  if (L > twoPi) L -= twoPi;

  // the Sun's right ascension
  float RA = toDeg*atan(0.91764 * tan(L));
  if (RA < 0)   RA += 360;
  if (RA > 360) RA -= 360;

  // right ascension value needs to be in the same quadrant as L
  int Lquadrant  = (floor( L/M_PI_2)) * 90;
  int RAquadrant = (floor(RA/90)) * 90;
  RA += Lquadrant - RAquadrant;
  RA /= 15;         // right ascension value needs to be converted into hours

  // calculate the Sun's declination
  float sinDec = 0.39782 * sin(L);
  float cosDec = cos(asin(sinDec));

//float decl = toDeg*asin(sinDec);

  // calculate the Sun's local hour angle
  float cosH = (cos(zenith) - (sinDec * sin(latitude*toRad))) / (cosDec * cos(latitude*toRad));
  if (cosH >  1) return 0;            // the Sun never rises on this location on the specified date
  if (cosH < -1) return 0;            // the Sun never sets on this location on the specified date

  // finish calculating H and convert into hours
  float H = 0;
  if (sunRise) 
    H = 360 - toDeg*acos(cosH);
  else
    H = toDeg*acos(cosH);
  H /= 15;

  // calculate local mean time of rising/setting
  float T = H + RA - (0.06571 * t) - 6.622;
  if (T < 0)  T += 24;
  if (T > 24) T -= 24;
  float UT = T - lngHour;
  float localT = UT + (float)tz / 60.0;

  tm.Hour = (uint8_t)localT;
  localT -= tm.Hour;
  localT *= 60;
  tm.Minute = (uint8_t)localT;
  localT -= tm.Minute;
  localT *= 60;
  tm.Second = (uint8_t)localT;

  time_t ret = makeTime(&tm);
  return ret;  
}

static float normalize(float v)
{
  v -= floor(v); 
  if (v < 0) v += 1;
  return v;
}

static time_t makeTime(const tmElements_t *tm)
{
// assemble time elements into time_t
// note year argument is offset from 1970 (see macros in time.h to convert to other formats)
// previous version used full four digit year (or digits since 2000),i.e. 2009 was 2009 or 9

  int i;
  uint32_t seconds;

  // seconds from 1970 till 1 jan 00:00:00 of the given year
  seconds= (*tm).Year*(SECS_PER_DAY * 365);
  for (i = 0; i < (*tm).Year; i++)
  {
    if (LEAP_YEAR(i))
    {
      seconds +=  SECS_PER_DAY;   // add extra days for leap years
    }
  }

  // add days for this year, months start from 1
  for (i = 1; i < (*tm).Month; i++)
  {
    if ( (i == 2) && LEAP_YEAR((*tm).Year))
    {
      seconds += SECS_PER_DAY * 29;
    }
    else
    {
      seconds += SECS_PER_DAY * monthDays[i-1];  //monthDay array starts from 0
    }
  }
  seconds+= ((*tm).Day-1) * SECS_PER_DAY;
  seconds+= (*tm).Hour * SECS_PER_HOUR;
  seconds+= (*tm).Minute * SECS_PER_MIN;
  seconds+= (*tm).Second;
  return (time_t)seconds;
}

static void breakTime(time_t timeInput, tmElements_t * tm)
{
// break the given time_t into time components
// this is a more compact version of the C library localtime function
// note that year is offset from 1970 !!!

  uint8_t year;
  uint8_t month, monthLength;
  uint32_t time;
  unsigned long days;

  time = (uint32_t)timeInput;
  (*tm).Second = time % 60;
  time /= 60; // now it is minutes
  (*tm).Minute = time % 60;
  time /= 60; // now it is hours
  (*tm).Hour = time % 24;
  time /= 24; // now it is days
  (*tm).Wday = ((time + 4) % 7) + 1;  // Sunday is day 1

  year = 0;
  days = 0;
  while((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time) {
    year++;
  }
  (*tm).Year = year; // year is offset from 1970

  days -= LEAP_YEAR(year) ? 366 : 365;
  time  -= days; // now it is days in this year, starting at 0

  days=0;
  month=0;
  monthLength=0;
  for (month=0; month<12; month++) {
    if (month==1) { // february
      if (LEAP_YEAR(year)) {
        monthLength=29;
      } else {
        monthLength=28;
      }
    } else {
      monthLength = monthDays[month];
    }

    if (time >= monthLength) {
      time -= monthLength;
    } else {
        break;
    }
  }
  (*tm).Month = month + 1;  // jan is month 1
  (*tm).Day = time + 1;     // day of month
}
