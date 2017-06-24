#ifndef CACHE_H_
#define CACHE_H_

#include <QString>
#include <QHash>

namespace vl {

//-----------------------------------//
class Cache
{
public:
	bool put(const QString &key, uint i1, uint i2);
	bool get(const QString &key, uint &i1, uint &i2) const;
	bool remove(const QString &key);

	int  count() const;
	void clear();

	Cache();
private:
	struct data {
		uint i1;
		uint i2;
		data(uint i1 = 0, uint i2 = 0) {
			this->i1 = i1;
			this->i2 = i2;
		}
	};
	QHash<QString, data> hashContainer;
};
//-----------------------------------//

inline int Cache::count() const
{
	return hashContainer.count();
}

inline void Cache::clear()
{
	hashContainer.clear();
}

} // end namespace vl

#endif // CACHE_H_
