"""
Unit tests for Viterna post-stall extrapolation.

These tests validate that the Viterna method produces physically realistic
aerodynamic coefficients across the full ±180° range.
"""

import math
import unittest
from aero_table.application.viterna import (
    compute_viterna_coefficients,
    extrapolate_viterna,
    apply_viterna_extrapolation,
    detect_stall_point,
    StallPoint
)
from aero_table.application.xfoil_pol_parser import ComputedXfoilValue


class TestViternaCoefficients(unittest.TestCase):
    """Test Viterna coefficient calculation."""
    
    def test_typical_stall_point(self):
        """Test coefficient calculation for typical positive stall."""
        stall = StallPoint(aoa_deg=12.0, cl=1.4, cd=0.025, cm=-0.08)
        coeffs = compute_viterna_coefficients(stall, cd_max=1.8)
        
        # A1 should be CD_max / 2
        self.assertAlmostEqual(coeffs.A1, 0.9, places=6)
        
        # B1 should be CD_max
        self.assertAlmostEqual(coeffs.B1, 1.8, places=6)
        
        # CM should match stall value
        self.assertAlmostEqual(coeffs.CM_constant, -0.08, places=6)
        
        # A2 and B2 should be finite numbers
        self.assertTrue(math.isfinite(coeffs.A2))
        self.assertTrue(math.isfinite(coeffs.B2))
    
    def test_negative_stall_point(self):
        """Test coefficient calculation for negative stall."""
        stall = StallPoint(aoa_deg=-10.0, cl=-1.2, cd=0.030, cm=-0.06)
        coeffs = compute_viterna_coefficients(stall, cd_max=1.8)
        
        # Basic sanity checks
        self.assertAlmostEqual(coeffs.A1, 0.9, places=6)
        self.assertAlmostEqual(coeffs.B1, 1.8, places=6)
        self.assertTrue(math.isfinite(coeffs.A2))
        self.assertTrue(math.isfinite(coeffs.B2))
    
    def test_high_angle_stall(self):
        """Test coefficient calculation near 90 degrees."""
        # Near 90 degrees, cos(alpha) approaches 0
        stall = StallPoint(aoa_deg=85.0, cl=0.5, cd=1.5, cm=-0.02)
        coeffs = compute_viterna_coefficients(stall, cd_max=1.8)
        
        # Should handle near-singularity gracefully
        self.assertTrue(math.isfinite(coeffs.A2))


class TestViternaExtrapolation(unittest.TestCase):
    """Test Viterna extrapolation formulas."""
    
    def setUp(self):
        """Set up test fixtures."""
        self.stall = StallPoint(aoa_deg=12.0, cl=1.4, cd=0.025, cm=-0.08)
        self.coeffs = compute_viterna_coefficients(self.stall, cd_max=1.8)
    
    def test_zero_lift_at_zero_and_180(self):
        """CL should be approximately zero at 0° and 180°."""
        cl_0, _, _ = extrapolate_viterna(0.0, self.coeffs)
        cl_180, _, _ = extrapolate_viterna(180.0, self.coeffs)
        
        # Allow small numerical errors
        self.assertLess(abs(cl_0), 0.1)
        self.assertLess(abs(cl_180), 0.1)
    
    def test_drag_always_positive(self):
        """CD should always be positive."""
        for aoa in range(-180, 181, 10):
            _, cd, _ = extrapolate_viterna(float(aoa), self.coeffs)
            self.assertGreater(cd, 0, f"CD should be positive at {aoa}°")
    
    def test_max_drag_near_90(self):
        """CD should be maximum near ±90°."""
        _, cd_90, _ = extrapolate_viterna(90.0, self.coeffs)
        _, cd_neg90, _ = extrapolate_viterna(-90.0, self.coeffs)
        _, cd_0, _ = extrapolate_viterna(0.0, self.coeffs)
        _, cd_45, _ = extrapolate_viterna(45.0, self.coeffs)
        
        # CD at 90° should be higher than at other angles
        self.assertGreater(cd_90, cd_0)
        self.assertGreater(cd_90, cd_45)
        self.assertGreater(cd_neg90, cd_0)
    
    def test_symmetry_properties(self):
        """Test symmetry properties of coefficients."""
        # CL should be antisymmetric around 0° for symmetric airfoils
        # (Note: This test is approximate since Viterna uses stall point data)
        cl_30, _, _ = extrapolate_viterna(30.0, self.coeffs)
        cl_150, _, _ = extrapolate_viterna(150.0, self.coeffs)
        
        # At 150°, should have similar magnitude to 30° but different sign
        # (both are post-stall)
        self.assertIsNotNone(cl_30)
        self.assertIsNotNone(cl_150)
    
    def test_angle_normalization(self):
        """Test that angles outside ±180° are normalized correctly."""
        cl_45, cd_45, cm_45 = extrapolate_viterna(45.0, self.coeffs)
        cl_405, cd_405, cm_405 = extrapolate_viterna(405.0, self.coeffs)  # 405 = 45 + 360
        
        # Should produce identical results
        self.assertAlmostEqual(cl_45, cl_405, places=6)
        self.assertAlmostEqual(cd_45, cd_405, places=6)
        self.assertAlmostEqual(cm_45, cm_405, places=6)


