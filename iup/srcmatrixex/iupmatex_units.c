/** \file
 * \brief IupMatrix Expansion Library.
 *
 * Original implementation from Bruno Kassar.
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <math.h>

#include "iup.h"
#include "iupcbs.h"

#include "iup_array.h"
#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_matrixex.h"

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif


enum /*QUANTITIES*/ {
  IMATEX_NOQUANTITY      ,
  IMATEX_LENGTH          ,
  IMATEX_TIME            ,
  IMATEX_MASS            ,
  IMATEX_TEMPERATURE     ,
  IMATEX_AREA            ,
  IMATEX_VOLUME          ,
  IMATEX_SPEED           ,
  IMATEX_ANGULAR_SPEED   ,
  IMATEX_ACCELERATION    ,
  IMATEX_PRESSURE        , /*PRESSURE OR MECHANICAL STRESS*/ 
  IMATEX_FORCE           ,
  IMATEX_FORCE_PER_LENGTH,
  IMATEX_MOMENTUM        ,
  IMATEX_ANGLE           ,
  IMATEX_SPECIFIC_MASS   ,
  IMATEX_SPECIFIC_WEIGHT ,
  IMATEX_ENERGY          ,
  IMATEX_POWER           ,
  IMATEX_FRACTION        ,
  IMATEX_QUANTITY_LAST   };

enum /*LENGTH*/ {
  IMATEX_METER      ,
  IMATEX_MILIMETER  ,
  IMATEX_CENTIMETER ,
  IMATEX_KILOMETER  ,
  IMATEX_INCH       ,
  IMATEX_FOOT       ,
  IMATEX_MILE       ,
  IMATEX_YARD       ,
  IMATEX_LENGTH_LAST};

enum /*TIME*/ {
  IMATEX_SECOND   ,
  IMATEX_MINUTE   ,
  IMATEX_HOUR     ,
  IMATEX_DAY      ,
  IMATEX_WEEK     ,
  IMATEX_TIME_LAST};                

enum /*MASS*/ {
  IMATEX_KILOGRAM ,
  IMATEX_GRAM     ,
  IMATEX_TON      ,
  IMATEX_POUND    ,
  IMATEX_OUNCE    ,
  IMATEX_MASS_LAST};                

enum /*TEMPERATURE*/ {                 
  IMATEX_KELVIN          ,
  IMATEX_CELSIUS         ,
  IMATEX_FAHRENHEIT      ,
  IMATEX_RANKINE         ,
  IMATEX_TEMPERATURE_LAST};                       

enum /*AREA*/ {
  IMATEX_METER2    ,
  IMATEX_INCH2     ,
  IMATEX_FOOT2     ,
  IMATEX_KILOMETER2,
  IMATEX_MILE2     ,
  IMATEX_YARD2     ,
  IMATEX_ACRE      ,
  IMATEX_HECTARE   ,
  IMATEX_AREA_LAST };

enum /*VOLUME*/ {
  IMATEX_METER3     ,
  IMATEX_INCH3      ,
  IMATEX_LITRE      ,
  IMATEX_GALLON     ,
  IMATEX_BARREL     ,/*PETROLEUM BARREL*/ 
  IMATEX_FOOT3      ,
  IMATEX_KILOMETER3 ,
  IMATEX_MILE3      ,
  IMATEX_YARD3      ,
  IMATEX_VOLUME_LAST};

enum /*SPEED*/ {
  IMATEX_METER_PER_SECOND  ,
  IMATEX_INCH_PER_SECOND   ,
  IMATEX_FOOT_PER_SECOND   ,
  IMATEX_KILOMETER_PER_HOUR,
  IMATEX_MILE_PER_HOUR     ,
  IMATEX_KNOT              ,
  IMATEX_SPEED_LAST        };

enum /*ANGULAR_SPEED*/ {
  IMATEX_RAD_PER_SECOND    ,
  IMATEX_ROT_PER_SECOND    ,
  IMATEX_DEG_PER_SECOND    ,
  IMATEX_RAD_PER_MINUTE    ,
  IMATEX_ROT_PER_MINUTE    ,
  IMATEX_DEG_PER_MINUTE    ,
  IMATEX_ANGULAR_SPEED_LAST};

enum /*ACCELERATION*/ {
  IMATEX_METER_PER_SECOND2,
  IMATEX_INCH_PER_SECOND2 ,
  IMATEX_KNOT_PER_SECOND  ,
  IMATEX_ACCELERATION_LAST};                        

enum /*PRESSURE OR MECHANICAL STRESS*/ {
  IMATEX_PASCAL         ,
  IMATEX_KILOPASCAL     ,
  IMATEX_ATMOSPHERE     ,
  IMATEX_MILIMETER_OF_HG,
  IMATEX_BAR            ,
  IMATEX_TORR           ,
  IMATEX_PRESSURE_LAST  };

enum /*FORCE*/ {
  IMATEX_NEWTON        ,
  IMATEX_KILONEWTON    ,
  IMATEX_KILOGRAM_FORCE,
  IMATEX_TON_FORCE     ,
  IMATEX_FORCE_LAST    }; 

enum /*FORCE_PER_LENGTH*/ {
  IMATEX_NEWTON_PER_METER        ,
  IMATEX_KILONEWTON_PER_METER    ,
  IMATEX_KILOGRAM_FORCE_PER_METER,
  IMATEX_TON_FORCE_PER_METER     ,
  IMATEX_FORCE_PER_LENGTH_LAST   }; 

