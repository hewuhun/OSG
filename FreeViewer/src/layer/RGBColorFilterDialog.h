/**************************************************************************************************
* @file RGBColorFilterDialog.h
* @note RGB自定义调色板
* @author c00005
* @data 2017-4-4
**************************************************************************************************/
#ifndef FE_RGB_COLOR_FILTER_H
#define FE_RGB_COLOR_FILTER_H

#include <mainWindow/FreeDialog.h>

#include <FeLayers/LayerSys.h>
#include <FeLayers/STFColorFilter.h>

#include <osgEarthUtil/RGBColorFilter>

#include "ui_RGBColorFilterDialog.h"

namespace FreeViewer
{
	/**
	  * @class CRGBColorFilterDialog
	  * @brief 影像图层对话框
	  * @note 添加影像图层弹出对话框
	  * @author w00040
	*/
	class CRGBColorFilterDialog : public CFreeDialog
	{
		Q_OBJECT

	public:
		/**  
		  * @brief 构造函数
		*/
		CRGBColorFilterDialog(FeLayers::CLayerSys* pLayerSys, QWidget *parent = 0);

		/**  
		  * @brief 析构函数
		*/
		~CRGBColorFilterDialog();

	protected:
		/**  
		  * @brief 初始化数据
		*/
		void InitWidget();

		/**  
		  * @brief 更新控件状态
		*/
		void UpdateCtrlState();

	protected slots:
		/**  
		  * @brief 打开影像图层路径槽函数
		*/
		void SlotNostalgiaVisible(bool);

		/**  
		  * @brief 打开影像图层路径槽函数
		*/
		void SlotRGBColorFilterVisible(bool);
		
		/**  
		  * @brief 透明度改变槽函数
		*/
		void SlotRGBColorChanged(int);

	private:
		Ui::RGBColorFilterDialog		 ui;

		osg::observer_ptr<FeLayers::CLayerSys>			m_opLayerSys;

		///老照片模式
		osg::ref_ptr<FeLayers::CSTFColorFilter>			m_rpSTFColorFilter;

		osg::ref_ptr<osgEarth::Util::RGBColorFilter>	m_rpRGBColorFilter;
	};
}

#endif
