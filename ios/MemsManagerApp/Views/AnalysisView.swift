import SwiftUI

struct AnalysisView: View {
    @EnvironmentObject private var vm: MemsAppViewModel

    var body: some View {
        List(vm.suggestions) { item in
            VStack(alignment: .leading, spacing: 4) {
                Text(item.title).bold()
                Text(item.detail).font(.footnote)
            }
        }
    }
}
