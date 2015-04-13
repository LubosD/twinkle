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

#ifndef _ADDRESSLISTVIEWITEM_H
#define _ADDRESSLISTVIEWITEM_H

#include "address_book.h"
#include "qlistview.h"

class AddressListViewItem : public QListViewItem {
private:
	t_address_card	address_card;
	
public:
	AddressListViewItem(QListView *parent, const t_address_card &card);
	t_address_card getAddressCard(void) const;
	void update(const t_address_card &card);
};


#endif