class TestViternaApplication(unittest.TestCase):
    """Test high-level Viterna application functions."""
    
    def test_apply_viterna_extrapolation(self):
        """Test applying Viterna to get ComputedXfoilValue."""
        positive_stall = StallPoint(aoa_deg=12.0, cl=1.4, cd=0.025, cm=-0.08)
        negative_stall = StallPoint(aoa_deg=-10.0, cl=-1.2, cd=0.030, cm=-0.06)
        
        # Test positive post-stall angle
        value = apply_viterna_extrapolation(
            aoa_deg=30.0,
            positive_stall=positive_stall,
            negative_stall=negative_stall,
            cd_max=1.8
        )
        
        self.assertIsInstance(value, ComputedXfoilValue)
        self.assertAlmostEqual(value.angle_of_attack, 30.0)
        self.assertGreater(value.drag_coefficient, 0)
        self.assertTrue(math.isfinite(value.lift_coefficient))
        
        # Test negative post-stall angle
        value = apply_viterna_extrapolation(
            aoa_deg=-30.0,
            positive_stall=positive_stall,
            negative_stall=negative_stall,
            cd_max=1.8
        )
        
        self.assertIsInstance(value, ComputedXfoilValue)
        self.assertAlmostEqual(value.angle_of_attack, -30.0)
        self.assertGreater(value.drag_coefficient, 0)
    
    def test_pressure_drag_equals_total_drag(self):
        """Post-stall, pressure drag should equal total drag."""
        positive_stall = StallPoint(aoa_deg=12.0, cl=1.4, cd=0.025, cm=-0.08)
        negative_stall = StallPoint(aoa_deg=-10.0, cl=-1.2, cd=0.030, cm=-0.06)
        
        value = apply_viterna_extrapolation(
            aoa_deg=45.0,
            positive_stall=positive_stall,
            negative_stall=negative_stall
        )
        
        # Post-stall, all drag is pressure drag
        self.assertAlmostEqual(
            value.drag_coefficient, 
            value.pressure_drag_coefficient,
            places=6
        )


