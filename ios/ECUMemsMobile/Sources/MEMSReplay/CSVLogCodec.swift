import Foundation
import MEMSCore

public struct CSVLogEntry: Equatable, Sendable {
    public var timestamp: Date
    public var values: [String: Double]

    public init(timestamp: Date, values: [String: Double]) {
        self.timestamp = timestamp
        self.values = values
    }
}

public enum CSVLogCodecError: Error, Equatable {
    case emptyContent
    case invalidHeader
}

public struct CSVLogCodec {
    public init() {}

    public func encode(_ entries: [CSVLogEntry]) -> String {
        guard let first = entries.first else {
            return ""
        }

        let keys = first.values.keys.sorted()
        let header = (["time"] + keys).joined(separator: ",")

        let formatter = ISO8601DateFormatter()

        let rows = entries.map { entry -> String in
            let values = keys.map { key in
                String(entry.values[key, default: 0.0])
            }
            return ([formatter.string(from: entry.timestamp)] + values).joined(separator: ",")
        }

        return ([header] + rows).joined(separator: "\n")
    }

    public func decode(_ csv: String) throws -> [CSVLogEntry] {
        let lines = csv
            .split(whereSeparator: \ .isNewline)
            .map(String.init)
            .filter { !$0.trimmingCharacters(in: .whitespaces).isEmpty }

        guard !lines.isEmpty else { throw CSVLogCodecError.emptyContent }

        let headerColumns = lines[0].split(separator: ",").map { String($0).trimmingCharacters(in: .whitespaces) }
        guard headerColumns.count > 1, headerColumns[0].lowercased() == "time" else {
            throw CSVLogCodecError.invalidHeader
        }

        let keys = Array(headerColumns.dropFirst())
        let formatter = ISO8601DateFormatter()

        return lines.dropFirst().compactMap { line in
            let columns = line.split(separator: ",").map { String($0).trimmingCharacters(in: .whitespaces) }
            guard columns.count == keys.count + 1 else {
                return nil
            }

            let timestamp = formatter.date(from: columns[0]) ?? Date(timeIntervalSince1970: 0)

            var values: [String: Double] = [:]
            for (index, key) in keys.enumerated() {
                values[key] = Double(columns[index + 1]) ?? 0.0
            }

            return CSVLogEntry(timestamp: timestamp, values: values)
        }
    }
}

public struct ReplayAnalyzer {
    private let entries: [CSVLogEntry]

    public init(entries: [CSVLogEntry]) {
        self.entries = entries
    }

    public var availableChannels: [String] {
        guard let first = entries.first else { return [] }
        return first.values.keys.sorted()
    }

    public func timeSeries(for channel: String) -> [(time: Date, value: Double)] {
        entries.map { ($0.timestamp, $0.values[channel, default: 0.0]) }
    }
}
