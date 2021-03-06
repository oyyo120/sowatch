#include "notificationsmodel.h"

using namespace sowatch;

#define FOREACH_TYPE_FROM_TO(x, from, to) \
	for(Notification::Type x = from; x < to; x = static_cast<Notification::Type>(x + 1))

#define FOREACH_TYPE_UNTIL(x, to) FOREACH_TYPE_FROM_TO(x, Notification::OtherNotification, to)

#define FOREACH_TYPE(x) FOREACH_TYPE_FROM_TO(x, Notification::OtherNotification, Notification::TypeCount)

NotificationsModel::NotificationsModel(QObject *parent) :
    QAbstractListModel(parent)
{
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
    setRoleNames(roleNames());
#endif
}

QHash<int, QByteArray> NotificationsModel::roleNames() const
{
    QHash<int, QByteArray> _roles = roleNames();
    _roles[Qt::DisplayRole] = QByteArray("title");
    _roles[ObjectRole] = QByteArray("object");
    _roles[BodyRole] = QByteArray("body");
    _roles[CountRole] = QByteArray("count");

    return _roles;
}

int NotificationsModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return size();
}

QVariant NotificationsModel::data(const QModelIndex &index, int role) const
{
	const Notification *n = at(index.row());
	if (!n) return QVariant();
	switch (role) {
	case Qt::DisplayRole:
		return QVariant::fromValue(n->title());
	case ObjectRole:
		return QVariant::fromValue(const_cast<sowatch::Notification*>(n));
	case BodyRole:
		return QVariant::fromValue(n->body());
	case CountRole:
		return QVariant::fromValue(n->count());
	}
	return QVariant();
}

int NotificationsModel::size() const
{
	int count = 0;
	FOREACH_TYPE(type) {
		count += _list[type].count();
	}
	return count;
}

int NotificationsModel::size(const QList<Notification::Type>& types) const
{
	int count = 0;
	foreach(Notification::Type type, types) {
		count += _list[type].count();
	}
	return count;
}

Notification * NotificationsModel::at(int position) const
{
	FOREACH_TYPE(type) {
		const int size = _list[type].size();
		if (position < size) {
			return _list[type].at(position);
		} else {
			position -= size;
		}
	}
	qWarning() << "Notification with index" << position << "not found";
	return 0;
}

Notification * NotificationsModel::at(Notification::Type type, int position) const
{
	const int size = _list[type].size();
	if (position < size) {
		return _list[type].at(position);
	} else {
		qWarning() << "Notification with index" << position << "not found";
		return 0;
	}
}

Notification * NotificationsModel::at(const QList<Notification::Type>& types, int position) const
{
	foreach(Notification::Type type, types) {
		const int size = _list[type].size();
		if (position < size) {
			return _list[type].at(position);
		} else {
			position -= size;
		}
	}
	qWarning() << "Notification with index" << position << "not found";
	return 0;
}

void NotificationsModel::add(Notification *n)
{
	const Notification::Type type = n->type();
	const int offset = getAppendOffsetForType(type);

	beginInsertRows(QModelIndex(), offset, offset);
	_list[type].append(n);
	endInsertRows();

	emit modelChanged();

	connect(n, SIGNAL(changed()), SLOT(handleNotificationChanged()));
}

void NotificationsModel::remove(Notification *n)
{
	const Notification::Type type = n->type();
	remove(type, n);
}

void NotificationsModel::remove(Notification::Type type, Notification *n)
{
	const int subindex = _list[type].indexOf(n);
	const int index = getOffsetForType(type) + subindex;

	Q_ASSERT(index >= 0);

	disconnect(n, 0, this, 0);

	beginRemoveRows(QModelIndex(), index, index);
	_list[type].removeAt(subindex);
	endRemoveRows();

	emit modelChanged();
}

int NotificationsModel::countByType(Notification::Type type) const
{
	int count = 0;
	Q_FOREACH(const Notification *n, _list[type]) {
		if (n->priority() != Notification::Silent) {
			count++;
		}
	}
	return count;
}

int NotificationsModel::fullCount() const
{
	int count = 0;
	FOREACH_TYPE(type) {
		count += fullCountByType(type);
	}
	return count;
}

int NotificationsModel::fullCountByType(Notification::Type type) const
{
	int count = 0;
	Q_FOREACH(const Notification *n, _list[type]) {
		if (n->priority() != Notification::Silent) {
			count += n->count();
		}
	}
	return count;
}

int NotificationsModel::fullCountByType(int type) const
{
	Q_ASSERT(type >= 0 && type < Notification::TypeCount);
	return fullCountByType(static_cast<Notification::Type>(type));
}

Notification* NotificationsModel::getMostRecentByType(Notification::Type type) const
{
	if (!_list[type].empty()) {
		// TODO Actually get the most recent (sort by date)
		return _list[type].first();
	} else {
		return 0;
	}
}

QObject* NotificationsModel::getMostRecentByType(int type) const
{
	Q_ASSERT(type >= 0 && type < Notification::TypeCount);
	return static_cast<QObject*>(getMostRecentByType(static_cast<Notification::Type>(type)));
}

Notification::Type NotificationsModel::getTypeOfDeletedNotification(Notification *n) const
{
	// Can't call any methods of 'n'
	FOREACH_TYPE(type) {
		if (_list[type].contains(n)) {
			return type;
		}
	}
	return Notification::OtherNotification;
}

int NotificationsModel::getOffsetForType(Notification::Type type) const
{
	int count = 0;
	FOREACH_TYPE_UNTIL(t, type) {
		count += _list[t].count();
	}
	return count;
}

int NotificationsModel::getAppendOffsetForType(Notification::Type type) const
{
	return getOffsetForType(type) + _list[type].count();
}

int NotificationsModel::getIndexForNotification(Notification *n) const
{
	Notification::Type type = n->type();
	const int subindex = _list[type].indexOf(n);

	Q_ASSERT(subindex >= 0);

	return getOffsetForType(type) + subindex;
}

void NotificationsModel::handleNotificationChanged()
{
	QObject *obj = sender();
	if (obj) {
		Notification* n = static_cast<Notification*>(obj);
		const int index = getIndexForNotification(n);

		emit dataChanged(createIndex(index, 0), createIndex(index, 0));
		emit modelChanged();
	}
}
