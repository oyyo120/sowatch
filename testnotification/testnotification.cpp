#include "testnotification.h"

using namespace sowatch;

TestNotification::TestNotification(Type type, const QString &title, const QString &body, QObject *parent)
    : Notification(parent),
      _type(type),
      _time(QDateTime::currentDateTime()),
      _title(title), _body(body)
{
	const int high = 60 * 1000, low = 30 * 1000;
	int rand = qrand() % ((high + 1) - low) + low;
	QTimer::singleShot(rand, this, SIGNAL(dismissed()));
}

Notification::Type TestNotification::type() const
{
	return _type;
}

uint TestNotification::count() const
{
	return 1;
}

QDateTime TestNotification::dateTime() const
{
	return _time;
}

QString TestNotification::title() const
{
	return _title;
}

QString TestNotification::body() const
{
	return _body;
}

void TestNotification::activate()
{
	// Do nothing
}

void TestNotification::dismiss()
{
	deleteLater(); // We do not want to keep those around.
}
