#include <FeUtils/PathRegistry.h>
#include <FeUtils/StrUtil.h>
#include <FeShell/SceneNodeSerialize.h>
#include <FeShell/FreeMarkSys.h>

using namespace FeUtil::TinyUtil;

namespace FeShell
{
	CFreeMarkSys::CFreeMarkSys(const std::string& strFilePath)
		:FeUtil::CSceneSysCall()
		,m_opMarkRoot(NULL)
		,m_strFilePath(strFilePath)
	{
		SetSysKey(FREE_MARK_SYSTEM_CALL_DEFAULT_KEY);
	}


	CFreeMarkSys::~CFreeMarkSys()
	{

	}

	bool CFreeMarkSys::InitiliazeImplement()
	{
		m_opMarkRoot = new FeExNode::CExComposeNode(m_opRenderContext.get());
		m_opSystemCallRoot->addChild(m_opMarkRoot.get());

		m_opTempGroup = new osg::Group();
		m_opSystemCallRoot->addChild(m_opTempGroup.get());

		m_rfpSceneMarkEditor = new CSceneMarkHandle();

		return Load();
	}

	bool CFreeMarkSys::UnInitiliazeImplement()
	{
		bool bSucessful = true;
		return bSucessful;
	}

	bool CFreeMarkSys::Save()
	{
		return Save(m_strFilePath);
	}

	bool CFreeMarkSys::Save( const std::string& strFilePath )
	{
		CMarkSerialization markSerialization;
		return markSerialization.Execute(m_opMarkRoot.get(), strFilePath);
	}

	bool CFreeMarkSys::Load( const std::string& strFilePath )
	{
		CMarkDeserialization deserialization(m_opRenderContext.get());
		return deserialization.Execute(m_opMarkRoot.get(), m_strFilePath);
	}

	bool CFreeMarkSys::Load()
	{
		return Load(m_strFilePath);
	}

	FeExNode::CExComposeNode* CFreeMarkSys::GetRootMark()
	{
		return m_opMarkRoot.get();
	}

	bool CFreeMarkSys::AddMark( FeExNode::CExternNode* pMark )
	{
		return AddMark(m_opMarkRoot.get(), pMark);
	}

	bool CFreeMarkSys::AddMark( FeExNode::CExComposeNode* pParent, FeExNode::CExternNode* pMark )
	{
		if(pParent && pMark)
		{
			return pParent->AddNode(pMark);
		}

		return false;
	}

	bool CFreeMarkSys::RemoveMark( FeExNode::CExternNode* pMark )
	{
		return RemoveMark(m_opMarkRoot.get(), pMark);
	}

	bool CFreeMarkSys::RemoveMark( FeExNode::CExComposeNode* pParent, FeExNode::CExternNode* pMark )
	{
		if(pParent && pMark)
		{
			return pParent->RemoveNode(pMark);
		}

		return false;
	}

	bool CFreeMarkSys::AddTempMark( FeExNode::CExternNode* pMark )
	{
		return m_opTempGroup->addChild(pMark);
	}

	bool CFreeMarkSys::RemoveTempMark( FeExNode::CExternNode* pMark )
	{
		return m_opTempGroup->removeChild(pMark);
	}

	void CFreeMarkSys::SetActiveEventInform(FeExNode::CExternNode* pEventInform)
	{
		if(m_opRenderContext.valid() && m_rfpSceneMarkEditor.valid())
		{
			m_rfpSceneMarkEditor->SetActiveEventInform(pEventInform);
			m_opRenderContext->AddEventHandler(m_rfpSceneMarkEditor.get());
			pEventInform->StartEdit();
		}
	}

	void CFreeMarkSys::RemoveActiveEventInform(FeExNode::CExternNode* pEventInform)
	{
		if(m_opRenderContext.valid() && m_rfpSceneMarkEditor.valid() && pEventInform)
		{
			pEventInform->StopEdit();
			m_opRenderContext->RemoveEventHandler(m_rfpSceneMarkEditor.get());
			m_rfpSceneMarkEditor->RemoveActiveEventInform();
		}
	}

	bool CFreeMarkSys::SetNodeActive(FeExNode::CExternNode* pExNode)
	{
		if(pExNode)
		{
			//pExNode->SetActive(!pExNode->GetActived());
		}
		return false;
	}

	FeExNode::CExternNode* CFreeMarkSys::GetActivedNode()
	{
		return NULL;
	}
}

