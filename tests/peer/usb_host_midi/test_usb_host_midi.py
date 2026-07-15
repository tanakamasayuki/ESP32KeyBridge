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


def test_notes(dut, peers):
    device = peers["device"]
    _wait_mounted(device)

    # Note 60 -> A (0x04): Note On presses, Note Off releases.
    device.write("a")
    device.expect_exact("SEND NOTE60_ON")
    dut.expect_exact("OUT keyboard:0004")

    device.write("A")
    device.expect_exact("SEND NOTE60_OFF")
    dut.expect_exact("OUT empty")


def test_two_notes_merge(dut, peers):
    device = peers["device"]
    _wait_mounted(device)

    # Two notes held at once appear together (note 60 -> A, note 62 -> B).
    device.write("a")
    device.expect_exact("SEND NOTE60_ON")
    dut.expect_exact("OUT keyboard:0004")

    device.write("b")
    device.expect_exact("SEND NOTE62_ON")
    dut.expect_exact("OUT keyboard:0004 keyboard:0005")

    device.write("A")
    device.expect_exact("SEND NOTE60_OFF")
    dut.expect_exact("OUT keyboard:0005")

    device.write("B")
    device.expect_exact("SEND NOTE62_OFF")
    dut.expect_exact("OUT empty")
