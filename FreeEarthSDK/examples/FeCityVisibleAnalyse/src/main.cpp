#include <QApplication>
#include <QMainWindow>
#include <QTextCodec>

#include <FeEarth/3DSceneWidget.h>
#include <FeUtils/PathRegistry.h>
#include <FeUtils/FreeViewPoint.h>
#include <FeEffects/Corridor.h>
#include <FeEffects/ViewAnalyseNode.h>
#include <FeExtNode/ExModelNode.h>


#include <FreeSampleFrame/include/FreeMainWindow.h>


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
	pWidget->SetTitle(QString::fromLocal8Bit("城市可视域分析效果演示"));
	FeEarth::C3DSceneWidget* p3DScenePanel = new FeEarth::C3DSceneWidget("");
	if(!p3DScenePanel->Initialize())
	{
		return -1;
	}

	p3DScenePanel->setMinimumSize(500, 500);
	pWidget->GetSceneLayout()->addWidget(p3DScenePanel);


	FeUtil::CRenderContext* pRenderContext = p3DScenePanel->GetSystemManager()->GetRenderContext();
	if(pRenderContext)
	{
		FeEffect::ViewAnalyseNode* vn = new FeEffect::ViewAnalyseNode(pRenderContext);
		pRenderContext->GetRoot()->addChild(vn);

		osg::Vec3d pos(120.3,23.5,16);
		FeExtNode::CExModelNodeOption opt;
		opt.ModelPath() =  FeFileReg->GetFullPath("model/jianzhuqun5.ive");
		FeExtNode::CExModelNode * model = new FeExtNode::CExModelNode(pRenderContext, &opt);
		model->SetPosition(pos);
		model->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::ON);


		osg::Vec3d up(0,0,1);
		osg::Vec3d ePos = pos;
		ePos.z() += 2;
		osg::Vec3d eyePos;
		FeUtil::DegreeLLH2XYZ(pRenderContext,ePos,eyePos);
		osg::Vec3d posDir = eyePos;
		posDir.normalize();
		osg::Vec3d rDir = up ^ posDir;
		rDir.normalize();
		osg::Vec3d nDir = posDir ^ rDir;
		nDir.normalize();

		osg::Vec3d lookPos = eyePos + nDir * 10;
		
		vn->setEyePos(eyePos);
		vn->setLookPos(lookPos);

		if(model)
		{
			vn->addChild(model);
		}

		vn->init();
	}


	/// 设置初始视角
	p3DScenePanel->GetSystemManager()->GetSystemService()->GetManipulatorManager()->SetHome(osgEarth::Viewpoint(
		120.30088, 23.501005, -179.93441, -42.789456, 1055.3453),0.5);
	p3DScenePanel->GetSystemManager()->GetSystemService()->GetManipulatorManager()->Home();
	
	/// 设置界面内容
	{
		CFreeLableWidget* pLabelWidget = new CFreeLableWidget();
		pLabelWidget->SetTitleText(QString::fromLocal8Bit("功能介绍"));
		pLabelWidget->SetContentText(QString::fromLocal8Bit(
			"    城市可视域分析，展示了在建筑群中以第一人称观察建筑物的视野情况。\n"
			"    其中，绿色部分表示可视区域，红色部分表示不可视区域。\n")
			);
		pWidget->AddControlWidget(pLabelWidget);
	}
	pWidget->AddControlItem(new QSpacerItem(10,20, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
	
	pWidget->ShowDialogNormal();
	pWidget->ShowDialogMaxisize();
	int nFlag = a.exec();

	return nFlag;
}




