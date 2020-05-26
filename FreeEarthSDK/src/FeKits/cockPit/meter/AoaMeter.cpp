#include <FeKits/cockPit/meter/AoaMeter.h>

#include <FeUtils/PathRegistry.h>

CAoaMeter::CAoaMeter(void) : osg::MatrixTransform() ,
m_pAoatape(NULL),
m_pAoa_off_flag(NULL),
m_dPitch(0.0),
m_dMaxPitchRange(30.0),
m_dMinPitchRange(-30.0)
{
	RebuildAoaMeter();
}


CAoaMeter::~CAoaMeter(void)
{
}

osg::Group* CAoaMeter::RebuildAoaMeter()
{
	osg::Group* pNewAoaMeter = new osg::Group;
	std::string strNodePath = FeFileReg->GetFullPath("meter/aoa-gauge/aoa-gauge.ac");
	osg::Node* pAoaMeter = osgDB::readNodeFile(strNodePath);
	if (!pAoaMeter)
	{
		osg::notify(osg::INFO)<<" 读取模型出错，模型正确路径：" + strNodePath<<std::endl;
		return NULL;
	}
	osg::Node* pChildLevelOne = pAoaMeter->asGroup()->getChild(0);
	osg::Group* pAoa_mask = pChildLevelOne->asGroup()->getChild(0)->asGroup();
	osg::Group* pAoa_mount = pChildLevelOne->asGroup()->getChild(1)->asGroup();
	osg::Group* pAoa_off_flag = pChildLevelOne->asGroup()->getChild(2)->asGroup();
	osg::Group* pAoa_tape = pChildLevelOne->asGroup()->getChild(3)->asGroup();

	pNewAoaMeter->addChild(pAoa_mask);
	pNewAoaMeter->addChild(pAoa_mount);
	pNewAoaMeter->addChild(pAoa_off_flag);
#ifndef _SHOWHIDEFLAG
#define _SHOWHIDEFLAG
#define HIDE 0x00
#define SHOW ~0x00
#endif
	pAoa_off_flag->setNodeMask(HIDE);
	pNewAoaMeter->addChild(pAoa_tape);

	m_pAoatape = pAoa_tape->getChild(0)->asGeode()->getDrawable(0)->asGeometry();
	m_pAoa_off_flag = pAoa_off_flag;
	
	addChild(pNewAoaMeter);

	return NULL;
}

void CAoaMeter::UpdatePitch( double dPitch )
{
	if (std::abs(dPitch - m_dPitch) < 0.000001)
	{
		return;
	}
	if (dPitch > m_dMaxPitchRange || dPitch < m_dMinPitchRange)
	{
		if (dPitch > (m_dMinPitchRange+m_dMaxPitchRange)/2.0)
		{
			dPitch = m_dMaxPitchRange;
		}
		else
		{
			dPitch = m_dMinPitchRange;
		}
	}
	osg::Vec2Array *pVecArray = static_cast<osg::Vec2Array*>(m_pAoatape->getTexCoordArray(0));
	int size = pVecArray->size();
	for (int i=0; i<size; i++)
	{
		double y = pVecArray->at(i).y();
#ifndef TEXTRANSFROM
#define TEXTRANSFROM 0.0235
#endif
		y += TEXTRANSFROM * (dPitch - m_dPitch);
		pVecArray->at(i).y() = y;
	}
	m_pAoatape->setTexCoordArray(0, pVecArray);
}

void CAoaMeter::AoaMeterOFF( bool bflag )
{
#ifndef _SHOWHIDEFLAG
#define _SHOWHIDEFLAG
#define HIDE 0x00
#define SHOW ~0x00
#endif
	unsigned int unFlag = bflag ? SHOW : HIDE;
	if (m_pAoa_off_flag->getNodeMask() == unFlag)
	{
		return;
	}
	m_pAoa_off_flag->setNodeMask(unFlag);
}
