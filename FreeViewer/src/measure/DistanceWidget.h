/**************************************************************************************************
* @file DistanceWidget.h
* @note 距离等简单测量结果的界面
* @author g00034
* @data 2017-01-19
**************************************************************************************************/

#ifndef DISTANCE_WIDGET_H
#define DISTANCE_WIDGET_H 1

#include <ui_DistanceWidget.h>

#include <measure/MeasureDisplayWidget.h>


namespace FreeViewer
{
	/**
	  * @class CMSimpleResultDlg
	  * @brief 简单测量结果显示对话框
	  * @note 只显示简单信息。如：距离，面积，高度等测量结果的显示
	  * @author g00034
	*/
	class CMSimpleResultDlg : public CMeasureDisplayWidget
	{
		Q_OBJECT

	public:
		~CMSimpleResultDlg(){}
	
	protected:
		CMSimpleResultDlg(QWidget* parent);
		
		/**  
		  * @note 是否显示计算结果信息
		*/
		void SetValueVisible(bool bVisible);

		/**  
		  * @note 是否显示测量点信息
		*/
		void SetPointsVisible(bool bVisible);

		/**  
		  * @note 处理测量点的显示
		*/
		virtual void ShowMeasurePoints(const osg::Vec3dArray& points);

		/**  
		  * @note 结果界面显示文字  (距离 面积等)
		*/
		virtual void setLabelValueUI(QString str);

	private:
		/**  
		  * @note 派生类实现该接口类处理测量消息，输入参数有效
		*/
		virtual void HandleMeasureMsg(EMeasureAction msg, CMeasure* pMeasure);

		/**  
		  * @note 获取结果值的格式化字符串
		*/
		virtual QString GetFormatValueString( double dVal){ return QString(); }

	protected:
		Ui::DistanceWidget				ui;
	};

}




namespace FreeViewer
{

	/**
	  * @class CMDistanceWidget
	  * @brief 距离测量结果
	  * @note  包括 直线距离，地表距离，高度差测量
	  * @author g00034
	*/
	class CMDistanceWidget : public CMSimpleResultDlg
	{
		Q_OBJECT

	public:
		CMDistanceWidget(QWidget* parent):CMSimpleResultDlg(parent){setLabelValueUI(tr("Distance Value"));}
	
		/**  
		  * @note 结果界面显示文字  (距离 面积等)
		*/
		virtual void setLabelValueUI(QString str);

	private:
		/**  
		  * @note 获取结果值的格式化字符串
		*/
		virtual QString GetFormatValueString( double dVal);



	};


	/**
	  * @class CMAreaWidget
	  * @brief  面积测量结果
	  * @note  包括 投影面积，地表面积测量
	  * @author g00034
	*/
	class CMAreaWidget : public CMSimpleResultDlg
	{
		Q_OBJECT

	public:
		CMAreaWidget(QWidget* parent):CMSimpleResultDlg(parent){setLabelValueUI(tr("Area Value"));}
	

	private:
		/**  
		  * @note 获取结果值的格式化字符串
		*/
		virtual QString GetFormatValueString( double dVal);
	};



	/**
	  * @class CMVisibleLineWidget
	  * @brief  通视线测量结果
	  * @author g00034
	*/
	class CMVisibleLineWidget : public CMSimpleResultDlg
	{
	public:
		~CMVisibleLineWidget(){}
		CMVisibleLineWidget(QWidget* parent):CMSimpleResultDlg(parent)
		{
			SetValueVisible(false);
		}

	private:
		/**  
		  * @note 处理测量点的显示
		*/
		virtual void ShowMeasurePoints(const osg::Vec3dArray& points);
		
	};

	/**
	  * @class CMRadialLineWidget
	  * @brief  视域分析测量结果
	  * @author g00034
	*/
	class CMRadialLineWidget : public CMSimpleResultDlg
	{
	public:
		~CMRadialLineWidget(){}
		CMRadialLineWidget(QWidget* parent):CMSimpleResultDlg(parent)
		{
			SetValueVisible(false);
			SetPointsVisible(false);
		}

	private:
		/**  
		  * @note 处理测量点的显示
		*/
		virtual void ShowMeasurePoints(const osg::Vec3dArray& points);
		
	};


	/**
	  * @class CMGradientWidget
	  * @brief  坡度坡向分析测量结果
	  * @author g00034
	*/
	class CMGradientWidget : public CMSimpleResultDlg
	{
	public:
		~CMGradientWidget(){}
		CMGradientWidget(QWidget* parent):CMSimpleResultDlg(parent)
		{
			SetValueVisible(false);
		}
	};

}


#endif //DISTANCE_WIDGET_H