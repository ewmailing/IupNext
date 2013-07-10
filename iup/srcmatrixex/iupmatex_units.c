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

/* Source for conversion names, symbols and factors:
   http://en.wikipedia.org/wiki/Conversion_of_units
*/

typedef struct _ImatExUnit {
  const char* name;
  const char* symbol;
  double factor;
} ImatExUnit;

#define IMATEX_LENGTH_COUNT 8
static const ImatExUnit IMATEX_LENGTH_UNITS [IMATEX_LENGTH_COUNT] = {
  {"meter"     ,"m" , 1},
  {"milimeter" ,"mm", 0.001   },
  {"centimeter","cm", 0.01    },
  {"kilometer" ,"km", 1000    },
  {"inch"      ,"in", 0.0254  },   /* (international) */
  {"foot"      ,"ft", 0.3048  },   /* (international) */
  {"mile"      ,"mi", 1609.344},   /* (international) */
  {"yard"      ,"yd", 0.9144  }};  /* (international) */

#define IMATEX_TIME_COUNT 5                
static const ImatExUnit IMATEX_TIME_UNITS [IMATEX_TIME_COUNT] = {
  {"second","s"  , 1},
  {"minute","min", 60},
  {"hour"  ,"h"  , 3600},
  {"day"   ,"d"  , 86400},
  {"week"  ,"wk" , 604800}};

#define IMATEX_MASS_COUNT 5               
static const ImatExUnit IMATEX_MASS_UNITS [IMATEX_MASS_COUNT] = {
  {"kilogram"   ,"kg" , 1},
  {"gram"       ,"g"  , 0.001           },
  {"ton"        ,"ton", 1000            },
  {"pound"      ,"lb" , 0.45359237      },  /* (international avoirdupois) */
  {"ounce"      ,"oz" , 0.45359237/16.0 }}; /* (international avoirdupois) */

#define IMATEX_CELSIUS 1
#define IMATEX_FAHRENHEIT 2
#define IMATEX_TEMPERATURE_COUNT 4                       
static const ImatExUnit IMATEX_TEMPERATURE_UNITS [IMATEX_TEMPERATURE_COUNT] = {
  {"Kelvin"           ,"K"  , 1},
  {"degree Celsius"   ,"°C" , 1},          /* ° \186=ISO8859-1  \xC2\xBA=UTF-8 */
  {"degree Fahrenheit","°F" , 5./9.},
  {"degree Rankine"   ,"°Ra", 5./9.}};

#define IMATEX_AREA_COUNT 8
static const ImatExUnit IMATEX_AREA_UNITS [IMATEX_AREA_COUNT] = {
  {"square meter"    ,"m²"   , 1},         /* ²  \178=ISO8859-1  \xC2\xB2=UTF-8 */
  {"square inch"     ,"sq in", 6.4516e-4},
  {"square foot"     ,"sq ft", 9.290304e-2},
  {"square kilometer","km²"  , 1.0e6},
  {"square mile"     ,"sq mi", 2.589988110336e6},
  {"square yard"     ,"sq yd", 0.83612736},    /* (international) */
  {"acre"            ,"ac"   , 4046.8564224},  /* (international) */
  {"hectare"         ,"ha"   , 10000       }};

#define IMATEX_VOLUME_COUNT 9
static const ImatExUnit IMATEX_VOLUME_UNITS [IMATEX_VOLUME_COUNT] = {
  {"cubic meter"    ,"m³"   , 1},           /* ³  \179=ISO8859-1  \xC2\xB3=UTF-8 */
  {"cubic inch"     ,"cu in", 16.387064e-6    },
  {"litre"          ,"L"    , 0.001           },
  {"gallon"         ,"gal"  , 3.785411784e-3},  /* (US fluid; Wine) */
  {"barrel"         ,"bl"   , 158.987294928e-3},  /* (petroleum) */
  {"cubic foot"     ,"cu ft", 0.028316846592},
  {"cubic kilometer","km³"  , 1.0e9},
  {"cubic mile"     ,"cu mi", 4168181825.440579584},
  {"cubic yard"     ,"cu yd", 0.764554857984}};

#define IMATEX_SPEED_COUNT 6
static const ImatExUnit IMATEX_SPEED_UNITS [IMATEX_SPEED_COUNT] = {
  {"meter per second"  ,"m/s" , 1},
  {"inch per second"   ,"ips" , 2.54e-2   },
  {"foot per second"   ,"fps" , 0.3048    },
  {"kilometer per hour","km/h", 1.0/3.6   },
  {"mile per hour"     ,"mph" , 0.44704   },
  {"knot"              ,"kn"  , 1.852/3.6 }};

#define IMATEX_ANGULAR_SPEED_COUNT 6
static const ImatExUnit IMATEX_ANGULAR_SPEED_UNITS [IMATEX_ANGULAR_SPEED_COUNT] = {
  {"radian per second"  ,"rad/s"  , 1},
  {"rotation per second","RPS"    , 2.0*M_PI  },
  {"degree per second"  ,"deg/s"  , M_PI/180.0},
  {"radian per minute"  ,"rad/min", 60        },
  {"rotation per minute","RPM"    , 120.0*M_PI},
  {"degree per minute"  ,"deg/min", M_PI/3.0  }};

