import SwiftUI

struct SummaryView: View {
    @EnvironmentObject private var vm: MemsAppViewModel

    var body: some View {
        List {
            Text("Régime: \(vm.latestSnapshot.engineRpm) tr/min")
            Text("Température liquide: \(vm.latestSnapshot.coolantTemp) °C")
            Text("MAP: \(vm.latestSnapshot.mapKpa) kPa")
            Text("Batterie: \(vm.latestSnapshot.batteryVoltage, specifier: "%.1f") V")
        }
    }
}
