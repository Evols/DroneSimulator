#include "Engine/StaticMeshActor.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

struct FScopedTestWorld
{
private:
    FTestWorldWrapper world_wrapper;
    AStaticMeshActor* actor;
    FAutomationTestBase* automation_test;

public:

    FScopedTestWorld(FAutomationTestBase* in_automation_test) : actor(nullptr), automation_test(in_automation_test)
    {
        if (!this->world_wrapper.CreateTestWorld(EWorldType::Game))
        {
            this->automation_test->AddError(TEXT("Failed to create test world."));
        }

        if (!this->world_wrapper.BeginPlayInTestWorld())
        {
            this->automation_test->AddError(TEXT("Failed to BeginPlay in test world."));
            return;
        }
    }

    UWorld* get_world() const
    {
        return this->world_wrapper.GetTestWorld();
    }

    AStaticMeshActor* spawn_sphere_actor(float mass_kg)
    {
        actor = world_wrapper.GetTestWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass());

        auto* component = actor->GetStaticMeshComponent();
        if (!component)
        {
            this->automation_test->AddError(TEXT("No static mesh component in spawned static mesh actor."));
            return nullptr;
        }

        auto* sphere_mesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
        if (!sphere_mesh)
        {
            this->automation_test->AddError(TEXT("Unable to load the sphere mesh."));
            return nullptr;
        }

        component->SetMobility(EComponentMobility::Movable);
        component->SetStaticMesh(sphere_mesh);
        component->SetSimulatePhysics(true);
        component->SetEnableGravity(false);
        component->SetAngularDamping(0.f);
        component->SetLinearDamping(0.f);
        component->SetMassOverrideInKg(NAME_None, mass_kg, true);

        return actor;
    }

    void tick(float delta_time)
    {
        const auto tick_result = this->world_wrapper.TickTestWorld(delta_time);
        if (!tick_result)
        {
            this->automation_test->AddError(TEXT("Failed to tick test world."));
        }
    }

    ~FScopedTestWorld()
    {
        if (this->actor != nullptr)
        {
            this->world_wrapper.GetTestWorld()->DestroyActor(this->actor);
            this->actor = nullptr;
        }
    }
};

