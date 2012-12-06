#include "TextFile.h"

void TextFile::close()
{
	QFile::close();
}

TextFile::TextFile( QString fileName, bool write ) : QFile(fileName)
{
	if (write) {
		open(QIODevice::WriteOnly | QIODevice::Text);
	}else {
		open(QIODevice::ReadOnly | QIODevice::Text);
	}
	

	stream.setDevice(this);
}

QString TextFile::readLine()
{
	return stream.readLine();
}

bool TextFile::atEnd()
{
	return stream.atEnd();
}

TextFile::~TextFile()
{
	close();
}

void TextFile::writeLine( QString str)
{
	stream<<str<<endl;
	stream.flush();
}

