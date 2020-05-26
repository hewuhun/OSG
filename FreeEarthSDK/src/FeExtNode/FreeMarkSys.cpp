#include <FeUtils/logger/LoggerDef.h>
#include <FeExtNode/FreeMarkSys.h>

#include <FeExtNode/ExLabelCullEventHandler.h>
#include <FeExtNode/ExLabelNode.h>
#include <FeExtNode/SceneNodeSerialize.h>

namespace FeExtNode
{
	FeExtNode::CFreeMarkSys::CFreeMarkSys()
		:FeUtil::CSceneSysCall()
		,m_rpMarkRoot(NULL)
	{
		SetSysKey(FREE_MARK_SYSTEM_CALL_DEFAULT_KEY);
	}

	CFreeMarkSys::~CFreeMarkSys()
	{

	}

	bool CFreeMarkSys::InitiliazeImplement()
	{
		//新建用于保存所有Mark的根节点，并加入到场景中
		CExComposeNodeOption* opt = new CExComposeNodeOption();
		opt->name() = "MarkRoot";
		opt->visible() = true;
		
		m_rpMarkRoot = new CExComposeNode(opt);
		if (m_opSystemCallRoot.valid())
		{
			m_opSystemCallRoot->addChild(m_rpMarkRoot);
		}

		m_rpEventListenMgr = new CFeEventListenMgr();

		return true;
	}

	bool CFreeMarkSys::UnInitiliazeImplement()
	{
		return true;
	}

	bool CFreeMarkSys::AddMark( CExternNode* pMark )
	{
		return AddMark(m_rpMarkRoot.get(), pMark);
	}

	bool CFreeMarkSys::AddMark( CExComposeNode* pParent, CExternNode* pMark )
	{
		if(pParent && pMark)
		{
			//return pParent->InsertNode(pMark);
			CExLabelNode* labelNode = dynamic_cast<CExLabelNode*>(pMark);
			bool bReturn = pParent->InsertNode(pMark);
			if(bReturn && labelNode)
			{
				bReturn = AddLabelHandler(labelNode);
			}
			return bReturn;
		}

		return false;
	}

	bool CFreeMarkSys::RemoveMark( CExternNode* pMark )
	{
		return RemoveMark(m_rpMarkRoot.get(), pMark);
	}

	bool CFreeMarkSys::RemoveMark( CExComposeNode* pParent, CExternNode* pMark )
	{
		if(pParent && pMark)
		{
			//return pParent->RemoveNode(pMark);
			osg::ref_ptr<CExLabelNode> labelNode = dynamic_cast<CExLabelNode*>(pMark); 
			bool bReturn = pParent->RemoveNode(pMark);
			if(bReturn && labelNode.valid())
			{
				bReturn = RemoveLabelHandler(labelNode);
			}
			return bReturn;
		}

		return false;
	}

	bool CFreeMarkSys::ClearMark()
	{
		if (m_rpMarkRoot.valid())
		{
			m_rpMarkRoot->Clear();

			return true;
		}

		return false;
	}

	CExternNode* CFreeMarkSys::GetMark( const FEID& nId )
	{
		if (m_rpMarkRoot.valid())
		{
			if(m_rpMarkRoot.get()->GetID() == nId) return m_rpMarkRoot.get();

			return m_rpMarkRoot->GetNode(nId);
		}

		return NULL;
	}

	CExComposeNode* CFreeMarkSys::GetRootMark()
	{
		return m_rpMarkRoot.get();
	}

	bool CFreeMarkSys::Start()
	{
		if(!m_rpSceneMarkHandler.valid())
		{
			m_rpSceneMarkHandler = new CSceneMarkHandler(m_rpMarkRoot, GetRenderContext());
			m_rpSceneMarkHandler->SetupEventListenMgr(m_rpEventListenMgr);
		}

		if (m_opRenderContext.valid())
		{
			m_opRenderContext->RemoveEventHandler(m_rpSceneMarkHandler); // 防止重复添加
			m_opRenderContext->AddEventHandler(m_rpSceneMarkHandler);
		}

		if (m_rpMarkRoot.valid())
		{
			m_rpMarkRoot->SetAcceptEvent(true);
		}

		return true;
	}

	bool CFreeMarkSys::Stop()
	{
		if (m_opRenderContext.valid())
		{
			m_opRenderContext->RemoveEventHandler(m_rpSceneMarkHandler);
			//m_rpSceneMarkHandler = NULL; // 不释放，否则注册的相关事件都将丢失
		}

		if (m_rpMarkRoot.valid())
		{
			m_rpMarkRoot->SetAcceptEvent(false);
		}

		return true;
	}

