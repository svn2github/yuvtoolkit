#include "YT_InterfaceImpl.h"
#include "ScoreWindow.h"

#ifndef MAX
#define MAX(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b)            (((a) < (b)) ? (a) : (b))
#endif

ScoreWindow::ScoreWindow( QWidget *parent, Qt::WindowFlags flags ) : QWidget(parent, flags)
{
	ui.setupUi(this);

	connect(ui.buttonNext, SIGNAL(clicked()), this, SIGNAL(onNext()));
	connect(ui.buttonPrevious, SIGNAL(clicked()), this, SIGNAL(onPrevious()));
	connect(ui.buttonFinish, SIGNAL(clicked()), this, SIGNAL(onFinish()));
	this->setMouseTracking(true);
	restrictMouse = false;
}

ScoreWindow::~ScoreWindow()
{

}

void ScoreWindow::enableButtons( bool p, bool n, bool f)
{
	ui.buttonPrevious->setEnabled(p);
	ui.buttonNext->setEnabled(n);
	ui.buttonFinish->setEnabled(f);
}


void ScoreWindow::changeButtonName(QString a, QString b, QString c)
{
	ui.buttonPrevious ->setText(a);
	ui.buttonNext->setText(b);
	ui.buttonFinish->setText(c);
}

void ScoreWindow::createButton(int num,QString name)
{	
	ButtonNameList = name.split(',');
	QGridLayout *box = new QGridLayout;
	ButtonList = (QRadioButton **) malloc (num * sizeof(QRadioButton));
	NumOfConditions = num;
	QLabel * label_static = new QLabel("What you choose is");
	label_cur_selected = new QLabel("Not_rated");
	box->addWidget(label_static,0,0);
	box->addWidget(label_cur_selected,0,1);
	for (int i=0;i<num;i++)
	{
		QRadioButton *button = new QRadioButton(ButtonNameList[i]);
		box->addWidget(button,1,i);
		connect(button,SIGNAL(clicked()),this,SLOT(updateCurSelect()));
		ButtonList[i] = button;
	};
	ui.widget_score->setLayout(box);
	ui.widget_score->show();
}

void ScoreWindow::createSlider(int num,int min, int max, QString name, QString scale)
{
	SliderNameList = name.split(',');
	ScaleNameList = scale.split(',');

	QGridLayout *box = new QGridLayout;
	SliderList = (QSlider **) malloc (num * sizeof (QSlider));
	label_cur_selected_list = (QLabel **) malloc (num * sizeof(QLabel));
	NumOfConditions = num;
	box->setAlignment(Qt::AlignHCenter);
	for (int i=0;i<num;i++)
	{
		QWidget *scale_widget = new QWidget;
		QVBoxLayout *scale_box = new QVBoxLayout;
		for (int j=0;j<ScaleNameList.size();j++)
		{
			QLabel* label_cur = new QLabel(ScaleNameList[j]);
			scale_box->addWidget(label_cur);
		}
		scale_widget->setLayout(scale_box);
		QLabel* label_cur = new QLabel("Not_rated");
		QSlider* slider= new QSlider();
		slider->setObjectName(SliderNameList[i]);
		slider->setRange(min, max);
		slider->setTickPosition(QSlider::TicksRight);
		slider->setTickInterval((max-min)/ScaleNameList.size());
		box->addWidget(label_cur,0,i*2);
		box->addWidget(slider,1,i*2);
		box->addWidget(scale_widget,1,i*2+1);
		connect(slider,SIGNAL(valueChanged(int)),label_cur,SLOT(setNum(int)));
		connect(slider,SIGNAL(valueChanged(int)),this,SLOT(updateCurSlider()));
		SliderList[i] = slider;
		label_cur_selected_list[i] = label_cur;
	};
	ui.widget_score->setLayout(box);
	ui.widget_score->show();
}

