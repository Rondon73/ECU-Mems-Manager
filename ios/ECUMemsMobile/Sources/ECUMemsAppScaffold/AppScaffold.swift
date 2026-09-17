import Foundation
import MEMSCore
import MEMSReplay
import MEMSStore

#if canImport(SwiftUI)
import SwiftUI

@available(iOS 17.0, *)
public struct ECUMemsAppScaffold: App {
    public init() {}

    public var body: some Scene {
        WindowGroup {
            RootView(viewModel: RootViewModel())
        }
    }
}

@available(iOS 17.0, *)
struct RootView: View {
    @State var viewModel: RootViewModel

    var body: some View {
        TabView {
            ConnectionView(viewModel: viewModel)
                .tabItem { Label("Connect", systemImage: "antenna.radiowaves.left.and.right") }

            DashboardView(viewModel: viewModel)
                .tabItem { Label("Dashboard", systemImage: "gauge.with.needle") }

            FaultsView(viewModel: viewModel)
                .tabItem { Label("Faults", systemImage: "exclamationmark.triangle") }

            ActuatorsView(viewModel: viewModel)
                .tabItem { Label("Actuators", systemImage: "switch.2") }

            LogsView()
                .tabItem { Label("Logs", systemImage: "doc.text") }

            AnalysisView()
                .tabItem { Label("Analysis", systemImage: "chart.xyaxis.line") }
        }
    }
}

@available(iOS 17.0, *)
@Observable
final class RootViewModel {
    var state: ConnectionState = .disconnected
    var latestSnapshot: LiveDataSnapshot?
    var faults: [FaultCode] = []
    var showSafetyPrompt = false

    private let session = ECUSession(transport: MockECUTransport())

    func connect() {
        Task {
            try? await session.connect()
            state = await session.state
        }
    }

    func poll() {
        Task {
            latestSnapshot = try? await session.pollOnce()
            state = await session.state
        }
    }

    func refreshFaults() {
        Task {
            faults = (try? await session.readFaultCodes()) ?? []
        }
    }

    func clearFaultsConfirmed() {
        Task {
            try? await session.clearFaultCodes(userConfirmed: true)
            refreshFaults()
        }
    }
}

@available(iOS 17.0, *)
struct ConnectionView: View {
    var viewModel: RootViewModel

    var body: some View {
        VStack(spacing: 12) {
            Text("Status: \(viewModel.state.rawValue)")
            Button("Connect") { viewModel.connect() }
            Button("Poll Live Data") { viewModel.poll() }
        }
        .padding()
    }
}

@available(iOS 17.0, *)
struct DashboardView: View {
    var viewModel: RootViewModel

    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            Text("Live Dashboard")
                .font(.headline)
            Text("RPM: \(viewModel.latestSnapshot?.engineRPM ?? 0)")
            Text("Coolant: \(viewModel.latestSnapshot?.coolantTempC ?? 0, specifier: "%.1f") °C")
            Text("Battery: \(viewModel.latestSnapshot?.batteryVoltage ?? 0, specifier: "%.1f") V")
            Text("MAP: \(viewModel.latestSnapshot?.mapKpa ?? 0, specifier: "%.1f") kPa")
        }
        .padding()
    }
}

@available(iOS 17.0, *)
struct FaultsView: View {
    var viewModel: RootViewModel

    var body: some View {
        VStack {
            HStack {
                Button("Refresh Faults") { viewModel.refreshFaults() }
                Button("Clear Faults") { viewModel.showSafetyPrompt = true }
            }
            List(viewModel.faults, id: \.self) { fault in
                VStack(alignment: .leading) {
                    Text(fault.code).bold()
                    Text(fault.description)
                }
            }
        }
        .alert("Confirm clear fault codes?", isPresented: Binding(
            get: { viewModel.showSafetyPrompt },
            set: { viewModel.showSafetyPrompt = $0 }
        )) {
            Button("Cancel", role: .cancel) {}
            Button("Clear", role: .destructive) { viewModel.clearFaultsConfirmed() }
        }
        .padding()
    }
}

@available(iOS 17.0, *)
struct ActuatorsView: View {
    var viewModel: RootViewModel

    var body: some View {
        VStack(spacing: 12) {
            Text("Actuator controls scaffold")
            Text("Add per-actuator confirmations before command execution.")
                .font(.caption)
                .foregroundStyle(.secondary)
        }
        .padding()
    }
}

@available(iOS 17.0, *)
struct LogsView: View {
    var body: some View {
        Text("CSV logging and export scaffold")
            .padding()
    }
}

@available(iOS 17.0, *)
struct AnalysisView: View {
    var body: some View {
        Text("Replay analysis scaffold")
            .padding()
    }
}
#endif
