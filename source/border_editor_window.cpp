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

#include "main.h"

#include "border_editor_window.h"
#include "brush.h"
#include "brush_enums.h"
#include "items.h"
#include "gui.h"
#include "gui_ids.h"

#include <pugixml.hpp>

// Edge layout for the grid:
//   Row 0:  [N]  [E]  [CNW] [CNE] [DNW] [DNE]
//   Row 1:  [S]  [W]  [CSW] [CSE] [DSW] [DSE]

BEGIN_EVENT_TABLE(BorderEditorDialog, wxDialog)
EVT_LISTBOX(BORDER_EDITOR_LISTBOX, BorderEditorDialog::OnSelectBorder)
EVT_BUTTON(BORDER_EDITOR_ADD, BorderEditorDialog::OnAddBorder)
EVT_BUTTON(BORDER_EDITOR_REMOVE, BorderEditorDialog::OnRemoveBorder)
EVT_TEXT(BORDER_EDITOR_ITEM_FILTER, BorderEditorDialog::OnItemFilterChange)
EVT_LISTBOX(BORDER_EDITOR_ITEM_LIST, BorderEditorDialog::OnSelectItem)
EVT_BUTTON(wxID_OK, BorderEditorDialog::OnSave)
EVT_BUTTON(wxID_CANCEL, BorderEditorDialog::OnCancel)
END_EVENT_TABLE()

const char* BorderEditorDialog::GetEdgeName(int edgeId) {
	switch (edgeId) {
		case NORTH_HORIZONTAL: return "N";
		case EAST_HORIZONTAL: return "E";
		case SOUTH_HORIZONTAL: return "S";
		case WEST_HORIZONTAL: return "W";
		case NORTHWEST_CORNER: return "CNW";
		case NORTHEAST_CORNER: return "CNE";
		case SOUTHWEST_CORNER: return "CSW";
		case SOUTHEAST_CORNER: return "CSE";
		case NORTHWEST_DIAGONAL: return "DNW";
		case NORTHEAST_DIAGONAL: return "DNE";
		case SOUTHEAST_DIAGONAL: return "DSE";
		case SOUTHWEST_DIAGONAL: return "DSW";
		default: return "?";
	}
}

const char* BorderEditorDialog::GetEdgeXMLName(int edgeId) {
	switch (edgeId) {
		case NORTH_HORIZONTAL: return "n";
		case EAST_HORIZONTAL: return "e";
		case SOUTH_HORIZONTAL: return "s";
		case WEST_HORIZONTAL: return "w";
		case NORTHWEST_CORNER: return "cnw";
		case NORTHEAST_CORNER: return "cne";
		case SOUTHWEST_CORNER: return "csw";
		case SOUTHEAST_CORNER: return "cse";
		case NORTHWEST_DIAGONAL: return "dnw";
		case NORTHEAST_DIAGONAL: return "dne";
		case SOUTHEAST_DIAGONAL: return "dse";
		case SOUTHWEST_DIAGONAL: return "dsw";
		default: return "";
	}
}

