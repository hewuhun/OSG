#include <FeSilverlining/SilverliningSys.h>
#include <FeUtils/PathRegistry.h>

namespace FeSilverLining
{
	CSilverLiningSys::CSilverLiningSys( const std::string& strTritonPath)
		:FeUtil::CSceneSysCall()
		,m_rpSilverLiningNode(NULL)
		,m_strSilverLiningPath(strTritonPath)
		,m_rpLight(NULL)
		,m_rpCamera(NULL)
	{
		SetSysKey(SILVERLINING_CALL_DEFAULT_KEY);

		if (m_strSilverLiningPath.empty())
		{
			m_strSilverLiningPath = FeFileReg->GetFullPath("silverLining/data");
		}
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

		//传递光照 h00021 2016-10-17
		m_rpSilverLiningNode->setSunLight(m_rpLight.get());

		//传递相机 h00021 2016-10-20
		//m_rpSilverLiningNode->setCamera(m_rpCamera.get());

		//h00017
		if(m_opRenderContext.valid())
		    m_rpSilverLiningNode->setCamera(m_opRenderContext->GetCamera());

		m_rpSilverLiningNode->Init();

		if(m_rpSilverLiningNode.valid())
		{
			if(!m_opSystemCallRoot->containsNode(m_rpSilverLiningNode.get()))
			{
				//m_rpSilverLiningNode->attach(m_opRenderContext->GetView(), 0);
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

	void CSilverLiningSys::SetLight( osg::Light* pLight )
	{
		if (!pLight)
		{
			return;
		}

		if (m_rpLight.valid())
		{
			m_rpLight = NULL;
		}

		m_rpLight = pLight;
	}

	void CSilverLiningSys::SetCamera( osg::Camera* pCamera )
	{
		if (!pCamera)
		{
			return;
		}

		if (m_rpCamera.valid())
		{
			m_rpCamera = NULL;
		}

		m_rpCamera = pCamera;
	}

	FeSilverLining::FeSilverLiningNode * CSilverLiningSys::GetSilverLiningNode()
	{
		return m_rpSilverLiningNode.get();
	}

}

