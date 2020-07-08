import CoreBluetooth

class HM10: BluetoothPeripheral {
  override var serviceUUID: CBUUID { return CBUUID(string: "FFE0") }
}
