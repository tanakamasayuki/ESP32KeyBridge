def test_usb_host_keyboard_receives_key_a(dut, peers):
    device = peers["device"]

    device.expect_exact("DEVICE_BEGIN 1")
    dut.expect_exact("HOST_CONNECTED")

    device.write("a")
    device.expect_exact("DEVICE_SEND_A 1")
    dut.expect_exact("KEYBRIDGE_KEY_A_PRESSED")
    dut.expect_exact("KEYBRIDGE_KEY_A_RELEASED")
