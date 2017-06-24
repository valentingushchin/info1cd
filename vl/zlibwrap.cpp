#include "zlibwrap.h"

namespace vl {

bool ZlibWrap::compress(const QByteArray &source, QByteArray &destination, int level) const
{
	destination.clear();
	uLong  sourceSize = static_cast<uLongf>(source.count());
	uLongf destinationSize = sourceSize + sourceSize / 100 + 12;

	destination.resize(static_cast<int>(destinationSize));

	const Bytef *in = reinterpret_cast<const Bytef*>(source.data());
	Bytef *out = reinterpret_cast<Bytef*>(destination.data());

	int result = compress2(out, &destinationSize, in, sourceSize, level);

	if (result != Z_OK) {
		return false;
	}

	destination.resize(static_cast<int>(destinationSize));

	return true;
}

bool ZlibWrap::unCompress(const QByteArray &source, QByteArray &destination) const
{
	destination.clear();
	uLong  sourceSize = static_cast<uLongf>(source.count());
	uLongf destinationSize = bufferLength; // undefined destination buffer size

	destination.resize(static_cast<int>(destinationSize));

	const Bytef *in = reinterpret_cast<const Bytef*>(source.data());
	Bytef *out = reinterpret_cast<Bytef*>(destination.data());

	int result = uncompress(out, &destinationSize, in, sourceSize);

	if (result != Z_OK) {
		return false;
	}

	destination.resize(static_cast<int>(destinationSize));

	return true;
}

bool ZlibWrap::unCompressRaw(QByteArray &source, QByteArray &destination) const
{
	QByteArray buffer;
	const quint8 a = 0x78;
	const quint8 b = 0x9C;
	buffer.append(a);
	buffer.append(b);
	buffer.append(source);

	destination.clear();
	uLong  sourceSize = static_cast<uLongf>(buffer.count());
	uLongf destinationSize = bufferLength;

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

ZlibWrap::ZlibWrap()
{
}

} // end namespace vl
