import XCTest
@testable import MEMSReplay

final class CSVLogCodecTests: XCTestCase {
    func testEncodeDecodeRoundTrip() throws {
        let codec = CSVLogCodec()
        let t0 = Date(timeIntervalSince1970: 1_700_000_000)

        let input = [
            CSVLogEntry(timestamp: t0, values: ["rpm": 900, "coolant": 82.5]),
            CSVLogEntry(timestamp: t0.addingTimeInterval(1), values: ["rpm": 910, "coolant": 82.7])
        ]

        let csv = codec.encode(input)
        let output = try codec.decode(csv)

        XCTAssertEqual(output.count, 2)
        XCTAssertEqual(output[0].values["rpm"], 900)
        XCTAssertEqual(output[1].values["coolant"], 82.7)
    }

    func testDecodeRejectsInvalidHeader() {
        let codec = CSVLogCodec()
        XCTAssertThrowsError(try codec.decode("rpm,coolant\n900,82"))
    }
}
