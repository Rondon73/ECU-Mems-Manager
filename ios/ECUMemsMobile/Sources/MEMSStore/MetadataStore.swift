import Foundation
import SQLite3
import MEMSCore

public struct EcuMetadata: Equatable, Sendable {
    public var id: Int64
    public var family: String
    public var version: String
    public var manufacturer: String?

    public init(id: Int64, family: String, version: String, manufacturer: String?) {
        self.id = id
        self.family = family
        self.version = version
        self.manufacturer = manufacturer
    }
}

public final class MetadataStore {
    private var db: OpaquePointer?

    public init() {}

    deinit {
        close()
    }

    public func open(path: String) throws {
        if sqlite3_open(path, &db) != SQLITE_OK {
            throw NSError(domain: "MetadataStore", code: 1)
        }
        try createTables()
    }

    public func close() {
        if db != nil {
            sqlite3_close(db)
            db = nil
        }
    }

    public func createTables() throws {
        let sql = """
        CREATE TABLE IF NOT EXISTS ecu (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            family TEXT NOT NULL,
            version TEXT NOT NULL,
            manufacturer TEXT
        );
        CREATE TABLE IF NOT EXISTS vehicle (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            make TEXT,
            model TEXT,
            engine TEXT,
            ecu_id INTEGER
        );
        CREATE TABLE IF NOT EXISTS dtc (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            code TEXT,
            description TEXT,
            ecu_id INTEGER
        );
        CREATE TABLE IF NOT EXISTS actuator (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT,
            command TEXT,
            ecu_id INTEGER
        );
        """

        if sqlite3_exec(db, sql, nil, nil, nil) != SQLITE_OK {
            throw NSError(domain: "MetadataStore", code: 2)
        }
    }

    @discardableResult
    public func addEcu(family: String, version: String, manufacturer: String?) throws -> Int64 {
        let sql = "INSERT INTO ecu (family, version, manufacturer) VALUES (?, ?, ?);"
        var statement: OpaquePointer?
        guard sqlite3_prepare_v2(db, sql, -1, &statement, nil) == SQLITE_OK else {
            throw NSError(domain: "MetadataStore", code: 3)
        }
        defer { sqlite3_finalize(statement) }

        sqlite3_bind_text(statement, 1, family, -1, SQLITE_TRANSIENT)
        sqlite3_bind_text(statement, 2, version, -1, SQLITE_TRANSIENT)
        if let manufacturer {
            sqlite3_bind_text(statement, 3, manufacturer, -1, SQLITE_TRANSIENT)
        } else {
            sqlite3_bind_null(statement, 3)
        }

        guard sqlite3_step(statement) == SQLITE_DONE else {
            throw NSError(domain: "MetadataStore", code: 4)
        }

        return sqlite3_last_insert_rowid(db)
    }

    public func listEcus() throws -> [EcuMetadata] {
        let sql = "SELECT id, family, version, manufacturer FROM ecu ORDER BY family, version;"
        var statement: OpaquePointer?
        guard sqlite3_prepare_v2(db, sql, -1, &statement, nil) == SQLITE_OK else {
            throw NSError(domain: "MetadataStore", code: 5)
        }
        defer { sqlite3_finalize(statement) }

        var result: [EcuMetadata] = []

        while sqlite3_step(statement) == SQLITE_ROW {
            let id = sqlite3_column_int64(statement, 0)
            let family = String(cString: sqlite3_column_text(statement, 1))
            let version = String(cString: sqlite3_column_text(statement, 2))
            let manufacturer = sqlite3_column_text(statement, 3).map { String(cString: $0) }

            result.append(EcuMetadata(id: id, family: family, version: version, manufacturer: manufacturer))
        }

        return result
    }
}