enum /*MOMENTUM*/ {
  IMATEX_NEWTON_METER             ,
  IMATEX_KILOGRAM_FORCE_METER     ,
  IMATEX_TON_FORCE_METER          ,
  IMATEX_NEWTON_CENTIMETER        ,
  IMATEX_KILOGRAM_FORCE_CENTIMETER,
  IMATEX_TON_FORCE_CENTIMETER     ,
  IMATEX_KILONEWTON_METER         ,
  IMATEX_MOMENTUM_LAST            };

enum /*ANGLE*/ {
  IMATEX_RADIAN    ,
  IMATEX_DEGREE    ,
  IMATEX_ANGLE_LAST};

enum /*SPECIFIC MASS*/ {
  IMATEX_KILOGRAM_PER_M3    ,
  IMATEX_TON_PER_M3         ,
  IMATEX_KILOGRAM_PER_LITRE ,
  IMATEX_SPECIFIC_MASS_LAST };

enum /*SPECIFIC WEIGHT*/ {
  IMATEX_KILOGRAM_FORCE_PER_M3    ,
  IMATEX_TON_FORCE_PER_M3         ,
  IMATEX_KILOGRAM_FORCE_PER_LITRE ,
  IMATEX_SPECIFIC_WEIGHT_LAST     };

enum /*ENERGY*/ {
  IMATEX_JOULE           ,
  IMATEX_KILOJOULE       ,
  IMATEX_CALORIE         ,
  IMATEX_BTU             ,
  IMATEX_KILOWATT_HOUR   ,
  IMATEX_HORSEPOWER_HOUR ,
  IMATEX_ENERGY_LAST     };

enum /*POWER*/ {
  IMATEX_WATT              ,
  IMATEX_KILOWATT          ,
  IMATEX_CALORIE_PER_SECOND,
  IMATEX_HORSEPOWER        ,
  IMATEX_POWER_LAST        };

enum /*FRACTION*/ {
  IMATEX_PERCENTAGE   ,
  IMATEX_PER_ONE      ,
  IMATEX_PER_TEN      ,
  IMATEX_PER_THOUSAND ,
  IMATEX_FRACTION_LAST};

#define IMATEX_UNIT_FIELDS 2

const char *IMATEX_LENGTH_UNITS [IMATEX_LENGTH_LAST*IMATEX_UNIT_FIELDS] = {
  "meter"     ,"m" , 
  "milimeter" ,"mm", 
  "centimeter","cm",
  "kilometer" ,"km",
  "inch"      ,"in",
  "foot"      ,"ft",
  "mile"      ,"mi",
  "yard"      ,"yd"};

const char *IMATEX_TIME_UNITS [IMATEX_TIME_LAST*IMATEX_UNIT_FIELDS] = {
  "second","s"  ,
  "minute","min",
  "hour"  ,"h"  ,
  "day"   ,"d"  ,
  "week"  ,"wk" };

const char *IMATEX_MASS_UNITS [IMATEX_MASS_LAST*IMATEX_UNIT_FIELDS] = {
  "kilogram"   ,"kg" ,  
  "gram"       ,"g"  ,  
  "ton"        ,"ton",  
  "pound"      ,"lb" ,  /* metric */
  "ounce"      ,"oz" };

const char *IMATEX_TEMPERATURE_UNITS [IMATEX_TEMPERATURE_LAST*IMATEX_UNIT_FIELDS] = {
  "Kelvin"           ,"K" ,
  "degree Celsius"   ,"°C",
  "degree Fahrenheit","°F",
  "degree Rankine"   ,"°Ra"};

const char *IMATEX_AREA_UNITS [IMATEX_AREA_LAST*IMATEX_UNIT_FIELDS] = {
  "square meter"    ,"m²"   , 
  "square inch"     ,"sq in", 
  "square foot"     ,"sq ft", 
  "square kilometer","km²"  , 
  "square mile"     ,"sq mi", 
  "square yard"     ,"sq yd", 
  "acre"            ,"ac"   , 
  "hectare"         ,"ha"   };

const char *IMATEX_VOLUME_UNITS [IMATEX_VOLUME_LAST*IMATEX_UNIT_FIELDS] = {
  "cubic meter"    ,"m³"   ,
  "cubic inch"     ,"cu in",
  "litre"          ,"L"    ,
  "gallon"         ,"gal"  ,
  "barrel"         ,"bl"   ,
  "cubic foot"     ,"cu ft",
  "cubic kilometer","km³"  ,
  "cubic mile"     ,"cu mi",
  "cubic yard"     ,"cu yd"};

const char *IMATEX_SPEED_UNITS [IMATEX_SPEED_LAST*IMATEX_UNIT_FIELDS] = {
  "meter per second"  ,"m/s" ,
  "inch per second"   ,"ips" ,
  "foot per second"   ,"fps" ,
  "kilometer per hour","km/h",
  "mile per hour"     ,"mph" ,
  "knot"              ,"kn"  };

const char *IMATEX_ANGULAR_SPEED_UNITS [IMATEX_ANGULAR_SPEED_LAST*IMATEX_UNIT_FIELDS] = {
  "radian per second"  ,"rad/s"  ,
  "rotation per second","RPS"    ,
  "degree per second"  ,"deg/s"  ,
  "radian per minute"  ,"rad/min",
  "rotation per minute","RPM"    ,
  "degree per minute"  ,"deg/min"};

