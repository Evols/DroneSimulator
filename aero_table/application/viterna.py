"""
Viterna post-stall extrapolation method.

This module implements the Viterna-Corrigan method for extrapolating
airfoil aerodynamic coefficients beyond stall to ±180 degrees.

References:
- Viterna, L.A. and Corrigan, R.D. (1981). "Fixed Pitch Rotor Performance of 
  Large Horizontal Axis Wind Turbines". DOE/NASA Workshop, Cleveland, OH.
"""

import math
from dataclasses import dataclass
from typing import Optional

from aero_table.application.xfoil_pol_parser import ComputedXfoilValue


@dataclass
class ViternaCoefficients:
    """Coefficients for Viterna extrapolation model."""
    A1: float  # CL coefficient for sin(2α) term
    A2: float  # CL coefficient for cos²(α)/sin(α) term
    B1: float  # CD coefficient for sin²(α) term
    B2: float  # CD coefficient for cos(α) term
    CM_constant: float  # Constant moment coefficient


@dataclass
class StallPoint:
    """Data at the stall point."""
    aoa_deg: float
    cl: float
    cd: float
    cm: float


def compute_viterna_coefficients(stall_point: StallPoint, cd_max: float = 1.8) -> ViternaCoefficients:
    """
    Compute Viterna coefficients from stall point data.
    
    Args:
        stall_point: Aerodynamic data at the stall angle
        cd_max: Maximum drag coefficient (typically 1.8-2.0 for airfoils)
        
    Returns:
        ViternaCoefficients for extrapolation
    """
    alpha_stall_rad = math.radians(stall_point.aoa_deg)
    
    sin_alpha = math.sin(alpha_stall_rad)
    cos_alpha = math.cos(alpha_stall_rad)
    
    # Viterna coefficients for lift
    A1 = cd_max / 2.0
    
    # Avoid division by zero near ±90 degrees
    if abs(cos_alpha) < 1e-6:
        A2 = 0.0
    else:
        A2 = (stall_point.cl - cd_max * sin_alpha * cos_alpha) * sin_alpha / (cos_alpha ** 2)
    
    # Viterna coefficients for drag
    B1 = cd_max
    B2 = stall_point.cd - cd_max * (sin_alpha ** 2)
    
    return ViternaCoefficients(
        A1=A1,
        A2=A2,
        B1=B1,
        B2=B2,
        CM_constant=stall_point.cm
    )


def extrapolate_viterna(aoa_deg: float, coeffs: ViternaCoefficients) -> tuple[float, float, float]:
    """
    Extrapolate CL, CD, CM using Viterna method.
    
    Args:
        aoa_deg: Angle of attack in degrees
        coeffs: Viterna coefficients
        
    Returns:
        Tuple of (CL, CD, CM)
    """
    # Normalize angle to [-180, 180] range
    alpha_deg = aoa_deg
    while alpha_deg > 180:
        alpha_deg -= 360
    while alpha_deg < -180:
        alpha_deg += 360
    
    alpha_rad = math.radians(alpha_deg)
    sin_alpha = math.sin(alpha_rad)
    cos_alpha = math.cos(alpha_rad)
    
    # Viterna lift coefficient
    # CL = A1 * sin(2α) + A2 * cos²(α) / sin(α)
    sin_2alpha = math.sin(2 * alpha_rad)
    
    # Handle singularity at α = 0, ±180 degrees
    if abs(sin_alpha) < 1e-6:
        cl = coeffs.A1 * sin_2alpha
    else:
        cl = coeffs.A1 * sin_2alpha + coeffs.A2 * (cos_alpha ** 2) / sin_alpha
    
    # Viterna drag coefficient
    # CD = B1 * sin²(α) + B2 * cos(α)
    cd = coeffs.B1 * (sin_alpha ** 2) + coeffs.B2 * cos_alpha
    
    # Ensure CD is always positive (physical constraint)
    cd = max(cd, 0.01)  # Minimum drag coefficient
    
    # Moment coefficient (typically constant or linearly interpolated)
    cm = coeffs.CM_constant
    
    return cl, cd, cm