#define IMATEX_ACCELERATION_COUNT 3
static const ImatExUnit IMATEX_ACCELERATION_UNITS [IMATEX_ACCELERATION_COUNT] = {
  {"meter per second squared","m/s²", 1},
  {"inch per second squared" ,"ips²", 2.54e-2   },
  {"knot per second"         ,"kn/s", 1.852/3.6}};

#define IMATEX_PRESSURE_COUNT 6
static const ImatExUnit IMATEX_PRESSURE_UNITS [IMATEX_PRESSURE_COUNT] = {
  {"Pascal"              ,"Pa"  , 1},
  {"kilopascal"          ,"kPa" , 1000    },
  {"atmosphere"          ,"atm" , 101325  },   /* (standard) */
  {"milimiter of mercury","mmHg", 101325.0/760.0},
  {"bar"                 ,"bar" , 1.0e5   },
  {"torr"                ,"torr", 101325.0/760.0}};

#define IMATEX_FORCE_COUNT 4
static const ImatExUnit IMATEX_FORCE_UNITS [IMATEX_FORCE_COUNT] = {
  {"Newton"        ,"N"  , 1},
  {"Kilonewton"    ,"kN" , 1000     },
  {"kilogram-force","kgf", 9.80665  },
  {"ton-force"     ,"tnf", 9.80665 * 1000}};  /* same as tonne (metric) */

#define IMATEX_FORCE_PER_LENGTH_COUNT 4
static const ImatExUnit IMATEX_FORCE_PER_LENGTH_UNITS [IMATEX_FORCE_PER_LENGTH_COUNT] = {
  {"Newton per meter"         ,"N/m"  , 1},
  {"Kilonewton per meter"     ,"kN/m" , 1000    },
  {"kilogram-force per meter" ,"kgf/m", 9.80665  },
  {"ton-force per meter"      ,"tnf/m", 9.80665 * 1000}};

#define IMATEX_MOMENTUM_COUNT 7
static const ImatExUnit IMATEX_MOMENTUM_UNITS [IMATEX_MOMENTUM_COUNT] = {
  {"Newton meter"             , "N-m"   , 1},
  {"kilogram-force meter"     , "kgf-m" , 9.80665  },
  {"ton-force meter"          , "tnf-m" , 9.80665 * 1000},
  {"Newton centimeter"        , "N-cm"  , 100      },
  {"kilogram-force centimeter", "kgf-cm", 9.80665 * 100},
  {"ton-force centimeter"     , "tnf-cm", 9.80665 * 1000 * 100},
  {"Kilonewton-meter"         , "kN-m"  , 1000     }};

#define IMATEX_ANGLE_COUNT 2
static const ImatExUnit IMATEX_ANGLE_UNITS [IMATEX_ANGLE_COUNT] = {
  {"radian", "rad", 1},
  {"degree", "deg", M_PI/180.0}};

#define IMATEX_SPECIFIC_MASS_COUNT 3
static const ImatExUnit IMATEX_SPECIFIC_MASS_UNITS [IMATEX_SPECIFIC_MASS_COUNT]  = {
  {"kilogram per cubic meter","kg/m³" , 1},
  {"ton per cubic meter"     ,"ton/m³", 1000},
  {"kilogram per litre"      ,"kg/L"  , 0.001}};

#define IMATEX_SPECIFIC_WEIGHT_COUNT 3
static const ImatExUnit IMATEX_SPECIFIC_WEIGHT_UNITS [IMATEX_SPECIFIC_WEIGHT_COUNT]  = {
  {"kilogram-force per cubic meter","kgf/m³", 1},
  {"ton-force per cubic meter"     ,"tnf/m³", 1000},
  {"kilogram-force per litre"      ,"kgf/L" , 0.001}};

#define IMATEX_ENERGY_COUNT   6
static const ImatExUnit IMATEX_ENERGY_UNITS[IMATEX_ENERGY_COUNT]  = {
  {"Joule"          ,"J"   , 1},
  {"Kilojoule"      ,"kJ"  , 1000                  },
  {"calorie"        ,"cal" , 4.1868                },  /* (International Table) */
  {"BTU"            ,"BTU" , 1054.5                },  /* (ISO) */
  {"Kilowatt-hour"  ,"kW.h", 3.6e6                 },
  {"horsepower-hour","hp.h", 2.684519537696172792e6}};
                                                 
#define IMATEX_POWER_COUNT    4
static const ImatExUnit IMATEX_POWER_UNITS[IMATEX_POWER_COUNT]  = {
  {"Watt"              ,"W"    , 1},
  {"Kilowatt"          ,"kW"   , 1000},
  {"calorie per second","cal/s", 4.1868},  /* (International Table) */
  {"horsepower"         ,"hp"  , 735.49875 }};  /* (metric) */

