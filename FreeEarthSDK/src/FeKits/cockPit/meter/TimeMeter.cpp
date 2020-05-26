#include <FeKits/cockPit/meter/TimeMeter.h>

#include <osgDB/ReadFile>

#include <FeUtils/PathRegistry.h>

//class CTimeChronoGraphCallback;

CTimeMeter::CTimeMeter(void)
{
	RebuildTimeMeter();
}


CTimeMeter::~CTimeMeter(void)
{
}



osg::Group* CTimeMeter::RebuildTimeMeter()
{
	//时间表
	osg::Group *pNewChronograph = new osg::Group;
	std::string strNodePath = FeFileReg->GetFullPath("meter/chronograph/chronograph.ac");
	osg::Node * pChronograph = osgDB::readNodeFile(strNodePath);
	if (!pChronograph)
	{
		osg::notify(osg::INFO)<<"读取模型出错，模型正确路径：" + strNodePath<<std::endl;
		return NULL;
	}
	//分析重构时间显示表
	osg::Group *pChildLv0 = pChronograph->asGroup()->getChild(0)->asGroup();
	osg::Group *pFace = pChildLv0->getChild(0)->asGroup();
	osg::Group *pNeedle = pChildLv0->getChild(1)->asGroup();

	osg::Node *pElapsedMinutesHand = pNeedle->getChild(0);// what is this needle?
	osg::Node *pHourHand = pNeedle->getChild(1);
	osg::Node *pMinuteHand = pNeedle->getChild(2);
	osg::Node *pSecondHand = pNeedle->getChild(3);

	osg::MatrixTransform *pHourHandMT = new osg::MatrixTransform;
	osg::MatrixTransform *pMinuteHandMT = new osg::MatrixTransform;
	osg::MatrixTransform *pSecondHandMT = new osg::MatrixTransform;
 	pHourHandMT->addChild(pHourHand);
	pMinuteHandMT->addChild(pMinuteHand);
	pSecondHandMT->addChild(pSecondHand);
	
	m_vectorNeedle.push_back(pHourHandMT);
	m_vectorNeedle.push_back(pMinuteHandMT);
	m_vectorNeedle.push_back(pSecondHandMT);

	setUpdateCallback(new CTimeChronoGraphCallback(m_vectorNeedle));

	osg::Group *pBKG = pChildLv0->getChild(2)->asGroup();
	osg::Group *pMount = pChildLv0->getChild(3)->asGroup();
	osg::Group *pDisk = pChildLv0->getChild(4)->asGroup();
	osg::Group *pKnod = pChildLv0->getChild(5)->asGroup();
	osg::Group *pButton = pChildLv0->getChild(6)->asGroup();
	osg::Group *pHotspots = pChildLv0->getChild(7)->asGroup();
	osg::Node *pChronometer_button = pHotspots->getChild(0);
	osg::Node *pTime_offset_right = pHotspots->getChild(1);
	osg::Node *pTime_offset_left = pHotspots->getChild(2);

	pNewChronograph->addChild(pFace);
	pNewChronograph->addChild(pBKG);
	pNewChronograph->addChild(pMount);
	pNewChronograph->addChild(pDisk);
	pNewChronograph->addChild(pKnod);
	pNewChronograph->addChild(pButton);
	pNewChronograph->addChild(pHourHandMT);
	pNewChronograph->addChild(pMinuteHandMT);
	pNewChronograph->addChild(pSecondHandMT);

	addChild(pNewChronograph);
	return NULL;
}
