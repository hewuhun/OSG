#include <FlightDemo.h>

#include <qfile.h>
#include <qtextstream.h>

#include <osg/MatrixTransform>
#include <osg/Geometry>
#include <osg/AutoTransform>
#include <osgDB/ReadFile>
 
#include <FeUtils/CoordConverter.h>
#include <FeUtils/PathRegistry.h>
#include <FeUtils/StrUtil.h>
#include <FeUtils/MiniAnimationBuilder.h>
#include <FeUtils/FlightMotionAlgorithm.h>
#include <FeUtils/MiniAnimationCtrl.h>

#include <FeExtNode/ExModelNode.h>
#include "FeExtNode/ExTextNode.h"
#include "FeExtNode/ExPlaceNode.h"

#include <FeSilverliningLib/Color.h>
#include <FeSilverlining/SilverliningSys.h>
CFlightDemo::CFlightDemo( FeUtil::CRenderContext* rc, FeManager::CFreeMarkSys* pMarkSys)
	: m_opRenderContext(rc)
	, m_opMarkSys(pMarkSys)
{
	m_speedTimes = 1;
}

CFlightDemo::~CFlightDemo()
{
}

// 从文件中加载模型运动路径
void ReadAnimationPathFromFile(const std::string& strFile, std::vector<osg::Vec3d>& vecPoints)
{
	std::string curPath = FeFileReg->GetFullPath("model/H6/routes/hangji2");
	QString qfn = QString::fromStdString(curPath);
	QFile file( qfn );
	if( file.open(QIODevice::ReadOnly | QIODevice::Text) ) 
	{
		QTextStream strStream( &file );
		QString line;
		while ( !strStream.atEnd()) 
		{
			line = strStream.readLine();
			QStringList subData = line.split("  ");
			if (3 == subData.count())
			{
				osg::Vec3d pos(subData.at(0).toDouble(), subData.at(1).toDouble(), subData.at(2).toDouble());
				if (pos.valid())
					vecPoints.push_back(pos);
			}
		}
		
		file.close();
	}
}

// 动画路径中事件回调，用于播放飞机起落架动画
class CPlayModelAnim : public FeUtil::CAnimTimerCallback::TimerCallFunc
{
public:
	CPlayModelAnim(FeEffect::CFeAnimationNode* pNode) : m_opAnimNode(pNode){}
	virtual void operator ()() 
	{
		if(m_opAnimNode.valid()) m_opAnimNode->PlayForTime(m_opAnimNode->GetPeriodSec());
	}
private:
	osg::observer_ptr<FeEffect::CFeAnimationNode> m_opAnimNode;
};



