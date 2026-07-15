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


def test_consumer_key_merges_into_key_set(dut, peers):
    device = peers["device"]
    _wait_mounted(device)

    device.write("v")
    device.expect_exact("SEND VOLUP_DOWN")
    dut.expect_exact("OUT consumer:00e9")

    device.write("V")
    device.expect_exact("SEND VOLUP_UP")
    dut.expect_exact("OUT empty")
