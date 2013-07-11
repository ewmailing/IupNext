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

//TODO

//NUMERICVALUE_CB

// UTF-8

// The metre (International spelling as used by the International Bureau of Weights and Measures) or 
// meter (American spelling),
// litre liter

//  (m_types[cUnitType::BENDING_STIFFNESS]).add_unit("kN.m²", 1.0, 1.0);
//  (m_types[cUnitType::BENDING_STIFFNESS]).add_unit("N.m²" , 0.001, 1000.0);
//  (m_types[cUnitType::INERTIA]).add_unit("m^4", 1.0, 1.0);
//  (m_types[cUnitType::INERTIA_MASS]).add_unit("(kN.s²/m).m²", 1.0, 1.0);
//  (m_types[cUnitType::KINEMATIC_VISCOSITY]).add_unit("m²/s", 1.0, 1.0);
//  (m_types[cUnitType::ROTATIONAL_MASS]).add_unit("kN.m.s²/rad", 1.0, 1.0);
//  (m_types[cUnitType::ROTATIONAL_MASS]).add_unit("N.m.s²/rad", 1.0/1000.0, 1000.0);
//  (m_types[cUnitType::ROTATIONAL_MASS]).add_unit("kN.m.s²/deg", 1.0/1.745329e-2, 1.745329e-2);
//  (m_types[cUnitType::ROTATIONAL_MASS]).add_unit("N.m.s²/deg", 1.0/1.745329e+1, 1.745329e+1);
//  (m_types[cUnitType::ROTATIONAL_SPRING]).add_unit("kN.m/deg", 1.0, 1.0);
//  (m_types[cUnitType::ROTATIONAL_SPRING]).add_unit("N.m/deg", 1.0/1000.0, 1000.0);
//  (m_types[cUnitType::ROTATIONAL_SPRING]).add_unit("kN.m/rad", 1.745329e-2, 1.0/1.745329e-2);
//  (m_types[cUnitType::ROTATIONAL_SPRING]).add_unit("N.m/rad", 1.745329e-2/1000.0, 1000.0/1.745329e-2);
//  (m_types[cUnitType::TRANSLATIONAL_MASS]).add_unit("kN.s²/m", 1.0, 1.0);
//  (m_types[cUnitType::TRANSLATIONAL_MASS]).add_unit("N.s²/m", 1.0/1000.0, 1000.0);
//  (m_types[cUnitType::THERMAL_COEFFICIENT]).add_unit("1/°C", 1.0, 1.0);

#define GRAVITY 9.80665

#define IMATEX_LENGTH_COUNT 9
static const ImatExUnit IMATEX_LENGTH_UNITS [IMATEX_LENGTH_COUNT] = {
  {"metre"        ,"m" , 1},
  {"centimetre"   ,"cm", 0.01    },
  {"milimetre"    ,"mm", 0.001   },
  {"kilometre"    ,"km", 1000    },   /* International yard and pound agreement in 1959 */                
  {"inch"         ,"in", 0.0254  },   /* 0.0254                 (in = 25.4 mm)   */
  {"foot"         ,"ft", 0.3048  },   /* 0.0254 * 12            (ft = 12 in)     */
  {"yard"         ,"yd", 0.9144  },   /* 0.0254 * 12 * 3        (yd = 3 ft)      */
  {"mile"         ,"mi", 1609.344},   /* 0.0254 * 12 * 3 * 1760 (mi = 1760 yd)   */
  {"nautical mile","NM", 1853.184}};  /* 6080 ft */
                                   

#define IMATEX_TIME_COUNT 5                
static const ImatExUnit IMATEX_TIME_UNITS [IMATEX_TIME_COUNT] = {
  {"second","s"  , 1},
  {"minute","min", 60},
  {"hour"  ,"h"  , 3600},     /* 60 * 60 */
  {"day"   ,"d"  , 86400},    /* 60 * 60 * 24 */
  {"week"  ,"wk" , 604800}};  /* 60 * 60 * 24 * 7 */

#define IMATEX_MASS_COUNT 5               
static const ImatExUnit IMATEX_MASS_UNITS [IMATEX_MASS_COUNT] = {
  {"kilogram"   ,"kg" , 1},
  {"gram"       ,"g"  , 0.001           },
  {"ton"        ,"ton", 1000            },
  {"pound"      ,"lb" , 0.45359237      },  /* International yard and pound agreement in 1959 */
  {"ounce"      ,"oz" , 0.45359237/16.0 }}; /* (international avoirdupois) */