BEGIN_DEFINE_SPEC(FUnrealTestsSpec, "DroneSimulator.UnrealTests",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
END_DEFINE_SPEC(FUnrealTestsSpec)

void FUnrealTestsSpec::Define()
{
    this->It("Rotator winding", [this]
    {
        this->TestNearlyEqual(TEXT("Pitch winding"), FRotator(90.f, 0.f, 0.f).RotateVector(FVector::ForwardVector), FVector::UpVector);
        this->TestNearlyEqual(TEXT("Yaw winding"), FRotator(0.f, 90.f, 0.f).RotateVector(FVector::ForwardVector), FVector::RightVector);
        this->TestNearlyEqual(TEXT("Roll winding"), FRotator(0.f, 0.f, 90.f).RotateVector(FVector::UpVector), FVector::RightVector);
    });

    this->It("Rotator to euler", [this]
    {
        // We can see that the Euler angles follow the same conventions as rotators:
        // - left-hand rule for pitch and roll
        // - right-hand rule for yaw.
        this->TestNearlyEqual(TEXT("Pitch"), FRotator(90.f, 0.f, 0.f).Euler(), FVector(0.f, 90.f, 0.f), 0.1f);
        this->TestNearlyEqual(TEXT("Yaw"), FRotator(0.f, 90.f, 0.f).Euler(), FVector(0.f, 0.f, 90.f), 0.1f);
        this->TestNearlyEqual(TEXT("Roll"), FRotator(0.f, 0.f, 90.f).Euler(), FVector(90.f, 0.f, 0.f), 0.1f);
    });

    Describe("Test chaos", [this]()
    {
        It("Add roll torque", [this]()
        {
            /*
             * Add roll torque.
             *
             * The conclusion is that the roll torque and angular velocity have the same direction (right-hand rule),
             * but the rotation follows the left-hand rule (when looking from the positive axis towards the origin).
             * So a *positive* roll torque results in a *positive* angular velocity, and a *negative* roll rotation.
             */

            FScopedTestWorld test_world(this);

            auto* actor = test_world.spawn_sphere_actor(1.f);
            auto* component = actor->GetStaticMeshComponent();

            // Make sure the moment of inertia is as expected
            constexpr auto ideal_moment_of_inertia = 1000.0;
            this->TestNearlyEqual(TEXT("Moment of inertia - Roll"), component->GetInertiaTensor().X, ideal_moment_of_inertia, 1.0);

            // Tick 100 times with 10ms delta time, to sum up to 1s
            constexpr float delta_time = 0.01;
            for (int32 i = 0; i < 100; ++i)
            {
                component->AddTorqueInRadians(FVector(ideal_moment_of_inertia * FMath::DegreesToRadians(180.0), 0.0, 0.0));
                test_world.tick(delta_time);
            }

            // Check rotation speed after 1s of applying torque
            const auto angular_velocity_x = FMath::RadiansToDegrees(component->BodyInstance.GetUnrealWorldAngularVelocityInRadians().X);
            this->TestNearlyEqual(TEXT("Angular velocity - Roll"), angular_velocity_x, 180.0, 2.0);

            // Check rotation after 1s of applying torque
            this->TestNearlyEqual(TEXT("Rotation - Roll"), component->GetComponentRotation().Roll, -90.0, 2.0);
        });

        It("Add pitch torque", [this]()
        {
            /*
             * Add pitch torque.
             *
             * The conclusion is that the torque and angular velocity have the same direction (right-hand rule),
             * but the rotation follows the left-hand rule (when looking from the positive axis towards the origin).
             * So a *positive* pitch torque results in a *positive* angular velocity, and a *negative* pitch rotation.
             */

            FScopedTestWorld test_world(this);

            auto* actor = test_world.spawn_sphere_actor(1.f);
            auto* component = actor->GetStaticMeshComponent();

            // Make sure the moment of inertia is as expected
            const auto ideal_moment_of_inertia = 1000.0;
            this->TestNearlyEqual(TEXT("Moment of inertia - Pitch"), component->GetInertiaTensor().Y, ideal_moment_of_inertia, 1.0);

            // Tick 100 times with 10ms delta time, to sum up to 1s
            constexpr float delta_time = 0.01;
            for (int32 i = 0; i < 100; ++i)
            {
                component->AddTorqueInRadians(FVector(0.0, ideal_moment_of_inertia * FMath::DegreesToRadians(140.0), 0.0));
                test_world.tick(delta_time);
            }

            // Check rotation speed after 1s of applying torque
            const auto angular_velocity_y = FMath::RadiansToDegrees(component->BodyInstance.GetUnrealWorldAngularVelocityInRadians().Y);
            this->TestNearlyEqual(TEXT("Angular velocity - Pitch"), angular_velocity_y, 140.0, 2.0);

            // Check rotation after 1s of applying torque
            this->TestNearlyEqual(TEXT("Rotation - Pitch"), component->GetComponentRotation().Pitch, -70.0, 2.0);
        });

        It("Add yaw torque", [this]()
        {
            /*
             * Add yaw torque.
             *
             * The conclusion is that the torque and angular velocity have the same direction (right-hand rule),
             * and the rotation also follows the right-hand rule (when looking from the positive axis towards the origin).
             * So a *positive* yaw torque results in a *positive* angular velocity, and a *positive* yaw rotation.
             * This is different from roll and pitch!
             */

            FScopedTestWorld test_world(this);

            auto* actor = test_world.spawn_sphere_actor(1.f);
            auto* component = actor->GetStaticMeshComponent();

            // Make sure the moment of inertia is as expected
            const auto ideal_moment_of_inertia = 1000.0;
            this->TestNearlyEqual(TEXT("Moment of inertia - Yaw"), component->GetInertiaTensor().Z, ideal_moment_of_inertia, 1.0);

            // Tick 100 times with 10ms delta time, to sum up to 1s
            constexpr float delta_time = 0.01;
            for (int32 i = 0; i < 100; ++i)
            {
                component->AddTorqueInRadians(FVector(0.0, 0.0, ideal_moment_of_inertia * FMath::DegreesToRadians(100.0)));
                test_world.tick(delta_time);
            }

            // Check rotation speed after 1s of applying torque
            const auto angular_velocity_z = FMath::RadiansToDegrees(component->BodyInstance.GetUnrealWorldAngularVelocityInRadians().Z);
            this->TestNearlyEqual(TEXT("Angular velocity - Yaw"), angular_velocity_z, 100.0, 2.0);

            // Check rotation after 1s of applying torque
            this->TestNearlyEqual(TEXT("Rotation - Yaw"), component->GetComponentRotation().Yaw, 50.0, 2.0);
        });
    });
}
