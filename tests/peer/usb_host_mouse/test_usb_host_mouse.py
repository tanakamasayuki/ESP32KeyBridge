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


def test_mouse_movement(dut, peers):
    device = peers["device"]
    _wait_mounted(device)

    # Relative motion is a one-shot delta: it reaches the input adapter as an
    # axis delta and is drained in the frame the bridge picks it up.
    device.write("x")
    device.expect_exact("SEND MOVE_X")
    dut.expect_exact("AXIS x=10 y=0 wheel=0")

    device.write("y")
    device.expect_exact("SEND MOVE_Y")
    dut.expect_exact("AXIS x=0 y=7 wheel=0")


def test_mouse_wheel(dut, peers):
    device = peers["device"]
    _wait_mounted(device)

    device.write("w")
    device.expect_exact("SEND WHEEL_3")
    dut.expect_exact("AXIS x=0 y=0 wheel=3")


def test_mouse_buttons(dut, peers):
    device = peers["device"]
    _wait_mounted(device)

    # Buttons arrive as MouseButton keys in the output set (left=1, right=2).
    device.write("l")
    device.expect_exact("SEND LBTN_DOWN")
    dut.expect_exact("OUT mouse_button:0001")

    device.write("L")
    device.expect_exact("SEND LBTN_UP")
    dut.expect_exact("OUT empty")

    device.write("r")
    device.expect_exact("SEND RBTN_DOWN")
    dut.expect_exact("OUT mouse_button:0002")

    device.write("R")
    device.expect_exact("SEND RBTN_UP")
    dut.expect_exact("OUT empty")