#define IMATEX_FRACTION_COUNT 4
static const ImatExUnit IMATEX_FRACTION_UNITS[IMATEX_FRACTION_COUNT]  = {
  {"percentage"  , "%"    , 1},
  {"per one"     , "/1"   , 100 },
  {"per ten"     , "/10"  , 10  },
  {"per thousand", "/1000", 0.1 }};

typedef struct _ImatExQuantity {
  const char* name;
  const ImatExUnit* units;
  int units_count;
} ImatExQuantity;

#define IMATEX_TEMPERATURE 4
#define IMATEX_QUANTITY_COUNT 20
const ImatExQuantity IMATEX_QUANTITIES [IMATEX_QUANTITY_COUNT] = {
  { "NONE"            , NULL                         , 0                            },
  { "Length"          , IMATEX_LENGTH_UNITS          , IMATEX_LENGTH_COUNT           },
  { "Time"            , IMATEX_TIME_UNITS            , IMATEX_TIME_COUNT             },
  { "Mass"            , IMATEX_MASS_UNITS            , IMATEX_MASS_COUNT             },
  { "Temperature"     , IMATEX_TEMPERATURE_UNITS     , IMATEX_TEMPERATURE_COUNT      },
  { "Area"            , IMATEX_AREA_UNITS            , IMATEX_AREA_COUNT             },
  { "Volume"          , IMATEX_VOLUME_UNITS          , IMATEX_VOLUME_COUNT           },
  { "Speed"           , IMATEX_SPEED_UNITS           , IMATEX_SPEED_COUNT            },
  { "Angular Speed"   , IMATEX_ANGULAR_SPEED_UNITS   , IMATEX_ANGULAR_SPEED_COUNT    },
  { "Acceleration"    , IMATEX_ACCELERATION_UNITS    , IMATEX_ACCELERATION_COUNT     },
  { "Pressure"        , IMATEX_PRESSURE_UNITS        , IMATEX_PRESSURE_COUNT         },
  { "Force"           , IMATEX_FORCE_UNITS           , IMATEX_FORCE_COUNT            },
  { "Force per length", IMATEX_FORCE_PER_LENGTH_UNITS, IMATEX_FORCE_PER_LENGTH_COUNT },
  { "Momentum"        , IMATEX_MOMENTUM_UNITS        , IMATEX_MOMENTUM_COUNT         },
  { "Angle"           , IMATEX_ANGLE_UNITS           , IMATEX_ANGLE_COUNT            },
  { "Specific Mass"   , IMATEX_SPECIFIC_MASS_UNITS   , IMATEX_SPECIFIC_MASS_COUNT    },
  { "Specific Weight" , IMATEX_SPECIFIC_WEIGHT_UNITS , IMATEX_SPECIFIC_WEIGHT_COUNT  },
  { "Energy"          , IMATEX_ENERGY_UNITS          , IMATEX_ENERGY_COUNT           },
  { "Power"           , IMATEX_POWER_UNITS           , IMATEX_POWER_COUNT            },
  { "Fraction"        , IMATEX_FRACTION_UNITS        , IMATEX_FRACTION_COUNT         },
};


/* Same definition as in IupMatrix */
static double iMatrixConvertFunc(double number, int quantity, int unit_from, int unit_to)
{
  /* this function is called only when unit_from!=unit_to */
  const ImatExUnit* units = IMATEX_QUANTITIES[quantity].units;

  if (quantity == IMATEX_TEMPERATURE)
  {
    if (unit_from!=0)
    {
      if (unit_from==IMATEX_CELSIUS)
        number = number + 273.15;
      else if (unit_from==IMATEX_FAHRENHEIT)
        number = number + 459.67;

      number = number * units[unit_from].factor;
    }

    if (unit_to!=0)
    {
      number = number / units[unit_to].factor;

      if (unit_from==IMATEX_CELSIUS)
        number = number - 273.15;
      else if (unit_from==IMATEX_FAHRENHEIT)
        number = number - 459.67;
    }
  }
  else
  {
    if (unit_from!=0)
      number = number * units[unit_from].factor;

    if (unit_to!=0)
      number = number / units[unit_to].factor;
  }

  return number;
}


/*****************************************************************************/


static int iMatrixFindQuantity(const char* value)
{
  int i;
  for (i=0; i<IMATEX_QUANTITY_COUNT; i++)
  {
    if (iupStrEqualNoCaseNoSpace(IMATEX_QUANTITIES[i].name, value))
      return i;
  }

  return -1;
}

static int iMatrixFindUnit(const ImatExUnit* units, int units_count, const char* value)
{
  int i;
  for (i=0; i<units_count; i++)
  {
    if (iupStrEqualNoCaseNoSpace(units[i].name, value))  /* field 0 is name */
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
    IupSetCallback(ih, "NUMERICCONVERT_FUNC", (Icallback)iMatrixConvertFunc);
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
    return (char*)IMATEX_QUANTITIES[quantity].units[unit].name;
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
    return (char*)IMATEX_QUANTITIES[quantity].units[unit].name;
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
    return (char*)IMATEX_QUANTITIES[quantity].units[unit].symbol;
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
    return (char*)IMATEX_QUANTITIES[quantity].units[unit].symbol;
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
