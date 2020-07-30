import CoreBluetooth
import UIKit

class ViewController: UIViewController {
  let deviceUUID = CBUUID(string: "D326B784-4FDA-C588-66BF-6BB05A175F46")
  let serviceUUID = CBUUID(string: "FFE0")
  let characteristicUUID = CBUUID(string: "FFE1")

  var hm10: Bluetooth?

  override func viewDidLoad() {
    super.viewDidLoad()

    hm10 = Bluetooth(deviceUUID: deviceUUID,
                               serviceUUID: serviceUUID,
                               characteristicUUID: characteristicUUID)
  }
}
