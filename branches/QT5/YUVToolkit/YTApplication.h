#ifndef YTAPPLICATION_H
#define YTAPPLICATION_H

#include <QApplication>

class YTApplication : public QApplication
{
	Q_OBJECT;
public:
	YTApplication(int & argc, char ** argv);
	virtual ~YTApplication();

protected:
	bool event(QEvent* event);
};

#endif // YTAPPLICATION_H
