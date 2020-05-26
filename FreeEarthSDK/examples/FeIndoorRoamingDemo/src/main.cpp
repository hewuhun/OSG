#include <QApplication>
#include <QMainWindow>
#include <QTextCodec>

#include <FeEarth/FreeEarth.h>
#include <FeEarth/3DSceneWidget.h>
#include <FeKits/manipulator/FirstPersonManipulator.h>

#include <FeUtils/EnvironmentVariableReader.h>
#include <FeUtils/CoordConverter.h>
#include <FeUtils/PathRegistry.h>

#include <FreeMainWindow.h>

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
	pWidget->SetTitle(QString::fromLocal8Bit("室内漫游效果演示"));
  
	FeEarth::C3DSceneWidget* p3DScenePanel = new FeEarth::C3DSceneWidget("");
	
	if(p3DScenePanel->Initialize())
	{
		cout<<" Initilize sucess!!!"<<endl;
	}
	else
	{
		return -1;
	}

	pWidget->GetSceneLayout()->addWidget(p3DScenePanel);

	FeShell::CSystemManager* pSystemManager = p3DScenePanel->GetSystemManager();
	if (pSystemManager)
	{
		FeUtil::CRenderContext *pContext = pSystemManager->GetRenderContext();
		if (NULL == pContext)
		{
			return 0;
		}

		std::string strFileName = FeFileReg->GetFullPath("model/manyoujianzhu_8.ive");


		 
//#if _WIN32
//		std::string strOctFileName = "";
//#else
 		std::string strOctFileName = FeFileReg->GetFullPath("model/manyoujianzhu_8.oct");
//#endif

		osg::ref_ptr<osg::Node> rpNode = osgDB::readNodeFile(strFileName);
		if(rpNode.valid())
		{
			rpNode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
			osg::ref_ptr<osg::MatrixTransform> rpMt = new osg::MatrixTransform;
			osg::Vec3d vecPos(108.87256, 34.195248, 419.61813);
			osg::Matrix matrix;
			FeUtil::DegreeLLH2Matrix(pContext,vecPos,matrix);

			rpMt->setMatrix(matrix);
			rpMt->addChild(rpNode);

			pContext->GetRoot()->addChild(rpMt);

			osg::ref_ptr<FeKit::FirstPersonManipulator> pPerMani = new FeKit::FirstPersonManipulator(pContext);
			pPerMani->SetSavePath(FeFileReg->GetFullPath("config/indoor.txt"));
			pPerMani->setHomePosition(osg::Vec3d(vecPos.x(), vecPos.y(), vecPos.z()+2), 0, 0);
			if(pPerMani->init(rpNode,strOctFileName))
			{
				osgViewer::View* pViewer = pContext->GetView();
				if(pViewer)
				{
					FeKit::CManipulatorManager* pManipulatorManager = dynamic_cast<FeKit::CManipulatorManager*>(pViewer->getCameraManipulator());
					if(pManipulatorManager)
					{
						pManipulatorManager->addMatrixManipulator('p', "PersonManipulator", pPerMani);
						pManipulatorManager->selectMatrixManipulatorWithKey('p');
					}
				}
			}

			//{
			//	CFreeLableWidget* pLabelWidget = new CFreeLableWidget(pWidget);
			//	pLabelWidget->SetTitleText(QString::fromLocal8Bit("功能说明"));
			//	pLabelWidget->SetContentText(QString::fromLocal8Bit(
			//		"    室内漫游通过操作键盘和鼠标实现对三"
			//		"维局部精细场景的漫游浏览功能，其中增加"
			//		"了碰撞检测、上下楼梯、跳跃等功能；用户"
			//		"还可以录制漫游脚本。\n")
			//		);
			//	pWidget->AddControlWidget(pLabelWidget);
			//}

			//{
			//	CFreeLableWidget* pLabelWidget = new CFreeLableWidget(pWidget);
			//	pLabelWidget->SetTitleText(QString::fromLocal8Bit("操作说明"));
			//	pLabelWidget->SetContentText(QString::fromLocal8Bit(
			//		"        按键 P:         漫游操作器\n"
			//		"        按键 U:         向前移动\n"
			//		"        按键 J:         向后移动\n"
			//		"        按键 H:         向左移动\n"
			//		"        按键 K:         向右移动\n"
			//		"        按键 空格:      跳跃\n"
			//		"        按键 L:         重置位置\n"
			//		"        鼠标  :         改变视角方向\n"
			//		"\n"
			//		"        按键 F9:        开始录制脚本\n"
			//		"        按键 F10:       停止录制脚本\n"
			//		"        按键 F11:       播放脚本\n"
			//		));
			//	pWidget->AddControlWidget(pLabelWidget);
			//}

		//	pWidget->AddControlItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));

			pWidget->ShowDialogNormal();
			pWidget->ShowDialogMaxisize();

			//pSystemManager->GetSystemService()->Locate(
			//	FeUtil::CFreeViewPoint(120.5012733308, 23.9842753057, 9940.3957404327, 126.9789496231, -9.0714030130, 9940.3957404327, 3),
			//	true);

			int nFlag = a.exec();

			return nFlag;
		}
	}

	return 0;
}

