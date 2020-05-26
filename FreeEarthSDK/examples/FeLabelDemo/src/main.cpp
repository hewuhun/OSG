#include <QApplication>
#include <QMainWindow>
#include <QTextCodec>

#include <FreeMainWindow.h>

#include <osgViewer/ViewerEventHandlers>

#include <FeUtils/StrUtil.h>
#include <FeUtils/PathRegistry.h>
#include <FeUtils/CoordConverter.h>
#include <FeUtils/EnvironmentVariableReader.h>
#include <FeUtils/MiniAnimationCtrl.h>

#include <FeManager/FreeMarkSys.h>
#include <FeExtNode/ExLabelNode.h>
#include <FeExtNode/ExLabelBindingNodeCallback.h>
#include <FeExtNode/ExLodModelNode.h>
#include <FeExtNode/ExModelNode.h>

#include <FeEarth/3DSceneWidget.h>
#include <FeEarth/FreeEarth.h>


using namespace FeShell;
using namespace FeExtNode;


#define _tr_(str) FeUtil::ToUTF(str)

FeUtil::CRenderContext* g_pRenderContext = NULL;
// const osg::Vec3d g_vecStart1(-117.36438, 33.291854, 240), g_vecEnd1(-117.33851, 33.312876, 240);
// const osg::Vec3d g_vecStart2(-117.36645, 33.294525, 240), g_vecEnd2(-117.33961, 33.313821, 240);
// const osg::Vec3d g_vecStart3(-117.37151, 33.29589, 240), g_vecEnd3(-117.34018, 33.314773, 240);
const osg::Vec3d g_vecStart1(111.16291, 10.783018, 1000000), g_vecEnd1(61.42838, 42.717614, 1000000);
const osg::Vec3d g_vecStart2(61.42838, 42.717614, 1000000), g_vecEnd2(145.79646, 44.283651, 1000000);
const osg::Vec3d g_vecStart3(145.79646, 44.283651, 1000000), g_vecEnd3(111.16291, 10.783018, 1000000);

/**
  * @class CPosUpdate
  * @brief 更新标牌的位置信息
  * @author g00034
*/
class CPosUpdate : public osg::NodeCallback
{
public:
	/**  
	  * @note 构造函数  
	  * @param pPosLabelItem [in] 包含位置信息的标牌项
	  * @param mtNode [in] 标牌绑定节点
	*/
	CPosUpdate(FeExtNode::CExLabelItemNode* pPosLabelItem, osg::MatrixTransform* mtNode)
		: m_opLabelItem(pPosLabelItem)
		,m_opMtNode(mtNode)
	{
	}

	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
	{
		if(nv->getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR)
		{
			if(!m_opMtNode.valid() || !m_opLabelItem.valid())
			{
				return;
			}

			FeExtNode::CExLabelItemNodeList childList = m_opLabelItem->GetChildList();
			if(childList.size() == 3)
			{
				osg::Vec3d vecLLH;
				FeUtil::XYZ2DegreeLLH(g_pRenderContext, m_opMtNode->getMatrix().getTrans(), vecLLH);
		
				FeExtNode::CExLabelItemNodeList::iterator iter = childList.begin();

				std::stringstream strStream;
				strStream << std::setprecision(4) << std::setiosflags(ios::fixed) << vecLLH.x();
				(iter++)->get()->SetText(_tr_("经度： " + strStream.str() + " 度"));

				std::stringstream strStream1;
				strStream1 << std::setprecision(4) << std::setiosflags(ios::fixed) << vecLLH.y();
				(iter++)->get()->SetText(_tr_("纬度： " + strStream1.str() + " 度"));

				std::stringstream strStream2;
				strStream2 << std::setprecision(1) << std::setiosflags(ios::fixed) << vecLLH.z();
				(iter++)->get()->SetText(_tr_("高度： " + strStream2.str() + " 米"));
			}
		}
	}

private:
	osg::observer_ptr<FeExtNode::CExLabelItemNode> m_opLabelItem;
	osg::observer_ptr<osg::MatrixTransform>        m_opMtNode;
};


