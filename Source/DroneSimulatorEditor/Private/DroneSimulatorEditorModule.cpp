#include "DroneSimulatorEditor/Public/DroneSimulatorEditor.h"
#include "DroneSimulatorEditor/Private/TypeActions/AssetTypeActions_DroneBattery.h"
#include "DroneSimulatorEditor/Private/TypeActions/AssetTypeActions_DroneFrame.h"
#include "DroneSimulatorEditor/Private/TypeActions/AssetTypeActions_DroneMotor.h"
#include "DroneSimulatorEditor/Private/TypeActions/AssetTypeActions_DronePropeller.h"
#include "DroneSimulatorEditor/Private/TypeActions/AssetTypeActions_FlightRecord.h"
#include "DroneSimulatorEditor/Private/TypeActions/AssetTypeActions_DroneAirfoil.h"
#include "DroneSimulatorEditor/Private/TypeActions/AssetTypeActions_DroneAirfoilSimplified.h"
#include "DroneSimulatorEditor/Private/Widgets/STimelinePanel.h"
#include "DroneSimulatorEditor/Private/Playback/FlightPlaybackManager.h"
#include "DroneSimulatorEditor/Private/Details/DroneAirfoilAssetDetails.h"
#include "DroneSimulatorGame/Assets/FlightRecordAsset.h"
#include "DroneSimulatorGame/Assets/DroneAirfoilAsset.h"

#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "Modules/ModuleManager.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Widgets/Docking/SDockTab.h"
#include "LevelEditor.h"
#include "Editor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/AssetData.h"
#include "Misc/DateTime.h"
#include "PropertyEditorModule.h"

IMPLEMENT_MODULE(FDroneSimulatorEditorModule, DroneSimulatorEditor);

const FName FDroneSimulatorEditorModule::timeline_panel_tab_name = FName(TEXT("DroneTimelinePanel"));

void FDroneSimulatorEditorModule::StartupModule()
{
	register_asset_tools();
	register_detail_customizations();
	register_timeline_tab();
	
	// Register PIE callbacks - FEditorDelegates is always available in editor modules
	pie_begin_handle = FEditorDelegates::BeginPIE.AddRaw(this, &FDroneSimulatorEditorModule::on_pie_began);
	pie_end_handle = FEditorDelegates::EndPIE.AddRaw(this, &FDroneSimulatorEditorModule::on_pie_ended);
}

void FDroneSimulatorEditorModule::ShutdownModule()
{
	// Unregister PIE callbacks
	if (pie_begin_handle.IsValid())
	{
		FEditorDelegates::BeginPIE.Remove(pie_begin_handle);
		pie_begin_handle.Reset();
	}

	if (pie_end_handle.IsValid())
	{
		FEditorDelegates::EndPIE.Remove(pie_end_handle);
		pie_end_handle.Reset();
	}

	unregister_timeline_tab();
	unregister_detail_customizations();
	unregister_asset_tools();
}

void FDroneSimulatorEditorModule::register_asset_tools()
{
	FAssetToolsModule& asset_tools_module = FAssetToolsModule::GetModule();
	IAssetTools& asset_tools = asset_tools_module.Get();

	TSharedPtr<FAssetTypeActions_DronePropellerBemt> propeller_bemt_actions = MakeShared<FAssetTypeActions_DronePropellerBemt>();
	registered_asset_type_actions.Add(propeller_bemt_actions);
	asset_tools.RegisterAssetTypeActions(propeller_bemt_actions.ToSharedRef());

	TSharedPtr<FAssetTypeActions_DronePropellerSimplified> propeller_simplified_actions = MakeShared<FAssetTypeActions_DronePropellerSimplified>();
	registered_asset_type_actions.Add(propeller_simplified_actions);
	asset_tools.RegisterAssetTypeActions(propeller_simplified_actions.ToSharedRef());

	TSharedPtr<FAssetTypeActions_DroneFrame> frame_actions = MakeShared<FAssetTypeActions_DroneFrame>();
	registered_asset_type_actions.Add(frame_actions);
	asset_tools.RegisterAssetTypeActions(frame_actions.ToSharedRef());

	TSharedPtr<FAssetTypeActions_DroneMotor> motor_actions = MakeShared<FAssetTypeActions_DroneMotor>();
	registered_asset_type_actions.Add(motor_actions);
	asset_tools.RegisterAssetTypeActions(motor_actions.ToSharedRef());

	TSharedPtr<FAssetTypeActions_DroneBattery> battery_actions = MakeShared<FAssetTypeActions_DroneBattery>();
	registered_asset_type_actions.Add(battery_actions);
	asset_tools.RegisterAssetTypeActions(battery_actions.ToSharedRef());

	TSharedPtr<FAssetTypeActions_DroneAirfoil> airfoil_actions = MakeShared<FAssetTypeActions_DroneAirfoil>();
	registered_asset_type_actions.Add(airfoil_actions);
	asset_tools.RegisterAssetTypeActions(airfoil_actions.ToSharedRef());

	TSharedPtr<FAssetTypeActions_DroneAirfoilSimplified> airfoil_simplified_actions = MakeShared<FAssetTypeActions_DroneAirfoilSimplified>();
	registered_asset_type_actions.Add(airfoil_simplified_actions);
	asset_tools.RegisterAssetTypeActions(airfoil_simplified_actions.ToSharedRef());

	TSharedPtr<FAssetTypeActions_FlightRecord> flight_record_actions = MakeShared<FAssetTypeActions_FlightRecord>();
	registered_asset_type_actions.Add(flight_record_actions);
	asset_tools.RegisterAssetTypeActions(flight_record_actions.ToSharedRef());
}

