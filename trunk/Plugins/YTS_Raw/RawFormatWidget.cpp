#include "RawFormatWidget.h"
#include "YTS_Raw.h"

#define COLOR_COUNT 17

COLOR_FORMAT colors[COLOR_COUNT] = 
{
	I420,
	YV12,
	YUY2,
	UYVY,
	YVYU,
	NV12,
	IMC2,
	IMC4,
	Y800,
	RGB24,
	BGR24,
	RGBX32,
	XRGB32,
	BGRX32,
	XBGR32,
	RGB565,
	BGR565,
};

QString color_names[COLOR_COUNT] = 
{
	"I420/IYUV",
	"YV12",
	"YUY2/YUYV",
	"UYVY",
	"YVYU",
	"NV12",
	"IMC2",
	"IMC4",
	"Gray Scale (8 bits)",
	"RGB (24 bits Little Endian) ",
	"BGR (24 bits Little Endian)",
	"RGBX (32 bits Little Endian)",
	"XRGB (32 bits Little Endian)",
	"BGRX (32 bits Little Endian)",
	"XBGR (32 bits Little Endian)",
	"RGB565 (16 bits Little Endian)",
	"BGR565 (16 bits Little Endian)",
};


RawFormatWidget::RawFormatWidget(YTS_Raw* r, QWidget *parent) : QWidget(parent), rawSource(r), m_Format(NULL)
{
	ui.setupUi(this);

	for (int i=0; i<COLOR_COUNT; i++)
	{
		ui.Color->addItem(color_names[i]);
	}

	ui.Stride0->setText("0");
	ui.Stride1->setText("0");
	ui.Stride2->setText("0");
	ui.Stride3->setText("0");
	
	ui.Width->setValidator(new QIntValidator(ui.Width));
	ui.Height->setValidator(new QIntValidator(ui.Height));

	ui.Stride0->setValidator(new QIntValidator(ui.Stride0));
	ui.Stride1->setValidator(new QIntValidator(ui.Stride1));
	ui.Stride2->setValidator(new QIntValidator(ui.Stride2));
	ui.Stride3->setValidator(new QIntValidator(ui.Stride3));

	connect(ui.Apply, SIGNAL(clicked()), this, SLOT(OnApply()));
}

RawFormatWidget::~RawFormatWidget()
{
	m_Format.clear();
}

void RawFormatWidget::OnApply()
{
	m_Format->SetColor(colors[ui.Color->currentIndex()]);
	m_Format->SetWidth(ui.Width->text().toInt());
	m_Format->SetHeight(ui.Height->text().toInt());
	m_Format->SetStride(0, ui.Stride0->text().toInt());
	m_Format->SetStride(1, ui.Stride1->text().toInt());
	m_Format->SetStride(2, ui.Stride2->text().toInt());
	m_Format->SetStride(3, ui.Stride3->text().toInt());

	double fps = ui.FPS->value();

	rawSource->ReInit(m_Format, fps);
}

void RawFormatWidget::showEvent( QShowEvent *event )
{
	if (!m_Format)
	{
		m_Format = GetHost()->NewFormat();
	}

	SourceInfo info;
	rawSource->GetInfo(info);

	ui.Title->setText(rawSource->GetPath());
	ui.Width->setText(QString::number(info.format->Width()));
	ui.Height->setText(QString::number(info.format->Height()));
	
	int idx = 0;
	for (int j=0; j<COLOR_COUNT; j++)
	{
		if (colors[j] == info.format->Color())
		{
			idx = j;
		}
	}
	ui.Color->setCurrentIndex(idx);
	ui.FPS->setValue(info.maxFps);
	/*ui.Stride0->setText(QString::number(info.format->Stride(0)));
	ui.Stride1->setText(QString::number(info.format->Stride(1)));
	ui.Stride2->setText(QString::number(info.format->Stride(2)));
	ui.Stride3->setText(QString::number(info.format->Stride(3)));*/
}
