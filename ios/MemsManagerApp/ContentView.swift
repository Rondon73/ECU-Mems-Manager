import SwiftUI

struct ContentView: View {
    var body: some View {
        TabView {
            ConnectionView()
                .tabItem { Label("Connexion", systemImage: "link") }
            SummaryView()
                .tabItem { Label("Résumé", systemImage: "gauge") }
            AnalysisView()
                .tabItem { Label("Analyse", systemImage: "chart.xyaxis.line") }
            LogsView()
                .tabItem { Label("Logs", systemImage: "doc.text") }
            SettingsView()
                .tabItem { Label("Réglages", systemImage: "gearshape") }
        }
    }
}
