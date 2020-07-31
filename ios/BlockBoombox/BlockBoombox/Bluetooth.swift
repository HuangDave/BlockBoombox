import CoreBluetooth

extension SerialPeripheral where Self: Bluetooth {
  public func send(data: Data) {
    guard isReady else { return }
    guard let writeCharacteristic = writeCharacteristic else { return }
    connectedPeripheral!.writeValue(data, for: writeCharacteristic, type: writeType)
  }
}

// MARK: -
open class Bluetooth: NSObject, SerialPeripheral {
  private(set) var centralManager : CBCentralManager!

  private(set) var pendingPeripheral: CBPeripheral?
  private(set) var connectedPeripheral: CBPeripheral?

  private(set) weak var writeCharacteristic: CBCharacteristic?
  private(set) var writeType: CBCharacteristicWriteType = .withoutResponse

  public private(set) var deviceUUID: CBUUID!
  public private(set) var serviceUUID: CBUUID!
  public private(set) var characteristicUUID: CBUUID!

  public var isPoweredOn: Bool { return centralManager.state == .poweredOn }
  public var isConnected: Bool { return connectedPeripheral != nil }
  public var isReady: Bool { return isPoweredOn && isConnected }
  public var isScanning: Bool { return centralManager.isScanning }

  // MARK: - Initialization

  public init(deviceUUID: CBUUID, serviceUUID: CBUUID, characteristicUUID: CBUUID) {
    super.init()
    self.deviceUUID = deviceUUID
    self.serviceUUID = serviceUUID
    self.characteristicUUID = characteristicUUID
    centralManager = CBCentralManager(delegate: self, queue: nil)
  }

  // MARK: - Connection

  public func startScan() {
    centralManager.scanForPeripherals(withServices: [serviceUUID])
  }

  public func stopScan() {
    centralManager.stopScan()
  }

  private func connectToPeripheral(_ peripheral: CBPeripheral) {
    guard pendingPeripheral == nil else { return }
    pendingPeripheral = peripheral
    centralManager.connect(peripheral)
  }

  public func disconnect() {
    if let peripheral = connectedPeripheral {
      centralManager.cancelPeripheralConnection(peripheral)
    } else if let peripheral = pendingPeripheral {
      centralManager.cancelPeripheralConnection(peripheral)
    }
  }
}

// MARK: - CBCentralManagerDelegate Implementation
extension Bluetooth: CBCentralManagerDelegate {
  public func centralManagerDidUpdateState(_ central: CBCentralManager) {
    switch central.state {
    case .unknown:
      print("CBCentralManager state: .unknown")
    case .resetting:
      print("CBCentralManager state: .resetting")
    case .unsupported:
      print("CBCentralManager state: .unsupported")
    case .unauthorized:
      print("CBCentralManager state: .unauthorized")
    case .poweredOff:
      print("CBCentralManager state: .poweredOff")
    case .poweredOn:
      print("CBCentralManager state: .poweredOn")
      startScan()
    default: print(central.state)
    }
  }

  public func centralManager(_ central: CBCentralManager,
                             didDiscover peripheral: CBPeripheral,
                             advertisementData: [String : Any], rssi RSSI: NSNumber) {
    if (peripheral.identifier.uuidString == deviceUUID.uuidString) {
      connectToPeripheral(peripheral)
    }
  }

  public func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
    stopScan()
    connectedPeripheral = peripheral
    pendingPeripheral = nil
    connectedPeripheral!.delegate = self
    connectedPeripheral!.discoverServices([serviceUUID])
  }

  public func centralManager(_ central: CBCentralManager,
                             didFailToConnect peripheral: CBPeripheral,
                             error: Error?) {
    print("Failed to connect to peripheral: \(peripheral)")
  }

  public func centralManager(_ central: CBCentralManager,
                             didDisconnectPeripheral peripheral: CBPeripheral,
                             error: Error?
  ) {
    print("Disconnected: \(peripheral)")
  }
}

// MARK: - CBPeripheralDelegate Implementation
extension Bluetooth: CBPeripheralDelegate {
  public func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
    guard let services = peripheral.services else { return }

    for service in services {
      print(service)
      peripheral.discoverCharacteristics([characteristicUUID], for: service)
    }
  }

  public func peripheral(_ peripheral: CBPeripheral,
                         didDiscoverCharacteristicsFor service: CBService,
                         error: Error?
  ) {
    guard let characteristics = service.characteristics else { return }

    for characteristic in characteristics {
      print(characteristic)
      if characteristic.uuid == characteristicUUID {
        writeCharacteristic = characteristics[0]
        writeType = characteristic.properties.contains(.write) ? .withResponse : .withoutResponse

        send(message: "Connected\n")
        return
      }
    }
  }
}