bool CFlightDemo::Init()
{
	// 创建飞机模型（自带动画）
	m_rpAirModel = new FeEffect::CFeAnimationNode(FeFileReg->GetFullPath("model/H6/boying737.ive"), 5.0);
	//m_rpAirModel = new FeEffect::CFeAnimationNode(FeFileReg->GetFullPath("D:\\OSG\\OSG_3.4.0\\data\\cow.osg"), 5.0);
	//SilverLining::Color::ToGrayscale()>;	

	 SilverLining::Color * pColor=new SilverLining::Color ;
	 pColor->ToGrayscale();

	//停止动画(若不停止，可看到起落架运动)
	m_rpAirModel->StopAnimation();
	
	FeExtNode::CExModelNode* pModelNode = new FeExtNode::CExModelNode(m_opRenderContext.get(), new FeExtNode::CExModelNodeOption);
	pModelNode->GetSceneNode()->addChild(m_rpAirModel);
	
	m_opMarkSys->AddMark(pModelNode);

	// 飞机信息更新
	m_rpAirModel->addUpdateCallback(new CModelPosReport(this, m_opRenderContext.get()));

	// 加载并创建飞机飞行路径
	std::vector<FeUtil::SDriveData> modelPath;
	std::vector<osg::Vec3d> vecPoints;
	ReadAnimationPathFromFile(FeFileReg->GetFullPath("model/H6/routes/hangji0"), vecPoints);

	if(m_opRenderContext.valid() && vecPoints.size() > 2)
	{
		// 球面插值拐角平滑
		FeUtil::FlightInterpolationAndSphereSmooth(m_opRenderContext.get(), vecPoints, modelPath, 1000, 5, 4000, 5);

		// 创建路径动画控制器
		FeUtil::CAnimTimerCallback* pAnimCB = new FeUtil::CAnimTimerCallback();
		m_rpAnimationCtrl = new FeUtil::CMiniAnimationCtrl(m_opRenderContext.get(), pModelNode->GetTransMT(), pAnimCB);
		m_rpAnimationCtrl->CreateAnimationPath(modelPath);
		m_rpAnimationCtrl->SetAnimationLoopMode(FeUtil::AnimationPath::NO_LOOPING);
		m_rpAnimationCtrl->Reset();
		
		// 起落架收起/降落位置
		const int nKeyPackUpIndex = modelPath.size() * 0.4 / 20.0; 
		const int nKeyPullDownIndex = modelPath.size() *16.0 / 20.0; 

		// 在动画中插入自定义事件用于飞机起落架动画
		pAnimCB->AddTimeCaller(modelPath.at(nKeyPackUpIndex).dTime, new CPlayModelAnim(m_rpAirModel));
		pAnimCB->AddTimeCaller(modelPath.at(nKeyPullDownIndex).dTime, new CPlayModelAnim(m_rpAirModel));

		// 起落架收起/降落位置标记
		//初始化文本的风格，使用UTF8，字体大小，颜色等
		osg::ref_ptr<osgEarth::Annotation::TextSymbol> rpTextSymbol = new osgEarth::Annotation::TextSymbol;
		rpTextSymbol->encoding() = osgEarth::Annotation::TextSymbol::ENCODING_UTF8;
		rpTextSymbol->alignment() = osgEarth::Annotation::TextSymbol::ALIGN_LEFT_CENTER;
		rpTextSymbol->font() = "simhei.ttf";
		rpTextSymbol->size() = 20;
		rpTextSymbol->fill()->color() = osgEarth::Color::Yellow;

		osgEarth::Annotation::Style style;
		style.addSymbol(rpTextSymbol);

		osg::ref_ptr<osg::Image> im = osgDB::readImageFile(FeFileReg->GetFullPath("texture/mark/placemark32.png"));

		m_OnOffLabel[0] = new osgEarth::Annotation::PlaceNode(m_opRenderContext->GetMapNode(),
			osgEarth::GeoPoint(m_opRenderContext->GetMapNode()->getMapSRS(), modelPath.at(nKeyPackUpIndex).vecLLH), 
			im.get(), FeUtil::ToUTF("收起落架"), style);
		m_OnOffLabel[0]->getOrCreateStateSet()->setRenderBinDetails(2000,"DepthSortedBin");
		m_OnOffLabel[1] = new osgEarth::Annotation::PlaceNode(m_opRenderContext->GetMapNode(),
			osgEarth::GeoPoint(m_opRenderContext->GetMapNode()->getMapSRS(), modelPath.at(nKeyPullDownIndex).vecLLH),
			im.get(), FeUtil::ToUTF("放起落架"), style);
		m_OnOffLabel[1]->getOrCreateStateSet()->setRenderBinDetails(2000,"DepthSortedBin");

		m_opRenderContext->GetRoot()->addChild(m_OnOffLabel[0].get());
		m_opRenderContext->GetRoot()->addChild(m_OnOffLabel[1].get());


		// 绘制航线
		osg::ref_ptr<osg::Geometry> rpGeometry = new osg::Geometry();
		osg::ref_ptr<osg::Vec3dArray> rpVertArr = new osg::Vec3dArray();

		osg::Vec3d centerPos = modelPath.at(0).vecLLH;
		osg::Vec3d wCenterPos;
		FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(),centerPos,wCenterPos);

		std::vector<FeUtil::SDriveData>::const_iterator it = modelPath.begin();
		for (;it != modelPath.end();it++)
		{
			osg::Vec3d wCurPos;
			FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(),(*it).vecLLH,wCurPos);
			wCurPos -= wCenterPos;

			rpVertArr->push_back(wCurPos);
		}

		osg::ref_ptr<osg::DrawArrays> rpIndex = new osg::DrawArrays(osg::PrimitiveSet::POINTS,0,modelPath.size());
		//rpIndex->setMode(osg::PrimitiveSet::LINE_STRIP);

		osg::ref_ptr<osg::Vec4dArray> rpColorArr = new osg::Vec4dArray();
		rpColorArr->push_back(osg::Vec4d(1,1,0,1));

		rpGeometry->setVertexArray(rpVertArr);
		rpGeometry->addPrimitiveSet(rpIndex);
		rpGeometry->setColorArray(rpColorArr,osg::Array::BIND_OVERALL); 

		osg::ref_ptr<osg::Geode> rpGeode = new osg::Geode();
		rpGeode->addDrawable(rpGeometry);

		osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform();
		osg::Matrix mat;
		mat.makeTranslate(wCenterPos);
		mt->setMatrix(mat);

		mt->addChild(rpGeode);
		mt->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
		//mt->setNodeMask(0);
		m_pathNode = mt.get();
		m_opRenderContext->GetRoot()->addChild(mt);

	}

	return true;
}