//
//int main()
//{
//	FeUtil::CEnvironmentVariableReader reader;
//	FeEarth::CFreeEarth* pFreeEarth = new FeEarth::CFreeEarth(reader.DoRead("FREE_EARTH_FILE_PATH"));
//
//	if(pFreeEarth->Initialize())
//	{
//		std::string strFileName = FeFileReg->GetFullPath("model/manyoujianzhu_8.ive");
//		std::string strOctFileName = FeFileReg->GetFullPath("model/manyoujianzhu_8.oct");
//
//		//std::string strFileName = FeFileReg->GetFullPath("model/jianzhumanyou1.ive");
//		//std::string strOctFileName = FeFileReg->GetFullPath("model/jianzhumanyou1.oct");
//
//		//std::string strFileName = FeFileReg->GetFullPath("model/jianzhumanyou1.ive");
//		//std::string strOctFileName = FeFileReg->GetFullPath("model/jianzhumanyou1.oct");
//
//		osg::Node* n = osgDB::readNodeFile(strFileName);
//		if(n)
//		{
//			osg::MatrixTransform* mt = new osg::MatrixTransform;
//			osg::Vec3d vecPos(120.3,23.5,20);
//			osg::Matrix matrix;
//			FeUtil::DegreeLLH2Matrix(pFreeEarth->GetRenderContext(),vecPos,matrix);
//
//			mt->setMatrix(matrix);
//			mt->addChild(n);
//
//			pFreeEarth->GetRenderContext()->GetRoot()->addChild(mt);
//
//			osg::ref_ptr<FeKit::FirstPersonManipulator> pPerMani = new FeKit::FirstPersonManipulator(pFreeEarth->GetRenderContext());
//			osg::Vec3d viewPoint(120.3,23.5,22);
//			pPerMani->setHomePosition(viewPoint,0,0);
//			if(pPerMani->init(n,strOctFileName))
//			{
//				osgViewer::View* pViewer = pFreeEarth->GetRenderContext()->GetView();
//				if(pViewer)
//				{
//					FeKit::CManipulatorManager* pManipulatorManager = dynamic_cast<FeKit::CManipulatorManager*>(pViewer->getCameraManipulator());
//					if(pManipulatorManager)
//					{
//						pManipulatorManager->addMatrixManipulator('p', "PersonManipulator", pPerMani);
//					}
//				}
//			}
//		}
//
//		while(1)
//		{
//			pFreeEarth->Frame();
//		}
//
//		pFreeEarth->UnInitialize();
//		delete pFreeEarth;
//		pFreeEarth = NULL;
//
//	}
//}