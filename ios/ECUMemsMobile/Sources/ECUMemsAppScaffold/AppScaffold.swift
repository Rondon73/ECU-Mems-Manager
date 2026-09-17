import Foundation
import MEMSCore
import MEMSReplay
import MEMSStore

#if canImport(SwiftUI)
import SwiftUI

public enum DeviceRole: String {
    case production
    case test
}

@available(iOS 17.0, *)
public struct ECUMemsAppScaffold: App {
    private let role: DeviceRole

    public init(role: DeviceRole = .test) {
        self.role = role
    }

    public var body: some Scene {
        WindowGroup {
            RootView(role: role)
                .tint(WorksBMCTheme.accentRed)
        }
    }
}

@available(iOS 17.0, *)
final class RootViewModel: ObservableObject {
    @Published var state: ConnectionState = .disconnected
    @Published var latestSnapshot: LiveDataSnapshot?
    @Published var faults: [FaultCode] = []
    @Published var showFaultClearConfirmation = false

    let role: DeviceRole
    private let session = ECUSession(transport: MockECUTransport())

    init(role: DeviceRole) {
        self.role = role
    }

    func connect() {
        Task { @MainActor in
            try? await session.connect()
            state = await session.state
        }
    }

    func poll() {
        Task { @MainActor in
            latestSnapshot = try? await session.pollOnce()
            state = await session.state
        }
    }

    func refreshFaults() {
        Task { @MainActor in
            faults = (try? await session.readFaultCodes()) ?? []
        }
    }

    func clearFaultsConfirmed() {
        Task { @MainActor in
            try? await session.clearFaultCodes(userConfirmed: true)
            faults = (try? await session.readFaultCodes()) ?? []
        }
    }
}

@available(iOS 17.0, *)
struct RootView: View {
    @StateObject private var viewModel: RootViewModel

    init(role: DeviceRole) {
        _viewModel = StateObject(wrappedValue: RootViewModel(role: role))
    }

    var body: some View {
        TabView {
            ConnectionView(viewModel: viewModel)
                .tabItem { Label("Connect", systemImage: "antenna.radiowaves.left.and.right") }

            DashboardView(viewModel: viewModel)
                .tabItem { Label("Dashboard", systemImage: "gauge.with.needle") }

            FaultsView(viewModel: viewModel)
                .tabItem { Label("Faults", systemImage: "exclamationmark.triangle") }

            ActuatorsView(role: viewModel.role)
                .tabItem { Label("Actuators", systemImage: "switch.2") }

            LogsView()
                .tabItem { Label("Logs", systemImage: "doc.text") }

            AnalysisView()
                .tabItem { Label("Analysis", systemImage: "chart.xyaxis.line") }
        }
        .background(WorksBMCTheme.background)
    }
}

@available(iOS 17.0, *)
struct ConnectionView: View {
    @ObservedObject var viewModel: RootViewModel

    var body: some View {
        VStack(spacing: 12) {
            Text("Device role: \(viewModel.role.rawValue)")
            Text("Status: \(viewModel.state.rawValue)")
            Button("Connect") { viewModel.connect() }
            Button("Poll Live Data") { viewModel.poll() }
        }
        .padding()
        .foregroundStyle(WorksBMCTheme.primaryText)
        .background(WorksBMCTheme.background)
    }
}

@available(iOS 17.0, *)
struct DashboardView: View {
    @ObservedObject var viewModel: RootViewModel

    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            Text("Live Dashboard").font(.headline)
            Text("RPM: \(viewModel.latestSnapshot?.engineRPM ?? 0)")
            Text("Coolant: \(viewModel.latestSnapshot?.coolantTempC ?? 0, specifier: "%.1f") °C")
            Text("Battery: \(viewModel.latestSnapshot?.batteryVoltage ?? 0, specifier: "%.1f") V")
            Text("MAP: \(viewModel.latestSnapshot?.mapKpa ?? 0, specifier: "%.1f") kPa")
        }
        .padding()
        .foregroundStyle(WorksBMCTheme.primaryText)
        .background(WorksBMCTheme.background)
    }
}

@available(iOS 17.0, *)
struct FaultsView: View {
    @ObservedObject var viewModel: RootViewModel

    var body: some View {
        VStack {
            HStack {
                Button("Refresh Faults") { viewModel.refreshFaults() }
                Button("Clear Faults") { viewModel.showFaultClearConfirmation = true }
            }
            List(viewModel.faults, id: \.self) { fault in
                VStack(alignment: .leading) {
                    Text(fault.code).bold().foregroundStyle(WorksBMCTheme.accentGold)
                    Text(fault.description).foregroundStyle(WorksBMCTheme.secondaryText)
                }
                .listRowBackground(WorksBMCTheme.panel)
            }
        }
        .alert("Confirm clear fault codes?", isPresented: $viewModel.showFaultClearConfirmation) {
            Button("Cancel", role: .cancel) {}
            Button("Clear", role: .destructive) { viewModel.clearFaultsConfirmed() }
        }
        .padding()
        .foregroundStyle(WorksBMCTheme.primaryText)
        .background(WorksBMCTheme.background)
    }
}

@available(iOS 17.0, *)
struct ActuatorsView: View {
    let role: DeviceRole

    var body: some View {
        VStack(spacing: 12) {
            Text("Actuator controls scaffold")
            Text(role == .test
                 ? "Test-device mode: commands should remain limited and explicitly confirmed."
                 : "Production mode: keep strict confirmation and operational safeguards.")
                .font(.caption)
                .foregroundStyle(WorksBMCTheme.secondaryText)
        }
        .padding()
        .foregroundStyle(WorksBMCTheme.primaryText)
        .background(WorksBMCTheme.background)
    }
}

@available(iOS 17.0, *)
struct LogsView: View {
    var body: some View {
        Text("CSV logging and export scaffold")
            .padding()
            .foregroundStyle(WorksBMCTheme.primaryText)
            .background(WorksBMCTheme.background)
    }
}

@available(iOS 17.0, *)
struct AnalysisView: View {
    var body: some View {
        Text("Replay analysis scaffold")
            .padding()
            .foregroundStyle(WorksBMCTheme.primaryText)
            .background(WorksBMCTheme.background)
    }
}
#endif
