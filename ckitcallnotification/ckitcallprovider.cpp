#include <QtCore/QtDebug>
#include <contextsubscriber/contextproperty.h>
#include "ckitcallnotification.h"
#include "ckitcallprovider.h"

using namespace sowatch;

CKitCallProvider::CKitCallProvider(QObject *parent) :
	NotificationProvider(parent),
	_activeCall(new ContextProperty("/com/nokia/CallUi/ActiveCall")),
	_notification(0)
{
	connect(_activeCall, SIGNAL(valueChanged()), SLOT(activeCallChanged()));
}

CKitCallProvider::~CKitCallProvider()
{

}

void CKitCallProvider::activeCallChanged()
{
	QVariantMap info = _activeCall->value().toMap();
	qDebug() << "active call changed" << info;
	if (!info.contains("state")) {
		qWarning() << "broken active call context property";
	}
	int state = info["state"].toInt();
	if (state == 0) {
		QString displayName = info["displayName"].toString();
		if (displayName.isEmpty()) {
			// Ignore call until display name is not empty.
			return;
		}
		// "Incoming call"
		if (_notification) {
			_notification->changeDisplayName(displayName);
		} else {
			_notification = new CKitCallNotification(displayName, this);
			emit incomingNotification(_notification);
		}
	} else {
		// Call is either answered, dropped, missed, ..
		if (_notification) {
			_notification->clear();
			_notification->deleteLater();
			_notification = 0;
		}
	}
}
