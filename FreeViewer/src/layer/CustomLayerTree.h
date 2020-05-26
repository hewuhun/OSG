/**************************************************************************************************
* @file CustomLayerTree.h
* @note 自定义图层树
* @author w00040
* @data 2017-1-17
**************************************************************************************************/
#ifndef CUSTOM_LAYER_TREE_H
#define CUSTOM_LAYER_TREE_H

#include <QWidget>
#include <QTreeWidgetItem>
#include <QFileDialog>
#include <QEvent>
#include <QInputDialog>
#include <QToolBar>

#include <mainWindow/UIObserver.h>
#include <mainWindow/FolderWidget.h>
#include <mainWindow/Free3DDockTreeWidget.h>

#include <layer/ImageLayerDialog.h>
#include <layer/ElevationLayerDialog.h>
#include <layer/VectorLayerDialog.h>
#include <layer/RGBColorFilterDialog.h>

#include <FeUtils/PathRegistry.h>
#include <FeLayers/LayerSys.h>
#include <FeLayers/CustomLayerReaderWriter.h>

#include <osgEarth/Config>
#include <layer/PropertyImageLayerDialog.h>

namespace FreeViewer
{
	/**
	  * @class CCustomLayerTree
	  * @brief 自定义图层
	  * @note 创建自定义图层树
	  * @author w00040
	*/
	class CCustomLayerTree : public CUIObserver
	{
		friend class CImageLayerDialog;
		friend class CVectorLayerDialog;
		friend class CElevationLayerDialog;
		Q_OBJECT

	public:
		/**  
		  * @brief 构造函数
		*/
		CCustomLayerTree(CFreeMainWindow* pMainWindow);

		/**  
		  * @brief 析构函数
		*/
		~CCustomLayerTree(void);

	protected:
		/**  
		  * @brief 初始化数据
		*/
		void InitWidget();
		
		/**  
		  * @brief 初始化环境，主要构件界面以及注册一些功能
		*/
		void BuildContext();

	public:
		/**  
		  * @brief 初始化场景
		*/
		void LoadData(std::string strPath);

		/**  
		  * @brief 序列化场景
		*/
		void SaveData();

	protected:
		/**  
		  * @brief 添加图层
		*/
		void AddItem(QTreeWidgetItem* pParentItem, FeLayers::IFeLayer * pLayer, FeLayers::CFeLayerConfig layerConfig);
		
		/**  
		  * @brief 删除图层文件夹，同时清除内容
		*/
		void DeleteItem(QTreeWidgetItem* pItem);

		/**  
		  * @brief 清空图层文件夹中的内容
		*/
		void ClearItem(QTreeWidgetItem* pItem);

		/**  
		  * @brief 解析图层配置，并添加图层
		*/
		void ParseLayer(osgEarth::Config& layersConfig);
		
		/**  
		  * @brief 保存图层配置
		*/
		void UnParseLayer(QTreeWidgetItem* pItem, FeLayers::CCustomLayerReaderWriter & cw);

	public slots:
		/**  
		  * @brief 树节点右键菜单槽函数
		*/
		void SlotContextMenuItem(QTreeWidgetItem* pItem, QPoint pos);

		/**  
		  * @brief 树节点可视化改变
		*/
		void SlotItemsVisibleChange(QTreeWidgetItem* pItem, bool visible);

		/**  
		  * @brief 设置影像图层透明度
		*/
		void SlotSetImageLayerOpacity(float fValue);

		void AddImageLayer(const std::string & name, const std::string & path, const std::string & driver);

		void AddFeatureLayer(const std::string & name, const std::string & path, const std::string & driver);

		void AddElevationLayer(const std::string & name, const std::string & path, const std::string & driver);
	protected slots:
		/**  
		  * @brief 添加图层文件夹
		*/
		void SlotAddLayerFolder();
		
		/**  
		  * @brief 添加图层文件夹
		*/
		void SlotAddLayerFolder(QString, bool);

		/**  
		  * @brief 删除图层文件夹
		*/
		void SlotDeleteLayerFolder();
		
		/**  
		  * @brief 删除图层文件夹
		*/
		void SlotClearLayerFolder();
		
		/**  
		  * @brief 右键菜单添加影像图层
		*/
		void SlotAddImageLayer(bool bFlag);

		/**  
		  * @brief 右键菜单添加高程图层
		*/
		void SlotAddElevationLayer(bool bFlag);
		
		/**  
		  * @brief 右键菜单添加矢量图层
		*/
		void SlotAddFeatureLayer(bool bFlag);
		
		/**  
		  * @brief 右键菜单添加天地图栅格矢量图层
		*/
		void SlotAddRastorLayer(bool bFlag);

		/**  
		  * @brief 添加图层
		*/
		void SlotAddLayer(FeLayers::IFeLayer* pLayer, FeLayers::CFeLayerConfig config);

		void SlotLayerProperty(bool);

		/**  
		  * @brief 删除图层
		*/
		void SlotDeleteLayer();

		/**  
		  * @brief 影像图层特效处理
		*/
		void SlotColorFilterDialog(bool);

	private:
		//后续需与SystemLayer中的Sys统一管理
		///自定义图层
		osg::ref_ptr<FeLayers::CLayerSys>		m_rpCustomLayerSys;

		///自定义图层悬浮窗
		CFree3DDockTreeWidget*					m_p3DTreeWidget;

		///影像图层特效管理器
		CRGBColorFilterDialog*					m_pRGBColorFilterDialog;

		///添加文件夹对话框
		CFolderWidget*							m_pFolderWidget;

		///用户定义图层的Item
		QTreeWidgetItem*						m_pCustomLayerItemRoot;

		///当前激活节点
		QTreeWidgetItem*						m_pActiveItem;

		QTreeWidgetItem*                        m_pImageLayerItemRoot;
		QTreeWidgetItem*                        m_pVectorLayerItemRoot;
		QTreeWidgetItem*                        m_pElevationLayerItemRoot;

		///影像对话框
		CImageLayerDialog*						m_pImageDialog;

		CImageLayerDialog*						m_pImageDialog2;

		///高程对话框
		CElevationLayerDialog*					m_pElevationDialog;
		CElevationLayerDialog*					m_pElevationDialog2;

		///矢量对话框
		CVectorLayerDialog*						m_pFeatureDialog;

		CVectorLayerDialog*						m_pFeatureDialog2;

		CVectorLayerDialog*                     m_pFeatureDialog3;
		///添加影像图层
		QAction*								m_pAddImageLayerAction;
		QAction*                                m_pPropertyAction;

		///添加高程图层
		QAction*								m_pAddElevationLayerAction;

		///添加矢量图层
		QAction*								m_pAddFeatureLayerAction;

		///添加天地图在线矢量栅格数据
		QAction*								m_pAddTDTRasterLayerAction;

		///删除图层
		QAction*								m_pDeleteLayerAction;

		///添加图层文件夹
		QAction*								m_pAddLayerFolderAction;

		///删除图层文件夹
		QAction*								m_pDelelteFolderAction;

		///清除图层文件夹内容
		QAction*								m_pClearFolderAction;

		///影像特效设置
		QAction*								m_pColorFilterAction;

		///鼠标是否右键菜单
		bool									m_bMouseRightMenu;

		PropertyImageLayerDialog*               m_pPropertyImageLayerDialog;
		/// 当前激活图层Config
		FeLayers::CFeLayerConfig				m_curLayerConfig;
		//osgEarth::Config						m_curLayerConfig;
	};											   
}

#endif