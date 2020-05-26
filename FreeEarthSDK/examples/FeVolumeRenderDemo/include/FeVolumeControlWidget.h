#ifndef FE_VOLUME_RENDER_CONTROL_WIDGET_H
#define FE_VOLUME_RENDER_CONTROL_WIDGET_H

#include <QWidget>

#include <VolumeRendering.h>

class CVolumeRenderControlPan: public QWidget
{
	Q_OBJECT

public:
	CVolumeRenderControlPan(FeVolumeRender::CVolumeRenderNode* pNode, QWidget* parent);

	~CVolumeRenderControlPan();

public:
	void InitWidget();


public slots:
	void SlotStepChanged(double);

	void SlotSampleNumChanged(int);

protected:
	osg::observer_ptr<FeVolumeRender::CVolumeRenderNode>  m_opVolumeRenderNode;
};


#endif