const char *IMATEX_ACCELERATION_UNITS [IMATEX_ACCELERATION_LAST*IMATEX_UNIT_FIELDS] = {
  "meter per second squared","m/s²",
  "inch per second squared" ,"ips²",
  "knot per second"         ,"kn/s"};

const char *IMATEX_PRESSURE_UNITS [IMATEX_PRESSURE_LAST*IMATEX_UNIT_FIELDS] = {
  "Pascal"              ,"Pa"  ,
  "kilopascal"          ,"kPa" ,
  "atmosphere"          ,"atm" ,
  "milimiter of mercury","mmHg",
  "bar"                 ,"bar" ,
  "torr"                ,"torr"};

const char *IMATEX_FORCE_UNITS [IMATEX_FORCE_LAST*IMATEX_UNIT_FIELDS] = {
  "Newton"        ,"N"  ,
  "Kilonewton"    ,"kN" ,
  "kilogram-force","kgf",
  "ton-force"     ,"tnf"};

const char *IMATEX_FORCE_PER_LENGTH_UNITS [IMATEX_FORCE_PER_LENGTH_LAST*IMATEX_UNIT_FIELDS] = {
  "Newton per meter"         ,"N/m"  ,
  "Kilonewton per meter"     ,"kN/m" ,
  "kilogram-force per meter" ,"kgf/m",
  "ton-force per meter"      ,"tnf/m"};

const char *IMATEX_MOMENTUM_UNITS [IMATEX_MOMENTUM_LAST*IMATEX_UNIT_FIELDS] = {
  "Newton meter"             , "N-m"   , 
  "kilogram-force meter"     , "kgf-m" , 
  "ton-force meter"          , "tnf-m" , 
  "Newton centimeter"        , "N-cm"  , 
  "kilogram-force centimeter", "kgf-cm", 
  "ton-force centimeter"     , "tnf-cm",
  "Kilonewton-meter"         , "kN-m"  };

const char *IMATEX_ANGLE_UNITS [IMATEX_ANGLE_LAST*IMATEX_UNIT_FIELDS] = {
  "radian", "rad",
  "degree", "deg"};

const char *IMATEX_SPECIFIC_MASS_UNITS [IMATEX_SPECIFIC_MASS_LAST*IMATEX_UNIT_FIELDS]  = {
  "kilogram per cubic meter","kg/m³" ,
  "ton per cubic meter"     ,"ton/m³",
  "kilogram per litre"      ,"kg/L"  };

const char *IMATEX_SPECIFIC_WEIGHT_UNITS [IMATEX_SPECIFIC_WEIGHT_LAST*IMATEX_UNIT_FIELDS]  = {
  "kilogram-force per cubic meter","kgf/m³",
  "ton-force per cubic meter"     ,"tnf/m³",
  "kilogram-force per litre"      ,"kgf/L" };

const char *IMATEX_ENERGY_UNITS[IMATEX_ENERGY_LAST*IMATEX_UNIT_FIELDS]  = {
  "Joule"          ,"J"   ,
  "Kilojoule"      ,"kJ"  ,
  "Calorie"        ,"cal" ,
  "BTU"            ,"BTU" ,
  "Kilowatt-hour"  ,"kW.h",
  "Horsepower-hour","hp.h"};
                                                 
const char *IMATEX_POWER_UNITS[IMATEX_POWER_LAST*IMATEX_UNIT_FIELDS]  = {
  "Watt"              ,"W"    ,
  "Kilowatt"          ,"kW"   ,
  "Calorie per second","cal/s",
  "Horsepower"         ,"hp"   };

const char *IMATEX_FRACTION_UNITS[IMATEX_FRACTION_LAST*IMATEX_UNIT_FIELDS]  = {
  "percentage"  , "%"    ,
  "per one"     , "/1"   ,
  "per ten"     , "/10"  ,
  "per thousand", "/1000"};

static double iMatrixExConvert_LENGTH (double a, int unit_from, int unit_to)
{
  double si;
  switch (unit_from)
  {
  case IMATEX_METER     : si = a           ; break;
  case IMATEX_MILIMETER : si = a * 0.001   ; break;
  case IMATEX_CENTIMETER: si = a * 0.01    ; break;
  case IMATEX_KILOMETER : si = a * 1000    ; break;
  case IMATEX_INCH      : si = a * 0.0254  ; break;
  case IMATEX_FOOT      : si = a * 0.3048  ; break;
  case IMATEX_MILE      : si = a * 1609.344; break;
  case IMATEX_YARD      : si = a * 0.9144  ; break;
  default: return a;                
  }                                                  
  switch (unit_to)
  {
  case IMATEX_METER     : return si;
  case IMATEX_MILIMETER : return si / 0.001   ;
  case IMATEX_CENTIMETER: return si / 0.01    ;
  case IMATEX_KILOMETER : return si / 1000    ;
  case IMATEX_INCH      : return si / 0.0254  ;
  case IMATEX_FOOT      : return si / 0.3048  ;
  case IMATEX_MILE      : return si / 1609.344;
  case IMATEX_YARD      : return si / 0.9144  ;
  default: return si;
  }
}