void ScoreWindow::createSliderandButton(int num,int min, int max, QString name, QString scale)
{
	SliderNameList = name.split(',');
	ScaleNameList = scale.split(',');

	QGridLayout *box = new QGridLayout;
	SliderList = (QSlider **) malloc (num * sizeof (QSlider));
	label_cur_selected_list = (QLabel **) malloc (num * sizeof(QLabel));
	NumOfConditions = num;
	box->setAlignment(Qt::AlignHCenter);
	QSignalMapper *sigmapper = new QSignalMapper(this);
	for (int i=0;i<num;i++)
	{
		QWidget *scale_widget = new QWidget;
		QVBoxLayout *scale_box = new QVBoxLayout;
		QPushButton *scale_button = new QPushButton;
		scale_button->setText(QString("%1").arg(i));
		connect(scale_button,SIGNAL(clicked()),sigmapper,SLOT(map()));
		sigmapper->setMapping(scale_button,i);
		connect(sigmapper,SIGNAL(mapped(int)),this,SLOT(playVideo(int)));
		for (int j=0;j<ScaleNameList.size();j++)
		{
			QLabel* label_cur = new QLabel(ScaleNameList[j]);
			scale_box->addWidget(label_cur);
		}
		scale_widget->setLayout(scale_box);
		QLabel* label_cur = new QLabel("Not_rated");
		QSlider* slider= new QSlider();
		slider->setObjectName(SliderNameList[i]);
		slider->setRange(min, max);
		slider->setTickPosition(QSlider::TicksRight);
		slider->setTickInterval((max-min)/ScaleNameList.size());
		box->addWidget(scale_button,0,i*2);
		box->addWidget(label_cur,1,i*2);
		box->addWidget(slider,2,i*2);
		box->addWidget(scale_widget,2,i*2+1);
		connect(slider,SIGNAL(valueChanged(int)),label_cur,SLOT(setNum(int)));
		connect(slider,SIGNAL(valueChanged(int)),this,SLOT(updateCurSlider()));
		SliderList[i] = slider;
		label_cur_selected_list[i] = label_cur;
	};
	ui.widget_score->setLayout(box);
	ui.widget_score->show();
}

void ScoreWindow::playVideo(int i)
{
	//QMessageBox::information( this, "Information", VideoListInCurrentScene[i]);
	emit playVideoInMainWindow(VideoListInCurrentScene[i]);
	enableSlider(i);
}

void ScoreWindow::updateCurSelect()
{
	for (int i=0; i < NumOfConditions; i++)
	{
		if (ButtonList[i]->isChecked() == true)
		{
				label_cur_selected->setText(ButtonList[i]->text());
				ui.buttonNext->setEnabled(true);
		}
	};
}

void ScoreWindow::updateCurSlider()
{
	bool enable = true;
	for (int i=0; i < NumOfConditions; i++)
	{
		QString SingleResult;
		QString cur_label = label_cur_selected_list[i]->text();
		if (cur_label.contains("Not_rated"))
		{
			enable = false;
		}		
	};
	if (enable)
		ui.buttonNext->setEnabled(true);
}

void ScoreWindow::initButton()
{
	for (int i=0; i < NumOfConditions; i++)
	{
		ButtonList[i]->setAutoExclusive(false);
		ButtonList[i]->setChecked(false);
		ButtonList[i]->setAutoExclusive(true);
	};
	label_cur_selected->setText("Not_rated");
}

void ScoreWindow::initSlider(bool slider_status)
{
	for (int i=0; i < NumOfConditions; i++)
		{
			SliderList[i]->setValue(0);
			SliderList[i]->setDisabled(slider_status);
			label_cur_selected_list[i]->setText("Not_rated");
	}
}

void ScoreWindow::enableSlider(int i)
{
	SliderList[i]->setDisabled(false);
}

QString ScoreWindow::getCurSliderResults(const QStringList& fileList)
{
	QString Results;
	Results.append("{");
	for (int i=0; i < NumOfConditions; i++)
	{
		QString SingleResult;
		const QString& fName = fileList[i];
		QString cur_label = label_cur_selected_list[i]->text();	
		SingleResult = "\"" + fName + "\":" + cur_label; 
		if (i != NumOfConditions -1)
		{
			SingleResult.append(",");
		};
		Results.append(SingleResult);
	};
	Results.append("}\n");
	return Results;
}

void ScoreWindow::getCurSliderResultsPlusTimestamp(QString timestamp)
{
	QString SingleResult;
	QString cur_label = label_cur_selected_list[0]->text();	
	SingleResult = "(" +  cur_label + ":" + timestamp + "),"; 
	sscqeresult.append(SingleResult);
}

