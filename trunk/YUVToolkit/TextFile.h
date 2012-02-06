#ifndef _MY_TEXT_FILE_
#define _MY_TEXT_FILE_

#include <QtCore>

class TextFile : public QFile
{
	Q_OBJECT;
public:
	TextFile(QString fileName);
	virtual ~TextFile();
public slots:
	void close();
	QString readLine();
	bool atEnd();
private:
	QTextStream stream;
};

#endif