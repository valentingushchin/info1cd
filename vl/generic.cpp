#include "generic.h"

namespace vl {

QString uuidGen()
{
	QTime time(QTime::currentTime());
	int timeStamp = time.msecsSinceStartOfDay();
	QString timeStampStr = QString::number(timeStamp, 16);
	int timeStampLen = timeStampStr.size();

	std::mt19937 mt(std::random_device{}());
	std::uniform_int_distribution<int> dist(0, 15);

	QString uuid;
	QTextStream stream(&uuid);
	stream.setIntegerBase(16);
	stream.setNumberFlags(QTextStream::UppercaseDigits);

	for (auto i = 0; i < 36 - timeStampLen; ++i) {
		switch (i) {
			case(8):
			case(13):
			case(18):
			case(23):
				stream << '-';
				break;
			default:
				stream << dist(mt);
		}
	}
	uuid.append(timeStampStr);
	return uuid;
}

QString getFileSizeStr(double len, const QString unit, bool showUnit,
                                bool separate, int precision)
{
        QString fileSizeStr = "%1";
//        double len = getFileSize();
        quint64 div = 1;
        auto offPrecision = false;
        if (unit == "auto") {
                QString unitTmp = "";

                if (len >= 1099511627776) {
                        div = 1099511627776;
                        unitTmp = "Tb";
                } else if (len >= 1073741824) {
                        div = 1073741824;
                        unitTmp = "Gb";
                } else if ((len < 1073741824) && (len >= 1048576)) {
                        div = 1048576;
                        unitTmp = "Mb";
                } else if ((len < 1048576) && (len >= 1024)) {
                        div = 1024;
                        unitTmp = "Kb";
                } else {
                        div = 1;
                        offPrecision = true;
                }

                len = len / div;
                QString tmp = "%1";
                auto delta = tmp.arg(len, 0, 'f', 0).size();

                if (precision < delta)
                        precision = 0;
                else if (precision >= delta)
                        precision -= delta;

                fileSizeStr = fileSizeStr.arg(len, 0, 'f', offPrecision ? 0 : precision);

                if (showUnit)
                        fileSizeStr.append(/*" " + */unitTmp);

                return fileSizeStr;
        }

        QString unitTmp = unit;
        if (unit == "Tb")
                div = 1099511627776;
        else if (unit == "Gb")
                div = 1073741824;
        else if (unit == "Mb")
                div = 1048576;
        else if (unit == "Kb")
                div = 1024;
        else {
                div = 1;
                unitTmp = "";
        }

        len = len / div;
        fileSizeStr = fileSizeStr.arg(len, 0, 'f', 0);
        if (separate) {
                QString tmp = "";
                auto counter = 0;

                for (auto i = fileSizeStr.size(); i > 0; --i) { //TODO
                        tmp.prepend(fileSizeStr.at(i - 1));
                        ++counter;
                        if (counter == 3) {
                                tmp.prepend(" ");
                                counter = 0;
                        }
                }
                fileSizeStr = tmp;
        }
        if (showUnit) {
                fileSizeStr.append(/*" " + */unitTmp);
        }
        return fileSizeStr;
}

} // end namespace vl
