/**************************************************************************************************
* @file ImageLayerDialog.h
* @note 影像图层属性对话框
* @author w00040
* @data 2017-1-20
**************************************************************************************************/
#ifndef IMAGE_LAYER_DIALOG_H
#define IMAGE_LAYER_DIALOG_H

#include <mainWindow/FreeDialog.h>

#include <FeUtils/PathRegistry.h>
#include <FeLayers/LayerSys.h>

#include <osgEarth/XmlUtils>

#include <QFileDialog>

#include "ui_ImageLayerDialog.h"

namespace FreeViewer
{
	/**
	  * @class CImageLayerDialog
	  * @brief 影像图层对话框
	  * @note 添加影像图层弹出对话框
	  * @author w00040
	*/
	class CImageLayerDialog : public CFreeDialog
	{
		friend class CCustomLayerTree;
		Q_OBJECT

	public:
		/**  
		  * @brief 构造函数
		*/
		CImageLayerDialog(QWidget *parent, FeLayers::CLayerSys * pCustomLayerSys);

		/**  
		  * @brief 析构函数
		*/
		~CImageLayerDialog();

	protected:
		/**  
		  * @brief 初始化数据
		*/
		void InitData();

		/**  
		  * @brief 检测输入的信息
		*/
		bool CheckInputInfo();

		/**  
		  * @brief 创建影像图层
		*/
		void CreateImageLayer();

	signals:
		/**  
		  * @brief 影像图层创建完成发送信号
		*/
		void SignalImageLayerFinish(FeLayers::IFeLayer *, FeLayers::CFeLayerConfig);

		/**  
		  * @brief 发送图层透明度值信号
		*/
		void SignalOpacityValue(float fValue);

	protected slots:
		/**  
		  * @brief 打开影像图层路径槽函数
		*/
		void SlotOpenDataOrPath();

		/**  
		  * @brief 打开缓存路径槽函数
		*/
		void SlotOpenCachePath();

		/**  
		  * @brief 确认按钮槽函数
		*/
		void SlotOk();

		/**  
		  * @brief 取消按钮槽函数
		*/
		void SlotCancel();

		/**  
		  * @brief 透明度改变槽函数
		*/
		void SlotOpacityChanged(int value);

		/**  
		  * @brief 缓存复选框状态改变槽函数
		*/
		void SlotCacheStateChanged(int state);

		/**  
		  * @brief 图层透明度复选框状态改变槽函数
		*/
		void SlotOpacityStateChanged(int state);

		void SetTile(QAbstractButton*);

		void SetPathState(QAbstractButton*);

	private:
		Ui::CImageLayerDialog ui;
		FeLayers::CLayerSys * m_pCustomLayerSys;
		QButtonGroup        * m_group1;
		QButtonGroup        * m_group2;
		bool                  m_bIsImage;
		std::string           m_strLayerRealName;
	
	};
}

#endif
