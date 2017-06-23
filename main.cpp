#include <QTextCodec>
#include <QString>
#include <iostream>
#include "vl/generic.h"
#include "vl/info1cd.h"

int main(int argc, char *argv[])
{
	Q_UNUSED(argc);
	Q_UNUSED(argv);

	QTextCodec *outcodec = QTextCodec::codecForName("CP866");

	QString fileName = "c:/dev/1CBases/2/1Cv8.1CD";
//        QString fileName = "d:/1/1Cv8.1CD";

	vl::Info1Cd db(fileName);
	if (!db.readStructureBase()) {
		std::cerr << "Error!" << std::endl;
		return -1;
	}

	std::string isModified = db.getIsModified() ? "yes" : "no";
	std::string updateIsCompleted = db.getUpdateNotCompleted() ? "yes" : "no";

	std::cout << "ConfigName: " << outcodec->fromUnicode(db.getConfigName()).constData() << std::endl;
	std::cout << "ConfigVersion: " << outcodec->fromUnicode(db.getConfigVersion()).constData() << std::endl;
	std::cout << "Modified: " << isModified << std::endl;
	std::cout << "UpdateNotCompleted: " << updateIsCompleted << std::endl;
	std::cout << "FormatVersion: " << outcodec->fromUnicode(db.getFormatVersionStr()).constData() << std::endl;
	std::cout << "BlockSize: " << db.getBlockSize() << std::endl;
	std::cout << "TableCount: " << db.getTableCount() << std::endl;
	std::cout << "Locale: " << outcodec->fromUnicode(db.getLocale()).constData() << std::endl;
	std::cout << "FileSize: " << outcodec->fromUnicode(vl::getFileSizeStr(db.getFileSize(), "Mb", true)).constData() << std::endl;

	return 0;
}