void CFlightDemo::Start()
{
	m_rpAirModel->Reset();
	m_rpAnimationCtrl->Start();
}


void CFlightDemo::Reset()
{
	m_rpAnimationCtrl->Reset();
	m_rpAirModel->Reset();
}


void CFlightDemo::ShowAirLine( bool bShow )
{
	if(m_pathNode.valid())
	{
		m_pathNode->setNodeMask(bShow?0xFF:0);
		
		if(m_OnOffLabel[0].valid())
		{
			m_OnOffLabel[0]->setNodeMask(bShow?0xFF:0);
		}

		if(m_OnOffLabel[1].valid())
		{
			m_OnOffLabel[1]->setNodeMask(bShow?0xFF:0);
		}
	}
}

void CFlightDemo::SetSpeed( int nSpeed )
{
	m_speedTimes = nSpeed;
	double speed = nSpeed;
	if(speed > 1)
	{
		speed *= 0.6;
	}
	m_rpAnimationCtrl->SetSpeedOffset(speed);
}


CModelPosReport::CModelPosReport( CFlightDemo* pFlightDemo, FeUtil::CRenderContext* pRC ) 
	: m_opFlightDemo(pFlightDemo)
	, m_opRenderContext(pRC)
{
}

void CModelPosReport::operator()( osg::Node* node, osg::NodeVisitor* nv )
{
	if(node)
	{
		osg::Matrix mxCur = node->getWorldMatrices()[0];

		if(m_mxLast != mxCur)
		{
			m_mxLast = mxCur;

			osg::Vec3d posLLH;
			FeUtil::XYZ2DegreeLLH(m_opRenderContext.get(), m_mxLast.getTrans(), posLLH);

			auto toDMS = [](double df)->QString{
				int d = (int)df;
				double dy = (df - d) * 60.0;
				int m = (int)dy;
				double my = (dy - m) * 60.0;
				double s = (int)(my * 10)/10.0;

				return QString().append(QString::number(d)).append(QString::fromLocal8Bit("°")).append(QString::number(m)).append(QString::fromLocal8Bit("\' ")).append(QString::number(s,'g',3)).append('\"');
			};

			QString dynStr;
			dynStr.append(QString::fromLocal8Bit("经度： ")).append(toDMS(posLLH.x())).append("\n");
			dynStr.append(QString::fromLocal8Bit("纬度： ")).append(toDMS(posLLH.y())).append("\n");
			double higth = (int)posLLH.z();
			dynStr.append(QString::fromLocal8Bit("高度： ")).append(QString::number(higth,'g',8)).append(" m ").append("\n");
			
			emit m_opFlightDemo->Sig_StateChanged(dynStr);
		}
	}

	traverse(node, nv);
}


