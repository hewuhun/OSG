#include <FeShell/SilverLiningSys.h>

#include <FeShell/SystemService.h>
#include <FeUtils/PathRegistry.h>

namespace FeShell
{
	CSilverLiningSys::CSilverLiningSys(  )
		:FeUtil::CSceneSysCall()
		,m_rpSilverLiningNode(NULL)
		,m_strSilverLiningPath("")
	{
		SetSysKey(SILVERLINING_CALL_DEFAULT_KEY);
		m_strSilverLiningPath = FeFileReg->GetFullPath("silverLining/data");
	}


	CSilverLiningSys::CSilverLiningSys( const std::string& strTritonPath )
		:FeUtil::CSceneSysCall()
		,m_rpSilverLiningNode(NULL)
		,m_strSilverLiningPath(strTritonPath)
	{
		SetSysKey(SILVERLINING_CALL_DEFAULT_KEY);
	}

	CSilverLiningSys::~CSilverLiningSys()
	{

	}

	bool CSilverLiningSys::InitiliazeImplement()
	{
		osgEarth::Map *pMap = NULL;

		if(m_opRenderContext.valid() && m_opRenderContext->GetMapNode())
		{
			pMap = m_opRenderContext->GetMapNode()->getMap();
		}

		FeSilverLining::FeSilverLiningOptions slOptions ;
		slOptions.drawClouds() = true;
		//slOptions.cloudsMaxAltitude() = 500000;
		slOptions.resourcePath() = m_strSilverLiningPath;
		m_rpSilverLiningNode = new FeSilverLining::FeSilverLiningNode(
			pMap, slOptions);

		if(m_rpSilverLiningNode.valid())
		{
			if(!m_opSystemCallRoot->containsNode(m_rpSilverLiningNode.get()))
			{
				m_rpSilverLiningNode->attach(m_opRenderContext->GetView(), 0);
				m_opSystemCallRoot->addChild(m_rpSilverLiningNode.get());
				//m_opRenderContext->SetTritonNode(m_rpTritonNode.get());
			}
		}

		return true;
	}

	bool CSilverLiningSys::UnInitiliazeImplement()
	{
		if(m_rpSilverLiningNode.valid())
		{
			//m_rpSilverLiningNode->Hide();

			if(m_opSystemCallRoot->containsNode(m_rpSilverLiningNode.get()))
			{
				m_opSystemCallRoot->removeChild(m_rpSilverLiningNode.get());
				//m_opRenderContext->SetTritonNode(NULL);
			}

			m_rpSilverLiningNode = NULL;
		}

		return true;
	}

	FeSilverLining::FeSilverLiningNode * CSilverLiningSys::GetSilverLiningNode()
	{
		return m_rpSilverLiningNode.get();
	}

}

