import SwiftUI

@main
struct MemsManagerApp: App {
    @StateObject private var viewModel = MemsAppViewModel()

    var body: some Scene {
        WindowGroup {
            ContentView()
                .environmentObject(viewModel)
        }
    }
}
