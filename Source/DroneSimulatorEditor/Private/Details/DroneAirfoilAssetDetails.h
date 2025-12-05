#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "Input/Reply.h"

class UDroneAirfoilAssetTable;
class UDroneAirfoilAssetSimplified;
struct FRawXfoilData;

/**
 * Custom details panel for UDroneAirfoilAsset
 * Displays airfoil data as tables and graphs
 */
class FDroneAirfoilAssetDetails : public IDetailCustomization
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view */
	static TSharedRef<IDetailCustomization> make_instance();

	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& detail_builder) override;

private:
	/** Creates a custom widget for displaying xfoil data as a table */
	TSharedRef<class SWidget> create_xfoil_data_table_widget(const FRawXfoilData& xfoil_data, const FText& table_title);
	
	/** Creates graph widgets for xfoil data */
	TSharedRef<class SWidget> create_xfoil_data_graph_widget(const FRawXfoilData& xfoil_data);
	
	/** Creates graph widgets for simplified airfoil model */
	TSharedRef<class SWidget> create_simplified_airfoil_graph_widget(const UDroneAirfoilAssetSimplified* simplified_asset);
	
	/** Weak pointer to the table asset being edited */
	TWeakObjectPtr<UDroneAirfoilAssetTable> airfoil_asset_table;
	
	/** Weak pointer to the simplified asset being edited */
	TWeakObjectPtr<UDroneAirfoilAssetSimplified> airfoil_asset_simplified;

	/** View mode state */
	TSharedPtr<bool> show_raw_as_graph;

	/** Shared state for hover tracking */
	struct FTableHoverState
	{
		int32 hovered_column_index = -1;
		TSharedPtr<class SWidget> last_hovered_widget;
	};
};