static double iMatrixExConvert_TIME  (double a, int unit_from, int unit_to)
{
  double si;
  switch (unit_from)
  {
  case IMATEX_SECOND: si = a ; break;
  case IMATEX_MINUTE: si = a * 60.00000 ; break;
  case IMATEX_HOUR  : si = a * 3600.000 ; break;
  case IMATEX_DAY   : si = a * 86400.00 ; break;
  case IMATEX_WEEK  : si = a * 604800.0 ; break;
  default: return a;
  }
  switch (unit_to)
  {
  case IMATEX_SECOND: return si;
  case IMATEX_MINUTE: return si * 0.016666667;
  case IMATEX_HOUR  : return si * 0.000277778;
  case IMATEX_DAY   : return si * 0.000011574;
  case IMATEX_WEEK  : return si * 0.000001653;
  default: return si;
  }
}

static double iMatrixExConvert_MASS  (double a, int unit_from, int unit_to)
{
  double si;
  switch (unit_from)
  {
  case IMATEX_KILOGRAM: si = a                  ;break;
  case IMATEX_GRAM    : si = a * 1.0e-3         ;break;
  case IMATEX_TON     : si = a * 1000.0000      ;break;
  case IMATEX_POUND   : si = a * 0.5000000      ;break;
  case IMATEX_OUNCE   : si = a * 28.349523125e-3;break;
  default: return a;
  }
  switch (unit_to)
  {
  case IMATEX_KILOGRAM: return si          ;
  case IMATEX_GRAM    : return si * 1.0e3  ;
  case IMATEX_TON     : return si * 1.0e-3 ;
  case IMATEX_POUND   : return si * 2      ;
  case IMATEX_OUNCE   : return si / 28.349523125e-3;
  default: return si;
  }
}

static double iMatrixExConvert_TEMPERATURE (double a, int unit_from, int unit_to)
{
  double si;
  switch (unit_from)
  {
  case IMATEX_KELVIN    : si = a                     ; break;
  case IMATEX_CELSIUS   : si = a + 273.15            ; break;
  case IMATEX_FAHRENHEIT: si = (a + 459.67)*0.5555556; break;
  case IMATEX_RANKINE   : si = a           *0.5555556; break;
  default: return a;
  }
  switch (unit_to)
  {
  case IMATEX_KELVIN    : return si;
  case IMATEX_CELSIUS   : return si - 273.15         ;
  case IMATEX_FAHRENHEIT: return si *1.80000 - 459.67;
  case IMATEX_RANKINE   : return si *1.80000         ;
  default: return si;
  }
}

static double iMatrixExConvert_AREA  (double a, int unit_from, int unit_to)
{
  double si;
  switch (unit_from)
  {
  case IMATEX_METER2    : si = a               ; break;
  case IMATEX_INCH2     : si = a * 647.70250e-6; break;
  case IMATEX_FOOT2     : si = a * 92.903040e-3; break;
  case IMATEX_KILOMETER2: si = a * 1.00000000e6; break;
  case IMATEX_MILE2     : si = a * 2.58998811e6; break;
  case IMATEX_YARD2     : si = a * 0.8394224400; break;
  case IMATEX_ACRE      : si = a * 4046.8564224; break;
  case IMATEX_HECTARE   : si = a * 1.0e4       ; break;
  default: return a;                    
  }                                                      
  switch (unit_to)
  {
  case IMATEX_METER2    : return si;
  case IMATEX_INCH2     : return si * 1.5439186970e3;
  case IMATEX_FOOT2     : return si * 10.76391042000;
  case IMATEX_KILOMETER2: return si * 1.000000000e-6;
  case IMATEX_MILE2     : return si * 0.386102158e-6;
  case IMATEX_YARD2     : return si * 1.191235291000;
  case IMATEX_ACRE      : return si / 4046.8564224  ;
  case IMATEX_HECTARE   : return si * 1.0e-4;
  default: return si;
  }
}

static double iMatrixExConvert_VOLUME (double a, int unit_from, int unit_to)
{
  double si;
  switch (unit_from)
  {
  case IMATEX_METER3    : si = a ; break;
  case IMATEX_INCH3     : si = a * 16.4840286300e-6; break;
  case IMATEX_LITRE     : si = a * 1.00000000000e-3; break;
  case IMATEX_GALLON    : si = a * 3.78541178400e-3; break; /*US fluid*/ 
  case IMATEX_BARREL    : si = a * 158.987294928e-3; break;
  case IMATEX_FOOT3     : si = a * 28.3168465920e-3; break;
  case IMATEX_KILOMETER3: si = a * 1.000000000000e9; break;
  case IMATEX_MILE3     : si = a * 4.168181825441e9; break;
  case IMATEX_YARD3     : si = a * 0.76455485798400; break;
  default: return a;
  }
  switch (unit_to)
  {
  case IMATEX_METER3    : return si;
  case IMATEX_INCH3     : return si / 16.4840286300e-6;
  case IMATEX_LITRE     : return si / 1.00000000000e-3;
  case IMATEX_GALLON    : return si / 3.78541178400e-3;
  case IMATEX_BARREL    : return si / 158.987294928e-3;
  case IMATEX_FOOT3     : return si / 28.3168465920e-3;
  case IMATEX_KILOMETER3: return si / 1.000000000000e9;
  case IMATEX_MILE3     : return si / 4.168181825441e9;
  case IMATEX_YARD3     : return si / 0.76455485798400;
  default: return si;
  }
}

