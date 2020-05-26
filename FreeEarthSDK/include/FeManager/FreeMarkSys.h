/**************************************************************************************************
* @file FreeMarkSys.h
* @note 场景标记的管理类
* @author c00005
* @data 2016-7-21
**************************************************************************************************/

#ifndef FE_FREE_MARK_SYSTEM_H
#define FE_FREE_MARK_SYSTEM_H

#include <FeManager/Export.h>

#include <FeExtNode/ExternNode.h>
#include <FeExtNode/ExComposeNode.h>
#include <FeExtNode/SceneMarkHandler.h>

#include <FeUtils/SceneSysCall.h>
#include <FeUtils/RenderContext.h>
#include <FeExtNode/SceneMarkHandler.h>
#include <FeExtNode/FeEventListenMgr.h>
#include <FeExtNode/ExLabelCullEventHandler.h>
#include <FeExtNode/ExLabelBindingNodeCallback.h>

namespace FeManager
{
	using namespace FeUtil;

	const std::string FREE_MARK_SYSTEM_CALL_DEFAULT_KEY("FreeMarkSystem");

	/**
	  * @class CFreeMarkSys
	  * @brief 场景标记系统
	  * @note 提供了管理场景标记的类，其中统一管理各种场景标记，并提供了添加、删除、修改等接口
	  * @author c00005
	*/
    class FEMANAGER_EXPORT CFreeMarkSys : public FeUtil::CSceneSysCall
    {
    public:
		/**  
		  * @note 场景标记系统的构造函数
		*/
        CFreeMarkSys();
		
		/**  
		  * @note 场景标记系统的析构函数
		*/
		virtual ~CFreeMarkSys();

	public:
		/**  
		  * @note 添加场景标记  
		  * @param pMark [in] 场景标记节点
		  * @param bAddToTemp [in] 作为临时节点，不添加到场景树
		  * @return bool 添加是否成功
		*/
		bool AddMark(FeExtNode::CExternNode* pMark, bool bAddToTemp=false);

		/**  
		  * @note 添加场景标记  
		  * @param pParent [in] 添加到的场景标记节点
		  * @param pMark [in] 场景标记节点
		  * @return bool 添加是否成功
		*/
		bool AddMark(FeExtNode::CExComposeNode* pParent, FeExtNode::CExternNode* pMark);

		/**  
		  * @note 移除场景标记  
		  * @param pMark [in] 场景标记节点
		  * @return bool 移除是否成功
		*/
		bool RemoveMark(FeExtNode::CExternNode* pMark);

		/**  
		  * @note 移除场景标记  
		  * @param pParent [in] 从pParent中移除场景标记节点
		  * @param pMark [in] 场景标记节点
		  * @return bool 移除是否成功
		*/
		bool RemoveMark(FeExtNode::CExComposeNode* pParent, FeExtNode::CExternNode* pMark);

		/**  
		  * @note 清空场景中的标记节点
		*/
		bool ClearMark();

		/**  
		  * @note 通过ID获取场景标记节点的指针
		  * @param nId [in] 场景节点的ID值
		  * @return CExternNode* 如果获取到，返回的标记节点的指针，否则返回空指针
		*/
		FeExtNode::CExternNode* GetMark(const FEID& nId);

		/**  
		  * @note 获取场景标记的根节点  
		*/
		FeExtNode::CExComposeNode* GetRootMark();


	public: // 标牌对外的操作使用统一接口 AddMark , RemoveMark
			 // 绑定节点功能由 CExLabelNode 提供
		/**  
		  * @note 为标牌添加处理器  
		  * @param pLabelNode [in] 标牌节点
		  * @return 是否添加成功
		*/
		bool AddLabelHandler(FeExtNode::CExLabelNode* pLabelNode);

		/**  
		  * @note 移除标牌处理器  
		  * @param pLabelNode [in] 标牌节点
		  * @return 是否移除成功
		*/
		bool RemoveLabelHandler(FeExtNode::CExLabelNode* pLabelNode);

		
	public:   
		/**  
		  * @note 开始场景管理系统
		*/
		virtual bool Start();
          
		/**  
		  * @note 停止场景管理系统
		*/
		virtual bool Stop();
		  
		/**  
		  * @note 获取场景渲染上下文，其中保存了MapNode和View等常用变量
		*/
		CRenderContext* GetRenderContext();

		/**
		*@note: 设置场景标记配置文件默认路径
		*/
		void SetDefaultMarkFilePath(const std::string& strPath);
		
		/**
		*@note: 保存程序场景标记到默认或者是指定的文件中
		*/
		bool Save();
		bool Save(const std::string& strFilePath);

		/**
		*@note: 从默认或者指定的场景标记配置文件中恢复场景节点
		*/
		bool Load();
		bool Load(const std::string& strFilePath);
		

		/**
		  * @note 开始编辑标记节点
		*/
		bool StartEdit(FeExtNode::CExternNode* pMark);

		/**
		  * @note 结束编辑标记节点
		*/
		bool StopEdit(FeExtNode::CExternNode* pMark);

		/**
		  * @note 获取事件监听管理器
		*/
		FeExtNode::CFeEventListenMgr* GetEventListenManager();

    protected:
		/**  
		  * @note 初始化场景管理系统  
		  * @return bool 初始化场景管理系统是否成功
		*/
		virtual bool InitiliazeImplement();
		
		/**  
		  * @note 反初始化场景管理系统  
		  * @return bool 反初始化场景管理系统是否成功
		*/
        virtual bool UnInitiliazeImplement();

	protected:
		///场景节点的根节点，用于保存所有的场景节点
		osg::ref_ptr<FeExtNode::CExComposeNode>					m_rpMarkRoot;

		//保存临时节点
		osg::ref_ptr<FeExtNode::CExComposeNode>					m_rpMarkRootTemp;

		osg::ref_ptr<FeExtNode::CSceneMarkHandler>				m_rpSceneMarkHandler;

		/// 标牌节点拣选事件回调
		osg::ref_ptr<FeExtNode::CExLabelCullEventHandler>		m_rpLabelCullEventHandler;
		
		/// 标牌事件处理
		osg::ref_ptr<FeExtNode::CExLabelMouseEventHandler>		m_rpLabelEventHandler;

		/// 事件监听管理
		osg::ref_ptr<FeExtNode::CFeEventListenMgr>				m_rpEventListenMgr;

		/// 默认的文件路径
		std::string												m_strFilePath;  
	};
}

#endif//FE_FREE_MARK_SYSTEM_H
