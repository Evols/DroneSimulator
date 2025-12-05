import unittest

from aero_table.application.xfoil_row_fixer import cubic_interpolation


class TestRowFixer(unittest.TestCase):
    def test_zero(self):
        interpolated = cubic_interpolation(0.0, 0.0, 0.0, 0.0)
        self.assertEqual(interpolated, 0.0)

    def test_constant(self):
        interpolated = cubic_interpolation(1.0, 1.0, 1.0, 1.0)
        self.assertEqual(interpolated, 1.0)

    def test_linear(self):
        interpolated = cubic_interpolation(1.0, 2.0, 4.0, 5.0)
        self.assertEqual(interpolated, 3.0)

    def test_square(self):
        interpolated = cubic_interpolation(2.0 ** 2, 3.0 ** 2, 5.0 ** 2, 6.0 ** 2)
        self.assertAlmostEqual(interpolated, 4.0 ** 2, delta=2.0)

    def test_extrema(self):
        interpolated = cubic_interpolation(4.0, 5.0, 5.0, 4.0)
        self.assertGreater(interpolated, 5.0)
        self.assertLess(interpolated, 6.0)

    def test_weird(self):
        interpolated = cubic_interpolation(4.0, 5.0, 3.0, 4.0)
        self.assertAlmostEqual(interpolated, 4.0, delta=0.1)

    def test_negative_constant(self):
        interpolated = cubic_interpolation(-1.0, -1.0, -1.0, -1.0)
        self.assertEqual(interpolated, -1.0)

    def test_negative_linear(self):
        interpolated = cubic_interpolation(-5.0, -4.0, -2.0, -1.0)
        self.assertEqual(interpolated, -3.0)

if __name__ == '__main__':
    unittest.main()
