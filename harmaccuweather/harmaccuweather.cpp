#include "harmaccuweather.h"

using namespace sowatch;

#define ACCUWEATHER_FILE_PATH "/home/user/.config/AccuWeather, Inc./awxapp.conf"

HarmAccuWeather::HarmAccuWeather(QObject *parent) :
	WeatherNotification(parent),
	_watcher(new QFileSystemWatcher(this)),
	_timer(new QTimer(this)),
	_lastUpdate(QDateTime::fromTime_t(0))
{
	// This only works on Harmattan either way, so I guess
	// hardcoding the path is OK.
	_watcher->addPath(ACCUWEATHER_FILE_PATH);
	connect(_watcher, SIGNAL(fileChanged(QString)), SLOT(fileChanged(QString)));

	_timer->setInterval(5000);
	_timer->setSingleShot(true);
	connect(_timer, SIGNAL(timeout()), SLOT(update()));

	// Perform an initial update
	update();
}

QSettings* HarmAccuWeather::getAccuweatherData()
{
	return new QSettings(ACCUWEATHER_FILE_PATH, QSettings::IniFormat);
}

Notification::Type HarmAccuWeather::type() const
{
	return Notification::WeatherNotification;
}

uint HarmAccuWeather::count() const
{
	return 1;
}

QDateTime HarmAccuWeather::dateTime() const
{
	return _lastUpdate;
}

QString HarmAccuWeather::title() const
{
	return _lastLocation;
}

QString HarmAccuWeather::body() const
{
	switch (_lastWxCode) {
	case 1:
	case 2:
		return tr("Sunny");

	case 3:
	case 4:
	case 5:
		return tr("Partly cloudy");
	case 6:
		return tr("Mostly cloudy");
	case 7:
	case 8:
		return tr("Cloudy");

	case 11:
		return tr("Fog");

	case 12:
		return tr("Light rain");
	case 13:
	case 14:
		return tr("Light rain with sun");
	case 18:
	case 26:
		return tr("Heavy rain");

	case 15:
	case 16:
		return tr("Thunderstorm");
	case 17:
		return tr("Thunderstorm with sun");

	case 19:
		return tr("Light snow");
	case 20:
	case 21:
		return tr("Light snow with sun");
	case 22:
		return tr("Heavy snow");
	case 29:
		return tr("Heavy rain and snow");

	case 25:
		return tr("Blizzard");

	case 30:
		return tr("Hot");
	case 31:
		return tr("Cold");
	case 32:
		return tr("Wind");


	// Night versions
	case 33:
	case 34:
		return tr("Clear");
	case 35:
		return tr("Partly cloudy");
	case 36:
	case 37:
		return tr("Mostly cloudy");
	case 38:
		return tr("Cloudy");
	case 39:
	case 40:
		return tr("Light rain");
	case 41:
	case 42:
		return tr("Thunderstorm");
	case 43:
		return tr("Light snow");
	case 44:
		return tr("Heavy snow");

	default:
		return QString("? %1").arg(_lastWxCode);
	}
}

WeatherNotification::WeatherType HarmAccuWeather::forecast() const
{
	switch (_lastWxCode) {
	case 1:
	case 2:
		return Sunny;

	case 3:
	case 4:
	case 5:
		return PartlyCloudy;

	case 6:
	case 7:
	case 8:
		return Cloudy;

	case 11:
		return Fog;

	case 12:
	case 13:
	case 14:
	case 18:
	case 26:
		return Rain;

	case 15:
	case 16:
	case 17:
		return Thunderstorm;

	case 19:
	case 20:
	case 21:
	case 22:
	case 23:
	case 24:
	case 25:
	case 29:
		return Snow;

	// Night versions
	case 33:
	case 34:
		return Sunny;
	case 35:
		return PartlyCloudy;
	case 36:
	case 37:
	case 38:
		return Cloudy;
	case 39:
	case 40:
		return Rain;
	case 41:
	case 42:
		return Thunderstorm;
	case 43:
	case 44:
		return Snow;

	default:
		return UnknownWeather;
	}
}

int HarmAccuWeather::temperature() const
{
	return _lastTemp;
}

WeatherNotification::Unit HarmAccuWeather::temperatureUnits() const
{
	return _metric ? Celsius : Fahrenheit;
}

void HarmAccuWeather::activate()
{
	// Launch accuweather?
}

void HarmAccuWeather::dismiss()
{
	// Do nothing
}

void HarmAccuWeather::fileChanged(const QString &path)
{
	Q_UNUSED(path);
	qDebug() << "accuweather config file changed";
	_timer->start();
}

void HarmAccuWeather::update()
{
	QSettings* s = getAccuweatherData();

	qDebug() << "reading accuweather config file";

	QDateTime lastUpdate = s->value("LastUpdate").toDateTime();
	if (lastUpdate > _lastUpdate) {
		_lastUpdate = lastUpdate;
		bool anythingChanged = false;

		qDebug() << "reading weather info at" << _lastUpdate;

		bool useMetric = s->value("useMetric").toBool();
		if (useMetric != _metric) {
			_metric = useMetric;
			anythingChanged = true;
		}

		int temp = s->value("LastTemp").toInt();
		if (_lastTemp != temp) {
			_lastTemp = temp;
			anythingChanged = true;
		}

		QString location = s->value("LastLocation").toString();
		if (_lastLocation != location) {
			_lastLocation = location;
			anythingChanged = true;
		}

		int wxCode = s->value("LastWxCode").toInt();
		if (_lastWxCode != wxCode) {
			_lastWxCode = wxCode;
			anythingChanged = true;
		}

		if (anythingChanged) {
			qDebug() << "weather info changed wxcode=" << wxCode;
			emit changed();
		}
	}

	delete s;
}
