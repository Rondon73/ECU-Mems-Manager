#include "mems_core.h"

#include <sstream>

namespace mems {
namespace core {

Snapshot fromRoscoData(const mems_data &data)
{
    Snapshot snapshot;
    snapshot.engineRpm = data.engine_rpm;
    snapshot.coolantTemp = data.coolant_temp;
    snapshot.ambientTemp = data.ambient_temp;
    snapshot.intakeAirTemp = data.intake_air_temp;
    snapshot.fuelTemp = data.fuel_temp;
    snapshot.mapKpa = data.map_kpa;
    snapshot.batteryVoltage = data.battery_voltage;
    snapshot.throttlePot = data.throttle_pot;
    snapshot.idleSwitch = data.idle_switch;
    snapshot.uk1 = data.uk1;
    snapshot.parkNeutralSwitch = data.park_neutral_switch;
    snapshot.faultCodes = data.fault_codes;
    snapshot.idleSetPoint = data.idle_set_point;
    snapshot.idleHot = data.idle_hot;
    snapshot.uk2 = data.uk2;
    snapshot.iacPosition = data.iac_position;
    snapshot.idleError = data.idle_error;
    snapshot.ignitionAdvanceOffset = data.ignition_advance_offset;
    snapshot.uk3 = data.uk3;
    snapshot.uk4 = data.uk4;
    snapshot.uk5 = data.uk5;
    snapshot.ignitionSwitch = data.ignition_switch;
    snapshot.throttleAngle = data.throttle_angle;
    snapshot.uk6 = data.uk6;
    snapshot.airFuelRatio = data.air_fuel_ratio;
    snapshot.lambdaVoltage = data.lambda_voltage;
    snapshot.lambdaSensorFrequency = data.lambda_sensor_frequency;
    snapshot.lambdaSensorDutycycle = data.lambda_sensor_dutycycle;
    snapshot.lambdaSensorStatus = data.lambda_sensor_status;
    snapshot.closedLoop = data.closed_loop;
    snapshot.longTermFuelTrim = data.long_term_fuel_trim;
    snapshot.shortTermFuelTrim = data.short_term_fuel_trim;
    snapshot.carbonCanisterDutycycle = data.carbon_canister_dutycycle;
    snapshot.idleBasePos = data.idle_base_pos;
    snapshot.uk7 = data.uk7;
    snapshot.ignitionAdvance2 = data.ignition_advance2;
    snapshot.idleSpeedOffset = data.idle_speed_offset;
    snapshot.dtc0 = data.dtc0;
    snapshot.dtc1 = data.dtc1;
    snapshot.dtc2 = data.dtc2;
    snapshot.dtc3 = data.dtc3;
    snapshot.dtc4 = data.dtc4;
    snapshot.dtc5 = data.dtc5;
    snapshot.uk11 = data.uk11;
    snapshot.uk12 = data.uk12;
    snapshot.uk13 = data.uk13;
    snapshot.uk14 = data.uk14;
    snapshot.uk15 = data.uk15;
    snapshot.uk16 = data.uk16;
    snapshot.uk1A = data.uk1A;
    snapshot.uk1B = data.uk1B;
    snapshot.uk1C = data.uk1C;
    snapshot.ignitionAdvance = data.ignition_advance;
    snapshot.coilTime = data.coil_time;
    snapshot.idleError2 = data.idle_error2;
    snapshot.uk10 = data.uk10;
    return snapshot;
}

std::vector<DiagnosticSuggestion> buildDiagnosticSuggestions(const Snapshot &snapshot)
{
    std::vector<DiagnosticSuggestion> suggestions;

    const uint16_t packedDtc01 = static_cast<uint16_t>(snapshot.dtc1 << 8) | snapshot.dtc0;
    const bool hasFaultCodes = (snapshot.faultCodes != 0) || (packedDtc01 != 0) ||
                               (snapshot.dtc2 != 0) || (snapshot.dtc3 != 0) ||
                               (snapshot.dtc4 != 0) || (snapshot.dtc5 != 0);

    if (hasFaultCodes) {
        suggestions.push_back({
            DiagnosticSuggestion::Severity::Critical,
            "Codes défaut actifs",
            "Des DTC sont présents; vérifier l'onglet défauts avant tout réglage moteur."
        });
    }

    if (snapshot.engineRpm > 0 && snapshot.idleError > 100) {
        suggestions.push_back({
            DiagnosticSuggestion::Severity::Warning,
            "Ralenti instable",
            "L'écart de ralenti dépasse 100 tr/min, ce qui peut indiquer un défaut de régulation."
        });
    }

    if (snapshot.engineRpm > 0 && snapshot.mapKpa > 80 && snapshot.engineRpm < 1200) {
        suggestions.push_back({
            DiagnosticSuggestion::Severity::Warning,
            "MAP élevée au ralenti",
            "Une pression collecteur élevée à bas régime peut indiquer une fuite de dépression."
        });
    }

    if (snapshot.batteryVoltage < 120) {
        suggestions.push_back({
            DiagnosticSuggestion::Severity::Warning,
            "Tension batterie basse",
            "La tension ECU est inférieure à 12.0V, ce qui peut perturber démarrage et allumage."
        });
    }

    if (snapshot.closedLoop && !snapshot.lambdaSensorStatus) {
        suggestions.push_back({
            DiagnosticSuggestion::Severity::Warning,
            "Sonde lambda incohérente",
            "Boucle fermée active alors que le diagnostic lambda signale un défaut potentiel."
        });
    }

    if ((snapshot.longTermFuelTrim > 120) || (snapshot.longTermFuelTrim < 80)) {
        suggestions.push_back({
            DiagnosticSuggestion::Severity::Warning,
            "Correction carburant long terme hors plage",
            "La correction long terme est éloignée de la zone nominale (80% - 120%)."
        });
    }

    if (suggestions.empty()) {
        suggestions.push_back({
            DiagnosticSuggestion::Severity::Info,
            "Aucune alerte critique détectée",
            "Les mesures principales ne déclenchent pas de règle d'alerte immédiate."
        });
    }

    return suggestions;
}

std::string csvHeaderLine()
{
    return "#time,"
           "80x01-02_engine-rpm,"
           "80x03_coolant_temp,"
           "80x04_ambient_temp,"
           "80x05_intake_air_temp,"
           "80x06_fuel_temp,"
           "80x07_map_kpa,"
           "80x08_battery_voltage,"
           "80x09_throttle_pot,"
           "80x0A_idle_switch,"
           "80x0B_uk1,"
           "80x0C_park_neutral_switch,"
           "80x0D-0E_fault_codes,"
           "80x0F_idle_set_point,"
           "80x10_idle_hot,"
           "80x11_uk2,"
           "80x12_iac_position,"
           "80x13-14_idle_error,"
           "80x15_ignition_advance_offset,"
           "80x16_ignition_advance,"
           "80x17-18_coil_time,"
           "80x19_crankshaft_position_sensor,"
           "80x1A_uk4,"
           "80x1B_uk5,"
           "7dx01_ignition_switch,"
           "7dx02_throttle_angle,"
           "7dx03_uk6,"
           "7dx04_air_fuel_ratio,"
           "7dx05_dtc2,"
           "7dx06_lambda_voltage,"
           "7dx07_lambda_sensor_frequency,"
           "7dx08_lambda_sensor_dutycycle,"
           "7dx09_lambda_sensor_status,"
           "7dx0A_closed_loop,"
           "7dx0B_long_term_fuel_trim,"
           "7dx0C_short_term_fuel_trim,"
           "7dx0D_carbon_canister_dutycycle,"
           "7dx0E_dtc3,"
           "7dx0F_idle_base_pos,"
           "7dx10_uk7,"
           "7dx11_dtc4,"
           "7dx12_ignition_advance2,"
           "7dx13_idle_speed_offset,"
           "7dx14_idle_error2,"
           "7dx14-15_uk10,"
           "7dx16_dtc5,"
           "7dx17_uk11,"
           "7dx18_uk12,"
           "7dx19_uk13,"
           "7dx1A_uk14,"
           "7dx1B_uk15,"
           "7dx1C_uk16,"
           "7dx1D_uk17,"
           "7dx1E_uk18,"
           "7dx1F_uk19";
}

std::string csvDataLine(const Snapshot &snapshot, const std::string &timestamp)
{
    std::ostringstream out;

    const uint16_t packedDtc = static_cast<uint16_t>(snapshot.dtc1 << 8) | snapshot.dtc0;
    const uint16_t packedIdleError2 = static_cast<uint16_t>(snapshot.idleError2 << 8) | snapshot.uk10;

    out << timestamp << ","
        << snapshot.engineRpm << ","
        << (static_cast<int>(snapshot.coolantTemp) - 55) << ","
        << (static_cast<int>(snapshot.ambientTemp) - 55) << ","
        << (static_cast<int>(snapshot.intakeAirTemp) - 55) << ","
        << (static_cast<int>(snapshot.fuelTemp) - 55) << ","
        << static_cast<int>(snapshot.mapKpa) << ","
        << (snapshot.batteryVoltage / 10.0) << ","
        << (snapshot.throttlePot * 0.02) << ","
        << static_cast<int>(snapshot.idleSwitch) << ","
        << static_cast<int>(snapshot.uk1) << ","
        << static_cast<int>(snapshot.parkNeutralSwitch) << ","
        << packedDtc << ","
        << static_cast<int>(snapshot.idleSetPoint) << ","
        << static_cast<int>(snapshot.idleHot) << ","
        << static_cast<int>(snapshot.uk2) << ","
        << static_cast<int>(snapshot.iacPosition) << ","
        << snapshot.idleError << ","
        << static_cast<int>(snapshot.ignitionAdvanceOffset) << ","
        << ((snapshot.ignitionAdvance * 0.5) - 24) << ","
        << (snapshot.coilTime * 0.002) << ","
        << static_cast<int>(snapshot.uk3) << ","
        << static_cast<int>(snapshot.uk4) << ","
        << static_cast<int>(snapshot.uk5) << ","
        << static_cast<int>(snapshot.ignitionSwitch) << ","
        << static_cast<int>(snapshot.throttleAngle) << ","
        << static_cast<int>(snapshot.uk6) << ","
        << static_cast<int>(snapshot.airFuelRatio) << ","
        << static_cast<int>(snapshot.dtc2) << ","
        << (snapshot.lambdaVoltage * 5) << ","
        << static_cast<int>(snapshot.lambdaSensorFrequency) << ","
        << static_cast<int>(snapshot.lambdaSensorDutycycle) << ","
        << static_cast<int>(snapshot.lambdaSensorStatus) << ","
        << static_cast<int>(snapshot.closedLoop) << ","
        << static_cast<int>(snapshot.longTermFuelTrim) << ","
        << static_cast<int>(snapshot.shortTermFuelTrim) << ","
        << static_cast<int>(snapshot.carbonCanisterDutycycle) << ","
        << static_cast<int>(snapshot.dtc3) << ","
        << static_cast<int>(snapshot.idleBasePos) << ","
        << static_cast<int>(snapshot.uk7) << ","
        << static_cast<int>(snapshot.dtc4) << ","
        << static_cast<int>(snapshot.ignitionAdvance2) << ","
        << static_cast<int>(snapshot.idleSpeedOffset) << ","
        << packedIdleError2 << ","
        << static_cast<int>(snapshot.dtc5) << ","
        << static_cast<int>(snapshot.uk11) << ","
        << static_cast<int>(snapshot.uk12) << ","
        << static_cast<int>(snapshot.uk13) << ","
        << static_cast<int>(snapshot.uk14) << ","
        << static_cast<int>(snapshot.uk15) << ","
        << static_cast<int>(snapshot.uk16) << ","
        << static_cast<int>(snapshot.uk1A) << ","
        << static_cast<int>(snapshot.uk1B) << ","
        << static_cast<int>(snapshot.uk1C);

    return out.str();
}

} // namespace core
} // namespace mems