static double iMatrixExConvert_SPEED (double a, int unit_from, int unit_to)
{
  double si;
  switch (unit_from)
  {
  case IMATEX_METER_PER_SECOND  : si = a ; break;
  case IMATEX_INCH_PER_SECOND   : si = a * 2.545e-2   ; break;
  case IMATEX_FOOT_PER_SECOND   : si = a * 0.304800   ; break;
  case IMATEX_KILOMETER_PER_HOUR: si = a * 2.777778e-1; break;
  case IMATEX_MILE_PER_HOUR     : si = a * 0.447040   ; break;
  case IMATEX_KNOT              : si = a * 0.514444   ; break;
  default: return a;
  }
  switch (unit_to)
  {
  case IMATEX_METER_PER_SECOND  : return si;
  case IMATEX_INCH_PER_SECOND   : return si / 25.45e-3   ;
  case IMATEX_FOOT_PER_SECOND   : return si / 0.304800   ;
  case IMATEX_KILOMETER_PER_HOUR: return si / 2.777778e-1;
  case IMATEX_MILE_PER_HOUR     : return si / 0.447040   ;
  case IMATEX_KNOT              : return si / 0.514444   ;
  default: return si;
  }
}

static double iMatrixExConvert_ANGULAR_SPEED(double a, int unit_from, int unit_to)
{
  double si;
  switch (unit_from)
  {
  case IMATEX_RAD_PER_SECOND: si = a ; break;
  case IMATEX_ROT_PER_SECOND: si = a * 2*M_PI   ; break;
  case IMATEX_DEG_PER_SECOND: si = a * M_PI/180 ; break;
  case IMATEX_RAD_PER_MINUTE: si = a * 60     ; break;
  case IMATEX_ROT_PER_MINUTE: si = a * 120*M_PI ; break;
  case IMATEX_DEG_PER_MINUTE: si = a * M_PI/3   ; break;
  default: return a;
  }
  switch (unit_to)
  {
  case IMATEX_RAD_PER_SECOND: return si;
  case IMATEX_ROT_PER_SECOND: return si / (2*M_PI  );
  case IMATEX_DEG_PER_SECOND: return si / (M_PI/180);
  case IMATEX_RAD_PER_MINUTE: return si / (60    );
  case IMATEX_ROT_PER_MINUTE: return si / (120*M_PI);
  case IMATEX_DEG_PER_MINUTE: return si / (M_PI/3  );
  default: return si;
  }
}

static double iMatrixExConvert_ACCELERATION(double a, int unit_from, int unit_to)
{
  double si;
  switch (unit_from)
  {
  case IMATEX_METER_PER_SECOND2: si = a ; break;
  case IMATEX_INCH_PER_SECOND2 : si = a * 2.545e-2   ; break;
  case IMATEX_KNOT_PER_SECOND  : si = a * 5.144444e-1; break;
  default: return a;
  }
  switch (unit_to)
  {
  case IMATEX_METER_PER_SECOND2: return si;
  case IMATEX_INCH_PER_SECOND2 : return si / 2.545e-2   ;
  case IMATEX_KNOT_PER_SECOND  : return si / 5.144444e-1;
  default: return si;
  }
}

static double iMatrixExConvert_PRESSURE (double a, int unit_from, int unit_to)
{
  double si;
  switch (unit_from)
  {
  case IMATEX_PASCAL         : si = a ; break;
  case IMATEX_KILOPASCAL     : si = a * 1000    ; break;
  case IMATEX_ATMOSPHERE     : si = a * 101325  ; break;
  case IMATEX_MILIMETER_OF_HG: si = a * 133.3224; break;
  case IMATEX_BAR            : si = a * 1.0e5   ; break;
  case IMATEX_TORR           : si = a * 133.3224; break;
  default: return a;
  }
  switch (unit_to)
  {
  case IMATEX_PASCAL         : return si;
  case IMATEX_KILOPASCAL     : return si * 0.001   ;
  case IMATEX_ATMOSPHERE     : return si / 101325  ;
  case IMATEX_MILIMETER_OF_HG: return si / 133.3224;
  case IMATEX_BAR            : return si / 1.0e5   ;
  case IMATEX_TORR           : return si / 133.3224;
  default: return si;
  }
}

static double iMatrixExConvert_FORCE (double a, int unit_from, int unit_to)
{
  double si;
  switch (unit_from)
  {
  case IMATEX_NEWTON        : si = a            ; break;
  case IMATEX_KILONEWTON    : si = a * 1.0e3    ; break;
  case IMATEX_KILOGRAM_FORCE: si = a * 9.80665  ; break;
  case IMATEX_TON_FORCE     : si = a * 9.80665e3; break;
  default: return a;
  }
  switch (unit_to)
  {
  case IMATEX_NEWTON        : return si           ;
  case IMATEX_KILONEWTON    : return si *1.0e-3   ;
  case IMATEX_KILOGRAM_FORCE: return si /9.80665  ;
  case IMATEX_TON_FORCE     : return si /9.80665e3;
  default: return si;
  }
}

