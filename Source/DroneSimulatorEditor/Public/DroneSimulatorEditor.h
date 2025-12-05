#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FAssetTypeActions_Base;
class SDockTab;
class FSpawnTabArgs;
class UFlightRecordAsset;
class STimelinePanel;

class DRONESIMULATOREDITOR_API FDroneSimulatorEditorModule : public IModuleInterface
{
public:

	virtual void StartupModule() override;

	virtual void ShutdownModule() override;

	static const FName timeline_panel_tab_name;

	/** Open a flight record in the timeline panel */
	void open_flight_record_in_timeline(UFlightRecordAsset* in_flight_record);

	/** Automatically load the latest flight record (called after PIE ends) */
	void auto_load_latest_flight_record();

private:

	void register_asset_tools();
	void unregister_asset_tools();

	void register_detail_customizations();
	void unregister_detail_customizations();

	TArray<TSharedPtr<FAssetTypeActions_Base>> registered_asset_type_actions;
	
	void register_timeline_tab();
	void unregister_timeline_tab();

	TSharedRef<SDockTab> spawn_timeline_tab(const FSpawnTabArgs& args);

	/** Weak pointer to the flight record that should be opened */
	TWeakObjectPtr<UFlightRecordAsset> pending_flight_record;

	/** Keep track of open timeline tabs by flight record */
	TMap<UFlightRecordAsset*, TWeakPtr<SDockTab>> open_timeline_tabs;

	/** Handle for PIE end event */
	FDelegateHandle pie_end_handle;

	/** Handle for PIE begin event */
	FDelegateHandle pie_begin_handle;

	/** Callback for when PIE ends */
	void on_pie_ended(bool is_simulating);

	/** Callback for when PIE begins */
	void on_pie_began(bool is_simulating);
};
