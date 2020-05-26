/**************************************************************************************************
* @file VectorLayerDialog.h
* @note 矢量图层属性对话框
* @author w00040
* @data 2017-1-19
**************************************************************************************************/
#ifndef PROPERTY_VECTOR_LAYER_DIALOG_H
#define PROPERTY_VECTOR_LAYER_DIALOG_H

#include <mainWindow/FreeDialog.h>

#include <QFileDialog>
#include <QColorDialog>
#include <QVector>
#include <QTreeWidget>
#include <FeUtils/PathRegistry.h>
#include <FeLayers/LayerSys.h>

#include "ui_PropertyVectorLayerDialog.h"

namespace FreeViewer
{
	/**
	  * @class CVectorLayerDialog
	  * @brief 矢量图层对话框
	  * @note 添加矢量图层弹出对话框
	  * @author w00040
	*/
	class PropertyVectorLayerDialog : public CFreeDialog
	{
		friend class CCustomLayerTree;
		Q_OBJECT

	public:
		/**  
		  * @brief 构造函数
		*/
		PropertyVectorLayerDialog(QWidget *parent, FeLayers::CLayerSys * pCustomLayerSys);

		/**  
		  * @brief 析构函数
		*/
		~PropertyVectorLayerDialog();
		 
	public:
		void SetLayerAndItem(FeLayers::IFeLayer * pLayer, QTreeWidgetItem * pItem);
	protected:
	
		/**  
		  * @brief 检测输入的信息
		*/
		bool CheckInputInfo();

	protected slots:
		
		/**  
		  * @brief 确认按钮槽函数
		*/
		void SlotOk();

		/**  
		  * @brief 取消按钮槽函数
		*/
		void SlotCancel();

	protected:


	private:
		Ui::PropertyVectorLayerDialog ui;
		FeLayers::CLayerSys *       m_pCustomLayerSys;
		FeLayers::IFeLayer  * m_pLayer;
		QTreeWidgetItem     * m_pItem;
	};
}

#endif
