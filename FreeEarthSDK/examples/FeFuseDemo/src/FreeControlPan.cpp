#include "FreeControlPan.h"
#include "UserMaskSource.h"

#include <FeUtils/StrUtil.h>
#include <FeUtils/FreeViewPoint.h>
#include <FeUtils/PathRegistry.h>
#include <FeUtils/CoordConverter.h>
#include <FeManager/FreeMarkSys.h>
#include <FeExtNode/ExLodModelNode.h>

osg::Vec3d g_VecUSAAirportPosition(-117.355167, 33.301255, -34);
osg::Vec3d g_VecRandomAirportPosition(-117.355167, 33.351255, 60);

CFreeControlPan::CFreeControlPan( FeShell::CSystemManager* pManager, QWidget* pParent )
	:QWidget(pParent)
	,m_opSystemManager(pManager)
	,m_rpRandomModel(NULL)
	,m_rpMaskLayer(NULL)
	,m_pTerrainMask(NULL)
{
	ui.setupUi(this);

	if (m_opSystemManager.valid())
	{
		CreateUsaAirport();

		CreateRandomAirport();
	}

	//ui.doubleSpinBox_Lon->setValue(g_VecRandomAirportPosition.x());
	//ui.doubleSpinBox_Lat->setValue(g_VecRandomAirportPosition.y());
	//ui.doubleSpinBox_Hei->setValue(g_VecRandomAirportPosition.z());

	InitWidget();

	InitContext();
}

CFreeControlPan::~CFreeControlPan()
{

}

void CFreeControlPan::InitWidget()
{
	ui.pushBtn_Current->setFixedHeight(30);
	ui.pushBtn_Usa->setFixedHeight(30);

	connect(ui.pushBtn_Usa, SIGNAL(clicked()), this, SLOT(SlotLocateUsa()));
	connect(ui.pushBtn_Current, SIGNAL(clicked()), this, SLOT(SlotLocateCurrent()));

	//connect(ui.doubleSpinBox_Lon, SIGNAL(valueChanged(double)), this, SLOT(SlotChangePosition(double)));
	//connect(ui.doubleSpinBox_Lat, SIGNAL(valueChanged(double)), this, SLOT(SlotChangePosition(double)));
	//connect(ui.doubleSpinBox_Hei, SIGNAL(valueChanged(double)), this, SLOT(SlotChangePosition(double)));
}

void CFreeControlPan::InitContext()
{
	
}

void CFreeControlPan::SlotLocateUsa()
{
	if (m_opSystemManager.valid())
	{
		m_opSystemManager->GetSystemService()->Locate(FeUtil::CFreeViewPoint(
			g_VecUSAAirportPosition.x(),
			g_VecUSAAirportPosition.y(),
			g_VecUSAAirportPosition.z()+5000,
			0.0,
			-90.0,
			g_VecUSAAirportPosition.z()+5000, 
			3));
	}
}

void CFreeControlPan::SlotLocateCurrent()
{
	m_opSystemManager->GetSystemService()->Locate(FeUtil::CFreeViewPoint(
		g_VecRandomAirportPosition.x(),
		g_VecRandomAirportPosition.y(),
		g_VecRandomAirportPosition.z()+5000,
		0.0,
		-90.0,
		g_VecRandomAirportPosition.z()+5000, 
		3));
}

void CFreeControlPan::SlotChangePosition( double )
{
	//osg::Vec3d pos(ui.doubleSpinBox_Lon->value(),ui.doubleSpinBox_Lat->value(),ui.doubleSpinBox_Hei->value());
	//if (m_rpRandomModel.valid())
	//{
	//	m_rpRandomModel->SetPositionByLLH(pos);

	//	m_opSystemManager->GetRenderContext()->GetMapNode()->getMap()->beginUpdate();
	//	if (m_rpMaskLayer.valid())
	//	{
	//		m_opSystemManager->GetRenderContext()->GetMapNode()->getMap()->removeTerrainMaskLayer(m_rpMaskLayer);
	//		m_rpMaskLayer = NULL;
	//	}

	//	osgEarth::Config config("mask");
	//	config.add("driver", "feature");
	//	config.add("name", "Radom AirPort");
	//	config.add("profile", "global-geodetic");

	//	osgEarth::MaskLayerOptions opt(config);
	//	opt.name() = config.value( "name" );

	//	CUserMaskSource* pUserMaskSource = new CUserMaskSource(opt);
	//	pUserMaskSource->SetBoundaryVertext(m_rpBoundarys);
	//	pUserMaskSource->SetPosition(pos);
	//	m_rpMaskLayer = new MaskLayer(opt, pUserMaskSource);
	//	m_opSystemManager->GetRenderContext()->GetMapNode()->getMap()->addTerrainMaskLayer(m_rpMaskLayer);
	//	m_opSystemManager->GetRenderContext()->GetMapNode()->getMap()->endUpdate();
	//	
	//}
}

