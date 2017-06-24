#include "generic.h"

namespace vl {

bool unCompressRaw(QByteArray &source, QByteArray &destination)
{
	QByteArray buffer;
	const quint8 a = 0x78;
	const quint8 b = 0x9C;
	buffer.append(a);
	buffer.append(b);
	buffer.append(source);

	destination.clear();
	uLong  sourceSize = static_cast<uLongf>(buffer.count());
	uLongf destinationSize = 5000000;

	destination.resize(static_cast<int>(destinationSize));

	Bytef *in = reinterpret_cast<Bytef*>(buffer.data());
	Bytef *out = reinterpret_cast<Bytef*>(destination.data());

	z_stream stream;
	stream.zalloc = Z_NULL;
	stream.zfree =  Z_NULL;
	stream.opaque = Z_NULL;
	stream.avail_in =  static_cast<uInt>(sourceSize);
	stream.next_in  =  static_cast<Bytef*>(in);
	stream.avail_out = static_cast<uInt>(destinationSize);
	stream.next_out =  static_cast<Bytef*>(out);

	int result = inflateInit(&stream);
	if (result != Z_OK) {
		return false;
	}
	result = inflate(&stream, Z_NO_FLUSH);
	if (result != Z_OK) {
		return false;
	}
	deflateEnd(&stream);

	destination.resize(static_cast<int>(stream.total_out));

	return true;
}

QString getFileSizeStr(double len, const QString unit, bool showUnit,
				bool separate, int precision)
{
	QString fileSizeStr = "%1";
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