void FDroneSimulatorEditorModule::unregister_asset_tools()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		FAssetToolsModule& asset_tools_module = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");
		IAssetTools& asset_tools = asset_tools_module.Get();

		for (const TSharedPtr<FAssetTypeActions_Base>& action : registered_asset_type_actions)
		{
			if (action.IsValid())
			{
				asset_tools.UnregisterAssetTypeActions(action.ToSharedRef());
			}
		}
	}

	registered_asset_type_actions.Empty();
}

void FDroneSimulatorEditorModule::register_detail_customizations()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	
	// Register the detail customization for UDroneAirfoilAssetTable
	PropertyModule.RegisterCustomClassLayout(
		UDroneAirfoilAssetTable::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FDroneAirfoilAssetDetails::make_instance)
	);

	// Register the detail customization for UDroneAirfoilAssetSimplified
	PropertyModule.RegisterCustomClassLayout(
		UDroneAirfoilAssetSimplified::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FDroneAirfoilAssetDetails::make_instance)
	);
}

void FDroneSimulatorEditorModule::unregister_detail_customizations()
{
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomClassLayout(UDroneAirfoilAssetTable::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(UDroneAirfoilAssetSimplified::StaticClass()->GetFName());
	}
}

void FDroneSimulatorEditorModule::register_timeline_tab()
{
	// Register with both the global tab manager and the level editor
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		timeline_panel_tab_name,
		FOnSpawnTab::CreateRaw(this, &FDroneSimulatorEditorModule::spawn_timeline_tab))
		.SetDisplayName(NSLOCTEXT("DroneSimulatorEditor", "TimelinePanelTabTitle", "Drone Timeline"))
		.SetTooltipText(NSLOCTEXT("DroneSimulatorEditor", "TimelinePanelTabTooltip", "Opens the Drone Timeline panel for playback control"))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetLevelEditorCategory())
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Cinematics"));

	// Also register with the Level Editor's tab manager for proper docking
	FLevelEditorModule& level_editor_module = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	TSharedPtr<FTabManager> LevelEditorTabManager = level_editor_module.GetLevelEditorTabManager();
	
	if (LevelEditorTabManager.IsValid())
	{
		LevelEditorTabManager->RegisterTabSpawner(
			timeline_panel_tab_name,
			FOnSpawnTab::CreateRaw(this, &FDroneSimulatorEditorModule::spawn_timeline_tab))
			.SetDisplayName(NSLOCTEXT("DroneSimulatorEditor", "TimelinePanelTabTitle", "Drone Timeline"))
			.SetTooltipText(NSLOCTEXT("DroneSimulatorEditor", "TimelinePanelTabTooltip", "Opens the Drone Timeline panel for playback control"))
			.SetGroup(WorkspaceMenu::GetMenuStructure().GetLevelEditorCategory())
			.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Cinematics"));
	}
}

void FDroneSimulatorEditorModule::unregister_timeline_tab()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(timeline_panel_tab_name);
	
	// Unregister from Level Editor if loaded
	if (FModuleManager::Get().IsModuleLoaded("LevelEditor"))
	{
		FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
		TSharedPtr<FTabManager> LevelEditorTabManager = LevelEditorModule.GetLevelEditorTabManager();
		
		if (LevelEditorTabManager.IsValid())
		{
			LevelEditorTabManager->UnregisterTabSpawner(timeline_panel_tab_name);
		}
	}
}

