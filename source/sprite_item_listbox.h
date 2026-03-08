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

#ifndef RME_SPRITE_ITEM_LISTBOX_H_
#define RME_SPRITE_ITEM_LISTBOX_H_

#include "main.h"

// A virtual list box that renders item sprites alongside text.
// Used by the Border Editor and Ground Brush Editor item pickers.
class ItemPickerListBox : public wxVListBox {
public:
	struct Entry {
		uint16_t id;
		uint16_t clientID;
		wxString name;
	};

	ItemPickerListBox(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize);

	void AddItem(uint16_t id, uint16_t clientID, const wxString& name);
	void ClearItems();
	void DoneAdding();
	size_t GetCount() const { return entries.size(); }
	const Entry& GetEntry(size_t index) const { return entries[index]; }
	uint16_t GetSelectedItemID() const;

protected:
	void OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const override;
	wxCoord OnMeasureItem(size_t n) const override;

private:
	std::vector<Entry> entries;
};

// A virtual list box that renders brush items with sprite, ID, chance, and name.
// Used by the Ground Brush Editor items list.
class BrushItemListBox : public wxVListBox {
public:
	struct Entry {
		uint16_t id;
		uint16_t clientID;
		int chance;
		wxString name;
	};

	BrushItemListBox(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize);

	void AddItem(uint16_t id, uint16_t clientID, int chance, const wxString& name);
	void ClearItems();
	void DoneAdding();
	size_t GetCount() const { return entries.size(); }
	const Entry& GetEntry(size_t index) const { return entries[index]; }

protected:
	void OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const override;
	wxCoord OnMeasureItem(size_t n) const override;

private:
	std::vector<Entry> entries;
};

#endif
