#if WITH_DEV_AUTOMATION_TESTS

#include "DroneSimulator/RotorModel/Bemt/PropellerThrust.h"
#include "DroneSimulator/Simulation/Math.h"

#include "Runtime/Core/Public/Misc/AutomationTest.h"

BEGIN_DEFINE_SPEC(FPropellerThrustSpec, "DroneSimulator.PropellerThrust", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)
END_DEFINE_SPEC(FPropellerThrustSpec)

void FPropellerThrustSpec::Define()
{
	this->Describe("Axial velocity", [this]
	{
		this->It("Zero air velocity", [this]
		{
			const auto wind_velocity = FVector(0.0, 0.0, 0.0);
			const auto prop_velocity = FVector(0.0, 0.0, 0.0);
			const auto v_axial = simulation_bemt::compute_axial_velocity(FVector::UpVector, wind_velocity, prop_velocity);
			this->TestNearlyEqual(TEXT("Axial velocity"), v_axial, 0.0);
		});

		this->It("Downwind", [this]
		{
			const auto wind_velocity = FVector(0.0, 0.0, -20.0);
			const auto prop_velocity = FVector(0.0, 0.0, 0.0);
			const auto v_axial = simulation_bemt::compute_axial_velocity(FVector::UpVector, wind_velocity, prop_velocity);
			this->TestNearlyEqual(TEXT("Axial velocity"), v_axial, 20.0);
		});

		this->It("Drone goes up", [this]
		{
			const auto wind_velocity = FVector(0.0, 0.0, 0.0);
			const auto prop_velocity = FVector(0.0, 0.0, 20.0);
			const auto v_axial = simulation_bemt::compute_axial_velocity(FVector::UpVector, wind_velocity, prop_velocity);
			this->TestNearlyEqual(TEXT("Axial velocity"), v_axial, 20.0);
		});

		this->It("Drone goes up with wind", [this]
		{
			const auto wind_velocity = FVector(0.0, 0.0, 30.0);
			const auto prop_velocity = FVector(0.0, 0.0, 20.0);
			const auto v_axial = simulation_bemt::compute_axial_velocity(FVector::UpVector, wind_velocity, prop_velocity);
			this->TestNearlyEqual(TEXT("Axial velocity"), v_axial, -10.0);
		});
	});

	this->Describe("Propeller thrust", [this]
	{
		FDronePropellerBemt propeller;
		propeller.num_blades = 3;
		propeller.radius = 0.0635; // 0.0635 is 5 inch prop
		propeller.hub_radius = 0.015;
		propeller.chord = 0.02;
		propeller.pitch = 0.0762; // 0.0762 is 3 inch pitch

		// TODO
		// propeller.cl_alpha_per_rad = 6.0;
		// propeller.cd_0 = 0.012;
		// propeller.cd_induced_k = 0.01;
		// propeller.max_angle_of_attack = 18.0;

		constexpr auto air_density = 1.225;
		const auto wind_velocity = FVector::ZeroVector;

		this->It("Zero air velocity", [this, &propeller, air_density, wind_velocity]
		{
			const auto prop_velocity = FVector(0.0, 0.0, 0.0);

			constexpr auto angular_speed = math::rpm_to_rad_per_sec(10000.0);
			const auto [result, _] = simulation_bemt::compute_thrust_and_torque(angular_speed, FVector::UnitZ(), wind_velocity, prop_velocity, air_density, &propeller);

			const auto thrust_N = result.thrust;
			this->TestGreaterEqual("Thrust", thrust_N, 1.0);

			const auto angle_of_attack_deg = FMath::RadiansToDegrees(result.angle_of_attack);
			this->TestGreaterEqual("Angle of attack", angle_of_attack_deg, 5.0);
		});

		this->It("Zero air velocity continuity", [this, &propeller, air_density, wind_velocity]
		{
			const auto prop_velocity = FVector(0.0, 0.0, 0.0);

			constexpr auto angular_speed_1 = math::rpm_to_rad_per_sec(10000.0);
			const auto [result_1, _] = simulation_bemt::compute_thrust_and_torque(angular_speed_1, FVector::UnitZ(), wind_velocity, prop_velocity, air_density, &propeller);
			const auto thrust_1 = result_1.thrust;

			constexpr auto angular_speed_2 = math::rpm_to_rad_per_sec(15000.0);
			const auto [result_2, __] = simulation_bemt::compute_thrust_and_torque(angular_speed_2, FVector::UnitZ(), wind_velocity, prop_velocity, air_density, &propeller);
			const auto thrust_2 = result_2.thrust;

			this->TestGreaterThan("Thrust", thrust_2, thrust_1);
		});

		this->It("Negative inflow continuity", [this, &propeller, air_density, wind_velocity]
		{
			const auto prop_velocity = FVector(0.0, 0.0, -100.0); // 100 m/s = 360km/h

			constexpr auto angular_speed_1 = math::rpm_to_rad_per_sec(10000.0);
			const auto [result_1, _] = simulation_bemt::compute_thrust_and_torque(angular_speed_1, FVector::UnitZ(), wind_velocity, prop_velocity, air_density, &propeller);
			const auto thrust_1 = result_1.thrust;

			constexpr auto angular_speed_2 = math::rpm_to_rad_per_sec(15000.0);
			const auto [result_2, __] = simulation_bemt::compute_thrust_and_torque(angular_speed_2, FVector::UnitZ(), wind_velocity, prop_velocity, air_density, &propeller);
			const auto thrust_2 = result_2.thrust;

			this->TestGreaterThan("Thrust", thrust_2, thrust_1);
		});

		this->It("Small positive inflow", [this, &propeller, air_density, wind_velocity]
		{
			// With a very small positive inflow, we expect the drone to produce thrust, and to have a positive angle of attack

			const auto prop_velocity = FVector(0.0, 0.0, 1.0);

			constexpr auto angular_speed = math::rpm_to_rad_per_sec(10000.0);
			const auto [result, _] = simulation_bemt::compute_thrust_and_torque(angular_speed, FVector::UnitZ(), wind_velocity, prop_velocity, air_density, &propeller);
			const auto thrust = result.thrust;
			const auto angle_of_attack = FMath::RadiansToDegrees(result.angle_of_attack);

			this->TestGreaterThan("Thrust", thrust, 0.1);
			this->TestGreaterThan("Angle of attack", angle_of_attack, 0.1);
		});

		this->It("Positive inflow", [this, &propeller, air_density, wind_velocity]
		{
			const auto prop_velocity = FVector(0.0, 0.0, 100.0);

			constexpr auto angular_speed_1 = math::rpm_to_rad_per_sec(1000.0);
			const auto [result_1, _] = simulation_bemt::compute_thrust_and_torque(angular_speed_1, FVector::UnitZ(), wind_velocity, prop_velocity, air_density, &propeller);
			const auto thrust_1 = result_1.thrust;

			constexpr auto angular_speed_2 = math::rpm_to_rad_per_sec(2000.0);
			const auto [result_2, __] = simulation_bemt::compute_thrust_and_torque(angular_speed_2, FVector::UnitZ(), wind_velocity, prop_velocity, air_density, &propeller);
			const auto thrust_2 = result_2.thrust;

			this->TestGreaterThan("Thrust", thrust_2, thrust_1);
		});
	});
}

#endif // WITH_DEV_AUTOMATION_TESTS
