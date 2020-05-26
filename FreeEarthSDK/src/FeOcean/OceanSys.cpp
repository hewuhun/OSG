#include <FeOcean/OceanSys.h>

#include <FeUtils/PathRegistry.h>

namespace FeOcean
{
	COceanSys::COceanSys(const std::string& strTritonPath)
		:CSceneSysCall()
		, m_rpTritonNode(NULL)
		, m_strTritonPath(strTritonPath)
	{
		SetSysKey(OCEAN_SYSTEM_CALL_DEFAULT_KEY);

		if (m_strTritonPath.empty())
		{
			m_strTritonPath = FeFileReg->GetFullPath("triton/data");
		}
	}

	COceanSys::~COceanSys()
	{

	}

	bool COceanSys::InitiliazeImplement()
	{
		m_rpTritonNode = new FeOcean::TritonNode(
			m_opRenderContext.get(), m_opRenderContext->GetMapSRS(), m_strTritonPath);

		m_opRenderContext->SetTritonNode(m_rpTritonNode);

		if (m_rpTritonNode.valid())
		{
			if (!m_opSystemCallRoot->containsNode(m_rpTritonNode.get()))
			{
				m_opSystemCallRoot->addChild(m_rpTritonNode.get());
				m_opRenderContext->SetTritonNode(m_rpTritonNode.get());
			}
		}

		/// Ocean Initialize
		if(m_opRenderContext->GetCamera())
		{
			osg::GraphicsContext* gc = m_opRenderContext->GetCamera()->getGraphicsContext();
			if(gc && gc->valid())
			{
				gc->makeCurrent();

				if(gc->getState())
				{
					m_rpTritonNode->Initialize(gc->getState()->getContextID());
				}

				gc->releaseContext();
			}
		}
	
		return true;
	}

	bool COceanSys::UnInitiliazeImplement()
	{
		if (m_rpTritonNode.valid())
		{
			m_rpTritonNode->Hide();

			if (m_opSystemCallRoot->containsNode(m_rpTritonNode.get()))
			{
				m_opSystemCallRoot->removeChild(m_rpTritonNode.get());
				m_opRenderContext->SetTritonNode(NULL);
			}

			m_rpTritonNode = NULL;
		}

		return true;
	}

	FeOcean::TritonNode * COceanSys::GetOceanNode()
	{
		return m_rpTritonNode.get();
	}

	void FeOcean::COceanSys::SetAtmosphere(osg::Node* pNode)
	{
		if (m_rpTritonNode.valid())
		{
			m_rpTritonNode->SetAtmosphere(pNode);
		}
	}

	void FeOcean::COceanSys::SetSunAndMoonNode(osg::Node* pSunNode, osg::Node* pMoonNode)
	{
		if (m_rpTritonNode.valid())
		{
			m_rpTritonNode->SetSun(pSunNode);
			m_rpTritonNode->SetMoon(pMoonNode);
		}
	}
}