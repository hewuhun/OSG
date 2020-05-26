/**************************************************************************************************
* @file ImageLayerAttributeDialog.h
* @note 系统图层右键影像图层属性对话框
* @author w00040
* @data 2017-2-6
**************************************************************************************************/
#ifndef IMAGE_LAYER_ATTRIBUTE_DIALOG_H
#define IMAGE_LAYER_ATTRIBUTE_DIALOG_H

#include <mainWindow/FreeDialog.h>

#include <QTreeWidgetItem>
#include <QListWidgetItem>

#include <FeLayers/LayerSys.h>

#include <osgDB/ConvertUTF>
#include <osgEarth/Map>

#include "ui_ImageLayerAttributeDialog.h"

namespace FreeViewer
{
	/**
	  * @class CImageLayerAttributeDialog
	  * @brief 影像图层属性对话框
	  * @note 系统图层右键设置影像图层属性对话框
	  * @author w00040
	*/
	class CImageLayerAttributeDialog : public CFreeDialog
	{
		Q_OBJECT

	public:
		/**  
		  * @brief 构造函数
		*/
		CImageLayerAttributeDialog(FeLayers::CLayerSys* pSystem, osgEarth::ImageLayer* pCurrentLayer, QWidget *parent = 0);

		/**  
		  * @brief 析构函数
		*/
		~CImageLayerAttributeDialog();

		/**  
		  * @brief 重新获取影像图层
		*/
		void ReGetImageLayer();

		typedef std::map<QListWidgetItem*, osg::observer_ptr<osgEarth::ImageLayer> >  ItemLayerMap;

		typedef std::map<QListWidgetItem*, double >  LayerOpacityMap;

	protected:
		/**  
		  * @brief 初始化数据
		*/
		void InitData();

		/**  
		  * @brief 初始化环境，主要构件界面以及注册一些功能
		*/
		void InitContext();

	private slots:
		/**  
		  * @brief 影像图层透明度改变槽函数
		*/
		void SlotOpacityChanged(int);

		/**  
		  * @brief 确认按钮槽函数
		*/
		void SlotOk();

		/**  
		  * @brief 应用按钮槽函数
		*/
		void SlotApply();

		/**  
		  * @brief 取消按钮槽函数
		*/
		void SlotCancel();

		/**  
		  * @brief 节点改变槽函数
		*/
		void SlotItemChanged();

	private:
		Ui::CImageLayerAttributeDialog ui;

		///图层系统
		osg::observer_ptr<FeLayers::CLayerSys>			m_opLayerSystem;

		///当前影像图层
		osg::observer_ptr<osgEarth::ImageLayer>			m_opCurrentImageLayer;

		///图层节点映射
		ItemLayerMap									m_itemLayerMap;

		///图层透明度映射 
		LayerOpacityMap									m_layerOpacityMap;
	};
}

#endif // IMAGELAYERATTRIBUTEDIALOG_H
