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
    print("Starting scan")
        centralManager.scanForPeripherals(withServices: nil)
//    centralManager.scanForPeripherals(withServices: [serviceUUID])
  }

  open func stopScan() {
    print("Stopping scan")
    centralManager.stopScan()
  }

  func connectToPeripheral(_ peripheral: CBPeripheral) {
    print("Attempting to connect \(peripheral)")
    pendingPeripheral = peripheral
    centralManager.connect(peripheral, options: nil)
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
      print("central.state is .unknown")
    case .resetting:
      print("central.state is .resetting")
    case .unsupported:
      print("central.state is .unsupported")
    case .unauthorized:
      print("central.state is .unauthorized")
    case .poweredOff:
      print("central.state is .poweredOff")
    case .poweredOn:
      print("central.state is .poweredOn")
      startScan()
    default: break
    }
  }

  public func centralManager(_ central: CBCentralManager,
                             didDiscover peripheral: CBPeripheral,
                             advertisementData: [String : Any], rssi RSSI: NSNumber) {
    print("didDiscover: \(peripheral)")
    centralManager.stopScan()
    pendingPeripheral = peripheral
    centralManager.connect(pendingPeripheral!)
  }

  public func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
    print("Connected!")
    connectedPeripheral = peripheral
    connectedPeripheral?.delegate = self
    connectedPeripheral!.discoverServices([serviceUUID])
  }

  public func centralManager(_ central: CBCentralManager,
                             didFailToConnect peripheral: CBPeripheral,
                             error: Error?) {

  }

  // MARK: - CBPeripheralDelegate

  public func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
    guard let services = peripheral.services else { return }

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
