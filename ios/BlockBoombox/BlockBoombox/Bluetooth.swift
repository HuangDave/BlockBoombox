import CoreBluetooth

open class Bluetooth: NSObject, CBCentralManagerDelegate, CBPeripheralDelegate, SerialPeripheral {
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

  // MARK: -

  public init(deviceUUID: CBUUID, serviceUUID: CBUUID, characteristicUUID: CBUUID) {
    super.init()
    self.deviceUUID = deviceUUID
    self.serviceUUID = serviceUUID
    self.characteristicUUID = characteristicUUID
    centralManager = CBCentralManager(delegate: self, queue: nil)
  }

  // MARK: -

  open func startScan() {
    print("Scanning for bluetooth peripheral: \(String(describing: deviceUUID))")
    centralManager.scanForPeripherals(withServices: [serviceUUID])
  }

  open func stopScan() {
    print("Stopping scan")
    centralManager.stopScan()
  }

  func connectToPeripheral(_ peripheral: CBPeripheral) {
    guard pendingPeripheral == nil else { return }
    print("Attempting to connect: \(peripheral)")
    pendingPeripheral = peripheral
    centralManager.connect(peripheral)
  }

  func disconnect() {
    if let peripheral = connectedPeripheral {
      centralManager.cancelPeripheralConnection(peripheral)
    } else if let peripheral = pendingPeripheral {
      centralManager.cancelPeripheralConnection(peripheral)
    }
  }

  // MARK: - CBCentralManagerDelegate

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
    print("Discovered: \(peripheral)")
    if (peripheral.identifier.uuidString == deviceUUID.uuidString) {
      connectToPeripheral(peripheral)
    }
  }

  public func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
    print("Connected to peripheral: \(peripheral)")
    print("Stopping scan...")
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
                             didDisconnectPeripheral peripheral: CBPeripheral, error: Error?) {
    print("Disconnected: \(peripheral)")
  }

  // MARK: - CBPeripheralDelegate

  public func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
    guard let services = peripheral.services else { return }
    print("Services: \(services.count)")
    for service in services {
      print(service)
      peripheral.discoverCharacteristics([characteristicUUID], for: service)
    }
  }

  public func peripheral(_ peripheral: CBPeripheral,
                         didDiscoverCharacteristicsFor service: CBService, error: Error?) {
    guard let characteristics = service.characteristics else { return }
    print("Characteristics: \(characteristics.count)")
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
