import Foundation

struct MemsSnapshot {
    let engineRpm: Int
    let coolantTemp: Int
    let mapKpa: Int
    let batteryVoltage: Double

    static let empty = MemsSnapshot(engineRpm: 0, coolantTemp: 0, mapKpa: 0, batteryVoltage: 0)
}

struct MemsDiagnosticSuggestion: Identifiable {
    enum Severity {
        case info
        case warning
        case critical
    }

    let id = UUID()
    let severity: Severity
    let title: String
    let detail: String
}
