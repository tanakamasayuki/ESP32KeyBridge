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


def test_key_press_reaches_output_remapped(dut, peers):
    device = peers["device"]
    _wait_mounted(device)

    device.write("a")
    device.expect_exact("SEND A_DOWN")
    dut.expect_exact("OUT keyboard:0005")  # A remapped to B by the bridge

    device.write("r")
    device.expect_exact("SEND ALL_UP")
    dut.expect_exact("OUT empty")


def test_modifier_only_press(dut, peers):
    device = peers["device"]
    _wait_mounted(device)

    device.write("c")
    device.expect_exact("SEND LCTRL_DOWN")
    dut.expect_exact("OUT keyboard:00e0")

    device.write("C")
    device.expect_exact("SEND LCTRL_UP")
    dut.expect_exact("OUT empty")


def test_multi_key_rollover(dut, peers):
    device = peers["device"]
    _wait_mounted(device)

    # Three keys held in one report; A is remapped to B, so the chord arrives
    # as B C E. Proves the input adapter forwards a multi-key snapshot and the
    # remap applies within it.
    device.write("k")
    device.expect_exact("SEND CHORD_ACE")
    dut.expect_exact("OUT keyboard:0005 keyboard:0006 keyboard:0008")

    device.write("r")
    device.expect_exact("SEND ALL_UP")
    dut.expect_exact("OUT empty")


def test_key_with_modifier(dut, peers):
    device = peers["device"]
    _wait_mounted(device)

    # LeftShift held together with A: the modifier and the remapped key both
    # reach the output set (modifiers are ordinary keys there, 0xe1).
    device.write("b")
    device.expect_exact("SEND SHIFT_A")
    dut.expect_exact("OUT keyboard:0005 keyboard:00e1")

    device.write("r")
    device.expect_exact("SEND ALL_UP")
    dut.expect_exact("OUT empty")


def test_terminal_host_lock_toggles_keyboard_led(dut, peers):
    device = peers["device"]
    _wait_mounted(device)

    # No lock-reporting output is connected, so the bridge acts as the
    # terminal host: a CapsLock tap toggles the shadow state, which is
    # forwarded to the keyboard LEDs.
    device.write("l")
    device.expect_exact("SEND CAPS_UP")
    device.expect_exact("LED numlock=0 capslock=1 scrolllock=0")

    device.write("l")
    device.expect_exact("SEND CAPS_UP")
    device.expect_exact("LED numlock=0 capslock=0 scrolllock=0")