void CFreeControlPan::CreateUsaAirport()
{
	FeManager::CFreeMarkSys* pFreeMarkSys = dynamic_cast<FeManager::CFreeMarkSys*>(
		m_opSystemManager->GetSystemService()->GetModuleSys(FeManager::FREE_MARK_SYSTEM_CALL_DEFAULT_KEY));

	if (pFreeMarkSys)
	{
		FeExtNode::CExLodModelGroupOption* optLodModel = new FeExtNode::CExLodModelGroupOption;
		optLodModel->name() = "USA Airport";
		optLodModel->minRange() = 0;
		optLodModel->maxRange() = 200000;
		optLodModel->PositionLLHDegree() = g_VecUSAAirportPosition;

		FeExtNode::CExLodModelGroup* pLodModelNode = new FeExtNode::CExLodModelGroup(optLodModel, m_opSystemManager->GetRenderContext());
		FeExtNode::CExLodNode* pLodNode = pLodModelNode->AddLodNode(
			FeFileReg->GetFullPath("model/airport.ive"), 
			FeExtNode::CExLodModelGroup::TRANS_ROTATE_SCALE_EFFECT);
		if (NULL == pLodNode)
		{
			return ;
		}
		pFreeMarkSys->AddMark(pLodModelNode);

		m_pTerrainMask = new FeUtil::CTerrainMask;
		m_pTerrainMask->Create(optLodModel->name(), m_opSystemManager->GetRenderContext(), FeFileReg->GetFullPath("examples-data/fuse.txt"));



	}
}

