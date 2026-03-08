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

#include "ground_brush_editor_window.h"
#include "ground_brush.h"
#include "brush.h"
#include "items.h"
#include "gui.h"
#include "gui_ids.h"

#include <pugixml.hpp>

BEGIN_EVENT_TABLE(GroundBrushEditorDialog, wxDialog)
EVT_LISTBOX(GROUND_BRUSH_EDITOR_LISTBOX, GroundBrushEditorDialog::OnSelectBrush)
EVT_BUTTON(GROUND_BRUSH_EDITOR_ADD, GroundBrushEditorDialog::OnAddBrush)
EVT_BUTTON(GROUND_BRUSH_EDITOR_REMOVE, GroundBrushEditorDialog::OnRemoveBrush)
EVT_BUTTON(GROUND_BRUSH_EDITOR_ITEM_ADD, GroundBrushEditorDialog::OnAddItem)
EVT_BUTTON(GROUND_BRUSH_EDITOR_ITEM_REMOVE, GroundBrushEditorDialog::OnRemoveItem)
EVT_BUTTON(GROUND_BRUSH_EDITOR_BORDER_ADD, GroundBrushEditorDialog::OnAddBorder)
EVT_BUTTON(GROUND_BRUSH_EDITOR_BORDER_REMOVE, GroundBrushEditorDialog::OnRemoveBorder)
EVT_BUTTON(GROUND_BRUSH_EDITOR_FRIEND_ADD, GroundBrushEditorDialog::OnAddFriend)
EVT_BUTTON(GROUND_BRUSH_EDITOR_FRIEND_REMOVE, GroundBrushEditorDialog::OnRemoveFriend)
EVT_TEXT(GROUND_BRUSH_EDITOR_ITEM_FILTER, GroundBrushEditorDialog::OnItemFilterChange)
EVT_LISTBOX(GROUND_BRUSH_EDITOR_ITEM_LIST, GroundBrushEditorDialog::OnSelectPickerItem)
EVT_BUTTON(wxID_OK, GroundBrushEditorDialog::OnSave)
EVT_BUTTON(wxID_CANCEL, GroundBrushEditorDialog::OnCancel)
END_EVENT_TABLE()

