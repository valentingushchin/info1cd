#include "info1cd.h"

namespace vl {

Info1Cd::Info1Cd(const QString &fileName) : fileStateIsOk(false), fileSize(0),
        formatVersionStr(""), formatVersion(BINARYVERSION::VOID), blockCount(0), blockSize(4096),
        configName(""), configVersion("")
{
        this->fileName = fileName;

        locale = "";
        tableCount = 0;
        isModified = false;
        updateNotCompleted = false;

        file.setFileName(this->fileName);

        if(!file.exists()) {
                return;
        }

        setFileSize(file.size());

        if (!file.open(QIODevice::ReadOnly)) {
                return;
        }
        if (!readHeaderBase()) {
                return;
        }
        setFileState(true);
}

Info1Cd::~Info1Cd()
{
        file.close();
}

bool Info1Cd::readHeaderBase()
{
        QByteArray block;
        if (!readBlock(block, 0)) {
                return false;
        }

        if (!((block.at(0) == '1') && (block.at(1) == 'C') &&
              (block.at(2) == 'D') && (block.at(3) == 'B') &&
              (block.at(4) == 'M') && (block.at(5) == 'S') &&
              (block.at(6) == 'V') && (block.at(7) == '8'))) {
                return false;
        }

        const char ver1 = block.at(8);
        const char ver2 = block.at(9);
        const char ver3 = block.at(10);
        const char ver4 = block.at(11);

        setBlockCount(makeUint32(block, 12));

        if ((ver1 == 8) && (ver2 == 3) && (ver3 == 8) && (ver4 == 0)) {
                formatVersion = BINARYVERSION::V8_3_8_0;
                formatVersionStr = "8.3.8.0";
                setBlockSize(makeUint32(block, 20));
        } else if ((ver1 == 8) && (ver2 == 2) && (ver3 == 14) && (ver4 == 0))  {
                formatVersion = BINARYVERSION::V8_2_14_0;
                formatVersionStr = "8.2.14.0";
                setBlockSize(4096);
        } else {
                formatVersion = BINARYVERSION::VOID;
                formatVersionStr = "";
        }
        return true;
}

bool Info1Cd::readBlock(QByteArray &block, quint32 index)
{
        block.clear();

        if (!file.seek(getBlockSize() * index)) {
                return false;
        }

        block = file.read(getBlockSize());

        if (block.size() != getBlockSize()) {
                return false;
        }
        return true;
}

bool Info1Cd::readBlobBlock(QByteArray &blobBlock, const QVector<quint32>& streamIndexes, quint32 blobIndex)
{
        blobBlock.clear();

        quint32 K = getBlockSize() >> 8; // number of 256-byte parts in a block
        qint32  I = blobIndex / K;       // addressable block
        quint32 C = blobIndex % K;       // number of the 256-byte part in the block

        if (I >= streamIndexes.count()) {
                return false;
        }

        quint64 offset = streamIndexes.at(I) * getBlockSize() + (C << 8);

        if (!file.seek(offset)) {
                return false;
        }

        blobBlock = file.read(256);

        if (blobBlock.size() != 256) {
                return false;
        }
        return true;
}

Info1Cd::IDXRET Info1Cd::getStreamDataIndexes_8_2_14(const quint32 streamIndex,
                        QVector<quint32>& dataIndexes, quint32& objectLength)
{
        dataIndexes.clear();

        QByteArray buffer;
        // stream header block
        if (!readBlock(buffer, streamIndex)) {
                return IDXRET::ERROR;
        }

        if (!((buffer.at(0) == '1') && (buffer.at(1) == 'C') &&
              (buffer.at(2) == 'D') && (buffer.at(3) == 'B') &&
              (buffer.at(4) == 'O') && (buffer.at(5) == 'B') &&
              (buffer.at(6) == 'V') && (buffer.at(7) == '8'))) {
                return IDXRET::ERROR;
        }

        // length of data stream
        objectLength = makeUint32(buffer, 8);

        // block indexes of allocation tables
        QVector<quint32> partIndexes;
        quint32 idx = 0;
        for (quint32 i = 24; i < 4096; i += 4) {
                idx = makeUint32(buffer, i);
                if (idx == 0) {
                        break;
                }
                partIndexes.append(idx);
        }
        auto partIndexesCount = partIndexes.count();
        if (partIndexesCount == 0) {
                return IDXRET::NONE;
        }

        // filling indexes from all tables of placements
        for (auto idx = 0; idx < partIndexesCount; ++idx) {
                if (!readBlock(buffer, partIndexes.at(idx))) {
                        return IDXRET::ERROR;
                }
                quint32 countIndexesInCurrentPartBlock = makeUint32(buffer, 0);
                if (countIndexesInCurrentPartBlock > 1023) {
                        return IDXRET::ERROR;
                }
                if (countIndexesInCurrentPartBlock > 0) {
                        for (quint32 iidx = 1; iidx < countIndexesInCurrentPartBlock + 1; ++iidx) {
                                dataIndexes.append(makeUint32(buffer, iidx << 2));
                        }
                }
        }
        if (dataIndexes.isEmpty()) {
                return IDXRET::NONE;
        }
        return IDXRET::OK;
}

Info1Cd::IDXRET Info1Cd::getStreamDataIndexes_8_3_8(const quint32 streamIndex, QVector<quint32>& dataIndexes,
                                                    quint64& objectLength)
{
        dataIndexes.clear();

        QByteArray buffer;
        // stream header block
        if (!readBlock(buffer, streamIndex)) {
                return IDXRET::ERROR;
        }

        // length of data stream
        objectLength = makeUint64(buffer, 16);

        // object type - short/full
        quint32 objectType = makeUint32(buffer, 0);

        // filling indexes without tables of placements
        if (objectType == 0xFD1C) {
                quint32 idx = 0;
                for (quint32 i = 24; i < getBlockSize(); i += 4) {
                        idx = makeUint32(buffer, i);
                        if (idx == 0) {
                                break;
                        }
                        dataIndexes.append(idx);
                }
                if (dataIndexes.isEmpty()) {
                        return IDXRET::NONE;
                }
        // filling indexes with tables of placements
        } else if (objectType == 0x01FD1C) {
                QByteArray secondBuffer;
                quint32 idx = 0;
                for (quint32 i = 24; i < getBlockSize(); i += 4) {
                        idx = makeUint32(buffer, i);
                        if (idx == 0) {
                                break;
                        }
                        if (!readBlock(secondBuffer, idx)) {
                                return IDXRET::ERROR;
                        }
                        quint32 iidx = 0;
                        for (quint32 ii = 0; ii < getBlockSize(); ii += 4) {
                                iidx = makeUint32(secondBuffer, ii);
                                if (iidx != 0) {
                                        dataIndexes.append(iidx);
                                } else {
                                        break;
                                }
                        }
                }
                if (dataIndexes.isEmpty()) {
                        return IDXRET::NONE;
                }
        } else {
                return IDXRET::ERROR;
        }
        return IDXRET::OK;
}

Info1Cd::SIGNRET Info1Cd::detectTableSignature_8_2_14(const QByteArray &block, const quint32 offset) const
{
        // ".C.O.N.F.I.G.".
        // ".C.O.N.F.I.G.S.A.V.E.".
        if (!((block.at(offset +  2) == '"') && (block.at(offset +  3) == 0) &&
              (block.at(offset +  4) == 'C') && (block.at(offset +  5) == 0) &&
              (block.at(offset +  6) == 'O') && (block.at(offset +  7) == 0) &&
              (block.at(offset +  8) == 'N') && (block.at(offset +  9) == 0) &&
              (block.at(offset + 10) == 'F') && (block.at(offset + 11) == 0) &&
              (block.at(offset + 12) == 'I') && (block.at(offset + 13) == 0) &&
              (block.at(offset + 14) == 'G') && (block.at(offset + 15) == 0))) {
                return SIGNRET::VOID;
        }
        if ((block.at(offset + 16) == '"') && (block.at(offset + 17) == 0)) {
                return SIGNRET::CONFIG;
        }
        if ((block.at(offset + 16) == 'S') && (block.at(offset + 17) == 0) &&
            (block.at(offset + 18) == 'A') && (block.at(offset + 19) == 0) &&
            (block.at(offset + 20) == 'V') && (block.at(offset + 21) == 0) &&
            (block.at(offset + 22) == 'E') && (block.at(offset + 23) == 0) &&
            (block.at(offset + 24) == '"') && (block.at(offset + 25) == 0)) {
                return SIGNRET::CONFIGSAVE;
        }
        return SIGNRET::VOID;
}

Info1Cd::SIGNRET Info1Cd::detectTableSignature_8_3_8(const QByteArray &block, const quint32 offset) const
{
        // 0x22,CONFIG,0x22
        // 0x22,CONFIGSAVE,0x22
        if (!((block.at(offset + 1) == 0x22) &&
              (block.at(offset + 2) == 'C') && (block.at(offset +  3) == 'O')  &&
              (block.at(offset + 4) == 'N') && (block.at(offset +  5) == 'F')  &&
              (block.at(offset + 6) == 'I') && (block.at(offset +  7) == 'G'))) {
                return SIGNRET::VOID;
        }
        if (block.at(offset +  8) == 0x22) {
                return SIGNRET::CONFIG;
        }
        if ((block.at(offset +  8) == 'S') && (block.at(offset +  9) == 'A') &&
            (block.at(offset + 10) == 'V') && (block.at(offset + 11) == 'E') &&
            (block.at(offset + 12) == 0x22)) {
                return SIGNRET::CONFIGSAVE;
        }
        return SIGNRET::VOID;
}

bool Info1Cd::getConfigTable_8_2_14(QString &configTable, QString &configsaveTable)
{
        quint32 tableConfigLength = 0;    // objectLength for result
        quint32 tableConfigSaveLength = 0;
        QByteArray configTableBin;        // binary result
        QByteArray configsaveTableBin;

        quint32 objectLength = 0;
        QVector<quint32> indexes;
        IDXRET ret = getStreamDataIndexes_8_2_14(2, indexes, objectLength);
        if (ret != IDXRET::OK) {
                return false;
        }

        // block search with table CONFIG
        quint32 configTableIndex = 0; // block index with table CONFIG
        quint32 configsaveTableIndex = 0; // block index with table CONFIGSAVE
        quint32 firstIndexOffset = 0;
        quint32 lastIndexOffset  = 0;

        QByteArray buffer;
        QByteArray secondBuffer;
        QVector<quint32> indexesBuffer;

        for (qint32 idx = 0; idx < indexes.count(); ++idx) {
                if (!readBlock(buffer, indexes.at(idx))) {
                        return false;
                }
                if (idx == 0) {
                        firstIndexOffset = 36; // locale + tableCount
                        locale.clear();
                        for (quint32 i = 0; i < 32; ++i) {
                                char c = static_cast<char>(buffer.at(i));
                                if (!c) {
                                        break;
                                }
                                locale.append(c);
                        }
                        setTableCount(makeUint32(buffer, 32));
                } else {
                        firstIndexOffset = 0;
                }

                if (idx == indexes.count() - 1) {
                        lastIndexOffset = objectLength % getBlockSize();
                } else {
                        lastIndexOffset = getBlockSize();
                }

                // search through the placement tables
                bool configIsFound = false;
                bool configsaveIsFound = false;
                for (quint32 ii = firstIndexOffset; ii < lastIndexOffset; ii += 4) {
                        quint32 iidx = makeUint32(buffer, ii);

                        quint32 localObjectLength = 0;
                        IDXRET ret = getStreamDataIndexes_8_2_14(iidx, indexesBuffer, localObjectLength);
                        if (ret != IDXRET::OK) {
                                return false;
                        }
                        auto countIndexes = indexesBuffer.count();
                        // one block is enough to describe the table
                        if (countIndexes != 1) {
                                continue;
                        }

                        quint32 scanIdx = indexesBuffer.at(0);
                        if (!readBlock(secondBuffer, scanIdx)) {
                                return false;
                        }

                        SIGNRET sign = detectTableSignature_8_2_14(secondBuffer);
                        if (sign == SIGNRET::CONFIG) {
                                configTableIndex = scanIdx;
                                tableConfigLength = localObjectLength;
                                configIsFound = true;
                        } else if (sign == SIGNRET::CONFIGSAVE) {
                                configsaveTableIndex = scanIdx;
                                tableConfigSaveLength = localObjectLength;
                                configsaveIsFound = true;
                        }

                        if (configIsFound && configsaveIsFound) {
                                break;
                        }
                }
                if ((configTableIndex != 0) && (configsaveTableIndex != 0)) {
                        break;
                }
        }

        if (!configTableIndex) {
                return false; // CONFIG table not found
        }
        if (!configsaveTableIndex) {
                return false; // CONFIGSAVE table not found
        }

        if (!readBlock(configTableBin, configTableIndex)) {
                return false;
        }
        if (!readBlock(configsaveTableBin, configsaveTableIndex)) {
                return false;
        }

        configTableBin.resize(tableConfigLength);
        configTable = QString::fromUtf16(reinterpret_cast<const ushort *>(configTableBin.data()),
                                         configTableBin.count() / 2);

        configsaveTableBin.resize(tableConfigSaveLength);
        configsaveTable = QString::fromUtf16(reinterpret_cast<const ushort *>(configsaveTableBin.data()),
                                             configsaveTableBin.count() / 2);
        return true;
}

bool Info1Cd::getConfigTable_8_3_8(QString &configTable, QString &configsaveTable)
{
        QByteArray configTableBin; // binary result
        QByteArray configsaveTableBin;

        quint64 objectLength = 0;
        QVector<quint32> indexes;
        IDXRET ret = getStreamDataIndexes_8_3_8(2, indexes, objectLength);
        if (ret != IDXRET::OK) {
                return false;
        }

        // block search with table CONFIG
        quint32 firstIndexOffset = 0;
        quint32 lastIndexOffset  = 0;
        QByteArray buffer;
        QByteArray secondBuffer;

        quint32 pageIdx = 1;
        char carry1 = 0;
        char carry2 = 0;
        bool carryDemand = false; // required to enable the transfer to the beginning of the block

        for (quint32 idx = 1; idx < objectLength ; ++idx) {
                if (!readBlobBlock(buffer, indexes, pageIdx)) {
                        return false;
                }

                quint32 dataSize = makeUint16(buffer, 4);
                if (idx % 2) { // if the block is odd
                        if (pageIdx == 1) {
                                firstIndexOffset = 32 + 4 + 6; // locale + tableCount + header
                                locale.clear();
                                for (quint32 i = 6; i < 32 + 6; ++i) {
                                        char c = static_cast<char>(buffer.at(i));
                                        if (!c) {
                                                break;
                                        }
                                        locale.append(c);
                                }
                                setTableCount(makeUint32(buffer, 32 + 6));
                        } else {
                                firstIndexOffset = 6;
                        }

                        if (dataSize == 250) {
                                lastIndexOffset = dataSize;
                                carry1 = buffer.at(254);
                                carry2 = buffer.at(255);
                                carryDemand = true;
                        } else {
                                lastIndexOffset = dataSize + 2;
                                carry1 = 0;
                                carry2 = 0;
                                carryDemand = false;
                        }
                } else { // if the block is even
                        if (carryDemand) {
                                buffer[4] = carry1;
                                buffer[5] = carry2;
                                firstIndexOffset = 4;
                        } else {
                                firstIndexOffset = 6;
                        }
                        lastIndexOffset = dataSize + 2;
                }

                // search by blob
                bool configIsFound = false;
                bool configsaveIsFound = false;
                for (quint32 iidx = firstIndexOffset; iidx < lastIndexOffset; iidx += 4) {
                        quint32 nextBlock = makeUint32(buffer, iidx);
                        if (!readBlobBlock(secondBuffer, indexes, nextBlock)) {
                                return false;
                        }
                        SIGNRET sign = detectTableSignature_8_3_8(secondBuffer, 6);
                        if (sign == SIGNRET::CONFIG) {
                                quint32 nextIdx  = makeUint32(secondBuffer, 0);
                                quint32 dataSize = makeUint16(secondBuffer, 4);
                                configTableBin = secondBuffer.mid(6, dataSize);

                                while (nextIdx) {
                                        if (!readBlobBlock(secondBuffer, indexes, nextIdx)) {
                                                return false;
                                        }
                                        nextIdx  = makeUint32(secondBuffer, 0);
                                        dataSize = makeUint16(secondBuffer, 4);
                                        configTableBin += secondBuffer.mid(6, dataSize);
                                }
                                configTable = QString::fromUtf8(reinterpret_cast<const char *>
                                                                (configTableBin.data()), configTableBin.count());
                                configIsFound = true;
                                if (configsaveIsFound) {
                                        return true;
                                }
                        } else if (sign == SIGNRET::CONFIGSAVE) {
                                quint32 nextIdx  = makeUint32(secondBuffer, 0);
                                quint32 dataSize = makeUint16(secondBuffer, 4);
                                configsaveTableBin = secondBuffer.mid(6, dataSize);

                                while (nextIdx) {
                                        if (!readBlobBlock(secondBuffer, indexes, nextIdx)) {
                                                return false;
                                        }
                                        nextIdx  = makeUint32(secondBuffer, 0);
                                        dataSize = makeUint16(secondBuffer, 4);
                                        configsaveTableBin += secondBuffer.mid(6, dataSize);
                                }
                                configsaveTable = QString::fromUtf8(reinterpret_cast<const char *>
                                                                (configsaveTableBin.data()), configsaveTableBin.count());
                                configsaveIsFound = true;
                                if (configIsFound) {
                                        return true;
                                }
                        } else {
                                continue;
                        }
                }
                pageIdx = makeUint32(buffer, 0);
        }
        return false; // table not found
}

bool Info1Cd::testUpdateNotCompleted_8_2_14(QString &configsaveTable)
{
        quint32 indexObject = 0;  // index of an object block with table entries
        quint32 indexBlob = 0;    // blob index
        quint32 recordLength = 0; // length of one table entry
        quint32 fileNameFieldLen = 0;
        quint32 binaryDataOffset = 0;

        if (!parseTable(configsaveTable, indexObject, indexBlob,
                        recordLength, fileNameFieldLen, binaryDataOffset)) {
                return false;
        }

        quint32 objectLength = 0;
        QVector<quint32> indexesObject;
        IDXRET ret = getStreamDataIndexes_8_2_14(indexObject, indexesObject, objectLength);
        if (ret != IDXRET::NONE) {
                quint32 maxRecordCount = objectLength / recordLength;
                for (quint32 i = 0; i < maxRecordCount; ++i) {
                        QString fileName;
                        uint blobIndex = 0;
                        uint blobLength = 0;

                        if (readRecord(indexesObject, i, recordLength, binaryDataOffset,
                                       fileName, blobIndex, blobLength)) {
                                if (fileName == "") {
                                        continue;
                                }
                                setUpdateNotCompleted(true);
                                break;
                        }
                }
        }
        return true;
}

bool Info1Cd::testUpdateNotCompleted_8_3_8(QString &configsaveTable)
{
        quint32 indexObject = 0;  // index of an object block with table entries
        quint32 indexBlob = 0;    // blob index
        quint32 recordLength = 0; // length of one table entry
        quint32 fileNameFieldLen = 0;
        quint32 binaryDataOffset = 0;
        if (!parseTable(configsaveTable, indexObject, indexBlob,
                        recordLength, fileNameFieldLen, binaryDataOffset)) {
                return false;
        }

        // table indexes
        quint64 objectLength = 0;
        QVector<quint32> indexesObject;
        IDXRET ret = getStreamDataIndexes_8_3_8(indexObject, indexesObject, objectLength);
        if (ret != IDXRET::NONE) {
                quint64 maxRecordCount = objectLength / recordLength;
                for (quint32 i = 0; i < maxRecordCount; ++i) {
                        QString fileName;
                        uint blobIndex = 0;
                        uint blobLength = 0;

                        if (readRecord(indexesObject, i, recordLength, binaryDataOffset,
                                       fileName, blobIndex, blobLength)) {
                                if (fileName == "") {
                                        continue;
                                }
                                setUpdateNotCompleted(true);
                                break;
                        }
                }
        }
        return true;
}

quint32 Info1Cd::calculateFieldSize(const QString &fieldType, quint32 fieldLength) const
{
        quint32 size = 0;

        if (fieldType == "B") {
                size = fieldLength;
        } else if (fieldType == "L")  {
                size = 1;
        } else if (fieldType == "N")  {
                size = (fieldLength + 2) >> 1;
                if ((fieldLength + 2) % 2) {
                        size++;
                }
        } else if (fieldType == "NC") {
                size = fieldLength << 1;
        } else if (fieldType == "NVC") {
                size = (fieldLength << 1) + 2;
        } else if (fieldType == "RV") {
                size = 16;
        } else if (fieldType == "NT") {
                size = 8;
        } else if (fieldType == "I")  {
                size = 8;
        } else if (fieldType == "DT") {
                size = 7;
        }
        return size;
}

bool Info1Cd::parseTable(QString table, quint32 &indexObject,  quint32 &indexBlob, quint32 &recordLength,
                         quint32 &filenameFieldLen, quint32 &binaryDataOffset) const
{
        QRegularExpression regExp;
        QRegularExpressionMatch match;

        bool conversionResult = false;

        regExp.setPattern("[\"\n\\0 ]");
        const QString tableStr = table.remove(regExp);

        // find Recordlock
        uint recordLock = 0;
        regExp.setPattern("\\{Recordlock,(\\d+)\\}");
        match = regExp.match(tableStr);

        QString recordLockStr;
        if (match.hasMatch()) {
                recordLockStr = match.captured(1);
        } else {
                return false;
        }

        recordLock = recordLockStr.toUInt(&conversionResult);
        if (!conversionResult) {
                return false;
        }

        // find object and blob indexes
        regExp.setPattern("\\{Files,(\\d+),(\\d+),[0-9]+\\}");
        match = regExp.match(tableStr);

        QString indexObjectStr;
        QString indexBlobStr;

        if (match.hasMatch()) {
                indexObjectStr = match.captured(1);
                indexBlobStr =   match.captured(2);
        } else {
                return false;
        }

        indexObject = indexObjectStr.toUInt(&conversionResult);
        if (!conversionResult) {
                return false;
        }
        indexBlob = indexBlobStr.toUInt(&conversionResult);
        if (!conversionResult) {
                return false;
        }

        // parse table structure
        regExp.setPattern("\\{(\\w+),(\\w+),(\\d+),(\\d+),(\\d+),(\\w+)\\}");
        QRegularExpressionMatchIterator i = regExp.globalMatch(tableStr);

        bool fieldTypeRvIsExist = false;
        recordLength = 0; // length of one table entry

        QString fieldName;
        QString fieldType;
        QString nullExistsStr;
        QString fieldLengthStr;
        QString fieldPrecisionStr;
        QString fieldCaseSensitive;

        while (i.hasNext()) {
                match = i.next();

                fieldName          = match.captured(1);
                fieldType          = match.captured(2);
                nullExistsStr      = match.captured(3);
                fieldLengthStr     = match.captured(4);
                fieldPrecisionStr  = match.captured(5);
                fieldCaseSensitive = match.captured(6);

                if (fieldType == "RV") {
                        fieldTypeRvIsExist = true;
                }

                uint nullExists = nullExistsStr.toUInt(&conversionResult);
                if (!conversionResult) {
                        return false;
                }

                quint32 fieldLength = fieldLengthStr.toUInt(&conversionResult);
                if (!conversionResult) {
                        return false;
                }

                quint32 fieldSize = calculateFieldSize(fieldType, fieldLength);

                if (fieldName == "FILENAME") {
                        filenameFieldLen = fieldSize;
                }
                if (fieldName == "BINARYDATA") {
                        binaryDataOffset = recordLength;
                }

                recordLength += fieldSize;

                if (nullExists == 1) {
                        recordLength++;
                }
                if (!fieldTypeRvIsExist && recordLock) {
                        recordLength += 8;
                }
        }

        if (!filenameFieldLen) {
                return false; // field FILENAME not found
        }
        if (!binaryDataOffset) {
                return false; // field BINARYDATA not found
        }
        return true;
}

bool Info1Cd::readRecord(const QVector<quint32> &dataIndexes, quint32 recordIndex, quint32 recordLength,
                         quint32 binaryDataOffset, QString &recordStr, quint32 &blobIndex, quint32 &blobLength)
{
        quint32 flatOffsetStart = recordIndex * recordLength; // record start index
        quint32 flatOffsetEnd   = flatOffsetStart + recordLength - 1; // end-of-record index

        quint32 idxStart = flatOffsetStart / getBlockSize(); // index of the start block
        quint32 idxEnd   = flatOffsetEnd / getBlockSize();   // endblock index

        quint32 iStart = flatOffsetStart % getBlockSize();  // relative index within the pick

        QByteArray pick; // pick
        QByteArray tmp;
        for (quint32 idx = idxStart; idx <= idxEnd; ++idx) {
                if(!readBlock(tmp, dataIndexes.at(idx))) {
                        return false;
                }
                pick += tmp;
        }

        if (pick.at(iStart) == 1) { // void block
                return false;
        }

        quint32 finalLength = makeUint16(pick, iStart + 1);

        blobIndex  = makeUint32(pick, iStart + binaryDataOffset);
        blobLength = makeUint32(pick, iStart + binaryDataOffset + 4);

        QByteArray recordData = pick.mid(iStart + 3, recordLength); // finalLength * 2

        recordStr.clear();
        recordStr = QString::fromUtf16(reinterpret_cast<const ushort *>(recordData.data()), finalLength);

        return true;
}

bool Info1Cd::parseInfoBlock(QString info, QString &configName, QString &configVersion)
{
        const QString content = info.remove(QRegularExpression("[\"\r\n\\0 ]")); // clean

        int openBraceCounter = 0;
        int closeBraceCounter = 0;
        int globalCounter = 0;
        for (auto i = 0; i < content.count(); ++i) {
                if (content.at(i) == '{') {
                        ++openBraceCounter;
                } else if (content.at(i) == '}') {
                        ++closeBraceCounter;
                }

                ++globalCounter;

                if ((openBraceCounter == 8) && (closeBraceCounter == 2)) {
                        break;
                }
        }

        int tmpIdx = content.indexOf(',', globalCounter + 1);
        configName = content.mid(globalCounter + 1, tmpIdx - globalCounter - 1);

        globalCounter = tmpIdx + 1;
        openBraceCounter = 0;
        closeBraceCounter = 0;

        for (auto i = globalCounter; i < content.count(); ++i) {
                if (content.at(i) == '{') {
                        ++openBraceCounter;
                } else if (content.at(i) == '}') {
                        ++closeBraceCounter;
                }

                ++globalCounter;

                if ((openBraceCounter == 6) && (closeBraceCounter == 8)) {
                        break;
                }
        }

        int commaCounter = 0;

        for (auto i = globalCounter; i < content.count(); ++i) {
                if (content.at(i) == ',') {
                        ++commaCounter;
                }

                ++globalCounter;

                if (commaCounter == 7) {
                        break;
                }
        }

        tmpIdx = content.indexOf(',', globalCounter);
        configVersion = content.mid(globalCounter, tmpIdx - globalCounter);

        return true;
}

bool Info1Cd::makeBlob(const QVector<quint32> &dataIndexes, quint32 blobStartIndex,
                       quint32 blobLength, QByteArray &blob)
{
        blob.clear();
        QByteArray pick;

        if(!readBlobBlock(pick, dataIndexes, blobStartIndex)) {
                return false;
        }

        for (;;) {
                quint32 nextBlockIndex = makeUint32(pick, 0);
                quint16 currentBlockLength = makeUint16(pick, 4);

                if (currentBlockLength > 250) { // structure error
                        return false;
                }

                if (currentBlockLength == 0) {
                        break;
                } else {
                        blob.append(pick.mid(4 + 2, currentBlockLength));
                }

                if (nextBlockIndex == 0) {
                        break;
                } else {
                        if(!readBlobBlock(pick, dataIndexes, nextBlockIndex)) {
                                return false;
                        }
                }

                if (static_cast<quint32>(blob.count()) > blobLength) { // structure error
                        return false;
                }
        }

        blob.resize(blobLength);
        return true;
}

bool Info1Cd::parseLastStage(const QVector<quint32> &indexesObject, const QVector<quint32> &indexesBlob,
                             quint32 recordLength, quint64 objectLength, quint64 blobLength, quint32 binaryDataOffset)
{
        Q_UNUSED(blobLength)

        QString rootName = "root";

        // search for rootBlobIndex and rootBlobLength for root
        quint64 maxRecordCount = objectLength / recordLength;

        quint32 rootStopCaching = 0;

        for (quint32 i = 0; i < maxRecordCount; ++i) {
                QString fileName;
                uint fBlobIndex = 0;
                uint fBlobLength = 0;

                if (readRecord(indexesObject, i, recordLength, binaryDataOffset, fileName, fBlobIndex, fBlobLength)) {
                        tableBlobInfoCache.put(fileName, fBlobIndex, fBlobLength);

                        if (fileName == rootName) {

                                rootStopCaching = i;
                                break;
                        }
                }
        }

        uint rootBlobIndex = 0;
        uint rootBlobLength = 0;
        if (!tableBlobInfoCache.get(rootName, rootBlobIndex, rootBlobLength)) {
                return false; // root record not found
        }

        QByteArray binary;
        if (!makeBlob(indexesBlob, rootBlobIndex, rootBlobLength, binary)) {
                return false;
        }

        QByteArray out;
        ZlibWrap zw;

        if (!zw.unCompressRaw(binary, out)) {
                return false;
        }

        // read nextFileName
        QString content = QString::fromUtf8(reinterpret_cast<const char *>(out.data()), out.count());
        QRegularExpression regExp;
        regExp.setPattern("[\"\n\\0 ]");
        content = content.remove(regExp);
        regExp.setPattern("\\{\\d+,(\\w+-\\w+-\\w+-\\w+-\\w+),(.*)\\}");
        QRegularExpressionMatch match = regExp.match(content);

        QString nextFileName;
        QString isModifiedMarker;
        if (match.hasMatch()) {
                nextFileName = match.captured(1);
                isModifiedMarker = match.captured(2);
                if (isModifiedMarker == "") {
                        setIsModified(false);
                } else {
                        setIsModified(true);
                }
        } else {
                return false; // nextFileName not found
        }

        uint nextBlobIndex = 0;
        uint nextBlobLength = 0;

        quint32 nextStopCaching = 0;

        if (!tableBlobInfoCache.get(nextFileName, nextBlobIndex, nextBlobLength)) {
                for (quint32 i = rootStopCaching + 1; i < maxRecordCount; ++i) {
                        QString fileName;
                        uint fBlobIndex = 0;
                        uint fBlobLength = 0;

                        if (readRecord(indexesObject, i, recordLength, binaryDataOffset,
                                       fileName, fBlobIndex, fBlobLength)) {
                                tableBlobInfoCache.put(fileName, fBlobIndex, fBlobLength);
                                if (fileName == nextFileName) {
                                        nextStopCaching = i;
                                        break;
                                }
                        }
                }
        }

        if (!tableBlobInfoCache.get(nextFileName, nextBlobIndex, nextBlobLength)) {
                return false; // nextFileName entry not found
        }

        // read nextFile
        if (!makeBlob(indexesBlob, nextBlobIndex, nextBlobLength, binary)) {
                return false;
        }

        if (!zw.unCompressRaw(binary, out)) {
                return false;
        }

        content = QString::fromUtf8(reinterpret_cast<const char *>(out.data()), out.count());

        QString confName;
        QString confVersion;

        if (!parseInfoBlock(content, confName, confVersion)) {
                return false;
        }

        setConfigName(confName);
        setConfigVersion(confVersion);

        return true;
}

bool Info1Cd::readStructureBase_8_2_14()
{
        if (!fileState()) {
                return false;
        }

        QString configTable;
        QString configsaveTable;
        if (!getConfigTable_8_2_14(configTable, configsaveTable)) {
                return false;
        }

        if (!testUpdateNotCompleted_8_2_14(configsaveTable)) {
                return false;
        }

        quint32 indexObject = 0;  // index of an object block with table entries
        quint32 indexBlob = 0;    // blob index
        quint32 recordLength = 0; // length of one table entry
        quint32 fileNameFieldLen = 0;
        quint32 binaryDataOffset = 0;

        if (!parseTable(configTable, indexObject, indexBlob,
                        recordLength, fileNameFieldLen, binaryDataOffset)) {
                return false;
        }

        // table indexes
        quint32 objectLength = 0;
        QVector<quint32> indexesObject;
        IDXRET ret = getStreamDataIndexes_8_2_14(indexObject, indexesObject, objectLength);
        if (ret != IDXRET::OK) {
                return false;
        }
        if (indexesObject.count() == 0) {
                return false;
        }

        // blob indexes
        quint32 blobLength = 0;
        QVector<quint32> indexesBlob;
        ret = getStreamDataIndexes_8_2_14(indexBlob, indexesBlob, blobLength);
        if (ret != IDXRET::OK) {
                return false;
        }

        if (indexesBlob.count() == 0) {
                return false;
        }

        if (!parseLastStage(indexesObject, indexesBlob, recordLength, objectLength, blobLength, binaryDataOffset)) {
                return false;
        }

        return true;
}

bool Info1Cd::readStructureBase_8_3_8()
{
        if (!fileState()) {
                return false;
        }

        QString configTable;
        QString configsaveTable;
        if (!getConfigTable_8_3_8(configTable, configsaveTable)) {
                return false;
        }

        if (!testUpdateNotCompleted_8_3_8(configsaveTable)) {
                return false;
        }

        quint32 indexObject = 0;  // index of an object block with table entries
        quint32 indexBlob = 0;    // blob index
        quint32 recordLength = 0; // length of one table entry
        quint32 fileNameFieldLen = 0;
        quint32 binaryDataOffset = 0;
        if (!parseTable(configTable, indexObject, indexBlob,
                        recordLength, fileNameFieldLen, binaryDataOffset)) {
                return false;
        }

        // table indexes
        quint64 objectLength = 0;
        QVector<quint32> indexesObject;
        IDXRET ret = getStreamDataIndexes_8_3_8(indexObject, indexesObject, objectLength);
        if (ret != IDXRET::OK) {
                return false;
        }
        if (indexesObject.count() == 0) {
                return false;
        }

        // blob indexes
        quint64 blobLength = 0;
        QVector<quint32> indexesBlob;
        ret = getStreamDataIndexes_8_3_8(indexBlob, indexesBlob, blobLength);
        if (ret != IDXRET::OK) {
                return false;
        }

        if (indexesBlob.count() == 0) {
                return false;
        }

        if (!parseLastStage(indexesObject, indexesBlob, recordLength, objectLength, blobLength, binaryDataOffset)) {
                return false;
        }

        return true;
}

bool Info1Cd::readStructureBase()
{
        if (formatVersion == BINARYVERSION::V8_2_14_0) {
                return readStructureBase_8_2_14();
        } else if (formatVersion == BINARYVERSION::V8_3_8_0) {
                return readStructureBase_8_3_8();
        }
        return false;
}

} // end namespace vl
