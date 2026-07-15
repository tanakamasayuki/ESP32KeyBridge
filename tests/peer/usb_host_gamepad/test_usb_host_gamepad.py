import pexpect


def _wait_mounted(device, retries=30):
    """Sync on the device's mount state via command-response (boot lines are
    not reliable across serial reopens)."""
    for _ in range(retries):
        device.write("p")
        try:
            device.expect_exact("PONG ready=1", timeout=1)
            return
        except pexpect.TIMEOUT:
            continue
    raise AssertionError("peer device never reported ready=1")


def test_buttons(dut, peers):
    device = peers["device"]
    _wait_mounted(device)

    # Button 1 -> A (0x04), button 2 -> B (0x05) via the adapter mapping.
    device.write("a")
    device.expect_exact("SEND BTN1_DOWN")
    dut.expect_exact("OUT keyboard:0004")

    device.write("b")
    device.expect_exact("SEND BTN2_DOWN")
    dut.expect_exact("OUT keyboard:0004 keyboard:0005")

    device.write("A")
    device.expect_exact("SEND BTN_UP")
    dut.expect_exact("OUT empty")


def test_hat(dut, peers):
    device = peers["device"]
    _wait_mounted(device)

    # The hat "up" direction maps to the Up arrow (0x52); centering releases.
    device.write("u")
    device.expect_exact("SEND HAT_UP")
    dut.expect_exact("OUT keyboard:0052")

    device.write("c")
    device.expect_exact("SEND HAT_CENTER")
    dut.expect_exact("OUT empty")
