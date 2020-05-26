/**************************************************************************************************
* @file SystemLayerTree.h
* @note 系统图层树
* @author w00040
* @data 2017-1-16
**************************************************************************************************/
#ifndef SYSTEM_LAYER_TREE_H
#define SYSTEM_LAYER_TREE_H

#include <QWidget>
#include <QTreeWidgetItem>

#include <FeLayers/LayerSys.h>

#include <mainWindow/UIObserver.h>
#include <mainWindow/FreeDockFrame.h>
#include <mainWindow/Free3DDockTreeWidget.h>
#include <layer/ImageLayerAttributeDialog.h>
#include <FeLayers/IFeLayer.h>

namespace FreeViewer
{
	/**
	  * @class CSystemLayerTree
	  * @brief 系统图层
	  * @note 创建系统图层树
	  * @author w00040
	*/
	class CSystemLayerTree : public CUIObserver
	{
		Q_OBJECT

	public:
		/**  
		  * @brief 构造函数
		*/
		CSystemLayerTree(CFreeMainWindow* pMainWindow);

		/**  
		  * @brief 析构函数
		*/
		~CSystemLayerTree(void);

	protected:
		/**  
		  * @brief 初始化数据
		*/
		void InitWidget();
		
		/**  
		  * @brief 初始化环境，主要构件界面以及注册一些功能
		*/
		void BuildContext();

	protected:
		/**  
		  * @brief 创建影像图层
		*/
		void CreateImageLayer();

		/**  
		  * @brief 创建高程图层
		*/
		void CreateElevationLayer();

		/**  
		  * @brief 创建矢量图层
		*/
		void CreateVectorLayer();

		/**  
		  * @brief 创建效果图层
		*/
		void CreateEffectLayer();

		/**  
		  * @brief 创建右键菜单
		*/
		QMenu* CreatePopMenu(osgEarth::Layer* pLayer);

		/**  
		  * @brief 添加图层树节点
		*/
		void AddLayerItems(QTreeWidgetItem* pParentItem, osgEarth::Layer* pLayer, const QString& strTitle, bool bVisible);

		void AddLayerItems(QTreeWidgetItem* pParentItem, FeLayers::IFeLayer* pLayer, const QString& strTitle, bool bVisible);
	public slots:
		/**  
		  * @brief 树节点复选框状态改变
		*/
		void SlotItemsVisibleChange(QTreeWidgetItem* pItem, bool visible);

		/**  
		  * @brief 树节点右键菜单槽函数
		*/
		void SlotContextMenuItem(QTreeWidgetItem* pItem, QPoint pos);
		
		/**  
		  * @brief 影像图层属性槽函数
		*/
		void SlotImageLayerAttribute(bool);

	private:
		///系统图层
		osg::ref_ptr<FeLayers::CLayerSys>		m_rpSystemLayerSys;

		///系统图层悬浮窗
		CFree3DDockTreeWidget*					m_p3DTreeWidget;

		///系统图层的Item
		QTreeWidgetItem*						m_pSystemLayerItemRoot;

		///当前激活的Item
		QTreeWidgetItem*						m_pActiveItem;

		///影像图层属性对话框
		CImageLayerAttributeDialog*				m_pImageLayerAttributeDialog;
	};
}

#endif