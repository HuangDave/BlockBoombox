import UIKit

class ViewController: UIViewController {

  var hm10 = HM10()

  override func viewDidLoad() {
    super.viewDidLoad()

    hm10.startScan()
  }
}
