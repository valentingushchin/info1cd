#ifndef GENERIC_H_
#define GENERIC_H_

#include <QString>
#include <QTextStream>
#include "zlib.h"

namespace vl {

bool unCompressRaw(QByteArray &source, QByteArray &destination);

// unit: "", "Kb", "Mb", "Gb", "Tb"
QString getFileSizeStr(double len, const QString unit = "", bool showUnit = false,
		       bool separate = true, int precision = 3);

inline quint16 makeUint16(const QByteArray &block, quint32 offset)
{
	quint16 i0 = static_cast<quint16>(block.at(offset + 0)) & 0x00ff;
	quint16 i1 = static_cast<quint16>(block.at(offset + 1));

	return i0 | (i1 << 8);
}

inline quint32 makeUint32(const QByteArray &block, const quint32 offset)
{
	const quint32 mask = 0x000000ff;

	quint32 i0 = static_cast<quint32>(block.at(offset + 0)) & mask;
	quint32 i1 = static_cast<quint32>(block.at(offset + 1)) & mask;
	quint32 i2 = static_cast<quint32>(block.at(offset + 2)) & mask;
	quint32 i3 = static_cast<quint32>(block.at(offset + 3));

	return i0 | (i1 << 8) | (i2 << 16) | (i3 << 24);
}

inline quint64 makeUint64(const QByteArray &block, quint32 offset)
{
	const quint64 mask = 0x00000000000000ff;

	quint64 i0 = static_cast<quint64>(block.at(offset + 0)) & mask;
	quint64 i1 = static_cast<quint64>(block.at(offset + 1)) & mask;
	quint64 i2 = static_cast<quint64>(block.at(offset + 2)) & mask;
	quint64 i3 = static_cast<quint64>(block.at(offset + 3)) & mask;
	quint64 i4 = static_cast<quint64>(block.at(offset + 4)) & mask;
	quint64 i5 = static_cast<quint64>(block.at(offset + 5)) & mask;
	quint64 i6 = static_cast<quint64>(block.at(offset + 6)) & mask;
	quint64 i7 = static_cast<quint64>(block.at(offset + 7));

	return i0 | (i1 << 8) | (i2 << 16) | (i3 << 24) |
			(i4 << 32) | (i5 << 40) | (i6 << 48) | (i7 << 56);
}

} // end namespace vl

#endif // GENERIC_H_