static double iMatrixExConvert_FORCE_PER_LENGTH(double a, int unit_from, int unit_to)
{
  double si;
  switch (unit_from)
  {
  case IMATEX_NEWTON_PER_METER        : si = a            ; break;
  case IMATEX_KILONEWTON_PER_METER    : si = a * 1.0e3    ; break;
  case IMATEX_KILOGRAM_FORCE_PER_METER: si = a * 9.80665  ; break;
  case IMATEX_TON_FORCE_PER_METER     : si = a * 9.80665e3; break;
  default: return a;
  }
  switch (unit_to)
  {
  case IMATEX_NEWTON_PER_METER        : return si           ;
  case IMATEX_KILONEWTON_PER_METER    : return si *1.0e-3   ;
  case IMATEX_KILOGRAM_FORCE_PER_METER: return si /9.80665  ;
  case IMATEX_TON_FORCE_PER_METER     : return si /9.80665e3;
  default: return si;
  }
}

static double iMatrixExConvert_MOMENTUM (double a, int unit_from, int unit_to)
{
  double si;
  switch (unit_from)
  {
  case IMATEX_NEWTON_METER             : si = a ; break;
  case IMATEX_KILOGRAM_FORCE_METER     : si = a * 9.8066500; break;
  case IMATEX_TON_FORCE_METER          : si = a * 9.80665e3; break;
  case IMATEX_NEWTON_CENTIMETER        : si = a * 1.00000e2; break;
  case IMATEX_KILOGRAM_FORCE_CENTIMETER: si = a * 9.80665e2; break;
  case IMATEX_TON_FORCE_CENTIMETER     : si = a * 9.80665e5; break;
  case IMATEX_KILONEWTON_METER         : si = a * 1.00000e3; break;
  default: return a;
  }
  switch (unit_to)
  {
  case IMATEX_NEWTON_METER             : return si;
  case IMATEX_KILOGRAM_FORCE_METER     : return si / 9.8066500;
  case IMATEX_TON_FORCE_METER          : return si / 9.80665e3;
  case IMATEX_NEWTON_CENTIMETER        : return si * 1.0000e-2;
  case IMATEX_KILOGRAM_FORCE_CENTIMETER: return si / 9.80665e2;
  case IMATEX_TON_FORCE_CENTIMETER     : return si / 9.80665e5;
  case IMATEX_KILONEWTON_METER         : return si * 1.0000e-3; break;
  default: return si;
  }
}

static double iMatrixExConvert_ANGLE (double a, int unit_from, int unit_to)
{
  double si;
  switch (unit_from)
  {
  case IMATEX_RADIAN: si = a ; break;
  case IMATEX_DEGREE: si = a * 0.017453292; break;
  default: return a;
  }
  switch (unit_to)
  {
  case IMATEX_RADIAN: return si;
  case IMATEX_DEGREE: return si * 57.29577951;
  default: return si;
  }
}

static double iMatrixExConvert_SPECIFIC_MASS (double a, int unit_from, int unit_to)
{
  double si;
  switch(unit_from)
  {
  case IMATEX_KILOGRAM_PER_M3   : si = a         ; break;
  case IMATEX_TON_PER_M3        : si = a * 1.00e3; break;
  case IMATEX_KILOGRAM_PER_LITRE: si = a * 1.0e-3; break;
  default: return a;
  }
  switch(unit_to)
  {
  case IMATEX_KILOGRAM_PER_M3   : return si;
  case IMATEX_TON_PER_M3        : return si * 1.0e-3;
  case IMATEX_KILOGRAM_PER_LITRE: return si * 1.00e3;
  default: return si;
  }
}
static double iMatrixExConvert_SPECIFIC_WEIGHT(double a, int unit_from, int unit_to)
{
  double si;
  switch(unit_from)
  {
  case IMATEX_KILOGRAM_FORCE_PER_M3   : si = a; break;
  case IMATEX_TON_FORCE_PER_M3        : si = a * 1.00e3; break;
  case IMATEX_KILOGRAM_FORCE_PER_LITRE: si = a * 1.0e-3; break;
  default: return a;
  }
  switch(unit_to)
  {
  case IMATEX_KILOGRAM_FORCE_PER_M3   : return si;
  case IMATEX_TON_FORCE_PER_M3        : return si * 1.0e-3;
  case IMATEX_KILOGRAM_FORCE_PER_LITRE: return si * 1.00e3;
  default: return si;
  }
}

static double iMatrixExConvert_ENERGY   (double a, int unit_from, int unit_to)
{
  double si;
  switch(unit_from)
  {
  case IMATEX_JOULE          : si = a                         ; break;
  case IMATEX_KILOJOULE      : si = a * 1.0e3                 ; break;
  case IMATEX_CALORIE        : si = a * 4.1868                ; break;
  case IMATEX_BTU            : si = a * 1054.5                ; break;
  case IMATEX_KILOWATT_HOUR  : si = a * 3.6e6                 ; break;
  case IMATEX_HORSEPOWER_HOUR: si = a * 2.684519537696172792e6; break;
  default: return a;
  }
  switch(unit_to)
  {
  case IMATEX_JOULE          : return si;
  case IMATEX_KILOJOULE      : return si * 1.0e-3;
  case IMATEX_CALORIE        : return si / 4.1868                ;
  case IMATEX_BTU            : return si / 1054.5                ;
  case IMATEX_KILOWATT_HOUR  : return si / 3.6e6                 ;
  case IMATEX_HORSEPOWER_HOUR: return si / 2.684519537696172792e6;
  default: return si;
  }
}

