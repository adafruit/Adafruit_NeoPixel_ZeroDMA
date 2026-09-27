import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from pinfinder import macro_candidates


class MacroCandidatesTest(unittest.TestCase):
    def test_same5x_datasheet_names_use_cmsis_a_suffix(self):
        self.assertEqual(macro_candidates("SAME54P20"), ["__SAME54P20A__"])
        self.assertEqual(macro_candidates("SAME53N20"), ["__SAME53N20A__"])
        self.assertEqual(macro_candidates("SAMD51J19"), ["__SAMD51J19A__"])

    def test_samd21_revision_suffix_is_preserved(self):
        self.assertEqual(macro_candidates("ATSAMD21G18A"), ["__SAMD21G18A__"])


if __name__ == "__main__":
    unittest.main()
