def test_usb_device_keyboard_output_sends_key_a(dut, peers):
    device = peers["device"]

    device.expect_exact("DEVICE_BEGIN 1")
    dut.expect_exact("HOST_CONNECTED")

    device.write("a")
    device.expect_exact("BRIDGE_SEND_A 1")
    dut.expect_exact("HOST_KEY_A_PRESSED")
    dut.expect_exact("HOST_KEY_A_RELEASED")