static double iMatrixExConvert_POWER  (double a, int unit_from, int unit_to)
{
  double si;
  switch(unit_from)
  {
  case IMATEX_WATT              : si = a            ; break;
  case IMATEX_KILOWATT          : si = a * 1.00000e3; break;
  case IMATEX_CALORIE_PER_SECOND: si = a * 4.1868000; break;
  case IMATEX_HORSEPOWER        : si = a * 735.49875; break;
  default: return a;
  }
  switch(unit_to)
  {
  case IMATEX_WATT              : return si;
  case IMATEX_KILOWATT          : return si * 1.0000e-3;
  case IMATEX_CALORIE_PER_SECOND: return si / 4.1868000;
  case IMATEX_HORSEPOWER        : return si / 735.49875;
  default: return si;
  }
}

static double iMatrixExConvert_FRACTION (double a, int unit_from, int unit_to)
{
  double si;
  switch(unit_from)
  {
  case IMATEX_PERCENTAGE  : si = a   ; break;
  case IMATEX_PER_ONE     : si = a * 1.0e2; break;
  case IMATEX_PER_TEN     : si = a * 1.0e1; break;
  case IMATEX_PER_THOUSAND: si = a * 1.0e-1; break;
  default: return a;
  }
  switch(unit_to)
  {
  case IMATEX_PERCENTAGE  : return si;
  case IMATEX_PER_ONE     : return si *1.0e-2;
  case IMATEX_PER_TEN     : return si *1.0e-1;
  case IMATEX_PER_THOUSAND: return si *1.0e1 ;
  default: return si;
  }
}

/* Same definition as in IupMatrix */
typedef double (*ImatExConvertFunc)(double number, int unit_from, int unit_to);

typedef struct _ImatExQuantities {
  const char* name;
  const char** units;
  int units_count;
  ImatExConvertFunc convert_func;
} ImatExQuantities;

const ImatExQuantities IMATEX_QUANTITIES [IMATEX_QUANTITY_LAST] = {
  { "NONE"            , NULL                         , 0                            , NULL                              },
  { "Length"          , IMATEX_LENGTH_UNITS          , IMATEX_LENGTH_LAST           , iMatrixExConvert_LENGTH           },
  { "Time"            , IMATEX_TIME_UNITS            , IMATEX_TIME_LAST             , iMatrixExConvert_TIME             },
  { "Mass"            , IMATEX_MASS_UNITS            , IMATEX_MASS_LAST             , iMatrixExConvert_MASS             },
  { "Temperature"     , IMATEX_TEMPERATURE_UNITS     , IMATEX_TEMPERATURE_LAST      , iMatrixExConvert_TEMPERATURE      },
  { "Area"            , IMATEX_AREA_UNITS            , IMATEX_AREA_LAST             , iMatrixExConvert_AREA             },
  { "Volume"          , IMATEX_VOLUME_UNITS          , IMATEX_VOLUME_LAST           , iMatrixExConvert_VOLUME           },
  { "Speed"           , IMATEX_SPEED_UNITS           , IMATEX_SPEED_LAST            , iMatrixExConvert_SPEED            },
  { "Angular Speed"   , IMATEX_ANGULAR_SPEED_UNITS   , IMATEX_ANGULAR_SPEED_LAST    , iMatrixExConvert_ANGULAR_SPEED    },
  { "Acceleration"    , IMATEX_ACCELERATION_UNITS    , IMATEX_ACCELERATION_LAST     , iMatrixExConvert_ACCELERATION     },
  { "Pressure"        , IMATEX_PRESSURE_UNITS        , IMATEX_PRESSURE_LAST         , iMatrixExConvert_PRESSURE         },
  { "Force"           , IMATEX_FORCE_UNITS           , IMATEX_FORCE_LAST            , iMatrixExConvert_FORCE            },
  { "Force per length", IMATEX_FORCE_PER_LENGTH_UNITS, IMATEX_FORCE_PER_LENGTH_LAST , iMatrixExConvert_FORCE_PER_LENGTH },
  { "Momentum"        , IMATEX_MOMENTUM_UNITS        , IMATEX_MOMENTUM_LAST         , iMatrixExConvert_MOMENTUM         },
  { "Angle"           , IMATEX_ANGLE_UNITS           , IMATEX_ANGLE_LAST            , iMatrixExConvert_ANGLE            },
  { "Specific Mass"   , IMATEX_SPECIFIC_MASS_UNITS   , IMATEX_SPECIFIC_MASS_LAST    , iMatrixExConvert_SPECIFIC_MASS    },
  { "Specific Weight" , IMATEX_SPECIFIC_WEIGHT_UNITS , IMATEX_SPECIFIC_WEIGHT_LAST  , iMatrixExConvert_SPECIFIC_WEIGHT  },
  { "Energy"          , IMATEX_ENERGY_UNITS          , IMATEX_ENERGY_LAST           , iMatrixExConvert_ENERGY           },
  { "Power"           , IMATEX_POWER_UNITS           , IMATEX_POWER_LAST            , iMatrixExConvert_POWER            },
  { "Fraction"        , IMATEX_FRACTION_UNITS        , IMATEX_FRACTION_LAST         , iMatrixExConvert_FRACTION         },
};


/*****************************************************************************/


