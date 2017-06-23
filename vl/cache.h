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

inline bool Cache::put(const QString &key, uint i1, uint i2)
{
        if (hashContainer.contains(key)) {
                return false;
        }

        hashContainer.insert(key, data(i1, i2));

        return true;
}

inline bool Cache::get(const QString &key, uint &i1, uint &i2) const
{
        if (!hashContainer.contains(key)) {
                return false;
        }

        i1 = hashContainer.value(key).i1;
        i2 = hashContainer.value(key).i2;

        return true;
}

inline bool Cache::remove(const QString &key)
{
        if (hashContainer.remove(key) == 0) {
                return false;
        }

        return true;
}

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