TSharedRef<SDockTab> FDroneSimulatorEditorModule::spawn_timeline_tab(const FSpawnTabArgs& args)
{
	UFlightRecordAsset* FlightRecordToOpen = pending_flight_record.Get();
	pending_flight_record.Reset();

	TSharedRef<STimelinePanel> TimelineWidget = SNew(STimelinePanel)
		.min_time(0.0f)
		.max_time(30.0f)
		.flight_record(FlightRecordToOpen);

	TSharedRef<SDockTab> NewTab = SNew(SDockTab)
		.TabRole(ETabRole::PanelTab)
		.Label(FlightRecordToOpen ? FText::FromString(FlightRecordToOpen->GetName()) : NSLOCTEXT("DroneSimulatorEditor", "TimelineTabLabel", "Drone Timeline"))
		[
			TimelineWidget
		];

	// Track the tab for this flight record
	if (FlightRecordToOpen)
	{
		open_timeline_tabs.Add(FlightRecordToOpen, NewTab);

		// Clean up the entry when the tab is closed
		NewTab->SetOnTabClosed(SDockTab::FOnTabClosedCallback::CreateLambda(
			[this, FlightRecordToOpen](TSharedRef<SDockTab> ClosedTab)
			{
				open_timeline_tabs.Remove(FlightRecordToOpen);
			}
		));
	}

	return NewTab;
}

void FDroneSimulatorEditorModule::open_flight_record_in_timeline(UFlightRecordAsset* in_flight_record)
{
	if (!in_flight_record)
	{
		return;
	}

	// Load the flight record into the global playback manager
	FFlightPlaybackManager::get().load_flight_record(in_flight_record);

	// Check if we already have a tab open for this flight record
	if (TWeakPtr<SDockTab>* existing_tab = open_timeline_tabs.Find(in_flight_record))
	{
		if (existing_tab->IsValid())
		{
			// Just bring the existing tab to front
			FGlobalTabmanager::Get()->DrawAttention(existing_tab->Pin().ToSharedRef());
			return;
		}
		else
		{
			// Tab is no longer valid, remove it
			open_timeline_tabs.Remove(in_flight_record);
		}
	}

	// Store the flight record to be opened
	pending_flight_record = in_flight_record;

	// Try to invoke the tab from the level editor tab manager first for proper docking
	if (FModuleManager::Get().IsModuleLoaded("LevelEditor"))
	{
		FLevelEditorModule& level_editor_module = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
		TSharedPtr<FTabManager> level_editor_tab_manager = level_editor_module.GetLevelEditorTabManager();
		
		if (level_editor_tab_manager.IsValid())
		{
			TSharedPtr<SDockTab> new_tab = level_editor_tab_manager->TryInvokeTab(timeline_panel_tab_name);
			if (new_tab.IsValid())
			{
				return;
			}
		}
	}

	// Fallback to global tab manager
	FGlobalTabmanager::Get()->TryInvokeTab(timeline_panel_tab_name);
}

void FDroneSimulatorEditorModule::on_pie_began(bool is_simulating)
{
	// Mark PIE as active and stop playback
	FFlightPlaybackManager& manager = FFlightPlaybackManager::get();
	manager.set_pie_active(true);
	manager.stop();
}

void FDroneSimulatorEditorModule::on_pie_ended(bool is_simulating)
{
	// Mark PIE as inactive
	FFlightPlaybackManager::get().set_pie_active(false);
	
	// Auto-load the latest flight record after PIE ends
	auto_load_latest_flight_record();
}

void FDroneSimulatorEditorModule::auto_load_latest_flight_record()
{
	// Get the asset registry
	FAssetRegistryModule& asset_registry_module = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& asset_registry = asset_registry_module.Get();

	// Find all FlightRecordAssets
	TArray<FAssetData> flight_record_assets;
	FARFilter filter;
	filter.ClassPaths.Add(UFlightRecordAsset::StaticClass()->GetClassPathName());
	filter.bRecursiveClasses = true;
	
	asset_registry.GetAssets(filter, flight_record_assets);

	if (flight_record_assets.Num() == 0)
	{
		return;
	}

	// Find the most recently modified asset
	FAssetData* latest_asset = nullptr;
	FDateTime latest_time = FDateTime::MinValue();

	for (FAssetData& asset_data : flight_record_assets)
	{
		// Get asset modification time from the asset registry tag
		FAssetDataTagMapSharedView::FFindTagResult found_tag = asset_data.TagsAndValues.FindTag("DateModified");
		
		// Try to get the actual file modification time
		FString package_file_path = FPackageName::LongPackageNameToFilename(asset_data.PackageName.ToString(), FPackageName::GetAssetPackageExtension());
		FDateTime file_time = IFileManager::Get().GetTimeStamp(*package_file_path);
		
		if (file_time > latest_time)
		{
			latest_time = file_time;
			latest_asset = &asset_data;
		}
	}

	// Load the latest asset
	if (latest_asset)
	{
		UFlightRecordAsset* flight_record = Cast<UFlightRecordAsset>(latest_asset->GetAsset());
		if (flight_record)
		{
			// Load it into the playback manager
			FFlightPlaybackManager::get().load_flight_record(flight_record);

			// If the timeline tab is already open, it will automatically update
			// Otherwise, optionally open it
			// Uncomment the next line to auto-open the timeline after PIE
			// OpenFlightRecordInTimeline(flight_record);
		}
	}
}
