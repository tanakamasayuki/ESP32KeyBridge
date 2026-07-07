def test_core_smoke(dut):
    dut.expect_exact("TEST_BEGIN core_smoke")
    dut.expect_exact("TEST_END")
    assert dut.expect_exact(["OK", "NG"]) == b"OK"