	CRenderContext* CFreeMarkSys::GetRenderContext()
	{
		return m_opRenderContext.get();
	}

	bool CFreeMarkSys::AddLabelHandler( CExLabelNode* pLabelNode)
	{
		if(!pLabelNode)
		{
			return false;
		}

		// 标牌裁剪处理事件回调类
		if(!m_rpLabelCullEventHandler.valid())
		{
			m_rpLabelCullEventHandler = new CExLabelCullEventHandler(m_opRenderContext.get());
			m_opRenderContext->AddEventHandler(m_rpLabelCullEventHandler);
		}

		/// 标牌事件处理器
		if (!m_rpLabelEventHandler.valid())
		{
			m_rpLabelEventHandler = new CExLabelMouseEventHandler();
			m_opRenderContext->AddEventHandler(m_rpLabelEventHandler);
		}

		if(m_rpLabelCullEventHandler.valid())
		{
			osg::notify(osg::INFO) << "add label, now has label: " 
				<< m_rpLabelCullEventHandler->GetValidLabelNum()+1 << std::endl;

			/// 标牌节点添加到裁剪处理器
			return m_rpLabelCullEventHandler->AddLabel(pLabelNode);
		}
		
		return false;
	}

	bool CFreeMarkSys::RemoveLabelHandler( CExLabelNode* pLabelNode )
	{
		if(!pLabelNode)
		{
			return false;
		}

		if(m_rpLabelCullEventHandler.valid())
		{
			m_rpLabelCullEventHandler->RemoveLabel(pLabelNode);
			
			osg::notify(osg::INFO) << "remove label, still has label: " 
				<< m_rpLabelCullEventHandler->GetValidLabelNum() << std::endl;

			// 已经没有标牌，移除标牌相关处理器
			if(m_rpLabelCullEventHandler->GetValidLabelNum() == 0)
			{	
				if(m_opRenderContext.valid())
				{
					m_opRenderContext->RemoveEventHandler(m_rpLabelCullEventHandler);
					m_rpLabelCullEventHandler = 0;
				}

				if (m_rpLabelEventHandler.valid())
				{
					m_opRenderContext->RemoveEventHandler(m_rpLabelEventHandler);
					m_rpLabelEventHandler = 0;
				}
			}
		}
		
		return true;
	}

	bool CFreeMarkSys::Save()
	{
		return Save(m_strFilePath);
	}

	bool CFreeMarkSys::Save( const std::string& strFilePath )
	{
		CMarkSerialization markSerialization;
		return markSerialization.Execute(m_rpMarkRoot.get(), strFilePath);
	}

	bool CFreeMarkSys::Load()
	{
		return Load(m_strFilePath);
	}

	bool CFreeMarkSys::Load( const std::string& strFilePath )
	{
		CMarkDeserialization deserialization(m_opRenderContext.get(), this);
		return deserialization.Execute(m_rpMarkRoot.get(), strFilePath);		
	}

	void CFreeMarkSys::SetDefaultMarkFilePath( const std::string& strPath )
	{
		m_strFilePath = strPath;
	}

	bool CFreeMarkSys::StartEdit( CExternNode* pMark )
	{
		if(pMark)
		{
			FeNodeEditor::CFeNodeEditTool* editorNode = pMark->GetOrCreateEditorTool();

			if(editorNode && !pMark->IsEditting() && m_opSystemCallRoot.valid())
			{
				editorNode->SetupEventListenMgr(m_rpEventListenMgr);
				pMark->StartEdit();
				return m_opSystemCallRoot->addChild(editorNode);
			}
		}
		return false;
	}

	bool CFreeMarkSys::StopEdit( CExternNode* pMark )
	{
		if(pMark)
		{
			FeNodeEditor::CFeNodeEditTool* editorNode = pMark->GetOrCreateEditorTool();

			if(editorNode && pMark->IsEditting() && m_opSystemCallRoot.valid())
			{
				pMark->StopEdit();
				return m_opSystemCallRoot->removeChild(editorNode);
			}
		}
		return false;
	}

	CFeEventListenMgr* CFreeMarkSys::GetEventListenManager()
	{
		return m_rpEventListenMgr.valid()?m_rpEventListenMgr.get():NULL;
	}

}