class TestStallDetection(unittest.TestCase):
    """Test stall point detection from xfoil data."""
    
    def test_detect_positive_stall(self):
        """Test detection of positive stall point."""
        # Create synthetic data with a clear stall at 12°
        aoa_list = [-5.0, 0.0, 5.0, 10.0, 12.0, 14.0, 16.0]
        values = [
            ComputedXfoilValue(-5.0, -0.5, 0.02, 0.02, -0.05, 0, 0),
            ComputedXfoilValue(0.0, 0.0, 0.015, 0.015, -0.04, 0, 0),
            ComputedXfoilValue(5.0, 0.6, 0.018, 0.018, -0.06, 0, 0),
            ComputedXfoilValue(10.0, 1.2, 0.022, 0.022, -0.07, 0, 0),
            ComputedXfoilValue(12.0, 1.4, 0.025, 0.025, -0.08, 0, 0),  # Stall point
            ComputedXfoilValue(14.0, 1.3, 0.035, 0.035, -0.07, 0, 0),
            ComputedXfoilValue(16.0, 1.1, 0.050, 0.050, -0.06, 0, 0),
        ]
        
        stall = detect_stall_point(values, aoa_list, search_positive=True)
        
        self.assertIsNotNone(stall)
        self.assertAlmostEqual(stall.aoa_deg, 12.0, places=1)
        self.assertAlmostEqual(stall.cl, 1.4, places=1)
    
    def test_detect_negative_stall(self):
        """Test detection of negative stall point."""
        aoa_list = [-16.0, -14.0, -10.0, -5.0, 0.0, 5.0]
        values = [
            ComputedXfoilValue(-16.0, -1.0, 0.050, 0.050, -0.05, 0, 0),
            ComputedXfoilValue(-14.0, -1.2, 0.035, 0.035, -0.06, 0, 0),
            ComputedXfoilValue(-10.0, -1.3, 0.025, 0.025, -0.07, 0, 0),  # Stall point
            ComputedXfoilValue(-5.0, -0.6, 0.018, 0.018, -0.05, 0, 0),
            ComputedXfoilValue(0.0, 0.0, 0.015, 0.015, -0.04, 0, 0),
            ComputedXfoilValue(5.0, 0.6, 0.018, 0.018, -0.06, 0, 0),
        ]
        
        stall = detect_stall_point(values, aoa_list, search_positive=False)
        
        self.assertIsNotNone(stall)
        self.assertAlmostEqual(stall.aoa_deg, -10.0, places=1)
        self.assertAlmostEqual(stall.cl, -1.3, places=1)
    
    def test_insufficient_data(self):
        """Test that None is returned with insufficient data."""
        aoa_list = [0.0, 5.0]
        values = [
            ComputedXfoilValue(0.0, 0.0, 0.015, 0.015, -0.04, 0, 0),
            ComputedXfoilValue(5.0, 0.6, 0.018, 0.018, -0.06, 0, 0),
        ]
        
        stall = detect_stall_point(values, aoa_list, search_positive=True)
        self.assertIsNone(stall)


class TestPhysicalConstraints(unittest.TestCase):
    """Test that Viterna satisfies physical constraints."""
    
    def test_finite_values_everywhere(self):
        """All values should be finite across full angle range."""
        stall = StallPoint(aoa_deg=12.0, cl=1.4, cd=0.025, cm=-0.08)
        coeffs = compute_viterna_coefficients(stall, cd_max=1.8)
        
        for aoa in range(-180, 181, 1):
            cl, cd, cm = extrapolate_viterna(float(aoa), coeffs)
            
            self.assertTrue(math.isfinite(cl), f"CL infinite at {aoa}°")
            self.assertTrue(math.isfinite(cd), f"CD infinite at {aoa}°")
            self.assertTrue(math.isfinite(cm), f"CM infinite at {aoa}°")
    
    def test_reasonable_magnitude(self):
        """Coefficients should have reasonable magnitudes."""
        stall = StallPoint(aoa_deg=12.0, cl=1.4, cd=0.025, cm=-0.08)
        coeffs = compute_viterna_coefficients(stall, cd_max=1.8)
        
        for aoa in range(-180, 181, 10):
            cl, cd, cm = extrapolate_viterna(float(aoa), coeffs)
            
            # CL should be bounded (flat plate theory: -2π to 2π)
            self.assertLess(abs(cl), 10, f"CL unreasonable at {aoa}°")
            
            # CD should be bounded (max around 2.0 for flat plate)
            self.assertLess(cd, 5.0, f"CD unreasonable at {aoa}°")
            
            # CM should be reasonable
            self.assertLess(abs(cm), 1.0, f"CM unreasonable at {aoa}°")


if __name__ == '__main__':
    unittest.main()

