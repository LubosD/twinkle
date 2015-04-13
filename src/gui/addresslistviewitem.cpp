/*
    Copyright (C) 2005-2009  Michel de Boer <michel@twinklephone.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "addresslistviewitem.h"

// Columns
#define COL_ADDR_NAME		0
#define COL_ADDR_PHONE	1
#define COL_ADDR_REMARK	2

AddressListViewItem::AddressListViewItem(QListView *parent, const t_address_card &card) :
		QListViewItem(parent, card.get_display_name().c_str(), 
			      card.sip_address.c_str(), card.remark.c_str()),
		address_card(card)
{}

t_address_card AddressListViewItem::getAddressCard(void) const {
	return address_card;
}

void AddressListViewItem::update(const t_address_card &card) {
	address_card = card;
	setText(COL_ADDR_NAME, card.get_display_name().c_str());
	setText(COL_ADDR_PHONE, card.sip_address.c_str());
	setText(COL_ADDR_REMARK, card.remark.c_str());
}
