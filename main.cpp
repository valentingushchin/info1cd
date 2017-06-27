#include <QCoreApplication>
#include <QTextStream>
#include <QCommandLineParser>
#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include "vl/generic.h"
#include "vl/info1cd.h"

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);
	QCoreApplication::setApplicationName("info1cd");
	QCoreApplication::setApplicationVersion("1.0");

	QCommandLineParser parser;
        parser.setApplicationDescription("\ninfo1cd v1.0 - Trying to find out information about the 1CD file\n\n"
                                         "Valentin Gushchin : 27.06.2017\n"
					 "https://github.com/valentingushchin/info1cd");
	parser.addHelpOption();
	parser.addVersionOption();

	QCommandLineOption jsonOption(QStringList() << "j" << "json",
		    QCoreApplication::translate("main", "Set the output format to json."));

	parser.addOption(jsonOption);

        QCommandLineOption encodingOption(QStringList() << "e" << "encode",
                    QCoreApplication::translate("main", "Setting the encoding of the output text. \n"
                                                        "<encoding>: UTF-8, CP-1251, CP-866 (default)."),
                    QCoreApplication::translate("main", "encoding"));

        parser.addOption(encodingOption);

	parser.addPositionalArgument("source", QCoreApplication::translate("main", "Full path to *.1CD file."));

	parser.process(app);

	const QStringList args = parser.positionalArguments();

	bool jsonFormat = parser.isSet(jsonOption);

	if (args.isEmpty()) {
		parser.showHelp(-100);
	}

	QString fileName = args.at(0);

	QTextStream cerr(stderr);

	vl::Info1Cd db(fileName);
	if (!db.readStructureBase()) {
		cerr << "Error parsing" << endl;
		return -1;
	}

        QString enc = parser.value(encodingOption);

        QTextStream cout(stdout);

        if (enc == "UTF-8") {
                cout.setCodec("UTF8");
        } else if (enc == "CP-1251") {
                cout.setCodec("CP1251");
        } else if ((enc == "CP-866") || enc.isEmpty()) {
                cout.setCodec("CP866");
        } else {
                parser.showHelp(-200);
        }

	if (jsonFormat) {
		QJsonObject jso;
		jso["ConfigName"] = db.getConfigName();
		jso["ConfigVersion"] = db.getConfigVersion();
		jso["Modified"] = db.getIsModified();
		jso["UpdateNotCompleted"] = db.getUpdateNotCompleted();
		jso["FormatVersion"] = db.getFormatVersionStr();
		jso["BlockSize"] = static_cast<int>(db.getBlockSize());
		jso["TableCount"] = static_cast<int>(db.getTableCount());
		jso["Locale"] = db.getLocale();
		jso["FileSize"] = db.getFileSize();

		QJsonDocument jsd(jso);
		cout << QString::fromUtf8(jsd.toJson()) << endl;

	} else {
		QString isModified = db.getIsModified() ? "yes" : "no";
		QString updateIsCompleted = db.getUpdateNotCompleted() ? "yes" : "no";

		cout << "ConfigName: " << db.getConfigName() << endl;
		cout << "ConfigVersion: " << db.getConfigVersion() << endl;
		cout << "Modified: " << isModified << endl;
		cout << "UpdateNotCompleted: " << updateIsCompleted << endl;
		cout << "FormatVersion: " << db.getFormatVersionStr() << endl;
		cout << "BlockSize: " << db.getBlockSize() << endl;
		cout << "TableCount: " << db.getTableCount() << endl;
		cout << "Locale: " << db.getLocale() << endl;
		cout << "FileSize: " << vl::getFileSizeStr(db.getFileSize(), "Mb", true) << endl;
	}

	return 0;
}
