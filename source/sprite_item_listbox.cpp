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

#include "sprite_item_listbox.h"
#include "gui.h"
#include "graphics.h"

// ============================================================================
// ItemPickerListBox
// ============================================================================

ItemPickerListBox::ItemPickerListBox(wxWindow* parent, wxWindowID id, const wxPoint &pos, const wxSize &size) :
	wxVListBox(parent, id, pos, size, wxLB_SINGLE) {
}

void ItemPickerListBox::AddItem(uint16_t id, uint16_t clientID, const wxString &name) {
	entries.push_back({ id, clientID, name });
}

void ItemPickerListBox::ClearItems() {
	entries.clear();
	SetItemCount(0);
}

void ItemPickerListBox::DoneAdding() {
	SetItemCount(entries.size());
	Refresh();
}

uint16_t ItemPickerListBox::GetSelectedItemID() const {
	int sel = GetSelection();
	if (sel == wxNOT_FOUND || sel < 0 || sel >= (int)entries.size()) {
		return 0;
	}
	return entries[sel].id;
}

void ItemPickerListBox::OnDrawItem(wxDC &dc, const wxRect &rect, size_t n) const {
	if (n >= entries.size()) {
		return;
	}

	const auto sprite = g_gui.gfx.getSprite(entries[n].clientID);
	if (sprite) {
		sprite->DrawTo(&dc, SPRITE_SIZE_32x32, rect.GetX(), rect.GetY(), rect.GetWidth(), rect.GetHeight());
	}

	if (IsSelected(n)) {
		dc.SetTextForeground(HasFocus() ? wxColor(0xFF, 0xFF, 0xFF) : wxColor(0x00, 0x00, 0xFF));
	} else {
		dc.SetTextForeground(wxColor(0x00, 0x00, 0x00));
	}

	dc.DrawText(wxString::Format("%u - %s", entries[n].id, entries[n].name), rect.GetX() + 40, rect.GetY() + 6);
}

wxCoord ItemPickerListBox::OnMeasureItem(size_t n) const {
	return 36;
}

// ============================================================================
// BrushItemListBox
// ============================================================================

BrushItemListBox::BrushItemListBox(wxWindow* parent, wxWindowID id, const wxPoint &pos, const wxSize &size) :
	wxVListBox(parent, id, pos, size, wxLB_SINGLE) {
}

void BrushItemListBox::AddItem(uint16_t id, uint16_t clientID, int chance, const wxString &name) {
	entries.push_back({ id, clientID, chance, name });
}

void BrushItemListBox::ClearItems() {
	entries.clear();
	SetItemCount(0);
}

void BrushItemListBox::DoneAdding() {
	SetItemCount(entries.size());
	Refresh();
}

void BrushItemListBox::OnDrawItem(wxDC &dc, const wxRect &rect, size_t n) const {
	if (n >= entries.size()) {
		return;
	}

	const auto sprite = g_gui.gfx.getSprite(entries[n].clientID);
	if (sprite) {
		sprite->DrawTo(&dc, SPRITE_SIZE_32x32, rect.GetX(), rect.GetY(), rect.GetWidth(), rect.GetHeight());
	}

	if (IsSelected(n)) {
		dc.SetTextForeground(HasFocus() ? wxColor(0xFF, 0xFF, 0xFF) : wxColor(0x00, 0x00, 0xFF));
	} else {
		dc.SetTextForeground(wxColor(0x00, 0x00, 0x00));
	}

	dc.DrawText(wxString::Format("%u", entries[n].id), rect.GetX() + 40, rect.GetY() + 6);
	dc.DrawText(wxString::Format("chance: %d", entries[n].chance), rect.GetX() + 110, rect.GetY() + 6);
	dc.DrawText(entries[n].name, rect.GetX() + 210, rect.GetY() + 6);
}

wxCoord BrushItemListBox::OnMeasureItem(size_t n) const {
	return 36;
}
