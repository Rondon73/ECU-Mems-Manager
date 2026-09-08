import Foundation

final class MemsAppViewModel: ObservableObject {
    @Published var isConnected = false
    @Published var selectedAdapterName = "Aucun adaptateur"
    @Published var latestSnapshot: MemsSnapshot = .empty
    @Published var suggestions: [MemsDiagnosticSuggestion] = []

    private(set) var sessionLogs: [String] = []

    func connect() {
        isConnected = true
    }

    func disconnect() {
        isConnected = false
    }

    func record(snapshot: MemsSnapshot) {
        latestSnapshot = snapshot
    }
}
