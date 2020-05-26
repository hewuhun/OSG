/**************************************************************************************************
* @file ViewPointLocalTree.h
* @note 视点图层
* @author w00040
* @data 2017-3-2
**************************************************************************************************/
#ifndef VIEW_POART_LOCAL_TREE
#define VIEW_POART_LOCAL_TREE

#include <QTreeWidgetItem>

#include <mainWindow/UIObserver.h>
#include <mainWindow/Free3DDockTreeWidget.h>

namespace FreeViewer
{
	/**
	  * @class CViewPointTree
	  * @brief 视点图层树
	  * @note 实现视点搜索定位功能
	  * @author w00040
	*/
	class CViewPointTree : public CUIObserver
	{
		Q_OBJECT

	public:
		/**  
		  * @brief 构造函数
		*/
		CViewPointTree(CFreeMainWindow* pMainWindow, std::string strDataPath);

		/**  
		  * @brief 析构函数
		*/
		~CViewPointTree(void);

	public:
		void LoadData();

		void SaveData();
		
	protected:
		/**  
		  * @brief 初始化界面
		*/
		void InitWidget();

		/**  
		  * @brief 初始化环境，主要构件界面以及注册一些功能
		*/
		void BuildContext();

		/**  
		  * @brief 清空图层文件夹中的内容
		*/
		void ClearItem(QTreeWidgetItem* pItem);

		/**  
		  * @brief 清除内容
		*/
		void DeleteItem(QTreeWidgetItem* pItem);

	public slots:
		/**  
		  * @brief 树节点右键菜单槽函数
		*/
		void SlotContextMenuItem(QTreeWidgetItem* pItem, QPoint pos);

		/**  
		  * @brief 树节点双击槽函数
		*/
		void SlotItemDBClicked(QTreeWidgetItem*);

	protected slots:
		/**  
		  * @brief 删除图层文件夹
		*/
		void SlotClearLayerFolder();

		/**  
		  * @brief 添加视点图层子节点按钮消息响应
		*/
		void SlotAddViewPoint();

		/**  
		  * @brief 添加视点图层子节点
		*/
		void SlotAddViewPoint(FeUtil::CFreeViewPoint);

		/**  
		  * @brief 删除视点图层子节点
		*/
		void SlotDeleteViewPoint();

	private:
		///视点图层悬浮窗
		CFree3DDockTreeWidget*					m_p3DTreeWidget;

		///视点图层的Item
		QTreeWidgetItem*						m_pViewPointItemRoot;

		///添加视点
		QAction*								m_pAddViewPointAction;

		///删除视点
		QAction*								m_pDeleteViewPointAction;

		///清除视点图层文件夹内容
		QAction*								m_pClearFolderAction;

		///当前激活节点
		QTreeWidgetItem*						m_pActiveItem;

		///配置文件的存放路径
		std::string								m_strDataPath;
	};
}

#endif