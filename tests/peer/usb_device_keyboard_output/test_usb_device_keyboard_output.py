import pexpect
import pytest


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


def test_keyboard_key(dut, peers):
    device = peers["device"]
    _wait_mounted(device)

    device.write("a")
    device.expect_exact("CMD A_DOWN")
    dut.expect_exact("KEY_STATE modifiers=00 a=1 b=0")

    device.write("r")
    device.expect_exact("CMD A_UP")
    dut.expect_exact("KEY_STATE modifiers=00 a=0 b=0")


def test_modifier_only_report(dut, peers):
    device = peers["device"]
    _wait_mounted(device)

    device.write("s")
    device.expect_exact("CMD SHIFT_DOWN")
    dut.expect_exact("KEY_STATE modifiers=02 a=0 b=0")

    device.write("S")
    device.expect_exact("CMD SHIFT_UP")
    dut.expect_exact("KEY_STATE modifiers=00 a=0 b=0")


def test_six_key_rollover(dut, peers):
    device = peers["device"]
    _wait_mounted(device)

    # Six keys held at once fill the boot keyboard report; all reach the wire.
    device.write("6")
    device.expect_exact("CMD SIX_DOWN")
    dut.expect_exact("KEY_STATE modifiers=00 a=1 b=1 c=1 d=1 e=1 f=1")

    device.write("R")
    device.expect_exact("CMD SIX_UP")
    dut.expect_exact("KEY_STATE modifiers=00 a=0 b=0 c=0 d=0 e=0 f=0")


def test_consumer_usage(dut, peers):
    device = peers["device"]
    _wait_mounted(device)

    device.write("v")
    device.expect_exact("CMD VOLUP_DOWN")
    dut.expect_exact("CONSUMER usage=00e9 pressed=1")

    device.write("V")
    device.expect_exact("CMD VOLUP_UP")
    dut.expect_exact("released=1")


def test_mouse_button_and_wheel(dut, peers):
    device = peers["device"]
    _wait_mounted(device)

    device.write("m")
    device.expect_exact("CMD LBUTTON_DOWN")
    dut.expect_exact("MOUSE buttons=01 x=0 y=0 wheel=0")

    device.write("M")
    device.expect_exact("CMD LBUTTON_UP")
    dut.expect_exact("MOUSE buttons=00 x=0 y=0 wheel=0")

    device.write("w")
    device.expect_exact("CMD WHEEL_3")
    dut.expect_exact("MOUSE buttons=00 x=0 y=0 wheel=3")


def test_mouse_movement(dut, peers):
    device = peers["device"]
    _wait_mounted(device)

    # Relative X/Y motion is a one-shot delta: one report carries it, the
    # next frame is back to zero.
    device.write("x")
    device.expect_exact("CMD MOVE_X")
    dut.expect_exact("MOUSE buttons=00 x=10 y=0 wheel=0")

    device.write("y")
    device.expect_exact("CMD MOVE_Y")
    dut.expect_exact("MOUSE buttons=00 x=0 y=7 wheel=0")


def test_composite_reports_in_one_frame(dut, peers):
    device = peers["device"]
    _wait_mounted(device)

    # One bridge frame changes all three HID classes; the composite device
    # emits keyboard, consumer, and mouse reports in that fixed order.
    device.write("z")
    device.expect_exact("CMD COMBO_DOWN")
    dut.expect_exact("KEY_STATE modifiers=00 a=1 b=0 c=0 d=0 e=0 f=0")
    dut.expect_exact("CONSUMER usage=00e9 pressed=1")
    dut.expect_exact("MOUSE buttons=01 x=5 y=0 wheel=0")

    device.write("Z")
    device.expect_exact("CMD COMBO_UP")
    dut.expect_exact("KEY_STATE modifiers=00 a=0 b=0 c=0 d=0 e=0 f=0")
    dut.expect_exact("released=1")
    dut.expect_exact("MOUSE buttons=00 x=0 y=0 wheel=0")


def _wait_lock(device, expected, retries=20):
    """The LED report crosses USB and a bridge update before the input sees
    it, so poll instead of expecting the first answer to match."""
    for _ in range(retries):
        device.write("q")
        try:
            device.expect_exact(expected, timeout=1)
            return
        except pexpect.TIMEOUT:
            continue
    raise AssertionError(f"lock state never became: {expected}")


@pytest.mark.skip(
    reason="EspUsbHost 2.2.0 setKeyboardLeds() only reaches boot-declared keyboard "
    "interfaces; the bridge's composite device merges its HID classes into one "
    "report-ID interface, so the test host cannot send the LED report. Reported "
    "upstream (LED output to report-ID keyboards). Real PCs handle report IDs, "
    "so the device-side path is covered by a manual OS check until then."
)
def test_host_leds_become_lock_state(dut, peers):
    device = peers["device"]
    _wait_mounted(device)

    # The USB Device output receives the LED output report and becomes the
    # bridge's lock authority; the state is pushed to the input adapter.
    dut.write("n")
    dut.expect_exact("LED_TX 1")
    _wait_lock(device, "LOCK num=1 caps=0 scroll=0 kana=0")

    dut.write("0")
    dut.expect_exact("LED_TX 1")
    _wait_lock(device, "LOCK num=0 caps=0 scroll=0 kana=0")
