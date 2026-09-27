import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


class SercomLifecycleContractTest(unittest.TestCase):
    def test_spi_transfer_is_completion_driven(self):
        source = (ROOT / "Adafruit_NeoPixel_SERCOM.cpp").read_text()
        header = (ROOT / "Adafruit_NeoPixel_SERCOM.h").read_text()

        self.assertNotIn("Adafruit_ZeroDMA", source)
        self.assertNotIn("Adafruit_ZeroDMA", header)
        self.assertIn("static void transferComplete(void *user, int status)", header)
        self.assertIn("spi->transfer(buffer, NULL, frameBytes, false, transferComplete, this)", source)
        self.assertIn("uint8_t *stagingBuf", header)
        self.assertIn("volatile bool transferActive", header)
        self.assertIn("volatile bool refreshPending", header)

    def test_show_publishes_staging_frame_through_spi(self):
        source = (ROOT / "Adafruit_NeoPixel_SERCOM.cpp").read_text()
        begin_body = source.split(
            "bool Adafruit_NeoPixel_SERCOM::begin(void)", maxsplit=1
        )[1].split("void Adafruit_NeoPixel_SERCOM::encodeInto", maxsplit=1)[0]
        show_body = source.split("void Adafruit_NeoPixel_SERCOM::show(void)", maxsplit=1)[1]
        self.assertNotIn("spi->transfer(", begin_body)
        self.assertIn("encodeInto(stagingBuf)", show_body)
        self.assertIn("startTransfer(activeBuf)", show_body)


if __name__ == "__main__":
    unittest.main()
