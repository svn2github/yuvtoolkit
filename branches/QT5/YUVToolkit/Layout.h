#include "YT_Interface.h"
#include <QWidget>

class VideoView;

class Layout : public QObject
{
	Q_OBJECT;
public:
	Layout(QWidget*);
	~Layout(void);

	void AddView(VideoView*);
	void RemoveView(VideoView*);

	void UpdateGeometry();

	// Get number of videos
	int GetVideoCount();

	// Get the size needed for displaying the video
	void GetDisplaySize( QSize& displaySize);

	// Panning + drag and drop repositioning
	void OnMouseMoveEvent( QMouseEvent* );
	void OnMousePressEvent( QMouseEvent* );
	void OnMouseReleaseEvent( QMouseEvent* );

	VideoView* FindVideoAtMoisePosition();
protected:
	void UpdateGrid();
private slots:
	
private:
	QWidget* parent;

	QList<VideoView*> m_VideoList;
	QMap<unsigned short, VideoView*> m_VideoGrid;
	unsigned short m_CountX; // number of video in x axis
	unsigned short m_CountY; // number of video in y axis
	

	VideoView* m_ActiveVidew;
	unsigned short m_ActiveX;
	unsigned short m_ActiveY;
	int m_ViewWidth, m_ViewHeight; // size of each video view
};