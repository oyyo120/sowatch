#ifndef SOWATCH_WATCHLETPLUGININTERFACE_H
#define SOWATCH_WATCHLETPLUGININTERFACE_H

#include <QtPlugin>
#include <QtCore/QSettings>
#include <QtCore/QStringList>
#include <QtCore/QUrl>
#include "sowatch_global.h"

namespace sowatch
{

class ConfigKey;
class Watchlet;
class Watch;

class SOWATCH_EXPORT WatchletPluginInterface
{
public:
	virtual ~WatchletPluginInterface();

	struct WatchletInfo {
		QString name;
		QUrl icon;
		QUrl phoneIcon;
		bool visible;
		QUrl configQmlUrl;

		inline WatchletInfo() : visible(false)
		{

		}
	};

	virtual QStringList watchlets() = 0;
	virtual WatchletInfo describeWatchlet(const QString& id, const QString& watchModel) = 0;
	virtual Watchlet* getWatchlet(const QString& id, ConfigKey *settings, Watch *watch) = 0;
};

}

Q_DECLARE_INTERFACE(sowatch::WatchletPluginInterface, "com.javispedro.sowatch.WatchletPluginInterface")

#endif // SOWATCH_WATCHLETPLUGININTERFACE_H
