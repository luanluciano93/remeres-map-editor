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

#ifndef RME_BORDER_EDITOR_WINDOW_H_
#define RME_BORDER_EDITOR_WINDOW_H_

#include "main.h"
#include "dcbutton.h"
#include "sprite_item_listbox.h"

class BorderEditorDialog : public wxDialog {
public:
	BorderEditorDialog(wxWindow* parent);
	~BorderEditorDialog();

private:
	// Working data for a single border
	struct BorderData {
		uint32_t id;
		uint16_t group;
		bool optional;
		uint32_t tiles[13]; // indexed by BorderType enum (0 unused, 1-12 = edges)
	};

	std::vector<BorderData> border_list;
	int current_selection = -1;
	uint16_t selected_item_id = 0;

	// Left panel: Border list
	wxListBox* border_listbox;
	wxButton* add_btn;
	wxButton* remove_btn;

	// Center panel: Properties + Edge grid
	wxSpinCtrl* id_spin;
	wxSpinCtrl* group_spin;
	wxCheckBox* optional_chkbox;
	DCButton* edge_buttons[13]; // index 0 = nullptr, 1-12 = edge buttons
	wxStaticText* edge_labels[13];

	// Right panel: Item picker
	wxTextCtrl* item_filter_text;
	ItemPickerListBox* item_list;
	wxStaticText* selected_item_label;

	// Event handlers
	void OnSelectBorder(wxCommandEvent &event);
	void OnAddBorder(wxCommandEvent &event);
	void OnRemoveBorder(wxCommandEvent &event);
	void OnEdgeClick(wxMouseEvent &event);
	void OnEdgeClear(wxMouseEvent &event);
	void OnItemFilterChange(wxCommandEvent &event);
	void OnSelectItem(wxCommandEvent &event);
	void OnPropertyChange(wxCommandEvent &event);
	void OnSave(wxCommandEvent &event);
	void OnCancel(wxCommandEvent &event);

	// Helpers
	void LoadBordersFromMemory();
	bool SaveBordersToXML();
	void RefreshBorderList();
	void UpdateEdgeGrid();
	void UpdatePropertiesFromSelection();
	void SaveCurrentBorderData();
	void PopulateItemList(const wxString &filter);
	wxString GetBorderDisplayName(const BorderData &border) const;

	static const char* GetEdgeName(int edgeId);
	static const char* GetEdgeXMLName(int edgeId);

	DECLARE_EVENT_TABLE()
};

#endif
