import Foundation

public protocol ECUTransport: Sendable {
    func open() async throws
    func close() async
    func send(_ command: ECUCommand) async throws -> ECUResponse
}

public protocol SafetyPolicy: Sendable {
    func authorize(_ action: SafetyCriticalAction, confirmedByUser: Bool) -> Bool
}

public struct ConfirmationSafetyPolicy: SafetyPolicy {
    public init() {}

    public func authorize(_ action: SafetyCriticalAction, confirmedByUser: Bool) -> Bool {
        switch action {
        case .clearFaultCodes, .actuator, .resetAdaptations, .resetECU:
            return confirmedByUser
        }
    }
}

public actor ECUSession {
    private let transport: ECUTransport
    private let safetyPolicy: SafetyPolicy

    public private(set) var state: ConnectionState = .disconnected
    public private(set) var latestSnapshot: LiveDataSnapshot?

    public init(transport: ECUTransport, safetyPolicy: SafetyPolicy = ConfirmationSafetyPolicy()) {
        self.transport = transport
        self.safetyPolicy = safetyPolicy
    }

    public func connect() async throws {
        state = .connecting
        do {
            try await transport.open()
            _ = try await transport.send(.connect)
            state = .connected
        } catch {
            state = .disconnected
            throw error
        }
    }

    public func disconnect() async {
        _ = try? await transport.send(.disconnect)
        await transport.close()
        state = .disconnected
    }

    public func startPolling() async throws {
        guard state == .connected || state == .polling else {
            throw ECUSessionError.notConnected
        }
        _ = try await transport.send(.startPolling)
        state = .polling
    }

    public func stopPolling() async throws {
        guard state == .polling else { return }
        _ = try await transport.send(.stopPolling)
        state = .connected
    }

    public func pollOnce() async throws -> LiveDataSnapshot {
        guard state == .connected || state == .polling else {
            throw ECUSessionError.notConnected
        }

        let response = try await transport.send(.startPolling)
        if case let .liveData(snapshot) = response {
            latestSnapshot = snapshot
            state = .polling
            return snapshot
        }

        let fallback = LiveDataSnapshot(
            timestamp: Date(),
            engineRPM: 0,
            coolantTempC: 0,
            intakeAirTempC: 0,
            batteryVoltage: 0,
            mapKpa: 0,
            throttlePercent: 0
        )
        latestSnapshot = fallback
        return fallback
    }

    public func readFaultCodes() async throws -> [FaultCode] {
        guard state != .disconnected else {
            throw ECUSessionError.notConnected
        }

        let response = try await transport.send(.readFaultCodes)
        if case let .faultCodes(codes) = response {
            return codes
        }

        return []
    }

    public func clearFaultCodes(userConfirmed: Bool) async throws {
        guard state != .disconnected else {
            throw ECUSessionError.notConnected
        }
        guard safetyPolicy.authorize(.clearFaultCodes, confirmedByUser: userConfirmed) else {
            throw ECUSessionError.unauthorized(.clearFaultCodes)
        }
        _ = try await transport.send(.clearFaultCodes)
    }

    public func runActuator(_ command: ActuatorCommand, userConfirmed: Bool) async throws {
        guard state != .disconnected else {
            throw ECUSessionError.notConnected
        }
        guard safetyPolicy.authorize(.actuator, confirmedByUser: userConfirmed) else {
            throw ECUSessionError.unauthorized(.actuator)
        }
        _ = try await transport.send(.actuator(command))
    }
}