BorderEditorDialog::BorderEditorDialog(wxWindow* parent) :
	wxDialog(parent, wxID_ANY, "Border Manager", wxDefaultPosition, wxSize(950, 600), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER) {
	// Initialize edge_buttons to nullptr
	for (int i = 0; i < 13; i++) {
		edge_buttons[i] = nullptr;
		edge_labels[i] = nullptr;
	}

	wxBoxSizer* main_sizer = newd wxBoxSizer(wxHORIZONTAL);

	// ========== LEFT PANEL: Border List ==========
	wxStaticBoxSizer* left_sizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Borders");

	border_listbox = newd wxListBox(this, BORDER_EDITOR_LISTBOX, wxDefaultPosition, wxSize(180, -1));
	left_sizer->Add(border_listbox, 1, wxEXPAND | wxALL, 5);

	wxBoxSizer* btn_sizer = newd wxBoxSizer(wxHORIZONTAL);
	add_btn = newd wxButton(this, BORDER_EDITOR_ADD, "Add Border");
	remove_btn = newd wxButton(this, BORDER_EDITOR_REMOVE, "Remove Border");
	btn_sizer->Add(add_btn, 1, wxRIGHT, 5);
	btn_sizer->Add(remove_btn, 1);
	left_sizer->Add(btn_sizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

	main_sizer->Add(left_sizer, 0, wxEXPAND | wxALL, 5);

	// ========== CENTER PANEL: Properties + Grid ==========
	wxBoxSizer* center_sizer = newd wxBoxSizer(wxVERTICAL);

	// Properties
	wxStaticBoxSizer* props_sizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Border Properties");

	wxFlexGridSizer* props_grid = newd wxFlexGridSizer(3, 10, 10);
	props_grid->Add(newd wxStaticText(this, wxID_ANY, "Auto ID:"), 0, wxALIGN_CENTER_VERTICAL);
	id_spin = newd wxSpinCtrl(this, wxID_ANY, "0", wxDefaultPosition, wxSize(80, -1), wxSP_ARROW_KEYS, 1, 9999);
	props_grid->Add(id_spin, 0);

	props_grid->Add(newd wxStaticText(this, wxID_ANY, "Group:"), 0, wxALIGN_CENTER_VERTICAL);
	group_spin = newd wxSpinCtrl(this, wxID_ANY, "0", wxDefaultPosition, wxSize(80, -1), wxSP_ARROW_KEYS, 0, 100);
	props_grid->Add(group_spin, 0);

	optional_chkbox = newd wxCheckBox(this, wxID_ANY, "Optional");
	props_grid->Add(optional_chkbox, 0, wxALIGN_CENTER_VERTICAL);

	props_sizer->Add(props_grid, 0, wxALL, 5);
	center_sizer->Add(props_sizer, 0, wxEXPAND | wxBOTTOM, 5);

	// Edge Grid
	wxStaticBoxSizer* grid_box = newd wxStaticBoxSizer(wxVERTICAL, this, "Border Grid (Click to assign Item ID)");

	// Layout: 2 rows x 6 columns
	// Row 0: N, E, CNW, CNE, DNW, DNE
	// Row 1: S, W, CSW, CSE, DSW, DSE
	wxFlexGridSizer* edge_grid = newd wxFlexGridSizer(6, 5, 5);

	// Edge IDs for each cell in row-major order
	int edge_order[] = {
		NORTH_HORIZONTAL, EAST_HORIZONTAL,
		NORTHWEST_CORNER, NORTHEAST_CORNER,
		NORTHWEST_DIAGONAL, NORTHEAST_DIAGONAL,
		SOUTH_HORIZONTAL, WEST_HORIZONTAL,
		SOUTHWEST_CORNER, SOUTHEAST_CORNER,
		SOUTHWEST_DIAGONAL, SOUTHEAST_DIAGONAL,
	};

	for (int i = 0; i < 12; i++) {
		int edgeId = edge_order[i];
		wxBoxSizer* cell_sizer = newd wxBoxSizer(wxVERTICAL);

		edge_buttons[edgeId] = newd DCButton(this, BORDER_EDITOR_EDGE_BASE + edgeId,
			wxDefaultPosition, DC_BTN_NORMAL, RENDER_SIZE_32x32, 0);
		edge_buttons[edgeId]->SetMinSize(wxSize(36, 36));

		// Bind click events
		edge_buttons[edgeId]->Bind(wxEVT_LEFT_DOWN, &BorderEditorDialog::OnEdgeClick, this);
		edge_buttons[edgeId]->Bind(wxEVT_RIGHT_DOWN, &BorderEditorDialog::OnEdgeClear, this);

		edge_labels[edgeId] = newd wxStaticText(this, wxID_ANY, GetEdgeName(edgeId),
			wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);

		cell_sizer->Add(edge_buttons[edgeId], 0, wxALIGN_CENTER);
		cell_sizer->Add(edge_labels[edgeId], 0, wxALIGN_CENTER | wxTOP, 2);

		edge_grid->Add(cell_sizer, 0, wxALIGN_CENTER);
	}

	grid_box->Add(edge_grid, 0, wxALL, 5);
	center_sizer->Add(grid_box, 0, wxEXPAND | wxBOTTOM, 5);

	// Selected item display
	selected_item_label = newd wxStaticText(this, wxID_ANY, "Selected Item: None");
	center_sizer->Add(selected_item_label, 0, wxLEFT | wxBOTTOM, 5);

	// OK / Cancel buttons
	wxBoxSizer* dialog_btn_sizer = newd wxBoxSizer(wxHORIZONTAL);
	dialog_btn_sizer->AddStretchSpacer();
	dialog_btn_sizer->Add(newd wxButton(this, wxID_CANCEL, "Cancel"), 0, wxRIGHT, 5);
	dialog_btn_sizer->Add(newd wxButton(this, wxID_OK, "Save Borders"), 0);
	center_sizer->Add(dialog_btn_sizer, 0, wxEXPAND | wxTOP, 5);

	main_sizer->Add(center_sizer, 1, wxEXPAND | wxALL, 5);

	// ========== RIGHT PANEL: Item Picker ==========
	wxStaticBoxSizer* right_sizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Select Item to Assign");

	item_filter_text = newd wxTextCtrl(this, BORDER_EDITOR_ITEM_FILTER, "",
		wxDefaultPosition, wxDefaultSize, 0);
	item_filter_text->SetHint("Filter items by name or ID...");
	right_sizer->Add(item_filter_text, 0, wxEXPAND | wxALL, 5);

	item_list = newd ItemPickerListBox(this, BORDER_EDITOR_ITEM_LIST, wxDefaultPosition, wxSize(250, -1));
	right_sizer->Add(item_list, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

	main_sizer->Add(right_sizer, 0, wxEXPAND | wxALL, 5);

	SetSizer(main_sizer);

	// Load data
	LoadBordersFromMemory();
	RefreshBorderList();
	PopulateItemList("");

	// Select first border if available
	if (!border_list.empty()) {
		border_listbox->SetSelection(0);
		current_selection = 0;
		UpdateEdgeGrid();
		UpdatePropertiesFromSelection();
	}

	Centre();
}

BorderEditorDialog::~BorderEditorDialog() {
}

// ============================================================
// Data Loading
// ============================================================

void BorderEditorDialog::LoadBordersFromMemory() {
	border_list.clear();

	const auto &borders = g_brushes.getBorders();
	for (const auto &pair : borders) {
		BorderData data;
		data.id = pair.first;
		data.group = pair.second->group;
		data.optional = false; // AutoBorder doesn't store this directly
		for (int i = 0; i < 13; i++) {
			data.tiles[i] = pair.second->tiles[i];
		}
		border_list.push_back(data);
	}

	// Sort by ID
	std::sort(border_list.begin(), border_list.end(),
		[](const BorderData &a, const BorderData &b) { return a.id < b.id; });
}

wxString BorderEditorDialog::GetBorderDisplayName(const BorderData &border) const {
	return wxString::Format("Border %u", border.id);
}

void BorderEditorDialog::RefreshBorderList() {
	int prev = border_listbox->GetSelection();
	border_listbox->Clear();

	for (const auto &border : border_list) {
		border_listbox->Append(GetBorderDisplayName(border));
	}

	if (prev >= 0 && prev < (int)border_list.size()) {
		border_listbox->SetSelection(prev);
	}
}

// ============================================================
// Edge Grid
// ============================================================

void BorderEditorDialog::UpdateEdgeGrid() {
	if (current_selection < 0 || current_selection >= (int)border_list.size()) {
		for (int i = 1; i <= 12; i++) {
			if (edge_buttons[i]) {
				edge_buttons[i]->SetSprite(0);
				edge_buttons[i]->Refresh();
			}
		}
		return;
	}

	const BorderData &border = border_list[current_selection];
	for (int i = 1; i <= 12; i++) {
		if (edge_buttons[i]) {
			uint32_t itemId = border.tiles[i];
			if (itemId > 0 && g_items.isValidID(itemId)) {
				const ItemType &type = g_items.getItemType(itemId);
				edge_buttons[i]->SetSprite(type.clientID);
			} else {
				edge_buttons[i]->SetSprite(0);
			}
			edge_buttons[i]->Refresh();
		}
	}
}

void BorderEditorDialog::UpdatePropertiesFromSelection() {
	if (current_selection < 0 || current_selection >= (int)border_list.size()) {
		id_spin->SetValue(0);
		group_spin->SetValue(0);
		optional_chkbox->SetValue(false);
		return;
	}

	const BorderData &border = border_list[current_selection];
	id_spin->SetValue(border.id);
	group_spin->SetValue(border.group);
	optional_chkbox->SetValue(border.optional);
}

void BorderEditorDialog::SaveCurrentBorderData() {
	if (current_selection < 0 || current_selection >= (int)border_list.size()) {
		return;
	}

	BorderData &border = border_list[current_selection];
	border.group = group_spin->GetValue();
	border.optional = optional_chkbox->GetValue();
	// ID changes are handled separately since they need uniqueness check
}

// ============================================================
// Item Picker
// ============================================================

void BorderEditorDialog::PopulateItemList(const wxString &filter) {
	item_list->ClearItems();

	wxString lowerFilter = filter.Lower();

	for (uint16_t id = g_items.getMinID(); id <= g_items.getMaxID(); id++) {
		if (!g_items.isValidID(id)) {
			continue;
		}

		const ItemType &type = g_items.getItemType(id);
		if (type.id == 0 || type.clientID == 0) {
			continue;
		}

		wxString name = wxString(type.name);
		wxString idStr = wxString::Format("%u", id);

		if (!lowerFilter.IsEmpty()) {
			if (name.Lower().Find(lowerFilter) == wxNOT_FOUND && idStr.Find(lowerFilter) == wxNOT_FOUND) {
				continue;
			}
		}

		wxString displayName = name.IsEmpty() ? wxString::Format("Item %u", id) : name;
		item_list->AddItem(id, type.clientID, displayName);
	}

	item_list->DoneAdding();
}

// ============================================================
// Event Handlers
// ============================================================

void BorderEditorDialog::OnSelectBorder(wxCommandEvent &event) {
	// Save current border data before switching
	SaveCurrentBorderData();

	int sel = border_listbox->GetSelection();
	if (sel == wxNOT_FOUND) {
		current_selection = -1;
		UpdateEdgeGrid();
		UpdatePropertiesFromSelection();
		return;
	}

	current_selection = sel;
	UpdateEdgeGrid();
	UpdatePropertiesFromSelection();
}

void BorderEditorDialog::OnAddBorder(wxCommandEvent &event) {
	// Find next available ID
	uint32_t nextId = 1;
	for (const auto &border : border_list) {
		if (border.id >= nextId) {
			nextId = border.id + 1;
		}
	}

	BorderData newBorder;
	newBorder.id = nextId;
	newBorder.group = 0;
	newBorder.optional = false;
	for (int i = 0; i < 13; i++) {
		newBorder.tiles[i] = 0;
	}

	border_list.push_back(newBorder);

	// Sort by ID
	std::sort(border_list.begin(), border_list.end(),
		[](const BorderData &a, const BorderData &b) { return a.id < b.id; });

	RefreshBorderList();

	// Select the new border
	for (int i = 0; i < (int)border_list.size(); i++) {
		if (border_list[i].id == nextId) {
			border_listbox->SetSelection(i);
			current_selection = i;
			break;
		}
	}

	UpdateEdgeGrid();
	UpdatePropertiesFromSelection();
}

void BorderEditorDialog::OnRemoveBorder(wxCommandEvent &event) {
	if (current_selection < 0 || current_selection >= (int)border_list.size()) {
		return;
	}

	int result = wxMessageBox(
		wxString::Format("Remove Border %u?", border_list[current_selection].id),
		"Confirm", wxYES_NO | wxICON_QUESTION, this);

	if (result != wxYES) {
		return;
	}

	border_list.erase(border_list.begin() + current_selection);

	if (current_selection >= (int)border_list.size()) {
		current_selection = (int)border_list.size() - 1;
	}

	RefreshBorderList();

	if (current_selection >= 0) {
		border_listbox->SetSelection(current_selection);
	}

	UpdateEdgeGrid();
	UpdatePropertiesFromSelection();
}

void BorderEditorDialog::OnEdgeClick(wxMouseEvent &event) {
	if (current_selection < 0 || current_selection >= (int)border_list.size()) {
		return;
	}

	if (selected_item_id == 0) {
		return;
	}

	// Find which edge button was clicked
	wxWindow* clicked = dynamic_cast<wxWindow*>(event.GetEventObject());
	if (!clicked) {
		return;
	}

	int clickedId = clicked->GetId();
	int edgeId = clickedId - BORDER_EDITOR_EDGE_BASE;

	if (edgeId >= 1 && edgeId <= 12) {
		border_list[current_selection].tiles[edgeId] = selected_item_id;
		UpdateEdgeGrid();
	}
}

void BorderEditorDialog::OnEdgeClear(wxMouseEvent &event) {
	if (current_selection < 0 || current_selection >= (int)border_list.size()) {
		return;
	}

	wxWindow* clicked = dynamic_cast<wxWindow*>(event.GetEventObject());
	if (!clicked) {
		return;
	}

	int clickedId = clicked->GetId();
	int edgeId = clickedId - BORDER_EDITOR_EDGE_BASE;

	if (edgeId >= 1 && edgeId <= 12) {
		border_list[current_selection].tiles[edgeId] = 0;
		UpdateEdgeGrid();
	}
}

void BorderEditorDialog::OnItemFilterChange(wxCommandEvent &event) {
	PopulateItemList(item_filter_text->GetValue());
}

void BorderEditorDialog::OnSelectItem(wxCommandEvent &event) {
	int sel = item_list->GetSelection();
	if (sel == wxNOT_FOUND || sel < 0 || sel >= (int)item_list->GetCount()) {
		return;
	}

	const auto &entry = item_list->GetEntry(sel);
	selected_item_id = entry.id;
	selected_item_label->SetLabel(wxString::Format("Selected Item: %u - %s", entry.id, entry.name));
}

void BorderEditorDialog::OnPropertyChange(wxCommandEvent &event) {
	SaveCurrentBorderData();
}

void BorderEditorDialog::OnSave(wxCommandEvent &event) {
	SaveCurrentBorderData();

	if (SaveBordersToXML()) {
		g_gui.SetStatusText("Borders saved successfully.");
		EndModal(wxID_OK);
	}
}

void BorderEditorDialog::OnCancel(wxCommandEvent &event) {
	EndModal(wxID_CANCEL);
}

// ============================================================
// XML Save
// ============================================================

bool BorderEditorDialog::SaveBordersToXML() {
	wxString path = g_gui.m_dataDirectory + "/materials/borders/borders.xml";

	pugi::xml_document doc;
	pugi::xml_node decl = doc.prepend_child(pugi::node_declaration);
	decl.append_attribute("version") = "1.0";
	decl.append_attribute("encoding") = "UTF-8";

	pugi::xml_node root = doc.append_child("materials");

	for (const auto &border : border_list) {
		pugi::xml_node border_node = root.append_child("border");
		border_node.append_attribute("id").set_value(border.id);

		if (border.group > 0) {
			border_node.append_attribute("group").set_value(border.group);
		}

		if (border.optional) {
			border_node.append_attribute("type").set_value("optional");
		}

		// Write edges in a consistent order
		int edge_write_order[] = {
			NORTH_HORIZONTAL, EAST_HORIZONTAL, SOUTH_HORIZONTAL, WEST_HORIZONTAL,
			NORTHWEST_CORNER, NORTHEAST_CORNER, SOUTHWEST_CORNER, SOUTHEAST_CORNER,
			NORTHWEST_DIAGONAL, NORTHEAST_DIAGONAL, SOUTHEAST_DIAGONAL, SOUTHWEST_DIAGONAL,
		};

		for (int edgeId : edge_write_order) {
			if (border.tiles[edgeId] > 0) {
				pugi::xml_node item_node = border_node.append_child("borderitem");
				item_node.append_attribute("edge").set_value(GetEdgeXMLName(edgeId));
				item_node.append_attribute("item").set_value(border.tiles[edgeId]);
			}
		}
	}

	if (!doc.save_file(path.ToUTF8().data(), "\t", pugi::format_default, pugi::encoding_utf8)) {
		wxMessageBox("Failed to save borders file:\n" + path, "Error", wxOK | wxICON_ERROR, this);
		return false;
	}

	// Also update in-memory borders
	for (const auto &border : border_list) {
		AutoBorder* existing = g_brushes.getBorder(border.id);
		if (existing) {
			existing->group = border.group;
			for (int i = 0; i < 13; i++) {
				existing->tiles[i] = border.tiles[i];
			}
		} else {
			AutoBorder* newBorder = newd AutoBorder(border.id);
			newBorder->group = border.group;
			for (int i = 0; i < 13; i++) {
				newBorder->tiles[i] = border.tiles[i];
			}
			g_brushes.addBorder(border.id, newBorder);
		}
	}

	return true;
}
