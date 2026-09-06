import SwiftUI

struct ConnectionView: View {
    @EnvironmentObject private var vm: MemsAppViewModel

    var body: some View {
        Form {
            Section("Adaptateur") {
                Text(vm.selectedAdapterName)
            }
            Section {
                Button(vm.isConnected ? "Déconnecter" : "Connecter") {
                    vm.isConnected ? vm.disconnect() : vm.connect()
                }
            }
        }
    }
}
