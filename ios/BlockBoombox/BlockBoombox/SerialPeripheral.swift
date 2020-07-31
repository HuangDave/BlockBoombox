import CoreBluetooth

public protocol SerialPeripheral {
  func send(data: Data)
  func send(bytes: [UInt8])
  func send(message: String)
}

extension SerialPeripheral {
  public func send(bytes: [UInt8]) {
    send(data: Data(bytes: bytes, count: bytes.count))
  }

  public func send(message: String) {
    send(bytes: Array(message.utf8))
  }
}
