#include "TextFile.h"

void TextFile::close()
{
	QFile::close();
}

TextFile::TextFile( QString fileName ) : QFile(fileName)
{
	open(QIODevice::ReadOnly | QIODevice::Text);

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