#define IMATEX_CELSIUS 1
#define IMATEX_FAHRENHEIT 2
#define IMATEX_TEMPERATURE_COUNT 4                       
static const ImatExUnit IMATEX_TEMPERATURE_UNITS [IMATEX_TEMPERATURE_COUNT] = {
  {"Kelvin"           ,"K"  , 1},
  {"degree Celsius"   ,"°C" , 1},          /* '°' \186=ISO8859-1  \xC2\xBA=UTF-8 */
  {"degree Fahrenheit","°F" , 5./9.},
  {"degree Rankine"   ,"°Ra", 5./9.}};

#define IMATEX_AREA_COUNT 10
static const ImatExUnit IMATEX_AREA_UNITS [IMATEX_AREA_COUNT] = {
  {"square metre"     ,"m²"   , 1},         /* '²'  \178=ISO8859-1  \xC2\xB2=UTF-8 */
  {"square centimetre","cm²"  , 1.0e-4},           /* 0.01² */
  {"square millimetre","mm²"  , 1.0e-6},           /* 0.001² */
  {"square kilometre" ,"km²"  , 1.0e6},            /* 1000² */
  {"square inch"      ,"sq in", 6.4516e-4 },       /* 0.0254² */
  {"square foot"      ,"sq ft", 9.290304e-2},      /* 0.3048² */   
  {"square yard"      ,"sq yd", 0.83612736},       /* 0.9144² */
  {"square mile"      ,"sq mi", 2.589988110336e6}, /* 1609.344² */
  {"acre"             ,"ac"   , 4046.8564224},     /* 0.9144² * 4840 (4840 sq yd) */
  {"hectare"          ,"ha"   , 1.0e4}};

#define IMATEX_VOLUME_COUNT 9
static const ImatExUnit IMATEX_VOLUME_UNITS [IMATEX_VOLUME_COUNT] = {
  {"cubic metre"    ,"m³"   , 1},           /* '³'  \179=ISO8859-1  \xC2\xB3=UTF-8 */
  {"cubic kilometre","km³"  , 1.0e9},
  {"cubic inch"     ,"cu in", 16.387064e-6},          /* 0.0254³ */
  {"cubic foot"     ,"cu ft", 0.028316846592},        /* 0.3048³ */   
  {"cubic yard"     ,"cu yd", 0.764554857984},        /* 0.9144³ */
  {"cubic mile"     ,"cu mi", 4168181825.440579584},  /* 1609.344³ */
  {"litre"          ,"L"    , 0.001           },
  {"gallon"         ,"gal"  , 3.785411784e-3},     /* (US fluid; Wine = 231 cu in) */
  {"barrel"         ,"bl"   , 0.158987294928}};    /* (petroleum = 42 gal) */

#define IMATEX_SPEED_COUNT 6
static const ImatExUnit IMATEX_SPEED_UNITS [IMATEX_SPEED_COUNT] = {
  {"metre per second"  ,"m/s" , 1},
  {"inch per second"   ,"ips" , 0.0254    },
  {"foot per second"   ,"fps" , 0.3048    },
  {"kilometre per hour","km/h", 1.0/3.6   },
  {"mile per hour"     ,"mph" , 0.44704   },    /* 1609.344 / 3600 */
  {"knot"              ,"kn"  , 1.852/3.6 }};   /* kn = 1 NM/h = 1.852 km/h */

#define IMATEX_ANGULAR_SPEED_COUNT 6
static const ImatExUnit IMATEX_ANGULAR_SPEED_UNITS [IMATEX_ANGULAR_SPEED_COUNT] = {
  {"radian per second"  ,"rad/s"  , 1},
  {"rotation per second","RPS"    , 2.0*M_PI  },
  {"degree per second"  ,"deg/s"  , M_PI/180.0},
  {"radian per minute"  ,"rad/min", 60        },
  {"rotation per minute","RPM"    , 120.0*M_PI},
  {"degree per minute"  ,"deg/min", M_PI/3.0  }};

#define IMATEX_ACCELERATION_COUNT 4
static const ImatExUnit IMATEX_ACCELERATION_UNITS [IMATEX_ACCELERATION_COUNT] = {
  {"metre per second squared","m/s²", 1},
  {"inch per second squared" ,"ips²", 0.0254   },
  {"knot per second"         ,"kn/s", 1.852/3.6 },
  {"mile per second squared" ,"mps²", 1609.344  }};