/**
  * @class AirGen
  * @brief 飞机模型生成类
  * @author g00034
*/
class AirGen : public osg::Referenced
{
public:
	std::string m_strAirType;      // 机型
	std::string m_strIDNum;        // 编号
	std::string m_strAirBorne;     // 机载
	std::string m_strActiveTime;   // 服役时间
	std::string m_strTotalMileage; // 飞行总里程

public:
	AirGen(FeManager::CFreeMarkSys* pMarkSys, FeUtil::CRenderContext* pRenderContext)
		: m_opMarkSys(pMarkSys)
		, m_opRenderContext(pRenderContext)
	{
	}

	void GenAir(const osg::Vec3d& vecStart, const osg::Vec3d& vecEnd, const osg::Vec4d& vecBkColor, double dSpeed)
	{
		if(m_opMarkSys.valid())
		{
			std::string strName = _tr_(m_strAirType); 

			/// 标牌绑定到节点
			osg::ref_ptr<CExLabelNode> label1 = new CExLabelNode(m_opRenderContext.get(), new CExLabelNodeOption);
			label1->SetText(strName);
			label1->SetBackgroundImage(FeFileReg->GetFullPath("texture/mark/label_title.png"));
			label1->SetBackgroundColor(vecBkColor);
			label1->SetTextColor(osg::Vec4d(255.0/255.0, 90.0/255.0, 0, 1.0));
			label1->SetFontSize(13);
			label1->SetWidth(130);
			m_opMarkSys->AddMark(label1);
			
			FeUtil::FEID idPosItem = 0;
			if(1)
			{
				osg::ref_ptr<CExLabelItemNode> itemPos = new CExLabelItemNode(m_opRenderContext.get(), new FeExtNode::CExLabelItemOption);
				itemPos->SetText(_tr_("属性信息"));
				itemPos->SetWidth(130);
				itemPos->SetBackgroundImage(FeFileReg->GetFullPath("texture/mark/label_item.png"));
				itemPos->SetBackgroundColor(vecBkColor);
				label1->AddLabelItem(itemPos);

				itemPos->GetChildItem(itemPos->AddChildItem(_tr_(m_strIDNum)))->SetWidth(150);
				itemPos->AddChildItem(_tr_(m_strAirBorne));
				itemPos->AddChildItem(_tr_(m_strTotalMileage));
				itemPos->AddChildItem(_tr_(m_strActiveTime));
				itemPos->SetChildItemVisible(false);

				idPosItem = label1->AddLabelItem(_tr_("实时信息"));
				FeUtil::FEID idItem = label1->GetLabelItem(idPosItem)->AddChildItem(_tr_("经度:"));
				label1->GetLabelItem(idPosItem)->GetChildItem(idItem)->SetWidth(150);
				label1->GetLabelItem(idPosItem)->AddChildItem(_tr_("纬度:"));
				label1->GetLabelItem(idPosItem)->AddChildItem(_tr_("高度:"));
			}

			/// 创建绑定节点
			FeExtNode::CExModelNodeOption* opt = new FeExtNode::CExModelNodeOption;
			opt->ModelPath() = FeFileReg->GetFullPath("model/WZ_9000.ive");
			osg::ref_ptr<FeExtNode::CExModelNode> bindNode = new FeExtNode::CExModelNode(m_opRenderContext.get(), opt);
			bindNode->SetScale(osg::Vec3d(10000,10000,10000));
			m_opMarkSys->AddMark(bindNode);

			// 创建模型动画
			osg::ref_ptr<FeUtil::CMiniAnimationCtrl> miniAniCtrl = new FeUtil::CMiniAnimationCtrl(m_opRenderContext.get(), bindNode);
			std::vector<osg::Vec3d> points;
			points.push_back(vecStart);
			points.push_back(vecEnd);
			miniAniCtrl->CreateAnimationPath(points, dSpeed*1000);
			miniAniCtrl->SetAnimationLoopMode(FeUtil::AnimationPath::LOOP);
			miniAniCtrl->Start();
			

			/// 标牌绑定到节点
			label1->BindingNode(bindNode);

			/// 位置信息更新
			if(0 != idPosItem)
			{
				label1->GetLabelItem(idPosItem)->addUpdateCallback(new CPosUpdate(label1->GetLabelItem(idPosItem), bindNode->GetTransMT()));
			}
		}
	}

private:
	osg::observer_ptr<FeManager::CFreeMarkSys> m_opMarkSys;
	osg::observer_ptr<FeUtil::CRenderContext>  m_opRenderContext;
};


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
	pWidget->SetTitle(QString::fromLocal8Bit("标牌效果演示"));

	FeEarth::C3DSceneWidget* p3DScenePanel = new FeEarth::C3DSceneWidget("");
	if(!p3DScenePanel->Initialize())
	{
		return -1;
	}

	p3DScenePanel->setMinimumSize(500, 500);
	pWidget->GetSceneLayout()->addWidget(p3DScenePanel);

	g_pRenderContext = p3DScenePanel->GetSystemManager()->GetRenderContext();
	FeManager::CFreeMarkSys* pFreeMarkSys = new FeManager::CFreeMarkSys();
	pFreeMarkSys->Initialize(g_pRenderContext);
	p3DScenePanel->GetSystemManager()->GetSystemService()->AddAppModuleSys(pFreeMarkSys);
	pFreeMarkSys->Start();


	/// 标牌绑定到指定位置
	{
	osg::Vec3d vecZL(108.94238, 34.26098, 410.77634);
	osg::Vec3d vecHG(108.87256, 34.195248, 409.61813);

	osg::ref_ptr<CExLabelNodeOption> optLabel2 = new CExLabelNodeOption;
	optLabel2->FontSize() = 13;
	optLabel2->Width() = 140;
	optLabel2->Height() = 30;
	optLabel2->Text() = _tr_("西安千街通联科技");
	osg::ref_ptr<CExLabelNode> labelNode2 = new CExLabelNode(g_pRenderContext, optLabel2);
	labelNode2->SetBackgroundImage(FeFileReg->GetFullPath("texture/mark/label_title.png"));
	labelNode2->SetBackgroundColor(osg::Vec4d(1,1,1,1));
	labelNode2->SetBindingPointLLH(vecHG);
	labelNode2->SetClampToTerrain(true);
	pFreeMarkSys->AddMark(labelNode2);

	osg::ref_ptr<CExLabelItemNode> itemPos = new CExLabelItemNode(g_pRenderContext, new FeExtNode::CExLabelItemOption);
	itemPos->SetText(_tr_("公司简介"));
	itemPos->SetWidth(140);
	itemPos->SetBackgroundImage(FeFileReg->GetFullPath("texture/mark/label_item.png"));
	itemPos->SetBackgroundColor(osg::Vec4d(1,1,1,1));
	labelNode2->AddLabelItem(itemPos);
	//itemPos->AddChildItem(_tr_("成立时间: 2012年7月"));
	//itemPos->AddChildItem(_tr_("注册资金: 2000万"));
	//itemPos->AddChildItem(_tr_("公司地址: 锦业时代"));
	//itemPos->AddChildItem(_tr_("公司规模: 50-100人"));
	itemPos->SetChildItemVisible(false);

	FeUtil::FEID idItem = labelNode2->AddLabelItem(_tr_("产品分类"));
	//labelNode2->GetLabelItem(idItem)->AddChildItem(_tr_("FreeEarthSDK"));
	//labelNode2->GetLabelItem(idItem)->AddChildItem(_tr_("FreeMapSDK"));
	//labelNode2->GetLabelItem(idItem)->AddChildItem(_tr_("FreeEditor"));
	//labelNode2->GetLabelItem(idItem)->AddChildItem(_tr_("FreeViewer"));
	labelNode2->GetLabelItem(idItem)->SetChildItemVisible(false);

	/*idItem = labelNode2->AddLabelItem(_tr_("联系方式"));
	labelNode2->GetLabelItem(idItem)->AddChildItem(_tr_("电话: 029-88756164"));
	labelNode2->GetLabelItem(idItem)->AddChildItem(_tr_("传真: 029-88719459"));*/
	labelNode2->GetLabelItem(idItem)->SetChildItemVisible(false);

	labelNode2->ShowLabelItem(false);
	}

	/// 标牌绑定到场景节点
	{
		AirGen air1(pFreeMarkSys, g_pRenderContext);
		air1.m_strActiveTime = "2005年3月21日";
		air1.m_strAirBorne = "核载: 12人次";
		air1.m_strAirType = "军用 WZ-01";
		air1.m_strIDNum = "型号: FH2203";
		air1.m_strTotalMileage = "总里程: 284300 km";
		air1.GenAir(g_vecStart1, g_vecEnd1, osg::Vec4d(1.0, 0, 0, 0.8), 100);

		AirGen air2(pFreeMarkSys, g_pRenderContext);
		air2.m_strActiveTime = "2005年3月21日";
		air2.m_strAirBorne = "核载: 12人次";
		air2.m_strAirType = "军用 WZ-02";
		air2.m_strIDNum = "型号: FK1003";
		air2.m_strTotalMileage = "总里程: 84300 km";
		air2.GenAir(g_vecStart2, g_vecEnd2, osg::Vec4d(0, 1.0, 0, 0.8), 100);

		AirGen air3(pFreeMarkSys, g_pRenderContext);
		air3.m_strActiveTime = "2005年3月21日";
		air3.m_strAirBorne = "核载: 12人次";
		air3.m_strAirType = "军用 WZ-03";
		air3.m_strIDNum = "型号: FH2013";
		air3.m_strTotalMileage = "总里程: 194300 km";
		air3.GenAir(g_vecStart3, g_vecEnd3, osg::Vec4d(1.0, 1.0, 1.0, 0.8), 100);
	}


	/// 设置初始视角
	p3DScenePanel->GetSystemManager()->GetSystemService()->GetManipulatorManager()->SetHome(
		osgEarth::Viewpoint(107.46391, 29.18479, -7.7930632,-89.90515, 11569042), 0.5);
	p3DScenePanel->GetSystemManager()->GetSystemService()->GetManipulatorManager()->Home();


	/// 设置界面内容
	{
		CFreeLableWidget* pLabelWidget = new CFreeLableWidget();
		pLabelWidget->SetTitleText(QString::fromLocal8Bit("功能介绍"));
		pLabelWidget->SetContentText(QString::fromLocal8Bit(
			"  标牌采用图文结合的方式，方便用于展示各"
			"种类型的信息。其提供了以下特性：\n"
			"  1. 静态标牌，绑定到固定位置。\n"
			"  2. 动态标牌，绑定到场景节点，随节点移动。\n"
			"  3. 支持添加多级目录，及菜单式收缩操作。\n"
			"  4. 使用灵活，可单独配置每一个标牌项。\n"));
		pWidget->AddControlWidget(pLabelWidget);
	}
	{
		CFreeLableWidget* pLabelWidget = new CFreeLableWidget();
		pLabelWidget->SetTitleText(QString::fromLocal8Bit("操作说明"));
		pLabelWidget->SetContentText(QString::fromLocal8Bit(
			"  1. 通过标头（标牌第一个节点）拖动标牌。\n"
			"  2. 双击标头显示（隐藏）标牌节点。\n"
			"  3. 单击标牌项显示（隐藏）子级菜单。\n"));
		pWidget->AddControlWidget(pLabelWidget);
	}
	pWidget->AddControlItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));

	pWidget->ShowDialogNormal();
	pWidget->ShowDialogMaxisize();

	int nFlag = a.exec();

	return nFlag;
}


