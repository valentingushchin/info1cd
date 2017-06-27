#ifndef INFO1CD_H_
#define INFO1CD_H_

#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QVector>
#include <QString>
#include <QRegularExpression>

#include "common.h"
#include "cache.h"

namespace vl {

enum class BINARYVERSION {VOID = 0, V8_2_14_0 = 1, V8_3_8_0 = 2};
enum class OBJECTBLOCKFORMAT {VOID = 0, V8_2_14_0 = 1, V8_3_8_0_SHORT = 2, V8_3_8_0_FULL = 3};

//-----------------------------------//
class Info1Cd
{
public:
	Info1Cd(const QString &fileName);
	~Info1Cd();

	bool fileState() const;

	QString getFileName() const;
	qint64  getFileSize() const;

	bool readHeaderBase();
	QString getFormatVersionStr() const;
	uint    getBlockCount() const;
	quint32 getBlockSize() const;

	bool readStructureBase();
	QString getConfigName() const;
	QString getConfigVersion() const;
	QString getLocale() const;
	quint32 getTableCount() const;
	bool getIsModified() const;
	bool getUpdateNotCompleted() const;

private:
	QFile file;

	bool fileStateIsOk;
	void setFileState(bool state);

	QString fileName;
	qint64 fileSize;
	QString formatVersionStr;
	BINARYVERSION formatVersion;

	quint32 blockCount;
	quint32 blockSize;

	void setFileName(const QString &fileName);
	void setFileSize(qint64 fileSize);
	void setBlockCount(quint32 blockCount);
	void setBlockSize(quint32 blockSize);

	QString configName;
	QString configVersion;
	QString locale;
	quint32 tableCount;
	bool isModified;
	bool updateNotCompleted;

	void setConfigName(const QString &name);
	void setConfigVersion(const QString &version);
	void setLocale(const QString &locale);
	void setTableCount(quint32 tableCount);
	void setIsModified(bool isModified);
	void setUpdateNotCompleted(bool updateNotCompleted);

	bool readStructureBase_8_2_14();
	bool readStructureBase_8_3_8();

	bool readBlock(QByteArray &block, quint32 index);
	bool readBlobBlock(QByteArray &blobBlock, const QVector<quint32> &streamIndexes, quint32 blobIndex);

	enum class IDXRET {ERROR = -1, NONE = 0, OK = 1};
	IDXRET getStreamDataIndexes_8_2_14(const quint32 streamIndex, QVector<quint32> &dataIndexes, quint32 &objectLength);
	IDXRET getStreamDataIndexes_8_3_8 (const quint32 streamIndex, QVector<quint32> &dataIndexes, quint64 &objectLength);

	enum class SIGNRET {VOID = 0, CONFIG = 1, CONFIGSAVE = 2};
	SIGNRET detectTableSignature_8_2_14(const QByteArray &block, const quint32 offset = 0) const;
	SIGNRET detectTableSignature_8_3_8 (const QByteArray &block, const quint32 offset = 0) const;

	bool getConfigTable_8_2_14(QString &configTable, QString &configsaveTable);
	bool getConfigTable_8_3_8 (QString &configTable, QString &configsaveTable);

	bool testUpdateNotCompleted_8_2_14(QString &configsaveTable);
	bool testUpdateNotCompleted_8_3_8(QString &configsaveTable);

	quint32 calculateFieldSize(const QString &fieldType, quint32 fieldLength) const;
	bool parseTable(QString table, quint32 &indexObject, quint32 &indexBlob, quint32 &recordLength,
			quint32 &filenameFieldLen, quint32 &binaryDataOffset) const;

	Cache tableBlobInfoCache;
	bool readRecord(const QVector<quint32> &dataIndexes, quint32 recordIndex, quint32 recordLength,
			quint32 binaryDataOffset, QString &recordStr, quint32 &blobIndex, quint32 &blobLength);

	bool parseInfoBlock(QString info, QString &configName, QString &configVersion);

	bool makeBlob(const QVector<quint32> &dataIndexes, quint32 blobStartIndex, quint32 blobLength, QByteArray &blob);
	bool parseLastStage(const QVector<quint32> &indexesObject, const QVector<quint32> &indexesBlob, quint32 recordLength,
			    quint64 objectLength, quint64 blobLength, quint32 binaryDataOffset);
};
//-----------------------------------//

inline void Info1Cd::setFileName(const QString &fileName)
{
	this->fileName = fileName;
}

inline bool Info1Cd::fileState() const
{
	return fileStateIsOk;
}

inline QString Info1Cd::getFileName() const
{
	return fileName;
}

inline void Info1Cd::setFileSize(qint64 fileSize)
{
	this->fileSize = fileSize;
}

inline void Info1Cd::setBlockCount(quint32 blockCount)
{
	this->blockCount = blockCount;
}

inline void Info1Cd::setBlockSize(quint32 blockSize)
{
	this->blockSize = blockSize;
}

inline void Info1Cd::setConfigName(const QString &name)
{
	configName = name;
}

inline void Info1Cd::setConfigVersion(const QString &version)
{
	configVersion = version;
}

inline void Info1Cd::setLocale(const QString &locale)
{
	this->locale = locale;
}

inline void Info1Cd::setTableCount(quint32 tableCount)
{
	this->tableCount = tableCount;
}

inline void Info1Cd::setIsModified(bool isModified)
{
	this->isModified = isModified;
}

inline void Info1Cd::setUpdateNotCompleted(bool updateNotCompleted)
{
	this->updateNotCompleted = updateNotCompleted;
}

inline qint64 Info1Cd::getFileSize() const
{
	return fileSize;
}

inline QString Info1Cd::getLocale() const
{
	return locale;
}

inline quint32 Info1Cd::getTableCount() const
{
	return tableCount;
}

inline bool Info1Cd::getIsModified() const
{
	return isModified;
}

inline bool Info1Cd::getUpdateNotCompleted() const
{
	return updateNotCompleted;
}

inline void Info1Cd::setFileState(bool state)
{
	fileStateIsOk = state;
}

inline QString Info1Cd::getFormatVersionStr() const
{
	return formatVersionStr;
}

inline uint Info1Cd::getBlockCount() const
{
	return blockCount;
}

inline quint32 Info1Cd::getBlockSize() const
{
	return blockSize;
}

inline QString Info1Cd::getConfigName() const
{
	return configName;
}

inline QString Info1Cd::getConfigVersion() const
{
	return configVersion;
}

} // end namespace vl

#endif // INFO1CD_H_
