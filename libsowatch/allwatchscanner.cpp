#include <QtCore/QDebug>

#include "registry.h"
#include "watchplugininterface.h"
#include "allwatchscanner.h"

using namespace sowatch;

AllWatchScanner::AllWatchScanner(QObject *parent) :
	WatchScanner(parent), _finishedCount(0)
{
	QList<WatchPluginInterface*> plugins = Registry::registry()->getWatchPlugins();
	foreach (WatchPluginInterface* driver, plugins) {
		WatchScanner* scanner = driver->getScanner(this);
		if (scanner) {
			_scanners += scanner;
			connect(scanner, SIGNAL(finished()), this, SLOT(handleFinished()));
			connect(scanner, SIGNAL(watchFound(QVariantMap)),
			        this, SIGNAL(watchFound(QVariantMap)));
		}
	}
}

void AllWatchScanner::start()
{
	_finishedCount = 0;
	if (_scanners.empty()) {
		emit finished();
	} else {
		foreach (WatchScanner* scanner, _scanners) {
			scanner->start();
		}
		emit started();
	}
}

void AllWatchScanner::handleFinished()
{
	qDebug() << "one finished";
	_finishedCount++;
	if (_finishedCount >= _scanners.length()) {
		qDebug() << "all finished";
		emit finished();
		_finishedCount = 0;
	}
}
