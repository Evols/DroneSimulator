#include "DroneSimulator/Gameplay/Recording/DroneFlightRecordingManager.h"
#include "DroneSimulator/Gameplay/DronePawn.h"
#include "DroneSimulator/Assets/FlightRecordAsset.h"

#include "EngineUtils.h"
#include "Runtime/AssetRegistry/Public/AssetRegistry/AssetRegistryModule.h"
#include "Runtime/CoreUObject/Public/UObject/SavePackage.h"
#include "DroneSimulator/DroneSimulator.h"

#if WITH_EDITOR
#include "Developer/AssetTools/Public/AssetToolsModule.h"
#endif

ADroneFlightRecordingManager::ADroneFlightRecordingManager()
{
	PrimaryActorTick.bCanEverTick = true;

	// We want to save the assets after physics is processed
	PrimaryActorTick.TickGroup = TG_PostPhysics;
}

void ADroneFlightRecordingManager::BeginPlay()
{
	Super::BeginPlay();

	this->setup_flight_record_asset();
}

void ADroneFlightRecordingManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Save any remaining recorded data when PIE ends
	this->record_flight_record_of_pawns();
	this->save_flight_record_asset();

	Super::EndPlay(EndPlayReason);
}

void ADroneFlightRecordingManager::setup_flight_record_asset()
{
#if WITH_EDITOR
	const FString base_path = TEXT("/Game/FlightRecords");
	const FString asset_base_name = FString::Printf(TEXT("FlightRecord_%s"), *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));

	FString package_name = base_path / asset_base_name;
	FString asset_name = asset_base_name;
	FAssetToolsModule& asset_tools_module = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	asset_tools_module.Get().CreateUniqueAssetName(package_name, TEXT(""), package_name, asset_name);

	UPackage* package = CreatePackage(*package_name);
	flight_record_asset = NewObject<UFlightRecordAsset>(package, *asset_name, RF_Public | RF_Standalone);

	FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").AssetCreated(flight_record_asset);

	this->save_flight_record_asset();
#endif
}

void ADroneFlightRecordingManager::save_flight_record_asset()
{
	if (this->flight_record_asset == nullptr)
	{
		UE_LOG(LogDroneSimulator, Warning, TEXT("Trying to save flight record asset but it is not created yet"));
		return;
	}

	auto* package = this->flight_record_asset->GetPackage();
	if (package == nullptr)
	{
		UE_LOG(LogDroneSimulator, Warning, TEXT("Trying to save flight record asset with no package"));
		return;
	}

	const auto _ = package->MarkPackageDirty();
	package->SetDirtyFlag(true);

	const FString file_path = FPackageName::LongPackageNameToFilename(package->GetName(), FPackageName::GetAssetPackageExtension());
	FSavePackageArgs save_arguments;
	save_arguments.TopLevelFlags = RF_Public | RF_Standalone;
	UPackage::SavePackage(package, this->flight_record_asset, *file_path, save_arguments);
}

void ADroneFlightRecordingManager::Tick(float delta_time)
{
	Super::Tick(delta_time);

	this->record_flight_record_of_pawns();
}

void ADroneFlightRecordingManager::record_flight_record_of_pawns()
{
	auto* world = GetWorld();
	if (world == nullptr || this->flight_record_asset == nullptr)
	{
		return;
	}

	for (TActorIterator<ADronePawn> actor_iterator(world); actor_iterator; ++actor_iterator)
	{
		auto pawn_flight_record = actor_iterator->consume_flight_record();

		TArray<FFlightRecordEvent> savable_flight_record;
		savable_flight_record.Reserve(pawn_flight_record.Num());
		for (const auto event : pawn_flight_record)
		{
			savable_flight_record.Push(FFlightRecordEvent(actor_iterator->GetFName(), event.event_time, event.event_tata));
		}

		this->flight_record_asset->events.Append(savable_flight_record);
	}
}

