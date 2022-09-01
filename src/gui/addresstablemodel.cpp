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

#include "addresstablemodel.h"
#include <QtDebug>
#include <algorithm>

AddressTableModel::AddressTableModel(QObject *parent, const list<t_address_card>& data)
	: QAbstractTableModel(parent)
{
	m_data = QList<t_address_card>::fromStdList(data);
}

int AddressTableModel::rowCount(const QModelIndex &parent) const
{
	return m_data.size();
}

int AddressTableModel::columnCount(const QModelIndex &parent) const
{
	return 3;
}

QVariant AddressTableModel::data(const QModelIndex &index, int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant();

	const int row = index.row();
	switch (index.column())
	{
	case COL_ADDR_NAME:
		return QString::fromStdString(m_data[row].get_display_name());
	case COL_ADDR_PHONE:
		return QString::fromStdString(m_data[row].sip_address);
	case COL_ADDR_REMARK:
		return QString::fromStdString(m_data[row].remark);
	default:
		return QVariant();
	}
}

QVariant AddressTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant();

	switch (section)
	{
	case COL_ADDR_NAME:
		return tr("Name");
	case COL_ADDR_PHONE:
		return tr("Phone");
	case COL_ADDR_REMARK:
		return tr("Remark");
	default:
		return QVariant();
	}
}

void AddressTableModel::appendAddress(const t_address_card& card)
{
	qDebug() << "Appending contact" << QString::fromStdString(card.get_display_name());

	beginInsertRows(QModelIndex(), m_data.size(), m_data.size());
	m_data << card;
	endInsertRows();
}

void AddressTableModel::removeAddress(int index)
{
	beginRemoveRows(QModelIndex(), index, index);

	m_data.removeAt(index);

	endRemoveRows();
}

void AddressTableModel::modifyAddress(int index, const t_address_card& card)
{
	m_data[index] = card;
	emit dataChanged(createIndex(index, 0), createIndex(index, 2));
}

void AddressTableModel::sort(int column, Qt::SortOrder order)
{
	std::sort(m_data.begin(), m_data.end(), [=](const t_address_card& a1, const t_address_card& a2) -> bool {
		bool retval = false;

		switch (column)
		{
		case COL_ADDR_NAME:
            if (order == Qt::DescendingOrder)
                retval = QString::fromStdString(a1.get_display_name()) > QString::fromStdString(a2.get_display_name());
            else
                retval = QString::fromStdString(a1.get_display_name()) < QString::fromStdString(a2.get_display_name());
			break;
		case COL_ADDR_PHONE:
            if (order == Qt::DescendingOrder)
                retval = a1.sip_address.compare(a2.sip_address) > 0;
            else
                retval = a1.sip_address.compare(a2.sip_address) < 0;
			break;
		case COL_ADDR_REMARK:
            if (order == Qt::DescendingOrder)
                retval = QString::fromStdString(a1.remark) > QString::fromStdString(a2.remark);
            else
                retval = QString::fromStdString(a1.remark) < QString::fromStdString(a2.remark);
			break;
		}

		return retval;
	});

	emit dataChanged(createIndex(0, 0), createIndex(m_data.size()-1, 2));
}
