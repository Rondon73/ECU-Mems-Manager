import Foundation

protocol MemsTransport {
    var adapterName: String { get }
    func connect() throws
    func disconnect()
    func nextSnapshot() async throws -> MemsSnapshot
}