GroundBrushEditorDialog::GroundBrushEditorDialog(wxWindow* parent) :
	wxDialog(parent, wxID_ANY, "Ground Brush Editor", wxDefaultPosition, wxSize(1100, 700), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER) {
	wxBoxSizer* main_sizer = newd wxBoxSizer(wxHORIZONTAL);

	// ========== LEFT PANEL: Brush List ==========
	wxStaticBoxSizer* left_sizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Ground Brushes");

	brush_listbox = newd wxListBox(this, GROUND_BRUSH_EDITOR_LISTBOX, wxDefaultPosition, wxSize(200, -1));
	left_sizer->Add(brush_listbox, 1, wxEXPAND | wxALL, 5);

	wxBoxSizer* btn_sizer = newd wxBoxSizer(wxHORIZONTAL);
	add_btn = newd wxButton(this, GROUND_BRUSH_EDITOR_ADD, "Add");
	remove_btn = newd wxButton(this, GROUND_BRUSH_EDITOR_REMOVE, "Remove");
	btn_sizer->Add(add_btn, 1, wxRIGHT, 5);
	btn_sizer->Add(remove_btn, 1);
	left_sizer->Add(btn_sizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

	main_sizer->Add(left_sizer, 0, wxEXPAND | wxALL, 5);

	// ========== CENTER PANEL: Properties + Lists ==========
	wxBoxSizer* center_sizer = newd wxBoxSizer(wxVERTICAL);

	// --- Properties Section ---
	wxStaticBoxSizer* props_sizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Properties");
	wxFlexGridSizer* props_grid = newd wxFlexGridSizer(2, 10, 10);
	props_grid->AddGrowableCol(1);

	props_grid->Add(newd wxStaticText(this, wxID_ANY, "Name:"), 0, wxALIGN_CENTER_VERTICAL);
	name_text = newd wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(200, -1));
	props_grid->Add(name_text, 1, wxEXPAND);

	props_grid->Add(newd wxStaticText(this, wxID_ANY, "Look ID:"), 0, wxALIGN_CENTER_VERTICAL);
	lookid_spin = newd wxSpinCtrl(this, wxID_ANY, "0", wxDefaultPosition, wxSize(100, -1), wxSP_ARROW_KEYS, 0, 65535);
	props_grid->Add(lookid_spin, 0);

	props_grid->Add(newd wxStaticText(this, wxID_ANY, "Z-Order:"), 0, wxALIGN_CENTER_VERTICAL);
	zorder_spin = newd wxSpinCtrl(this, wxID_ANY, "0", wxDefaultPosition, wxSize(100, -1), wxSP_ARROW_KEYS, -9999, 9999);
	props_grid->Add(zorder_spin, 0);

	props_sizer->Add(props_grid, 0, wxEXPAND | wxALL, 5);

	wxBoxSizer* chkbox_sizer = newd wxBoxSizer(wxHORIZONTAL);
	randomize_chkbox = newd wxCheckBox(this, wxID_ANY, "Randomize");
	randomize_chkbox->SetValue(true);
	solo_optional_chkbox = newd wxCheckBox(this, wxID_ANY, "Solo Optional");
	chkbox_sizer->Add(randomize_chkbox, 0, wxRIGHT, 15);
	chkbox_sizer->Add(solo_optional_chkbox, 0);
	props_sizer->Add(chkbox_sizer, 0, wxLEFT | wxBOTTOM, 5);

	center_sizer->Add(props_sizer, 0, wxEXPAND | wxBOTTOM, 5);

	// --- Items Section ---
	wxStaticBoxSizer* items_sizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Items (ID / Chance)");

	items_listctrl = newd BrushItemListBox(this, GROUND_BRUSH_EDITOR_ITEMS_LIST, wxDefaultPosition, wxSize(-1, 120));
	items_sizer->Add(items_listctrl, 1, wxEXPAND | wxALL, 5);

	wxBoxSizer* item_ctrl_sizer = newd wxBoxSizer(wxHORIZONTAL);
	item_ctrl_sizer->Add(newd wxStaticText(this, wxID_ANY, "Chance:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	chance_spin = newd wxSpinCtrl(this, wxID_ANY, "1000", wxDefaultPosition, wxSize(80, -1), wxSP_ARROW_KEYS, 1, 100000);
	item_ctrl_sizer->Add(chance_spin, 0, wxRIGHT, 10);
	item_add_btn = newd wxButton(this, GROUND_BRUSH_EDITOR_ITEM_ADD, "Add Item");
	item_remove_btn = newd wxButton(this, GROUND_BRUSH_EDITOR_ITEM_REMOVE, "Remove Item");
	item_ctrl_sizer->Add(item_add_btn, 0, wxRIGHT, 5);
	item_ctrl_sizer->Add(item_remove_btn, 0);
	items_sizer->Add(item_ctrl_sizer, 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);

	center_sizer->Add(items_sizer, 1, wxEXPAND | wxBOTTOM, 5);

	// --- Borders Section ---
	wxStaticBoxSizer* borders_sizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Borders");

	borders_listctrl = newd wxListCtrl(this, GROUND_BRUSH_EDITOR_BORDERS_LIST, wxDefaultPosition, wxSize(-1, 100), wxLC_REPORT | wxLC_SINGLE_SEL);
	borders_listctrl->InsertColumn(0, "Align", wxLIST_FORMAT_LEFT, 60);
	borders_listctrl->InsertColumn(1, "To", wxLIST_FORMAT_LEFT, 120);
	borders_listctrl->InsertColumn(2, "Border ID", wxLIST_FORMAT_LEFT, 80);
	borders_sizer->Add(borders_listctrl, 1, wxEXPAND | wxALL, 5);

	wxBoxSizer* border_ctrl_sizer = newd wxBoxSizer(wxHORIZONTAL);

	border_ctrl_sizer->Add(newd wxStaticText(this, wxID_ANY, "Align:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	wxArrayString align_choices;
	align_choices.Add("outer");
	align_choices.Add("inner");
	border_align_choice = newd wxChoice(this, wxID_ANY, wxDefaultPosition, wxSize(70, -1), align_choices);
	border_align_choice->SetSelection(0);
	border_ctrl_sizer->Add(border_align_choice, 0, wxRIGHT, 10);

	border_ctrl_sizer->Add(newd wxStaticText(this, wxID_ANY, "To:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	border_to_text = newd wxTextCtrl(this, wxID_ANY, "all", wxDefaultPosition, wxSize(90, -1));
	border_ctrl_sizer->Add(border_to_text, 0, wxRIGHT, 10);

	border_ctrl_sizer->Add(newd wxStaticText(this, wxID_ANY, "ID:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	border_id_spin = newd wxSpinCtrl(this, wxID_ANY, "0", wxDefaultPosition, wxSize(70, -1), wxSP_ARROW_KEYS, 0, 9999);
	border_ctrl_sizer->Add(border_id_spin, 0, wxRIGHT, 10);

	border_add_btn = newd wxButton(this, GROUND_BRUSH_EDITOR_BORDER_ADD, "Add");
	border_remove_btn = newd wxButton(this, GROUND_BRUSH_EDITOR_BORDER_REMOVE, "Remove");
	border_ctrl_sizer->Add(border_add_btn, 0, wxRIGHT, 5);
	border_ctrl_sizer->Add(border_remove_btn, 0);

	borders_sizer->Add(border_ctrl_sizer, 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);

	// Optional border
	wxBoxSizer* opt_border_sizer = newd wxBoxSizer(wxHORIZONTAL);
	opt_border_sizer->Add(newd wxStaticText(this, wxID_ANY, "Optional Border ID:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	optional_border_spin = newd wxSpinCtrl(this, wxID_ANY, "0", wxDefaultPosition, wxSize(80, -1), wxSP_ARROW_KEYS, 0, 9999);
	opt_border_sizer->Add(optional_border_spin, 0);
	opt_border_sizer->Add(newd wxStaticText(this, wxID_ANY, "(0 = none)"), 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);
	borders_sizer->Add(opt_border_sizer, 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);

	center_sizer->Add(borders_sizer, 1, wxEXPAND | wxBOTTOM, 5);

	// --- Friends/Enemies Section ---
	wxStaticBoxSizer* friends_sizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Friends / Enemies");

	wxBoxSizer* radio_sizer = newd wxBoxSizer(wxHORIZONTAL);
	friends_radio = newd wxRadioButton(this, wxID_ANY, "Friends", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	enemies_radio = newd wxRadioButton(this, wxID_ANY, "Enemies");
	friends_radio->SetValue(true);
	radio_sizer->Add(friends_radio, 0, wxRIGHT, 15);
	radio_sizer->Add(enemies_radio, 0);
	friends_sizer->Add(radio_sizer, 0, wxALL, 5);

	friends_listbox = newd wxListBox(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 80));
	friends_sizer->Add(friends_listbox, 1, wxEXPAND | wxLEFT | wxRIGHT, 5);

	wxBoxSizer* friend_ctrl_sizer = newd wxBoxSizer(wxHORIZONTAL);
	friend_ctrl_sizer->Add(newd wxStaticText(this, wxID_ANY, "Brush name:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	friend_name_text = newd wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(120, -1));
	friend_ctrl_sizer->Add(friend_name_text, 1, wxRIGHT, 10);
	friend_add_btn = newd wxButton(this, GROUND_BRUSH_EDITOR_FRIEND_ADD, "Add");
	friend_remove_btn = newd wxButton(this, GROUND_BRUSH_EDITOR_FRIEND_REMOVE, "Remove");
	friend_ctrl_sizer->Add(friend_add_btn, 0, wxRIGHT, 5);
	friend_ctrl_sizer->Add(friend_remove_btn, 0);
	friends_sizer->Add(friend_ctrl_sizer, 0, wxEXPAND | wxALL, 5);

	center_sizer->Add(friends_sizer, 1, wxEXPAND | wxBOTTOM, 5);

	// --- OK/Cancel ---
	wxBoxSizer* dialog_btn_sizer = newd wxBoxSizer(wxHORIZONTAL);
	dialog_btn_sizer->AddStretchSpacer();
	dialog_btn_sizer->Add(newd wxButton(this, wxID_CANCEL, "Cancel"), 0, wxRIGHT, 5);
	dialog_btn_sizer->Add(newd wxButton(this, wxID_OK, "Save Ground Brushes"), 0);
	center_sizer->Add(dialog_btn_sizer, 0, wxEXPAND | wxTOP, 5);

	main_sizer->Add(center_sizer, 1, wxEXPAND | wxALL, 5);

	// ========== RIGHT PANEL: Item Picker ==========
	wxStaticBoxSizer* right_sizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Item Picker");

	item_filter_text = newd wxTextCtrl(this, GROUND_BRUSH_EDITOR_ITEM_FILTER, "", wxDefaultPosition, wxDefaultSize, 0);
	item_filter_text->SetHint("Filter items by name or ID...");
	right_sizer->Add(item_filter_text, 0, wxEXPAND | wxALL, 5);

	item_list = newd ItemPickerListBox(this, GROUND_BRUSH_EDITOR_ITEM_LIST, wxDefaultPosition, wxSize(250, -1));
	right_sizer->Add(item_list, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

	selected_item_label = newd wxStaticText(this, wxID_ANY, "Selected: None");
	right_sizer->Add(selected_item_label, 0, wxLEFT | wxBOTTOM, 5);

	main_sizer->Add(right_sizer, 0, wxEXPAND | wxALL, 5);

	SetSizer(main_sizer);

	// Load data
	LoadBrushesFromMemory();
	RefreshBrushList();
	PopulateItemList("");

	// Select first brush if available
	if (!brush_list.empty()) {
		brush_listbox->SetSelection(0);
		current_selection = 0;
		UpdatePropertiesFromSelection();
	}

	Centre();
}

GroundBrushEditorDialog::~GroundBrushEditorDialog() {
}

// ============================================================
// Data Loading
// ============================================================

std::string GroundBrushEditorDialog::BrushIDToName(uint32_t id) const {
	if (id == 0xFFFFFFFF) {
		return "all";
	}
	if (id == 0) {
		return "none";
	}

	// Find brush by ID
	const auto &brushMap = g_brushes.getMap();
	for (const auto &pair : brushMap) {
		if (pair.second && pair.second->getID() == id) {
			return pair.second->getName();
		}
	}
	return "unknown(" + std::to_string(id) + ")";
}

void GroundBrushEditorDialog::LoadBrushesFromMemory() {
	brush_list.clear();

	const auto &brushMap = g_brushes.getMap();
	// Use a set to avoid duplicates (multimap can have duplicates)
	std::set<Brush*> seen;

	for (const auto &pair : brushMap) {
		Brush* brush = pair.second;
		if (!brush || !brush->isGround()) {
			continue;
		}
		if (seen.count(brush)) {
			continue;
		}
		seen.insert(brush);

		GroundBrush* gb = brush->asGround();
		if (!gb) {
			continue;
		}

		BrushData data;
		data.name = gb->getName();
		data.look_id = (uint16_t)gb->getLookID();
		data.z_order = gb->getZOrder();
		data.randomize = gb->getRandomize();
		data.use_only_optional = gb->getUseOnlyOptional();

		// Load items (chances are cumulative in memory, convert to individual)
		const auto &borderItems = gb->getBorderItems();
		int prev_chance = 0;
		for (const auto &ci : borderItems) {
			ItemEntry entry;
			entry.id = ci.id;
			entry.chance = ci.chance - prev_chance; // Convert cumulative to individual
			prev_chance = ci.chance;
			data.items.push_back(entry);
		}

		// Load border references
		const auto &borderBlocks = gb->getBorderBlocks();
		for (const auto* bb : borderBlocks) {
			BorderEntry entry;
			entry.outer = bb->outer;
			entry.super = bb->super;
			entry.to = bb->to;
			entry.to_name = BrushIDToName(bb->to);
			entry.border_id = bb->autoborder ? bb->autoborder->id : 0;
			data.borders.push_back(entry);
		}

		// Load optional border
		AutoBorder* optBorder = gb->getOptionalBorder();
		data.optional_border_id = optBorder ? optBorder->id : 0;

		// Load friends/enemies
		data.hate_friends = gb->getHateFriends();
		const auto &friendIds = gb->getFriends();
		for (uint32_t fid : friendIds) {
			data.friends.push_back(BrushIDToName(fid));
		}

		brush_list.push_back(data);
	}

	// Sort by name
	std::sort(brush_list.begin(), brush_list.end(), [](const BrushData &a, const BrushData &b) { return a.name < b.name; });
}

wxString GroundBrushEditorDialog::GetBrushDisplayName(const BrushData &brush) const {
	return wxString(brush.name);
}

void GroundBrushEditorDialog::RefreshBrushList() {
	int prev = brush_listbox->GetSelection();
	brush_listbox->Clear();

	for (const auto &brush : brush_list) {
		brush_listbox->Append(GetBrushDisplayName(brush));
	}

	if (prev >= 0 && prev < (int)brush_list.size()) {
		brush_listbox->SetSelection(prev);
	}
}

// ============================================================
// Properties
// ============================================================

void GroundBrushEditorDialog::UpdatePropertiesFromSelection() {
	if (current_selection < 0 || current_selection >= (int)brush_list.size()) {
		name_text->SetValue("");
		lookid_spin->SetValue(0);
		zorder_spin->SetValue(0);
		randomize_chkbox->SetValue(true);
		solo_optional_chkbox->SetValue(false);
		items_listctrl->ClearItems();
		borders_listctrl->DeleteAllItems();
		friends_listbox->Clear();
		optional_border_spin->SetValue(0);
		return;
	}

	const BrushData &brush = brush_list[current_selection];
	name_text->SetValue(wxString(brush.name));
	lookid_spin->SetValue(brush.look_id);
	zorder_spin->SetValue(brush.z_order);
	randomize_chkbox->SetValue(brush.randomize);
	solo_optional_chkbox->SetValue(brush.use_only_optional);
	optional_border_spin->SetValue(brush.optional_border_id);

	if (brush.hate_friends) {
		enemies_radio->SetValue(true);
	} else {
		friends_radio->SetValue(true);
	}

	RefreshItemsList();
	RefreshBordersList();
	RefreshFriendsList();
}

void GroundBrushEditorDialog::SaveCurrentBrushData() {
	if (current_selection < 0 || current_selection >= (int)brush_list.size()) {
		return;
	}

	BrushData &brush = brush_list[current_selection];
	brush.name = name_text->GetValue().ToStdString();
	brush.look_id = lookid_spin->GetValue();
	brush.z_order = zorder_spin->GetValue();
	brush.randomize = randomize_chkbox->GetValue();
	brush.use_only_optional = solo_optional_chkbox->GetValue();
	brush.optional_border_id = optional_border_spin->GetValue();
	brush.hate_friends = enemies_radio->GetValue();
}

void GroundBrushEditorDialog::RefreshItemsList() {
	items_listctrl->ClearItems();
	if (current_selection < 0 || current_selection >= (int)brush_list.size()) {
		return;
	}

	const BrushData &brush = brush_list[current_selection];
	for (const auto &item : brush.items) {
		wxString itemName = wxString::Format("Item %u", item.id);
		uint16_t clientID = 0;
		if (g_items.isValidID(item.id)) {
			const ItemType &type = g_items.getItemType(item.id);
			if (!type.name.empty()) {
				itemName = wxString(type.name);
			}
			clientID = type.clientID;
		}
		items_listctrl->AddItem(item.id, clientID, item.chance, itemName);
	}
	items_listctrl->DoneAdding();
}

void GroundBrushEditorDialog::RefreshBordersList() {
	borders_listctrl->DeleteAllItems();
	if (current_selection < 0 || current_selection >= (int)brush_list.size()) {
		return;
	}

	const BrushData &brush = brush_list[current_selection];
	int idx = 0;
	for (const auto &border : brush.borders) {
		long pos = borders_listctrl->InsertItem(idx, border.outer ? "outer" : "inner");
		borders_listctrl->SetItem(pos, 1, wxString(border.to_name));
		borders_listctrl->SetItem(pos, 2, wxString::Format("%u", border.border_id));
		idx++;
	}
}

void GroundBrushEditorDialog::RefreshFriendsList() {
	friends_listbox->Clear();
	if (current_selection < 0 || current_selection >= (int)brush_list.size()) {
		return;
	}

	const BrushData &brush = brush_list[current_selection];
	for (const auto &name : brush.friends) {
		friends_listbox->Append(wxString(name));
	}
}

// ============================================================
// Item Picker
// ============================================================

void GroundBrushEditorDialog::PopulateItemList(const wxString &filter) {
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

void GroundBrushEditorDialog::OnSelectBrush(wxCommandEvent &event) {
	SaveCurrentBrushData();

	int sel = brush_listbox->GetSelection();
	if (sel == wxNOT_FOUND) {
		current_selection = -1;
		UpdatePropertiesFromSelection();
		return;
	}

	current_selection = sel;
	UpdatePropertiesFromSelection();
}

void GroundBrushEditorDialog::OnAddBrush(wxCommandEvent &event) {
	SaveCurrentBrushData();

	BrushData newBrush;
	newBrush.name = "new_ground_brush";
	newBrush.look_id = 0;
	newBrush.z_order = 0;
	newBrush.randomize = true;
	newBrush.use_only_optional = false;
	newBrush.optional_border_id = 0;
	newBrush.hate_friends = false;

	// Make unique name
	int suffix = 1;
	while (true) {
		bool exists = false;
		for (const auto &b : brush_list) {
			if (b.name == newBrush.name) {
				exists = true;
				break;
			}
		}
		if (!exists) {
			break;
		}
		newBrush.name = "new_ground_brush_" + std::to_string(suffix++);
	}

	brush_list.push_back(newBrush);

	// Sort by name
	std::sort(brush_list.begin(), brush_list.end(), [](const BrushData &a, const BrushData &b) { return a.name < b.name; });

	RefreshBrushList();

	// Select the new brush
	for (int i = 0; i < (int)brush_list.size(); i++) {
		if (brush_list[i].name == newBrush.name) {
			brush_listbox->SetSelection(i);
			current_selection = i;
			break;
		}
	}

	UpdatePropertiesFromSelection();
}

void GroundBrushEditorDialog::OnRemoveBrush(wxCommandEvent &event) {
	if (current_selection < 0 || current_selection >= (int)brush_list.size()) {
		return;
	}

	int result = wxMessageBox(
		wxString::Format("Remove ground brush '%s'?", wxString(brush_list[current_selection].name)),
		"Confirm", wxYES_NO | wxICON_QUESTION, this
	);

	if (result != wxYES) {
		return;
	}

	brush_list.erase(brush_list.begin() + current_selection);

	if (current_selection >= (int)brush_list.size()) {
		current_selection = (int)brush_list.size() - 1;
	}

	RefreshBrushList();

	if (current_selection >= 0) {
		brush_listbox->SetSelection(current_selection);
	}

	UpdatePropertiesFromSelection();
}

void GroundBrushEditorDialog::OnAddItem(wxCommandEvent &event) {
	if (current_selection < 0 || current_selection >= (int)brush_list.size()) {
		return;
	}

	if (selected_item_id == 0) {
		wxMessageBox("Please select an item from the Item Picker first.", "Info", wxOK | wxICON_INFORMATION, this);
		return;
	}

	ItemEntry entry;
	entry.id = selected_item_id;
	entry.chance = chance_spin->GetValue();

	brush_list[current_selection].items.push_back(entry);
	RefreshItemsList();
}

void GroundBrushEditorDialog::OnRemoveItem(wxCommandEvent &event) {
	if (current_selection < 0 || current_selection >= (int)brush_list.size()) {
		return;
	}

	int sel = items_listctrl->GetSelection();
	if (sel == wxNOT_FOUND || sel < 0 || sel >= (int)brush_list[current_selection].items.size()) {
		return;
	}

	brush_list[current_selection].items.erase(brush_list[current_selection].items.begin() + sel);
	RefreshItemsList();
}

void GroundBrushEditorDialog::OnAddBorder(wxCommandEvent &event) {
	if (current_selection < 0 || current_selection >= (int)brush_list.size()) {
		return;
	}

	BorderEntry entry;
	entry.outer = (border_align_choice->GetSelection() == 0); // 0=outer, 1=inner
	entry.super = false;

	wxString toStr = border_to_text->GetValue().Trim();
	if (toStr == "all" || toStr.IsEmpty()) {
		entry.to = 0xFFFFFFFF;
		entry.to_name = "all";
	} else if (toStr == "none") {
		entry.to = 0;
		entry.to_name = "none";
	} else {
		Brush* brush = g_brushes.getBrush(toStr.ToStdString());
		if (brush) {
			entry.to = brush->getID();
			entry.to_name = toStr.ToStdString();
		} else {
			wxMessageBox("Brush '" + toStr + "' not found.", "Warning", wxOK | wxICON_WARNING, this);
			entry.to = 0xFFFFFFFF;
			entry.to_name = toStr.ToStdString();
		}
	}

	entry.border_id = border_id_spin->GetValue();

	brush_list[current_selection].borders.push_back(entry);
	RefreshBordersList();
}

void GroundBrushEditorDialog::OnRemoveBorder(wxCommandEvent &event) {
	if (current_selection < 0 || current_selection >= (int)brush_list.size()) {
		return;
	}

	long sel = borders_listctrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (sel < 0 || sel >= (long)brush_list[current_selection].borders.size()) {
		return;
	}

	brush_list[current_selection].borders.erase(brush_list[current_selection].borders.begin() + sel);
	RefreshBordersList();
}

void GroundBrushEditorDialog::OnAddFriend(wxCommandEvent &event) {
	if (current_selection < 0 || current_selection >= (int)brush_list.size()) {
		return;
	}

	wxString name = friend_name_text->GetValue().Trim();
	if (name.IsEmpty()) {
		return;
	}

	brush_list[current_selection].friends.push_back(name.ToStdString());
	RefreshFriendsList();
	friend_name_text->Clear();
}

void GroundBrushEditorDialog::OnRemoveFriend(wxCommandEvent &event) {
	if (current_selection < 0 || current_selection >= (int)brush_list.size()) {
		return;
	}

	int sel = friends_listbox->GetSelection();
	if (sel == wxNOT_FOUND || sel >= (int)brush_list[current_selection].friends.size()) {
		return;
	}

	brush_list[current_selection].friends.erase(brush_list[current_selection].friends.begin() + sel);
	RefreshFriendsList();
}

void GroundBrushEditorDialog::OnItemFilterChange(wxCommandEvent &event) {
	PopulateItemList(item_filter_text->GetValue());
}

void GroundBrushEditorDialog::OnSelectPickerItem(wxCommandEvent &event) {
	int sel = item_list->GetSelection();
	if (sel == wxNOT_FOUND || sel < 0 || sel >= (int)item_list->GetCount()) {
		return;
	}

	const auto &entry = item_list->GetEntry(sel);
	selected_item_id = entry.id;
	selected_item_label->SetLabel(wxString::Format("Selected: %u - %s", entry.id, entry.name));
}

void GroundBrushEditorDialog::OnSave(wxCommandEvent &event) {
	SaveCurrentBrushData();

	if (SaveBrushesToXML()) {
		g_gui.SetStatusText("Ground brushes saved successfully.");
		EndModal(wxID_OK);
	}
}

void GroundBrushEditorDialog::OnCancel(wxCommandEvent &event) {
	EndModal(wxID_CANCEL);
}

// ============================================================
// XML Save
// ============================================================

bool GroundBrushEditorDialog::SaveBrushesToXML() {
	wxString path = g_gui.m_dataDirectory + "/materials/brushs/grounds.xml";

	pugi::xml_document doc;
	pugi::xml_node decl = doc.prepend_child(pugi::node_declaration);
	decl.append_attribute("version") = "1.0";
	decl.append_attribute("encoding") = "UTF-8";

	pugi::xml_node root = doc.append_child("materials");

	for (const auto &brush : brush_list) {
		pugi::xml_node brush_node = root.append_child("brush");
		brush_node.append_attribute("name").set_value(brush.name.c_str());
		brush_node.append_attribute("type").set_value("ground");

		if (brush.look_id > 0) {
			brush_node.append_attribute("lookid").set_value(brush.look_id);
		}

		if (brush.z_order != 0) {
			brush_node.append_attribute("z-order").set_value(brush.z_order);
		}

		if (brush.use_only_optional) {
			brush_node.append_attribute("solo_optional").set_value(true);
		}

		if (!brush.randomize) {
			brush_node.append_attribute("randomize").set_value(false);
		}

		// Items
		for (const auto &item : brush.items) {
			pugi::xml_node item_node = brush_node.append_child("item");
			item_node.append_attribute("id").set_value(item.id);
			item_node.append_attribute("chance").set_value(item.chance);
		}

		// Borders
		for (const auto &border : brush.borders) {
			pugi::xml_node border_node = brush_node.append_child("border");
			border_node.append_attribute("align").set_value(border.outer ? "outer" : "inner");

			if (border.to_name != "all") {
				border_node.append_attribute("to").set_value(border.to_name.c_str());
			}

			border_node.append_attribute("id").set_value(border.border_id);

			if (border.super) {
				border_node.append_attribute("super").set_value(true);
			}
		}

		// Optional border
		if (brush.optional_border_id > 0) {
			pugi::xml_node opt_node = brush_node.append_child("optional");
			opt_node.append_attribute("id").set_value(brush.optional_border_id);
		}

		// Friends/Enemies
		for (const auto &friendName : brush.friends) {
			const char* tag = brush.hate_friends ? "enemy" : "friend";
			pugi::xml_node friend_node = brush_node.append_child(tag);
			friend_node.append_attribute("name").set_value(friendName.c_str());
		}
	}

	if (!doc.save_file(path.ToUTF8().data(), "\t", pugi::format_default, pugi::encoding_utf8)) {
		wxMessageBox("Failed to save ground brushes file:\n" + path, "Error", wxOK | wxICON_ERROR, this);
		return false;
	}

	return true;
}
