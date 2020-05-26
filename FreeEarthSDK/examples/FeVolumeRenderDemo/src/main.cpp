#include <VolumeRendering.h>

#include <FeEarth/FreeEarth.h>
#include <FeEarth/3DSceneWidget.h>
#include <FeShell/SystemSerializer.h>
#include <FeUtils/EnvironmentVariableReader.h>
#include <FeUtils/PathRegistry.h>
#include <FeUtils/CoordConverter.h>

#include <QTextCodec>
#include <QApplication>

#include <FreeMainWindow.h>
#include <FeVolumeControlWidget.h>

osg::Vec3d	g_vecPosition = osg::Vec3d(120.726, 22.03, 110000);
double		g_dScale = 100000;
FeUtil::CFreeViewPoint g_viewPoint = FeUtil::CFreeViewPoint(116.72166900706461,22.118882429732565,39708.989737277851,80.273974840965437,-4.8587988914405589,39708.989737277851, 3);

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QTextCodec *codec = NULL;
#ifdef WIN32
	codec = QTextCodec::codecForName("GBK");
#else
	codec = QTextCodec::codecForName("UTF-8");
#endif 
	if (codec)
	{
#ifdef QT4_VERSION
		QTextCodec::setCodecForLocale(codec);
 		QTextCodec::setCodecForCStrings(codec);
 		QTextCodec::setCodecForTr(codec);
#endif
	}

	CFreeMainWindow* pWidget = new CFreeMainWindow();
	pWidget->SetTitle(QString::fromLocal8Bit("体渲染效果演示"));

	FeEarth::C3DSceneWidget* p3DScenePanel = new FeEarth::C3DSceneWidget("");
	if(!p3DScenePanel->Initialize())
	{
		return -1;
	}

	pWidget->GetSceneLayout()->addWidget(p3DScenePanel);

	FeShell::CSystemManager* pSystemManager = p3DScenePanel->GetSystemManager();
	if (pSystemManager)
	{
		FeVolumeRender::CVolumeRenderNode* pVolumeRenderNode = new FeVolumeRender::CVolumeRenderNode(pSystemManager->GetRenderContext());

		osg::ref_ptr<osg::MatrixTransform> pMt = new osg::MatrixTransform();
		osg::Matrix transMat;
		FeUtil::DegreeLLH2Matrix(pSystemManager->GetRenderContext(), g_vecPosition, transMat);

		osg::Matrix scaleMat;
		scaleMat.makeScale(g_dScale, g_dScale, g_dScale);

		osg::Matrix rotateMat;
		rotateMat = osg::Matrix::rotate(osg::PI,osg::Vec3(0,1,0));

		pMt->addChild(pVolumeRenderNode);
		pMt->setMatrix(scaleMat * rotateMat * transMat);
		pSystemManager->GetRenderContext()->GetRoot()->addChild(pMt);

		osg::observer_ptr<FeShell::CEnvironmentSys> pFreeSkySys = pSystemManager->GetSystemService()->GetEnvironmentSys();
		if (pFreeSkySys.valid())
		{
			FeKit::CFreeSky* pFreeSky = pFreeSkySys->GetSkyNode();
			if(pFreeSky)
			{
				pFreeSky->Show();
				pFreeSky->SetAmbient(osg::Vec4d(1.0, 1.0, 1.0, 1.0));
				pFreeSky->SetDateTime(osgEarth::DateTime(2016, 1, 1, 16.0));
			}
		}

		{
			CFreeLableWidget* pLabelWidget = new CFreeLableWidget(pWidget);
			pLabelWidget->SetTitleText(QString::fromLocal8Bit("功能说明"));
			pLabelWidget->SetContentText(QString::fromLocal8Bit(
				"    体渲染技术是一种用于显示离散三维采"
				"样数据集的二维投影的技术。\n"
				"    本例中通过通过光线追踪技术实现点云"
				"数据的立体渲染，并将渲染效果集成到三维"
				"数字地球上，用户可对体渲染中的采样点数"
				"进行调整以观察体渲染效果。\n"));
			pWidget->AddControlWidget(pLabelWidget);
		}

		{
			CFreeLableWidget* pLabelWidget = new CFreeLableWidget(pWidget);
			pLabelWidget->SetTitleText(QString::fromLocal8Bit("操作面板"));
			pWidget->AddControlWidget(pLabelWidget);
		}

		CVolumeRenderControlPan* pControlPan = new CVolumeRenderControlPan(pVolumeRenderNode, pWidget);
		pWidget->AddControlWidget(pControlPan);

		pWidget->AddControlItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));

		pWidget->ShowDialogNormal();
		pWidget->ShowDialogMaxisize();

		pSystemManager->GetSystemService()->Locate(g_viewPoint, true);
		int nFlag = a.exec();

		return nFlag;
	}

	return 0;
}