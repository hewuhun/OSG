/**************************************************************************************************
* @file Free3DDockTreeWidget.h
* @note 三维场景树的Dock窗口
* @author c00005
* @data 2017-2-28
**************************************************************************************************/
#ifndef FREE_THREE_DOCK_TREE_WIDGET_H
#define FREE_THREE_DOCK_TREE_WIDGET_H

#include <QWidget>
#include <QTreeWidgetItem>
#include <QContextMenuEvent>

#include <mainWindow/FreeDockWidget.h>
#include <mainWindow/FreeTreeWidget.h>

#include <osgEarth/Layer>
#include <osgEarth/Config>
#include <FeExtNode/ExternNode.h>
#include <FeUtils/FreeViewPoint.h>
#include <FeLayers/IFeLayer.h>

//声明Qt类型
Q_DECLARE_METATYPE(osgEarth::Layer*);
Q_DECLARE_METATYPE(FeLayers::IFeLayer*);
Q_DECLARE_METATYPE(FeExtNode::CExternNode*);
Q_DECLARE_METATYPE(osgEarth::Config);
Q_DECLARE_METATYPE(FeUtil::CFreeViewPoint);

namespace FreeViewer
{
	///QTreeWidgetItem中保存Data的具体数据
	//0:保存Item类型
	//1之后保存数据

	///QTreeWidgetItem所代表的具体类型，保存在Data中
	enum EItemType
	{
		//系统图层标记
		E_SYSTEM_LAYER = 0,
		E_SYSTEM_LAYER_ROOT,//系统图层根节点
		E_SYSTEM_LAYER_IMAGE_FOLDER, // 影像图层组节点
		E_SYSTEM_LAYER_NODE,//系统图层具体图层
		
		//用户图层标记
		E_CUSTOM_LAYER,
		E_CUSTOM_LAYER_ROOT,
		E_CUSTOM_IMAGE_LAYER_ROOT,
		E_CUSTOM_ELEVATION_LAYER_ROOT,
		E_CUSTOM_VECTOR_LAYER_ROOT,
		E_CUSTOM_LAYER_FOLDER,
		E_CUSTOM_LAYER_NODE,

		//视点图层标记
		E_VIEWPOINT_LAYER,
		E_VIEWPOINT_LAYER_ROOT,
		E_VIEWPOINT_LAYER_FOLDER,
		E_VIEWPOINT_LAYER_NODE,
		
		//标绘图层标记
		E_MARK_LAYER,
		E_MARK_LAYER_ROOT,
		E_MARK_FOLDER,
		E_MARK_NODE,

		//军标图层标记
		E_PLOT_LAYER,
		E_PLOT_LAYER_ROOT,
		E_PLOT_FOLDER,
		E_PLOT_NODE,
		DEFAULT
	};

	class CUIObserver;

	/**
	  * @class CFree3DDockTreeWidget
	  * @brief 三维场景树的Dock窗口
	  * @note 主要封装了三维场景树的操作,负责分发不同树节点的信号和槽
	  * @author c00005
	*/
	class CFree3DDockTreeWidget : public CFreeDockWidget
	{
		Q_OBJECT

	public:
		/**  
		 * @brief 构造函数
		 */
		CFree3DDockTreeWidget(QString titleText, QWidget *parent = 0);

		/**  
		 * @brief 析构函数
		 */
		~CFree3DDockTreeWidget();

	public:
		/**  
		  * @brief 初始化树控件
		*/
		void InitWidget();
		
	public:
		/**  
		  * @brief 获取到树控件
		*/
		CFreeTreeWidget* GetTreeWidget();
		
		/**  
		  * @brief 保存节点类型对应的系统，后面会根据系统类型调用系统的槽函数
		*/
		void PushBackSystem(EItemType type, CUIObserver* pSystem);

	public slots:
		/**  
		  * @brief 响应右键弹出菜单
		*/
		void SlotContextMenuItem(QTreeWidgetItem*, QPoint);

		/**  
		  * @brief 响应Item选中或未选中
		*/
		void SlotItemVisibleChange(QTreeWidgetItem*, bool);

		/**  
		  * @brief 响应Item双击事件
		*/
		void SlotItemDoubleClicked(QTreeWidgetItem*);
		
		/**  
		  * @brief 根据当前树形结构更新滚动条区域
		*/
		void SlotUpdateScroll();

	protected:
		///自定义树控件
		CFreeTreeWidget*				m_p3DSceneTreeWidget;

		///保存类型和系统的map对
		typedef std::map<EItemType, CUIObserver*> TypeSystemMap;
		TypeSystemMap					m_mapTypeSystem;
	};
}


#endif // FREE_THREE_DOCK_TREE_WIDGET_H