void ScoreWindow::writeSSCQEResultsFile()
{
	QTextStream out(FileHandle);
	out<<"{\"" + currentVideo + "\":" + sscqeresult + "}";
	out.flush();
	sscqeresult.clear();
}

void ScoreWindow::setCurrentVideo(QString cur)
{
	currentVideo = cur;
}

QString ScoreWindow::getCurButtonResults(const QStringList& fileList)
{
	QString Results;
	Results.append("{");
	for (int i=0; i < NumOfConditions; i++)
	{
		QString SingleResult;
		const QString& fName = fileList[i];
		if (ButtonList[i]->isChecked() == true)
		{		
			SingleResult = "\"" + fName + "\":1"; 
		}
		else
		{
			SingleResult = "\"" + fName + "\":0"; 
		};
		if (i != NumOfConditions -1)
		{
			SingleResult.append(",");
		};
		Results.append(SingleResult);
	};
	Results.append("}\n");
	return Results;
}

void ScoreWindow::openResultsFile(QString fname, int mode)
{
	FileHandle = new QFile(fname);
	switch(mode) {
	case 0: FileHandle->open(QIODevice::WriteOnly | QIODevice::Text); break;
	case 1: FileHandle->open(QIODevice::ReadOnly | QIODevice::Text); break;
	case 2: FileHandle->open(QIODevice::Append | QIODevice::Text); break;
	default:
		break;
	}
}

void ScoreWindow::writeResultsFile(QString results)
{
	QTextStream out(FileHandle);
	out<<results;
	out.flush();
}

void ScoreWindow::closeResultsFile()
{
	FileHandle->close();
}

QVariant ScoreWindow::shuffleList(QVariant origin, bool shuffle_scene, bool shuffle_video, bool keep_ref)
{
	std::srand(std::time(0));
	QList<QVariant> videoList = origin.toList();
	int size = videoList.size();
	QList<QStringList> c_videoList;

	for(int i=0; i<size; i++)
	{
		QStringList temp = videoList[i].toStringList();
		c_videoList.append(temp);
	}
	
	if (shuffle_scene == true)
		std::random_shuffle(c_videoList.begin(),c_videoList.end());
	if (shuffle_video == true)
		{
			int len = origin.toList().size();
			for (int i = 0; i< len; i++)
			{
				if (keep_ref == true)
				{
					int video_size = c_videoList[i].size();
					QStringList temp;
					for (int j = 1; j < video_size; j++)
						temp.append(c_videoList[i][j]);
					std::random_shuffle(temp.begin(),temp.end());
					for (int j = 1; j < video_size; j++)
						c_videoList[i][j] = temp[j-1];
				}
				else
					std::random_shuffle(c_videoList[i].begin(),c_videoList[i].end());
			}
	}

	QList<QVariant> output;
	
	for(int i=0; i<size; i++)
	{
		QVariant temp(c_videoList[i]);
		output.append(temp);
	}

	return QVariant(output);	
}


void ScoreWindow::SetVideoListInCurrentScene(QStringList cur)
{
	VideoListInCurrentScene = cur;
}

void ScoreWindow::wheelEvent(QWheelEvent *event)
{
	int numDegree = event->delta() /8;
	int numSteps = numDegree /15;

	if (event->orientation() == Qt:: Vertical)
	{
		int curSliderValue = SliderList[0]->value();
		int value = curSliderValue + numSteps * 10;
		value = MIN(MAX(value,0),100);
		SliderList[0]->setValue(value);
	}
}

void ScoreWindow::mouseMoveEvent(QMouseEvent * event)
{
	if (restrictMouse == true)
	{
		int min_x = this->mapToGlobal(this->pos()).x();
		int min_y = this->mapToGlobal(this->pos()).y();
		int height = this->height();
		int width = this->width();
		int cur_x = event->x();
		int cur_y = event->y();
		int new_x, new_y;
		new_x = MIN(MAX(min_x,cur_x),min_x+width);
		new_y = MIN(MAX(min_y,cur_y),min_y+height);
		QCursor::setPos(new_x, new_y);
	}
}

void ScoreWindow::enableRestrictMouse(bool status)
{
	restrictMouse = true;
}
