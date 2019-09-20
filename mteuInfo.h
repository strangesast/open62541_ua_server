#include <string>
#include <map>

class EuInfo {
public:
   string m_code;
   int m_unitId;
   string m_displayName;
   string m_description;

public:
   EuInfo(string code, int unitId, string displayName, string description)
   {
       m_code = code;
       m_unitId = unitId;
       m_displayName = displayName;
       m_description = description;
   }
};

static map<string, EuInfo *> euStore = {
    {"AMPERE", new EuInfo("AMP", 4279632, "A", "Amps") },
    {"CELSIUS", new EuInfo("CEL", 4408652, "◦Celsuis", "Degrees Celsius") },
    {"COUNT", new EuInfo("", 0, "", "Count") },
    {"DECIBEL", new EuInfo("2N", 12878, "dB", "Sound Level") },
    {"DEGREE", new EuInfo("DD", 17476, "◦", "degree [unit of angle]") },
    {"DEGREE/SECOND", new EuInfo("E96", 4536630, "◦/s", "Angular degrees per second") },
    {"DEGREE/SECONDˆ2", new EuInfo("M45", 5059637, "◦", "Angular acceleration in degrees per second squared") },
    {"HERTZ", new EuInfo("HTZ", 4740186, "Hz", "Frequency measured in cycles per second") },
    {"JOULE", new EuInfo("JOU", 4869973, "J", "A measurement of energy.") },
    {"KILOGRAM", new EuInfo("KGM", 4933453, "kg", "kilogram") },
    {"LITER", new EuInfo("LTR", 5002322, "l", "Litre") },
    {"LITER/SECOND", new EuInfo("G51", 4666673, "l/s", "Litre per second") },
    {"MICRO_RADIAN", new EuInfo("B97", 4340023, "μ", "microradian - Measurement of Tilt") },
    {"MILLIMETER", new EuInfo("MMT", 5066068, "mm", "millimetre") },
    {"MILLIMETER/SECOND", new EuInfo("C16", 4403510, "mm/s", "millimetre per second") },
    {"MILLIMETER/SECONDˆ2", new EuInfo("M41", 5059633, "mm/s2", "Acceleration in millimeters per second squared") },
    {"MILLIMETER_3D", new EuInfo("MMT", 5066068, "mm(R3", "A point in space identified by X") },
    {"NEWTON", new EuInfo("NEW", 5129559, "N", "Force in Newtons") },
    {"NEWTON_METER", new EuInfo("NU", 20053, "N·m", "Torque") },
    {"OHM", new EuInfo("OHM", 5195853, "Ω", "Measure of Electrical Re- sistance") },
    {"PASCAL", new EuInfo("PAL", 5259596, "Pa", "Pressure in Newtons per square meter") },
    {"PASCAL_SECOND", new EuInfo("C65", 4404789, "Pa·s", "Measurement of Viscosity") },
    {"PERCENT", new EuInfo("P1", 20529, "%", "Percent") },
    {"PH", new EuInfo("Q30", 5321520, "pH", "pH (potential of Hydrogen) - A measure of the acidity or alkalinity of a solution") },
    {"REVOLUTION/MINUTE", new EuInfo("RPM", 5394509, "r/min", "revolutions per minute") },
    {"SECOND", new EuInfo("SEC", 5457219, "s", "second [unit of time]") },
    {"SIEMENS/METER", new EuInfo("D10", 4469040, "S/m", "siemens per metre - A mea- surement of Electrical Con- ductivity") },
    {"VOLT", new EuInfo("VLT", 5655636, "V", "volt") },
    {"VOLT_AMPERE", new EuInfo("D46", 4469814, "VA", "volt - ampere") },
    {"VOLT_AMPERE_REACTIVE", new EuInfo("", -1, "VAR", "Volt-Ampere Reactive (VAR)") },
    {"WATT", new EuInfo("WTT", 5723220, "W", "watt") },
    {"WATT_SECOND", new EuInfo("J55", 4863285, "W·s", "Measurement of electrical energy") }
};