#define IMATEX_FORCE_COUNT 6
static const ImatExUnit IMATEX_FORCE_UNITS [IMATEX_FORCE_COUNT] = {
  {"Newton"        ,"N"  , 1},              /* N = kg·m/s² */
  {"Kilonewton"    ,"kN" , 1000     },
  {"kilogram-force","kgf", GRAVITY  },
  {"pound-force"   ,"lbf", GRAVITY * 0.45359237},          /* lbf = g × lb */
  {"kip-force"     ,"kip", GRAVITY * 0.45359237 * 1000},   /* kip = g × 1000 lb */
  {"ton-force"     ,"tnf", GRAVITY * 0.45359237 * 2000}};  /* tnf = g × 2000 lb */

#define IMATEX_PRESSURE_COUNT 8
static const ImatExUnit IMATEX_PRESSURE_UNITS [IMATEX_PRESSURE_COUNT] = {
  {"Pascal"               ,"Pa"  , 1},      /* Pa = N/m² = kg/(m·s²) */
  {"kilopascal"           ,"kPa" , 1000    },
  {"atmosphere"           ,"atm" , 101325  },   /* (standard) */
  {"milimiter of mercury" ,"mmHg", 133.322387415},    /* 13595.1 kg/m³ × mm × g = 13595.1 * 0.001 * GRAVITY */
  {"bar"                  ,"bar" , 1.0e5   },                                                       
  {"torr"                 ,"torr", 101325.0/760.0},   /* aprox 133.3224 */
  {"pound per square inch","psi" , 4.4482216152605/6.4516e-4},  /* psi = lbf/in² = GRAVITY*0.45359237/0,0254² */
  {"kip per square inch"  ,"ksi" , 4.4482216152605/6.4516e-1}}; /* ksi = kip/in² */          

#define IMATEX_FORCE_PER_LENGTH_COUNT 4            /* same as (Linear Weight) */
static const ImatExUnit IMATEX_FORCE_PER_LENGTH_UNITS [IMATEX_FORCE_PER_LENGTH_COUNT] = {
  {"Newton per metre"         ,"N/m"  , 1},      
  {"Kilonewton per metre"     ,"kN/m" , 1000    },
  {"kilogram-force per metre" ,"kgf/m", GRAVITY  },
  {"ton-force per metre"      ,"tnf/m", GRAVITY * 1000}};

#define IMATEX_MOMENT_COUNT 8           /* Torque */
static const ImatExUnit IMATEX_MOMENT_UNITS [IMATEX_MOMENT_COUNT] = {
  {"Newton metre"             , "N·m"   , 1},   /* '·'  \183=ISO8859-1  \xC2B7=UTF-8 */ 
  {"kilogram-force metre"     , "kgf·m" , GRAVITY  },
  {"ton-force metre"          , "tnf·m" , GRAVITY * 1000},
  {"Newton centimetre"        , "N·cm"  , 100      },
  {"kilogram-force centimetre", "kgf·cm", GRAVITY * 100},
  {"ton-force centimetre"     , "tnf·cm", GRAVITY * 1000 * 100},
  {"Kilonewton-metre"         , "kN·m"  , 1000     },
  {"metre kilogram"           , "m·kg"  , 1.0/GRAVITY}};

#define IMATEX_ANGLE_COUNT 2
static const ImatExUnit IMATEX_ANGLE_UNITS [IMATEX_ANGLE_COUNT] = {
  {"radian", "rad", 1},
  {"degree", "deg", M_PI/180.0}};

#define IMATEX_SPECIFIC_MASS_COUNT 3
static const ImatExUnit IMATEX_SPECIFIC_MASS_UNITS [IMATEX_SPECIFIC_MASS_COUNT]  = {
  {"kilogram per cubic metre","kg/m³" , 1},
  {"ton per cubic metre"     ,"ton/m³", 1000},
  {"kilogram per litre"      ,"kg/L"  , 0.001}};

#define IMATEX_SPECIFIC_WEIGHT_COUNT 6
static const ImatExUnit IMATEX_SPECIFIC_WEIGHT_UNITS [IMATEX_SPECIFIC_WEIGHT_COUNT]  = {
  {"Newton per cubic metre"        ,"N/m³"  , 1},      
  {"Kilonewton per cubic metre"    ,"kN/m³" , 1000},
  {"kilogram-force per cubic metre","kgf/m³", GRAVITY},
  {"ton-force per cubic metre"     ,"tnf/m³", GRAVITY * 1000},
  {"pound-force per cubic foot"    ,"lbf/ft³", (GRAVITY * 0.45359237) / 0.028316846592},
  {"kilogram-force per litre"      ,"kgf/L" , GRAVITY * 0.001}};
                                                              
