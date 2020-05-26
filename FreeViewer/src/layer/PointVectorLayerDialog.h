/**************************************************************************************************
* @file VectorLayerDialog.h
* @note 矢量图层属性对话框
* @author w00040
* @data 2017-1-19
**************************************************************************************************/
#ifndef POINT_VECTOR_LAYER_DIALOG_H
#define POINT_VECTOR_LAYER_DIALOG_H

#include <mainWindow/FreeDialog.h>

#include <QFileDialog>
#include <QColorDialog>
#include <QVector>

#include <FeUtils/PathRegistry.h>
#include <FeLayers/LayerSys.h>

#include "ui_PointVectorLayerDialog.h"

namespace FreeViewer
{
	/**
	  * @class CVectorLayerDialog
	  * @brief 矢量图层对话框
	  * @note 添加矢量图层弹出对话框
	  * @author w00040
	*/
	class CPointVectorLayerDialog : public CFreeDialog
	{
		friend class CCustomLayerTree;
		Q_OBJECT

	public:
		/**  
		  * @brief 构造函数
		*/
		CPointVectorLayerDialog(QWidget *parent, FeLayers::CLayerSys * pCustomLayerSys);

		/**  
		  * @brief 析构函数
		*/
		~CPointVectorLayerDialog();

		/**  
		  * @brief 获取使用的高度-贴合
		*/
		QString GetLaminating(QString str);

		/**  
		  * @brief 获取使用的高度-技术
		*/
		QString GetTechnology(QString str);

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
		  * @brief 创建矢量图层
		*/
		void CreateVectorLayer();

		/**  
		  * @brief 设置控件样式
		*/
		void SetStyle();

	signals:
		/**  
		  * @brief 矢量图层创建完成发送信号
		*/
		void SignalVectorLayerFinish(FeLayers::IFeLayer *, FeLayers::CFeLayerConfig);

	protected slots:
		/**  
		  * @brief 打开矢量图层路径槽函数
		*/
		void SlotOpenDataPath();

		/**  
		  * @brief 打开图标路径槽函数
		*/
		void SlotOpenIconPath();

		/**  
		  * @brief 设置线颜色槽函数
		*/
		void SlotSetLineColor();

		/**  
		  * @brief 设置填充色槽函数
		*/
		void SlotSetLineFillColor();

		/**  
		  * @brief 设置字体颜色
		*/
		void SlotSetFontColor();

		/**  
		  * @brief 设置阴影颜色
		*/
		void SlotSetFontShadowColor();

		/**  
		  * @brief 确认按钮槽函数
		*/
		void SlotOk();

		/**  
		  * @brief 取消按钮槽函数
		*/
		void SlotCancel();

	protected:
		///线颜色
		QColor						m_colorLine;

		///填充色
		QColor						m_colorFill;

		///字体颜色
		QColor						m_colorFont;

		///阴影颜色
		QColor						m_colorFontShadow;

		// 高度-贴合
		QVector<QString>			m_vecLaminating;

		// 高度-技术
		QVector<QString>			m_vecTechnology;

	private:
		Ui::CPointVectorLayerDialog ui;
		FeLayers::CLayerSys *       m_pCustomLayerSys;
		bool                        m_bIsProperty;
	};
}

#endif
