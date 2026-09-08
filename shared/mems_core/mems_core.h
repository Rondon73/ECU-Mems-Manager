#ifndef MEMS_CORE_H
#define MEMS_CORE_H

#include <cstdint>
#include <string>
#include <vector>

#include "rosco.h"

namespace mems {
namespace core {

struct Snapshot {
    uint16_t engineRpm;
    uint8_t coolantTemp;
    uint8_t ambientTemp;
    uint8_t intakeAirTemp;
    uint8_t fuelTemp;
    uint8_t mapKpa;
    uint8_t batteryVoltage;
    uint8_t throttlePot;
    uint8_t idleSwitch;
    uint8_t uk1;
    uint8_t parkNeutralSwitch;
    uint8_t faultCodes;
    uint8_t idleSetPoint;
    uint8_t idleHot;
    uint8_t uk2;
    uint8_t iacPosition;
    uint16_t idleError;
    uint8_t ignitionAdvanceOffset;
    uint8_t uk3;
    uint8_t uk4;
    uint8_t uk5;
    uint8_t ignitionSwitch;
    uint8_t throttleAngle;
    uint8_t uk6;
    uint8_t airFuelRatio;
    uint8_t lambdaVoltage;
    uint8_t lambdaSensorFrequency;
    uint8_t lambdaSensorDutycycle;
    uint8_t lambdaSensorStatus;
    uint8_t closedLoop;
    uint8_t longTermFuelTrim;
    uint8_t shortTermFuelTrim;
    uint8_t carbonCanisterDutycycle;
    uint8_t idleBasePos;
    uint8_t uk7;
    uint8_t ignitionAdvance2;
    uint8_t idleSpeedOffset;
    uint8_t dtc0;
    uint8_t dtc1;
    uint8_t dtc2;
    uint8_t dtc3;
    uint8_t dtc4;
    uint8_t dtc5;
    uint8_t uk11;
    uint8_t uk12;
    uint8_t uk13;
    uint8_t uk14;
    uint8_t uk15;
    uint8_t uk16;
    uint8_t uk1A;
    uint8_t uk1B;
    uint8_t uk1C;
    uint8_t ignitionAdvance;
    uint16_t coilTime;
    uint8_t idleError2;
    uint8_t uk10;
};

struct DiagnosticSuggestion {
    enum class Severity {
        Info,
        Warning,
        Critical
    };

    Severity severity;
    std::string title;
    std::string detail;
};

Snapshot fromRoscoData(const mems_data &data);
std::vector<DiagnosticSuggestion> buildDiagnosticSuggestions(const Snapshot &snapshot);
std::string csvHeaderLine();
std::string csvDataLine(const Snapshot &snapshot, const std::string &timestamp);

} // namespace core
} // namespace mems

#endif // MEMS_CORE_H
