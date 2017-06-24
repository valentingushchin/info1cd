#include "cache.h"

namespace vl {

bool Cache::put(const QString &key, uint i1, uint i2)
{
	if (hashContainer.contains(key)) {
		return false;
	}

	hashContainer.insert(key, data(i1, i2));

	return true;
}

bool Cache::get(const QString &key, uint &i1, uint &i2) const
{
	if (!hashContainer.contains(key)) {
		return false;
	}

	i1 = hashContainer.value(key).i1;
	i2 = hashContainer.value(key).i2;

	return true;
}

bool Cache::remove(const QString &key)
{
	if (hashContainer.remove(key) == 0) {
		return false;
	}

	return true;
}

Cache::Cache()
{
}

} // end namespace vl