def apply_viterna_extrapolation(
    aoa_deg: float,
    positive_stall: StallPoint,
    negative_stall: StallPoint,
    cd_max: float = 1.8
) -> ComputedXfoilValue:
    """
    Apply Viterna extrapolation for a given angle of attack.
    
    Args:
        aoa_deg: Angle of attack in degrees
        positive_stall: Stall point for positive angles
        negative_stall: Stall point for negative angles
        cd_max: Maximum drag coefficient
        
    Returns:
        ComputedXfoilValue with extrapolated coefficients
    """
    # Determine which stall point to use based on angle of attack
    if aoa_deg >= 0:
        stall_point = positive_stall
    else:
        stall_point = negative_stall
    
    # Compute Viterna coefficients
    coeffs = compute_viterna_coefficients(stall_point, cd_max)
    
    # Extrapolate
    cl, cd, cm = extrapolate_viterna(aoa_deg, coeffs)
    
    # Create value object (use zeros for transition points as they're not meaningful post-stall)
    return ComputedXfoilValue(
        angle_of_attack=aoa_deg,
        lift_coefficient=cl,
        drag_coefficient=cd,
        pressure_drag_coefficient=cd,  # Post-stall, assume all drag is pressure drag
        moment_coefficient=cm,
        top_transition=0.0,
        bottom_transition=0.0
    )


def detect_stall_point(
    values: list[Optional[ComputedXfoilValue]], 
    aoa_list: list[float],
    search_positive: bool = True
) -> Optional[StallPoint]:
    """
    Detect the stall point from xfoil data.
    
    For positive stall: finds the maximum CL in positive angles
    For negative stall: finds the minimum CL in negative angles
    
    Args:
        values: List of computed xfoil values (may contain None)
        aoa_list: List of angles of attack
        search_positive: True for positive stall, False for negative stall
        
    Returns:
        StallPoint if found, None otherwise
    """
    valid_data = []
    for aoa, value in zip(aoa_list, values):
        if value is not None:
            if search_positive and aoa > 0:
                valid_data.append((aoa, value))
            elif not search_positive and aoa < 0:
                valid_data.append((aoa, value))
    
    if len(valid_data) < 3:
        return None
    
    # Find local extremum (max for positive, min for negative)
    stall_idx = None
    if search_positive:
        # Look for first local maximum in CL
        for i in range(1, len(valid_data) - 1):
            prev_cl = valid_data[i - 1][1].lift_coefficient
            curr_cl = valid_data[i][1].lift_coefficient
            next_cl = valid_data[i + 1][1].lift_coefficient
            
            if prev_cl < curr_cl and curr_cl > next_cl:
                stall_idx = i
                break
        
        # Fallback: use global maximum
        if stall_idx is None:
            stall_idx = max(range(len(valid_data)), 
                          key=lambda i: valid_data[i][1].lift_coefficient)
    else:
        # Look for first local minimum in CL
        for i in range(1, len(valid_data) - 1):
            prev_cl = valid_data[i - 1][1].lift_coefficient
            curr_cl = valid_data[i][1].lift_coefficient
            next_cl = valid_data[i + 1][1].lift_coefficient
            
            if prev_cl > curr_cl and curr_cl < next_cl:
                stall_idx = i
                break
        
        # Fallback: use global minimum
        if stall_idx is None:
            stall_idx = min(range(len(valid_data)), 
                          key=lambda i: valid_data[i][1].lift_coefficient)
    
    stall_aoa, stall_value = valid_data[stall_idx]
    
    return StallPoint(
        aoa_deg=stall_aoa,
        cl=stall_value.lift_coefficient,
        cd=stall_value.drag_coefficient,
        cm=stall_value.moment_coefficient
    )

