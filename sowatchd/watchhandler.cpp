#include "watchhandler.h"

using namespace sowatch;

WatchHandler::WatchHandler(ConfigKey *config, QObject *parent)
    : QObject(parent),
      _config(config->getSubkey("", this))
{
	Registry *registry = Registry::registry();

	qDebug() << "Starting watch handler on" << _config->key();

	connect(_config, SIGNAL(subkeyChanged(QString)),
	        SLOT(handleConfigSubkeyChanged(QString)));

	// Connect to the registry in case plugins are unloaded
	connect(registry, SIGNAL(driverUnloaded(QString)),
	        SLOT(handleDriverUnloaded(QString)));
	connect(registry, SIGNAL(watchletLoaded(QString)),
	        SLOT(updateWatchlets()));
	connect(registry, SIGNAL(watchletUnloaded(QString)),
	        SLOT(handleWatchletUnloaded(QString)));
	connect(registry, SIGNAL(notificationProviderLoaded(QString)),
	        SLOT(updateProviders()));
	connect(registry, SIGNAL(notificationProviderUnloaded(QString)),
	        SLOT(handleProviderUnloaded(QString)));

	// Get the watch driver
	const QString driver = _config->value("driver").toString();
	if (driver.isEmpty()) {
		qWarning() << "Watch" << _config->value("name") << "has no driver setting";
		return;
	}


	WatchPluginInterface *watchPlugin = registry->getWatchPlugin(driver);
	if (!watchPlugin) {
		qWarning() << "Invalid driver" << driver;
		return;
	}

	// Create the watch object from the plugin
	_watch = watchPlugin->getWatch(driver, _config, this);
	if (!_watch) {
		qWarning() << "Driver" << driver << "failed to initiate watch";
		return;
	}

	// Setup watch status connections
	connect(_watch, SIGNAL(connected()),
	        SIGNAL(statusChanged()));
	connect(_watch, SIGNAL(disconnected()),
	        SIGNAL(statusChanged()));

	// Now create the UI server
	_server = new WatchServer(_watch, this);

	// Configure the server
	_server->setNextWatchletButton(_config->value("next-watchlet-button").toString());

	updateProviders();
	updateWatchlets();
}

QString WatchHandler::status() const
{
	if (_watch && _watch->isConnected()) {
		return "connected";
	} else if (_config->value("enable").toBool()) {
		return "enabled";
	} else {
		return "disabled";
	}
}

void WatchHandler::updateWatchlets()
{
	Registry *registry = Registry::registry();

	if (!_server) return;

	QStringList newWatchlets = _config->value("watchlets").toStringList();
	QStringList curWatchlets = _watchlet_order;

	if (newWatchlets == curWatchlets) return; // Nothing to do

	// TODO: Something better than removing all and readding
	foreach (const QString& s, curWatchlets) {
		Watchlet* watchlet = _watchlets[s];
		_server->removeWatchlet(watchlet);
		delete watchlet;
	}

	_watchlet_order.clear();
	_watchlets.clear();

	foreach (const QString& s, newWatchlets) {
		WatchletPluginInterface *plugin = registry->getWatchletPlugin(s);
		if (!plugin) {
			qWarning() << "Unknown watchlet" << s;
			continue;
		}
		ConfigKey *subconfig = _config->getSubkey(s);
		Watchlet* watchlet = plugin->getWatchlet(s, subconfig, _server);
		_watchlet_order << s;
		_watchlets[s] = watchlet;
		_server->addWatchlet(watchlet);
		delete subconfig;
	}

	qDebug() << "Watchlets reloaded: " << _watchlets.keys();
}

void WatchHandler::updateProviders()
{
	Registry *registry = Registry::registry();

	if (!_server) return;

	QSet<QString> curProviders = _providers.keys().toSet();
	QSet<QString> newProviders = _config->value("providers").toStringList().toSet();
	QSet<QString> removed = curProviders - newProviders;
	QSet<QString> added = newProviders - curProviders;

	qDebug() << "Providers to remove: " << removed;
	qDebug() << "Providers to add: " << added;

	foreach (const QString& s, removed) {
		NotificationProvider *provider = _providers[s];
		_server->removeProvider(provider);
		_providers.remove(s);
		delete provider;
	}

	foreach (const QString& s, added) {
		NotificationPluginInterface *plugin = registry->getNotificationPlugin(s);
		if (!plugin) {
			qWarning() << "Unknown notification provider" << s;
			continue;
		}
		ConfigKey *subconfig = _config->getSubkey(s);
		NotificationProvider *provider = plugin->getProvider(s, subconfig, _server);
		_server->addProvider(provider);
		_providers[s] = provider;
		delete subconfig;
	}

	qDebug() << "Providers reloaded: " << _providers.keys();
}

void WatchHandler::handleConfigSubkeyChanged(const QString &subkey)
{
	if (subkey == "watchlets") {
		qDebug() << "Watchlets list changed";
		updateWatchlets();
	} else if (subkey == "providers") {
		qDebug() << "Providers list changed";
		updateProviders();
	} else if (subkey == "next-watchlet-button" && _server) {
		_server->setNextWatchletButton(_config->value("next-watchlet-button").toString());
	}
}

void WatchHandler::handleDriverUnloaded(const QString &id)
{
	if (id == _config->value("driver").toString()) {
		// Emergency disconnection!
		qWarning("Unloading driver of active watch!");
		if (_server) {
			delete _server;
			_server = 0;
		}
		if (_watch) {
			delete _watch;
			_watch = 0;
		}
		emit statusChanged();
	}
}

void WatchHandler::handleWatchletUnloaded(const QString &id)
{
	if (_watchlets.contains(id)) {
		qDebug() << "Unloading watchlet" << id << "from watch";
		Watchlet* watchlet = _watchlets[id];
		_server->removeWatchlet(watchlet);
		_watchlets.remove(id);
		delete watchlet;
	}
}

void WatchHandler::handleProviderUnloaded(const QString &id)
{
	if (_providers.contains(id)) {
		qDebug() << "Unloading provider" << id << "from watch";
		NotificationProvider *provider = _providers[id];
		_server->removeProvider(provider);
		_providers.remove(id);
		delete provider;
	}
}