void CFreeControlPan::CreateRandomAirport()
{
	FeManager::CFreeMarkSys* pFreeMarkSys = dynamic_cast<FeManager::CFreeMarkSys*>(
		m_opSystemManager->GetSystemService()->GetModuleSys(FeManager::FREE_MARK_SYSTEM_CALL_DEFAULT_KEY));

	if (pFreeMarkSys)
	{
		FeExtNode::CExLodModelGroupOption* optLodModel = new FeExtNode::CExLodModelGroupOption;
		optLodModel->name() = "Random Airport";
		optLodModel->minRange() = 0;
		optLodModel->maxRange() = 200000;
		optLodModel->PositionLLHDegree() = g_VecRandomAirportPosition;

		m_rpRandomModel = new FeExtNode::CExLodModelGroup(optLodModel, m_opSystemManager->GetRenderContext());
		FeExtNode::CExLodNode* pLodNode = m_rpRandomModel->AddLodNode(
			FeFileReg->GetFullPath("model/airport.ive"), 
			FeExtNode::CExLodModelGroup::TRANS_ROTATE_SCALE_EFFECT);
		if (NULL == pLodNode)
		{
			return ;
		}
		pFreeMarkSys->AddMark(m_rpRandomModel);

		//if (false == m_rpBoundarys.valid())
		//{
		//	std::string strBoundarys = 
		//		"-1355.93469238 -1516.11804199 50.8614463806, -1379.51220703 -1481.60644531 50.8828315735,\
		//		-1421.64355469 -1419.93652344 50.9210472107, -1436.1854248 -1398.65075684 50.9342346191,\
		//		-1353.62805176 -1349.55761719 51.1821174622, -1114.43994141 -1207.32348633 51.9002952576,\
		//		-1114.28674316 -1207.2322998 51.9007568359, -745.273071289 -1009.97698975 52.9666175842,\
		//		-740.903442383 -997.516052246 52.9984664917, -830.846313477 -913.875366211 52.9888572693,\
		//		-884.425170898 -864.050720215 52.983127594, -966.047058105 -788.148010254 52.974407196,\
		//		-990.829956055 -773.105102539 52.9565544128, -1002.27258301 -729.430419922 53.0180740356,\
		//		-992.402832031 -719.767150879 53.0549163818, -974.253356934 -675.811401367 53.1723976135,\
		//		-963.643005371 -619.330444336 53.2995491028, -937.094116211 -550.216003418 53.4805488586,\
		//		-907.014160156 -509.136627197 53.6149139404, -860.804992676 -460.523132324 53.7938041687,\
		//		-839.866516113 -443.99621582 53.8644142151, -681.285400391 -281.717132568 54.4696769714,\
		//		-620.503723145 -219.518081665 54.7016639709, -547.652648926 -131.33416748 55.0056152344,\
		//		-39.6546440125 380.845550537 56.9299507141, 66.0545883179 504.678619385 57.3631591797,\
		//		76.8326339722 515.551574707 57.4039993286, 153.596099854 592.991210938 57.6948661804,\
		//		178.534454346 620.290893555 57.7934303284, 711.641845703 999.934692383 59.5130729675, \
		//		747.586181641 1018.36303711 59.6154022217, 804.463562012 1036.4362793 59.7562675476,\
		//		930.005249023 1062.33557129 60.0406150818, 996.665588379 1082.40490723 60.2035942078,\
		//		1063.75366211 1102.60314941 60.3676223755, 1117.16833496 1101.82958984 60.4662055969,\
		//		1286.18151855 1099.38220215 60.778137207, 1289.67651367 1080.81970215 60.7494277954, \
		//		1389.04211426 979.601257324 60.7433052063, 1435.42297363 954.97833252 60.783416748,\
		//		1520.92565918 945.365722656 60.9253158569, 1580.21911621 938.146484375 61.0226669312,\
		//		1609.38549805 910.873718262 61.0254974365, 1348.2109375 740.199157715 60.2121200562,\
		//		799.259094238 -143.772186279 57.5049171448, 836.590209961 -214.17376709 57.4411277771,\
		//		843.58404541 -281.717163086 57.3259391785, 851.827148438 -361.325408936 57.1901779175,\
		//		917.059326172 -489.537384033 57.0688476562, 930.660095215 -672.006103516 56.747756958,\
		//		963.950317383 -718.786682129 56.7212638855, 1112.07995605 -823.287841797 56.8002433777,\
		//		1129.13317871 -845.708435059 56.789604187, 1138.64770508 -880.622192383 56.7411117554,\
		//		1136.40917969 -911.172241211 56.678894043, 1091.00012207 -984.346679688 56.4548568726,\
		//		1067.7590332 -1009.29858398 56.3639297485, 1084.45507813 -1032.41809082 56.3512916565,\
		//		1012.6572876 -1109.0189209 56.0713157654, 991.085510254 -1100.56469727 56.0469665527,\
		//		949.76739502 -1151.5300293 55.8727760315, 888.775390625 -1183.60644531 55.6976051331,\
		//		862.807556152 -1203.96350098 55.6102981567, 754.577209473 -1319.31787109 55.1884765625,\
		//		639.750915527 -1390.45141602 54.8382873535, 605.825317383 -1441.26123047 54.6782341003,\
		//		508.26663208 -1442.63171387 54.492893219, 432.769317627 -1471.73010254 54.2962112427, \
		//		393.229705811 -1513.88549805 54.1420822144, 260.576934814 -1541.36047363 53.8414230347, \
		//		168.856323242 -1521.26245117 53.7077903748, 76.83253479 -1477.58288574 53.6183815002, \
		//		17.7155227661 -1449.52270508 53.5609436035, -25.8605461121 -1405.00024414 53.5638847351,\
		//		-114.586425781 -1397.67541504 53.4116020203, -142.859420776 -1413.56921387 53.3284568787,\
		//		-216.098953247 -1429.8581543 53.1603317261, -306.11416626 -1432.93920898 52.9858703613,\
		//		-353.44039917 -1404.59875488 52.9510498047, -397.818237305 -1428.00964355 52.8234596252,\
		//		-520.461181641 -1451.91894531 52.548324585, -593.211975098 -1474.03649902 52.3700447083,\
		//		-674.924682617 -1502.67700195 52.1625862122, -750.470092773 -1549.93151855 51.9313316345,\
		//		-803.859375 -1582.1661377 51.7701034546, -897.445983887 -1616.39526367 51.5297927856,\
		//		-1031.27416992 -1620.37634277 51.2715530396, -1228.97290039 -1452.15380859 51.2207489014";


		//	m_rpBoundarys = new osg::Vec3dArray;

		//	std::vector<std::string> strings;
		//	FeUtil::StringSplit(strBoundarys, ",", strings);
		//	for (int i = 0; i < strings.size(); i++)
		//	{
		//		double x, y, z;
		//		std::stringstream ss(strings.at(i));
		//		ss>>x>>y>>z;

		//		m_rpBoundarys->push_back(osg::Vec3d(x, y, z));
		//	}
		//}
		//

		//osgEarth::Config config("mask");
		//config.add("driver", "feature");
		//config.add("name", "Radom AirPort");
		//config.add("profile", "global-geodetic");

		//osgEarth::MaskLayerOptions opt(config);
		//opt.name() = config.value( "name" );

		//CUserMaskSource* pUserMaskSource = new CUserMaskSource(opt);
		//pUserMaskSource->SetBoundaryVertext(m_rpBoundarys);
		//pUserMaskSource->SetPosition(g_VecRandomAirportPosition);
		//m_rpMaskLayer = new MaskLayer(opt, pUserMaskSource);
		//m_opSystemManager->GetRenderContext()->GetMapNode()->getMap()->addTerrainMaskLayer(m_rpMaskLayer);
	}
}
