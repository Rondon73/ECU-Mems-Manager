import Foundation

public enum ConnectionState: String, Codable, Sendable {
    case disconnected
    case connecting
    case connected
    case polling
}

public struct LiveDataSnapshot: Codable, Equatable, Sendable {
    public var timestamp: Date
    public var engineRPM: Int
    public var coolantTempC: Double
    public var intakeAirTempC: Double
    public var batteryVoltage: Double
    public var mapKpa: Double
    public var throttlePercent: Double

    public init(
        timestamp: Date,
        engineRPM: Int,
        coolantTempC: Double,
        intakeAirTempC: Double,
        batteryVoltage: Double,
        mapKpa: Double,
        throttlePercent: Double
    ) {
        self.timestamp = timestamp
        self.engineRPM = engineRPM
        self.coolantTempC = coolantTempC
        self.intakeAirTempC = intakeAirTempC
        self.batteryVoltage = batteryVoltage
        self.mapKpa = mapKpa
        self.throttlePercent = throttlePercent
    }
}

public struct FaultCode: Codable, Equatable, Hashable, Sendable {
    public var code: String
    public var description: String

    public init(code: String, description: String) {
        self.code = code
        self.description = description
    }
}

public enum ActuatorCommand: String, Codable, Sendable {
    case fuelPumpOn
    case fuelPumpOff
    case acRelayOn
    case acRelayOff
    case ptcRelayOn
    case ptcRelayOff
    case purgeValveOn
    case purgeValveOff
    case o2HeaterOn
    case o2HeaterOff
    case fan1On
    case fan1Off
}

public enum SafetyCriticalAction: String, Codable, Sendable {
    case clearFaultCodes
    case actuator
    case resetAdaptations
    case resetECU
}

public enum ECUCommand: Sendable {
    case connect
    case disconnect
    case startPolling
    case stopPolling
    case readFaultCodes
    case clearFaultCodes
    case actuator(ActuatorCommand)
    case resetAdaptations
    case resetECU
}

public enum ECUResponse: Sendable {
    case ok
    case liveData(LiveDataSnapshot)
    case faultCodes([FaultCode])
}

public enum ECUSessionError: Error, Equatable {
    case notConnected
    case unauthorized(SafetyCriticalAction)
}
