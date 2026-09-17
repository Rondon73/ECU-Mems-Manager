import Foundation

public actor MockECUTransport: ECUTransport {
    private var isOpen = false
    private var tick = 0

    public init() {}

    public func open() async throws {
        isOpen = true
    }

    public func close() async {
        isOpen = false
    }

    public func send(_ command: ECUCommand) async throws -> ECUResponse {
        guard isOpen || command == .connect else {
            throw ECUSessionError.notConnected
        }

        switch command {
        case .connect, .disconnect, .stopPolling, .clearFaultCodes, .actuator, .resetAdaptations, .resetECU:
            return .ok
        case .readFaultCodes:
            return .faultCodes([
                FaultCode(code: "12", description: "Throttle position fault"),
                FaultCode(code: "45", description: "Lambda rich indication")
            ])
        case .startPolling:
            tick += 1
            return .liveData(
                LiveDataSnapshot(
                    timestamp: Date(),
                    engineRPM: 850 + tick,
                    coolantTempC: 82.0,
                    intakeAirTempC: 24.0,
                    batteryVoltage: 13.9,
                    mapKpa: 38.0,
                    throttlePercent: 2.3
                )
            )
        }
    }
}
