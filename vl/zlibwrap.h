#ifndef ZLIBWRAP_H_
#define ZLIBWRAP_H_

#include <QByteArray>
#include <QString>

#include "zlib.h"

//#define CHUNK 16384

namespace vl {

class ZlibWrap
{
public:
	bool compress(const QByteArray &source, QByteArray &destination,
		      int level = Z_DEFAULT_COMPRESSION) const;
	bool unCompress(const QByteArray &source, QByteArray &destination) const;
	bool unCompressRaw(QByteArray source, QByteArray &destination) const;

	ZlibWrap();

private:
	uLong bufferLength = 1048576;
};

} // end namespace vl

#endif // ZLIBWRAP_H_
