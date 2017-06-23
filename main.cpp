#include <QCoreApplication>
#include <QDebug>
#include "vl/generic.h"
#include "vl/info1cd.h"

int main(int argc, char *argv[])
{
        QCoreApplication app(argc, argv);

        QString fileName = "c:/dev/1CBases/1/1Cv8.1CD";
        vl::Info1Cd db(fileName);
        if (!db.readStructureBase()) {
                qDebug() << "ERROR!";
                return -1;
        }

        qDebug() << "ConfigName: " << db.getConfigName();
        qDebug() << "ConfigVersion: " << db.getConfigVersion();
        qDebug() << "TableCount: " << db.getTableCount();
        qDebug() << "FileSize: " << vl::getFileSizeStr(db.getFileSize(), "auto", true);
        qDebug() << "FormatVersion: " << db.getFormatVersionStr();
        qDebug() << "BlockSize: " << db.getBlockSize();
        qDebug() << "Locale: " << db.getLocale();
        qDebug() << "Modified: " << db.getIsModified();
        qDebug() << "UpdateNotCompleted: " << db.getUpdateNotCompleted();

        return app.exec();
}
