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

static map<string, EuInfo> euStore = {
    {"AMPERE", EuInfo("AMP", 4279632, "A", "Amps") },
    {"CELSIUS", EuInfo("CEL", 4408652, "◦Celsuis", "Degrees Celsius") },
    {"COUNT", EuInfo("", 0, "", "Count") },
    {"DECIBEL", EuInfo("2N", 12878, "dB", "Sound Level") },
    {"DEGREE", EuInfo("DD", 17476, "◦", "degree [unit of angle]") },
    {"DEGREE/SECOND", EuInfo("E96", 4536630, "◦/s", "Angular degrees per second") },
    {"DEGREE/SECONDˆ2", EuInfo("M45", 5059637, "◦", "Angular acceleration in degrees per second squared") },
    {"HERTZ", EuInfo("HTZ", 4740186, "Hz", "Frequency measured in cycles per second") },
    {"JOULE", EuInfo("JOU", 4869973, "J", "A measurement of energy.") },
    {"KILOGRAM", EuInfo("KGM", 4933453, "kg", "kilogram") },
    {"LITER", EuInfo("LTR", 5002322, "l", "Litre") },
    {"LITER/SECOND", EuInfo("G51", 4666673, "l/s", "Litre per second") },
    {"MICRO_RADIAN", EuInfo("B97", 4340023, "μ", "microradian - Measurement of Tilt") },
    {"MILLIMETER", EuInfo("MMT", 5066068, "mm", "millimetre") },
    {"MILLIMETER/SECOND", EuInfo("C16", 4403510, "mm/s", "millimetre per second") },
    {"MILLIMETER/SECONDˆ2", EuInfo("M41", 5059633, "mm/s2", "Acceleration in millimeters per second squared") },
    {"MILLIMETER_3D", EuInfo("MMT", 5066068, "mm(R3", "A point in space identified by X") },
    {"NEWTON", EuInfo("NEW", 5129559, "N", "Force in Newtons") },
    {"NEWTON_METER", EuInfo("NU", 20053, "N·m", "Torque") },
    {"OHM", EuInfo("OHM", 5195853, "Ω", "Measure of Electrical Re- sistance") },
    {"PASCAL", EuInfo("PAL", 5259596, "Pa", "Pressure in Newtons per square meter") },
    {"PASCAL_SECOND", EuInfo("C65", 4404789, "Pa·s", "Measurement of Viscosity") },
    {"PERCENT", EuInfo("P1", 20529, "%", "Percent") },
    {"PH", EuInfo("Q30", 5321520, "pH", "pH (potential of Hydrogen) - A measure of the acidity or alkalinity of a solution") },
    {"REVOLUTION/MINUTE", EuInfo("RPM", 5394509, "r/min", "revolutions per minute") },
    {"SECOND", EuInfo("SEC", 5457219, "s", "second [unit of time]") },
    {"SIEMENS/METER", EuInfo("D10", 4469040, "S/m", "siemens per metre - A mea- surement of Electrical Con- ductivity") },
    {"VOLT", EuInfo("VLT", 5655636, "V", "volt") },
    {"VOLT_AMPERE", EuInfo("D46", 4469814, "VA", "volt - ampere") },
    {"VOLT_AMPERE_REACTIVE", EuInfo("", -1, "VAR", "Volt-Ampere Reactive (VAR)") },
    {"WATT", EuInfo("WTT", 5723220, "W", "watt") },
    {"WATT_SECOND", EuInfo("J55", 4863285, "W·s", "Measurement of electrical energy") }
};
