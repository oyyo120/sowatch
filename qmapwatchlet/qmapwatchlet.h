#ifndef QMAPWATCHLET_H
#define QMAPWATCHLET_H

#include <sowatch.h>

namespace sowatch
{

class QMapWatchlet : public DeclarativeWatchlet
{
    Q_OBJECT
public:
	explicit QMapWatchlet(Watch* watch);

	static const QLatin1String myId;
};

}

#endif // QMAPWATCHLET_H
