/*
	Copyright (C) 2015 Lubos Dolezel <lubos@dolezel.info>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _ADDRESSTABLEMODEL_H
#define _ADDRESSTABLEMODEL_H

#include "address_book.h"
#include <QAbstractTableModel>
#include <QList>

// Columns
#define COL_ADDR_NAME		0
#define COL_ADDR_PHONE	1
#define COL_ADDR_REMARK	2

class AddressTableModel : public QAbstractTableModel
{
protected:
	QList<t_address_card>	m_data;
	
public:
	AddressTableModel(QObject *parent, const list<t_address_card>& data);
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

	void appendAddress(const t_address_card& card);
	void removeAddress(int index);
	void modifyAddress(int index, const t_address_card& card);

	t_address_card getAddress(int index) { return m_data[index]; }
};


#endif