#define IMATEX_ENERGY_COUNT   6
static const ImatExUnit IMATEX_ENERGY_UNITS[IMATEX_ENERGY_COUNT]  = {
  {"Joule"          ,"J"   , 1},                  /* J = m * N */
  {"Kilojoule"      ,"kJ"  , 1000                  },
  {"calorie"        ,"cal" , 4.1868                },  /* (International Table) */
  {"BTU"            ,"BTU" , 1.05505585262e3       },  /* (International Table) */
  {"Kilowatt-hour"  ,"kW·h", 3.6e6                 },
  {"horsepower-hour","hp·h", 2.684519537696172792e6}};
                                                               
#define IMATEX_POWER_COUNT    4
static const ImatExUnit IMATEX_POWER_UNITS[IMATEX_POWER_COUNT]  = {
  {"Watt"              ,"W"    , 1},               /* W = J/s */
  {"Kilowatt"          ,"kW"   , 1000},
  {"calorie per second","cal/s", 4.1868},  /* (International Table) */
  {"horsepower"        ,"hp"   , 745.69987158227022 }};  /* hp = 550 ft lbf/s (imperial mechanical)   */
                                                         /*      550 * 0.3048 *  9.80665 * 0.45359237 */   
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

#define IMATEX_TEMPERATURE 1
#define IMATEX_QUANTITY_COUNT 24
const ImatExQuantity IMATEX_QUANTITIES [IMATEX_QUANTITY_COUNT] = {
  { "NONE"             , NULL                         , 0                             },
  { "Temperature"      , IMATEX_TEMPERATURE_UNITS     , IMATEX_TEMPERATURE_COUNT      },  /* must not be change from here */
  { "Length"           , IMATEX_LENGTH_UNITS          , IMATEX_LENGTH_COUNT           },
  { "Time"             , IMATEX_TIME_UNITS            , IMATEX_TIME_COUNT             },
  { "Mass"             , IMATEX_MASS_UNITS            , IMATEX_MASS_COUNT             },
  { "Area"             , IMATEX_AREA_UNITS            , IMATEX_AREA_COUNT             },
  { "Volume"           , IMATEX_VOLUME_UNITS          , IMATEX_VOLUME_COUNT           },
  { "Speed"            , IMATEX_SPEED_UNITS           , IMATEX_SPEED_COUNT            },
  { "Velocity"         , IMATEX_SPEED_UNITS           , IMATEX_SPEED_COUNT            },
  { "Angular Speed"    , IMATEX_ANGULAR_SPEED_UNITS   , IMATEX_ANGULAR_SPEED_COUNT    },
  { "Acceleration"     , IMATEX_ACCELERATION_UNITS    , IMATEX_ACCELERATION_COUNT     },
  { "Pressure"         , IMATEX_PRESSURE_UNITS        , IMATEX_PRESSURE_COUNT         },
  { "Mechanical Stress", IMATEX_PRESSURE_UNITS        , IMATEX_PRESSURE_COUNT         },
  { "Force"            , IMATEX_FORCE_UNITS           , IMATEX_FORCE_COUNT            },
  { "Force per length" , IMATEX_FORCE_PER_LENGTH_UNITS, IMATEX_FORCE_PER_LENGTH_COUNT },
  { "Linear Weight"    , IMATEX_FORCE_PER_LENGTH_UNITS, IMATEX_FORCE_PER_LENGTH_COUNT },
  { "Torque"           , IMATEX_MOMENT_UNITS          , IMATEX_MOMENT_COUNT           },
  { "Moment of Force"  , IMATEX_MOMENT_UNITS          , IMATEX_MOMENT_COUNT           },
  { "Angle"            , IMATEX_ANGLE_UNITS           , IMATEX_ANGLE_COUNT            },
  { "Specific Mass"    , IMATEX_SPECIFIC_MASS_UNITS   , IMATEX_SPECIFIC_MASS_COUNT    },
  { "Specific Weight"  , IMATEX_SPECIFIC_WEIGHT_UNITS , IMATEX_SPECIFIC_WEIGHT_COUNT  },
  { "Energy"           , IMATEX_ENERGY_UNITS          , IMATEX_ENERGY_COUNT           },
  { "Power"            , IMATEX_POWER_UNITS           , IMATEX_POWER_COUNT            },
  { "Fraction"         , IMATEX_FRACTION_UNITS        , IMATEX_FRACTION_COUNT         },
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

  return 0;
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
  return 0;
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
  return 0;
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
