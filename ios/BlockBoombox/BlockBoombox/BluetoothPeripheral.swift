import CoreBluetooth

open class BluetoothPeripheral: NSObject, CBCentralManagerDelegate, CBPeripheralDelegate {
  private(set) var centralManager : CBCentralManager!

  private var pendingPeripheral: CBPeripheral?
  private var connectedPeripheral: CBPeripheral?

  weak var writeCharacteristic: CBCharacteristic?
  private var writeType: CBCharacteristicWriteType = .withoutResponse

  open var serviceUUID: CBUUID { fatalError("Must override with desired UUID") }

  public var isPoweredOn: Bool { return centralManager.state == .poweredOn }
  public var isConnected: Bool { return connectedPeripheral != nil }
  public var isReady: Bool { return isPoweredOn && isConnected }
  public var isScanning: Bool { return centralManager.isScanning }

  // MARK: -

  public  override init() {
    super.init()
    centralManager = CBCentralManager(delegate: self, queue: nil)
  }

  // MARK: -

  open func startScan() {
    print("Scanning for bluetooth peripheral: \(serviceUUID)")
    centralManager.scanForPeripherals(withServices: [])
//    centralManager.scanForPeripherals(withServices: [serviceUUID])
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

  // MARK: - Data Transfer

  func send(bytes: [UInt8]) {
    guard isReady else { return }

    let data = Data(bytes: bytes, count: bytes.count)
    connectedPeripheral!.writeValue(data, for: writeCharacteristic!, type: writeType)
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
    if (peripheral.identifier.uuidString == serviceUUID.uuidString) {
      connectToPeripheral(peripheral)
    }
  }

  public func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
    stopScan()
    print("Connected to peripheral: \(peripheral)")
    connectedPeripheral = peripheral
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

  }

  // MARK: - CBPeripheralDelegate

  public func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
    guard let services = peripheral.services else { return }
    print("Discovered services: \(services.count)")
    for service in services {
      print(service)
      peripheral.discoverCharacteristics(nil, for: service)
    }
  }

  public func peripheral(_ peripheral: CBPeripheral,
                         didDiscoverCharacteristicsFor service: CBService, error: Error?) {
    guard let characteristics = service.characteristics else { return }
    for characteristic in characteristics {
      print(characteristic)
    }
  }
}
