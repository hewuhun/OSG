#include <FeEarth/3DSceneWidget.h>
#include <QApplication>
#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>
#include <FeOcean/OceanSys.h>
#include <FeUtils/PathRegistry.h>
#include <osgGA/StateSetManipulator>
#include <FeOcean/OceanNode.h>
#include <FeOcean/TritonNode.h>
#include <osgEarth/GeoTransform>
#include <osg/io_utils>
#include <FeExtNode/ExModelNode.h>
#include <FeManager/FreeMarkSys.h>

class MyEventCallback : public osgGA::GUIEventHandler
{
public:
	MyEventCallback(FeOcean::COceanSys * pOceanSys)
	{

		m_pOceanSys = pOceanSys;
		m_pTritonNode= nullptr;
	}
public:
	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) 
	{ 
		osgViewer::View * view = dynamic_cast<osgViewer::View*>(&aa);
		if (view == nullptr)
		{
			return false;
		}

		if (m_pOceanSys == nullptr)
		{
			return false;
		}
		if (m_pTritonNode == nullptr)
		{
			m_pTritonNode = m_pOceanSys->GetOceanNode();

		}


		if (m_pTritonNode != nullptr)
		{

			m_pTritonNode->SetSeaLevel(1000);
		}
		return false; 
	}

private:
	FeOcean::COceanSys * m_pOceanSys;
	FeOcean::TritonNode * m_pTritonNode;
};

class MyShipCallback : public osg::NodeCallback
{
public:
	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
	{
		osgEarth::GeoTransform * ts = dynamic_cast<osgEarth::GeoTransform*>(node);
		const osgEarth::GeoPoint p = ts->getPosition();
		static float k = 0.0;
		ts->setPosition(osgEarth::GeoPoint(p.getSRS(), p.x() + k, p.y(), p.z()));
		k +=0.00001;
		std::cout << ts->getChild(0)->getWorldMatrices()[0].getTrans() << std::endl;
	}
};


int main(int argc, char ** argv)
{
#if 1
	QApplication app(argc, argv);
	FeEarth::C3DSceneWidget * w = new FeEarth::C3DSceneWidget;
	w->SetEarthPath(FeFileReg->GetFullPath("earth/FreeEarth_flat_Ocean.earth"));
	if(!w->Initialize())
	{
		return -1;
	}

	FeUtil::CRenderContext* rc = w->GetSystemManager()->GetSystemService()->GetRenderContext();

	rc->GetView()->addEventHandler(new osgGA::StateSetManipulator(rc->GetCamera()->getOrCreateStateSet()));
	osgEarth::MapNode * mapNode = rc->GetMapNode();


	
	FeManager::CFreeMarkSys* pFreeMarkSys = new FeManager::CFreeMarkSys();
	pFreeMarkSys->Initialize(rc);
	w->GetSystemManager()->GetSystemService()->AddAppModuleSys(pFreeMarkSys);
	//pFreeMarkSys->Start();

	/// 加载模型
	osg::Vec3d pos2(110.0,34.0,2000000.0);


	// 加载卫星模型
	osg::ref_ptr<FeExtNode::CExModelNodeOption> rpOP2 = new FeExtNode::CExModelNodeOption();
	rpOP2->name() = "123";
	rpOP2->ModelPath() = FeFileReg->GetFullPath("model/weixing.ive");
	rpOP2->Position() = pos2;
	rpOP2->Scale() = osg::Vec3d(10000.0,10000.0,10000.0);
	osg::ref_ptr<FeExtNode::CExModelNode> rpNodeWinxing = new FeExtNode::CExModelNode(rc,rpOP2);
	pFreeMarkSys->AddMark(rpNodeWinxing);




	w->show();
	return app.exec();
#else

	osgViewer::Viewer viewer;

	osg::Node * node = osgDB::readNodeFile("D:\\work\\bjyd\\osgEarth_2.7.0\\tests\\gdal_tiff.earth");
	viewer.setSceneData(node);
	viewer.setCameraManipulator(new osgGA::TrackballManipulator);
	while (true)
	{
		viewer.frame();
	}
#endif
}

#if 0



#endif 