static int iMatrixFindQuantity(const char* value)
{
  int i;
  for (i=0; i<IMATEX_QUANTITY_LAST; i++)
  {
    if (iupStrEqualNoCaseNoSpace(IMATEX_QUANTITIES[i].name, value))
      return i;
  }

  return -1;
}

static int iMatrixFindUnit(const char** units, int units_count, const char* value)
{
  int i;
  for (i=0; i<units_count; i++)
  {
    if (iupStrEqualNoCaseNoSpace(units[i*IMATEX_UNIT_FIELDS+0], value))  /* field 0 is name */
      return i;
  }

  return -1;
}

static int iMatrixExSetNumericQuantityAttrib(Ihandle* ih, int col, const char* value)
{
  int quantity = iMatrixFindQuantity(value);
  if (quantity < 0)
    return 0;

  if (quantity == 0)
  {
    IupSetCallback(ih, "NUMERICCONVERT_FUNC", NULL);
    IupSetAttributeId(ih, "NUMERICQUANTITYINDEX", col, NULL);
  }
  else
  {
    /* set the callback before setting the attribute */
    IupSetCallback(ih, "NUMERICCONVERT_FUNC", (Icallback)IMATEX_QUANTITIES[quantity].convert_func);
    IupSetIntId(ih, "NUMERICQUANTITYINDEX", col, quantity);
  }

  return 1;
}

static char* iMatrixExGetNumericQuantityAttrib(Ihandle* ih, int col)
{
  int quantity = IupGetIntId(ih, "NUMERICQUANTITYINDEX", col);
  if (!quantity)
    return NULL;
  else
    return (char*)IMATEX_QUANTITIES[quantity].name;
}

static int iMatrixExSetNumericUnitAttrib(Ihandle* ih, int col, const char* value)
{
  int unit;
  int quantity = IupGetIntId(ih, "NUMERICQUANTITYINDEX", col);
  if (!quantity)
    return 0;

  unit = iMatrixFindUnit(IMATEX_QUANTITIES[quantity].units, IMATEX_QUANTITIES[quantity].units_count, value);
  if (unit < 0)
    return 0;

  IupSetIntId(ih, "NUMERICUNITINDEX", col, unit);
  return 1;
}

static char* iMatrixExGetNumericUnitAttrib(Ihandle* ih, int col)
{
  int quantity = IupGetIntId(ih, "NUMERICQUANTITYINDEX", col);
  if (!quantity)
    return NULL;
  else
  {
    int unit = IupGetIntId(ih, "NUMERICUNITINDEX", col);
    return (char*)IMATEX_QUANTITIES[quantity].units[unit*IMATEX_UNIT_FIELDS+0];  /* field 0 is name */
  }
}

static int iMatrixExSetNumericUnitShownAttrib(Ihandle* ih, int col, const char* value)
{
  int unit;
  int quantity = IupGetIntId(ih, "NUMERICQUANTITYINDEX", col);
  if (!quantity)
    return 0;

  unit = iMatrixFindUnit(IMATEX_QUANTITIES[quantity].units, IMATEX_QUANTITIES[quantity].units_count, value);
  if (unit < 0)
    return 0;

  IupSetIntId(ih, "NUMERICUNITSHOWNINDEX", col, unit);
  return 1;
}

static char* iMatrixExGetNumericUnitShownAttrib(Ihandle* ih, int col)
{
  int quantity = IupGetIntId(ih, "NUMERICQUANTITYINDEX", col);
  if (!quantity)
    return NULL;
  else
  {
    int unit = IupGetIntId(ih, "NUMERICUNITSHOWNINDEX", col);
    return (char*)IMATEX_QUANTITIES[quantity].units[unit*IMATEX_UNIT_FIELDS+0];  /* field 0 is name */
  }
}

static char* iMatrixExGetNumericUnitSymbolAttrib(Ihandle* ih, int col)
{
  int quantity = IupGetIntId(ih, "NUMERICQUANTITYINDEX", col);
  if (!quantity)
    return NULL;
  else
  {
    int unit = IupGetIntId(ih, "NUMERICUNITINDEX", col);
    return (char*)IMATEX_QUANTITIES[quantity].units[unit*IMATEX_UNIT_FIELDS+2];  /* field 1 is symbol */
  }
}

static char* iMatrixExGetNumericUnitShownSymbolAttrib(Ihandle* ih, int col)
{
  int quantity = IupGetIntId(ih, "NUMERICQUANTITYINDEX", col);
  if (!quantity)
    return NULL;
  else
  {
    int unit = IupGetIntId(ih, "NUMERICUNITSHOWNINDEX", col);
    return (char*)IMATEX_QUANTITIES[quantity].units[unit*IMATEX_UNIT_FIELDS+2];  /* field 1 is symbol */
  }
}

void iupMatrixExRegisterUnits(Iclass* ic)
{
  iupClassRegisterAttributeId(ic, "NUMERICQUANTITY", iMatrixExGetNumericQuantityAttrib, iMatrixExSetNumericQuantityAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "NUMERICUNIT", iMatrixExGetNumericUnitAttrib, iMatrixExSetNumericUnitAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "NUMERICUNITSHOWN", iMatrixExGetNumericUnitShownAttrib, iMatrixExSetNumericUnitShownAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "NUMERICUNITSYMBOL", iMatrixExGetNumericUnitSymbolAttrib, NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "NUMERICUNITSHOWNSYMBOL", iMatrixExGetNumericUnitShownSymbolAttrib, NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);
}
