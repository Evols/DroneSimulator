#if WITH_DEV_AUTOMATION_TESTS

#include "DroneSimulatorCore/RotorModel/Bemt/PropellerThrust.h"
#include "DroneSimulatorCore/Utils/Math.h"

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

		propeller.airfoil = FDroneAirfoil(FDroneAirfoilSimplified());

		constexpr auto air_density = 1.225;
		const auto wind_velocity = FVector::ZeroVector;

		this->It("Zero air velocity", [this, propeller, air_density, wind_velocity]
		{
			const auto prop_velocity = FVector(0.0, 0.0, 0.0);

			constexpr auto angular_speed = math::rpm_to_rad_per_sec(10000.0);
			const auto [result, _] = simulation_bemt::compute_thrust_and_torque(angular_speed, FVector::UnitZ(), wind_velocity, prop_velocity, air_density, &propeller);

			const auto thrust_N = result.thrust;
			this->TestGreaterEqual("Thrust", thrust_N, 1.0);

			const auto angle_of_attack_deg = FMath::RadiansToDegrees(result.angle_of_attack);
			this->TestGreaterEqual("Angle of attack", angle_of_attack_deg, 5.0);
		});

		this->It("Zero air velocity continuity", [this, propeller, air_density, wind_velocity]
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

		this->It("Negative inflow continuity", [this, propeller, air_density, wind_velocity]
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

		this->It("Small positive inflow", [this, propeller, air_density, wind_velocity]
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

		this->It("Positive inflow", [this, propeller, air_density, wind_velocity]
		{
			// Check throttle response in powered climb. At 100 m/s and 1000 RPM,
			// the rotor is windmilling; monotonic positive thrust is not a valid expectation.
			const auto prop_velocity = FVector(0.0, 0.0, 5.0);

			constexpr auto angular_speed_1 = math::rpm_to_rad_per_sec(10000.0);
			const auto [result_1, _] = simulation_bemt::compute_thrust_and_torque(angular_speed_1, FVector::UnitZ(), wind_velocity, prop_velocity, air_density, &propeller);
			const auto thrust_1 = result_1.thrust;

			constexpr auto angular_speed_2 = math::rpm_to_rad_per_sec(15000.0);
			const auto [result_2, __] = simulation_bemt::compute_thrust_and_torque(angular_speed_2, FVector::UnitZ(), wind_velocity, prop_velocity, air_density, &propeller);
			const auto thrust_2 = result_2.thrust;

			this->TestGreaterThan("Powered climb produces lift", thrust_1, 0.0);
			this->TestGreaterThan("Thrust", thrust_2, thrust_1);
		});

		this->It("Finite loads at high axial airflow", [this, propeller, air_density, wind_velocity]
		{
			for (const double axial_velocity : { -100.0, 100.0 })
			{
				for (const double rpm : { 1000.0, 2000.0 })
				{
					const auto [result, result_log] = simulation_bemt::compute_thrust_and_torque(math::rpm_to_rad_per_sec(rpm),
						FVector::UpVector, wind_velocity, FVector(0.0, 0.0, axial_velocity), air_density, &propeller);
					this->TestTrue(TEXT("Finite thrust"), FMath::IsFinite(result.thrust));
					this->TestTrue(TEXT("Finite torque"), FMath::IsFinite(result.torque));
					this->TestTrue(TEXT("Finite induction"), FMath::IsFinite(result.v_induced));
				}
			}
		});

		this->It("Continuous loads across zero axial airflow", [this, propeller, air_density]
		{
			for (const double rpm : { 3000.0, 10000.0, 20000.0 })
			{
				const double angular_speed = math::rpm_to_rad_per_sec(rpm);
				const auto [hover, hover_log] = simulation_bemt::compute_thrust_and_torque(angular_speed,
					FVector::UpVector, FVector::ZeroVector, FVector::ZeroVector, air_density, &propeller);
				this->TestGreaterThan(TEXT("Hover induces downstream airflow"), hover.v_induced, 0.0);
				this->TestGreaterThan(TEXT("Hover produces lift"), hover.thrust, 0.0);

				// Exercise both wind and drone motion, including the exact zero boundary.
				for (const double axial_velocity : { -1e-3, -1e-6, 0.0, 1e-6, 1e-3 })
				{
					const auto [result, result_log] = simulation_bemt::compute_thrust_and_torque(angular_speed,
						FVector::UpVector, FVector::ZeroVector, FVector(0.0, 0.0, axial_velocity), air_density, &propeller);
					const FString context = FString::Printf(TEXT("RPM %.0f, axial %.6f"), rpm, axial_velocity);
					this->TestTrue(context + TEXT(" finite loads"), FMath::IsFinite(result.thrust) && FMath::IsFinite(result.torque));
					this->TestGreaterThan(context + TEXT(" downstream induction"), result.v_induced, 0.0);
					this->TestNearlyEqual(context + TEXT(" continuous thrust"), result.thrust, hover.thrust, FMath::Abs(hover.thrust) * 0.005);
					this->TestNearlyEqual(context + TEXT(" continuous torque"), result.torque, hover.torque, FMath::Abs(hover.torque) * 0.005);
					this->TestNearlyEqual(context + TEXT(" continuous induction"), result.v_induced, hover.v_induced, FMath::Abs(hover.v_induced) * 0.005);

					const auto [wind_result, wind_log] = simulation_bemt::compute_thrust_and_torque(angular_speed,
						FVector::UpVector, FVector(0.0, 0.0, -axial_velocity), FVector::ZeroVector, air_density, &propeller);
					this->TestNearlyEqual(context + TEXT(" equivalent wind thrust"), wind_result.thrust, result.thrust);
					this->TestNearlyEqual(context + TEXT(" equivalent wind torque"), wind_result.torque, result.torque);
				}
			}
		});
	});
}

#endif // WITH_DEV_AUTOMATION_TESTS
