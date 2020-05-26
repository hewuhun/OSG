/**************************************************************************************************
* @file ElevationLayerDialog.h
* @note 高程图层属性对话框
* @author w00040
* @data 2017-1-20
**************************************************************************************************/
#ifndef ELEVATION_LAYER_DIALOG_H
#define ELEVATION_LAYER_DIALOG_H

#include <mainWindow/FreeDialog.h>

#include <FeUtils/PathRegistry.h>
#include <FeLayers/LayerSys.h>

#include <osgEarth/XmlUtils>

#include <QFileDialog>

#include "ui_ElevationLayerDialog.h"

namespace FreeViewer
{
	/**
	  * @class CElevationLayerDialog
	  * @brief 高程图层对话框
	  * @note 添加高程图层弹出对话框
	  * @author w00040
	*/
	class CElevationLayerDialog : public CFreeDialog
	{

		friend class CCustomLayerTree;
		Q_OBJECT

	public:
		/**  
		  * @brief 构造函数
		*/
		CElevationLayerDialog(QWidget *parent, FeLayers::CLayerSys * pCustomLayerSys);

		/**  
		  * @brief 析构函数
		*/
		~CElevationLayerDialog();

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
		  * @brief 创建高程图层
		*/
		void CreateElevationLayer();

	signals:
		/**  
		  * @brief 高程图层创建完成发送信号
		*/
		void SignaElevationLayerFinish(FeLayers::IFeLayer*, FeLayers::CFeLayerConfig);

	protected slots:
		/**  
		  * @brief 打开高程图层路径槽函数
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
		  * @brief 缓存复选框状态改变槽函数
		*/
		void SlotStateChanged(int state);

	private:
		Ui::CElevationLayerDialog ui;
		FeLayers::CLayerSys * m_pCustomLayerSys;
	};
}

#endif
