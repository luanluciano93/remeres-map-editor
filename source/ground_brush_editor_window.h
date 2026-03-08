//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#ifndef RME_GROUND_BRUSH_EDITOR_WINDOW_H_
#define RME_GROUND_BRUSH_EDITOR_WINDOW_H_

#include "main.h"
#include "sprite_item_listbox.h"

#include <wx/listctrl.h>

class GroundBrushEditorDialog : public wxDialog {
public:
	GroundBrushEditorDialog(wxWindow* parent);
	~GroundBrushEditorDialog();

private:
	// Working data for a single ground brush
	struct ItemEntry {
		uint16_t id;
		int chance;
	};

	struct BorderEntry {
		bool outer; // true=outer, false=inner
		bool super;
		uint32_t to; // 0xFFFFFFFF=all, 0=none, otherwise brush ID
		std::string to_name; // display name: "all", "none", or brush name
		uint32_t border_id; // AutoBorder ID reference
	};

	struct BrushData {
		std::string name;
		uint16_t look_id;
		int32_t z_order;
		bool randomize;
		bool use_only_optional;
		std::vector<ItemEntry> items;
		std::vector<BorderEntry> borders;
		uint32_t optional_border_id; // 0 = none
		std::vector<std::string> friends; // brush names
		bool hate_friends; // false=friends, true=enemies
	};

	std::vector<BrushData> brush_list;
	int current_selection = -1;
	uint16_t selected_item_id = 0;

	// Left panel: Brush list
	wxListBox* brush_listbox;
	wxButton* add_btn;
	wxButton* remove_btn;

	// Center panel: Properties
	wxTextCtrl* name_text;
	wxSpinCtrl* lookid_spin;
	wxSpinCtrl* zorder_spin;
	wxCheckBox* randomize_chkbox;
	wxCheckBox* solo_optional_chkbox;

	// Center panel: Items list
	BrushItemListBox* items_listctrl;
	wxSpinCtrl* chance_spin;
	wxButton* item_add_btn;
	wxButton* item_remove_btn;

	// Center panel: Borders list
	wxListCtrl* borders_listctrl;
	wxChoice* border_align_choice;
	wxTextCtrl* border_to_text;
	wxSpinCtrl* border_id_spin;
	wxButton* border_add_btn;
	wxButton* border_remove_btn;

	// Center panel: Optional border
	wxSpinCtrl* optional_border_spin;

	// Center panel: Friends/Enemies
	wxListBox* friends_listbox;
	wxRadioButton* friends_radio;
	wxRadioButton* enemies_radio;
	wxTextCtrl* friend_name_text;
	wxButton* friend_add_btn;
	wxButton* friend_remove_btn;

	// Right panel: Item picker
	wxTextCtrl* item_filter_text;
	ItemPickerListBox* item_list;
	wxStaticText* selected_item_label;

	// Event handlers
	void OnSelectBrush(wxCommandEvent &event);
	void OnAddBrush(wxCommandEvent &event);
	void OnRemoveBrush(wxCommandEvent &event);
	void OnAddItem(wxCommandEvent &event);
	void OnRemoveItem(wxCommandEvent &event);
	void OnAddBorder(wxCommandEvent &event);
	void OnRemoveBorder(wxCommandEvent &event);
	void OnAddFriend(wxCommandEvent &event);
	void OnRemoveFriend(wxCommandEvent &event);
	void OnItemFilterChange(wxCommandEvent &event);
	void OnSelectPickerItem(wxCommandEvent &event);
	void OnSave(wxCommandEvent &event);
	void OnCancel(wxCommandEvent &event);

	// Helpers
	void LoadBrushesFromMemory();
	bool SaveBrushesToXML();
	void RefreshBrushList();
	void UpdatePropertiesFromSelection();
	void SaveCurrentBrushData();
	void PopulateItemList(const wxString &filter);
	void RefreshItemsList();
	void RefreshBordersList();
	void RefreshFriendsList();
	wxString GetBrushDisplayName(const BrushData &brush) const;

	// Resolve brush ID to name
	std::string BrushIDToName(uint32_t id) const;

	DECLARE_EVENT_TABLE()
};

#endif
