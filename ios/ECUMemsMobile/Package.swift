// swift-tools-version: 6.3
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

let package = Package(
    name: "ECUMemsMobile",
    platforms: [
        .iOS(.v17),
        .macOS(.v13)
    ],
    products: [
        .library(
            name: "ECUMemsCore",
            targets: ["MEMSCore", "MEMSReplay", "MEMSStore"]
        ),
        .library(
            name: "ECUMemsAppScaffold",
            targets: ["ECUMemsAppScaffold"]
        )
    ],
    targets: [
        .target(
            name: "MEMSCore"
        ),
        .target(
            name: "MEMSReplay",
            dependencies: ["MEMSCore"]
        ),
        .target(
            name: "MEMSStore",
            dependencies: ["MEMSCore"]
        ),
        .target(
            name: "ECUMemsAppScaffold",
            dependencies: ["MEMSCore", "MEMSReplay", "MEMSStore"]
        ),
        .testTarget(
            name: "MEMSReplayTests",
            dependencies: ["MEMSReplay", "MEMSCore"]
        )
    ],
    swiftLanguageModes: [.v6]
)
