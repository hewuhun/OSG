/**************************************************************************************************
* @file ViewPluginInterface.h
* @note 显示视图下的挂件等工具，包括导航器、鸟瞰图、比例尺、红外、网格等特效
* @author l00008
* @data 2015-4-27
**************************************************************************************************/
#ifndef VIEW_PLUGIN_INTERFACE_H
#define VIEW_PLUGIN_INTERFACE_H 1

#include <FeKits/KitsCtrl.h>
#include <FeKits/screenInfo/SceneViewInfo.h>

#include <mainWindow/UIObserver.h>

namespace FreeViewer
{
	/**
	* @class CKitsViewUnit
	* @note View菜单的管理节点，其中定义了不同的菜单执行不同的操作
	* @author l00008
	*/
	class CKitViewUnit : public QObject
	{
		Q_OBJECT

	public:
		/**  
		  * @note 构造函数 
		  * @param pKitsCtrl [in] 挂件的指针
		  * @param parent [in] 父窗口
		*/
		CKitViewUnit(FeKit::CKitsCtrl* pKitsCtrl, QObject* parent);
		
		/**  
		  * @note 析构函数 
		*/
		~CKitViewUnit();

	public slots:
		/**  
		  * @note 槽函数，用于响应挂件的显示或隐藏 
		  * @param bState [in] 当前按钮的状态，决定挂件的显隐
		*/
		void SlotDoAction(bool bState);

	protected:
		///挂件
		FeKit::CKitsCtrl*    m_pKitsCtrl;
	};
}

namespace FreeViewer
{
	/**
	* @class CViewPluginInterface
	* @note 管理视图挂件窗口，用于初始化并连接挂件的按钮以及信号槽
	* @author l00008
	*/
	class CViewPluginInterface : public CUIObserver
	{
		Q_OBJECT

	public:
		/**  
		  * @note 构造函数 
		*/
		CViewPluginInterface( CFreeMainWindow* pMainWindow);

		/**  
		  * @note 析构函数 
		*/
		~CViewPluginInterface();

	protected:
		/**
		*note：初始化窗口
		*/
		void InitWidget();

		/**
		*note：初始化环境，主要构件界面以及注册一些功能
		*/
		void BuildContext();

		/**  
		  * @note 创建挂件的菜单栏和工具栏的按钮，并连接信号和槽，只响应显示或者隐藏的操作 
		  * @param pKitsCtrl [in] 挂件对象
		  * @param strNormalIcon [in] 按钮正常状态的图标
		  * @param strHoverIcon [in] 按钮划过状态的图标
		  * @param strPressIcon [in] 按钮按下状态的图标
		  * @param strDisableIcon [in] 按钮失效状态的图标
		*/
		void BuildKitAction(
			FeKit::CKitsCtrl* pKitsCtrl,
			const QString& strNormalIcon,
			const QString& strHoverIcon,
			const QString& strPressIcon,
			const QString& strDisableIcon);

	protected:
		osg::ref_ptr<FeKit::CSceneViewInfo> m_pSceneViewInfo;
	};
}
#endif
