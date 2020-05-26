/**************************************************************************************************
* @file PropertyWidget.h
* @note 属性对话框类
* @author g00034
* @data 2017-02-20
**************************************************************************************************/
#ifndef PROPERTY_WIDGET_H
#define PROPERTY_WIDGET_H 1

#include <FeExtNode/ExternNode.h>

#include <mainWindow/FreeDialog.h>

#include "ui_PropertyWidget.h"


namespace FreeViewer
{
	class CBaseStyleWidget;

	/**
      * @class CPropertyWidget
      * @note 属性对话框类，其中包含了文件的名称、描述信息、视点等等
      * @author g00034
    */
    class CPropertyWidget : public CFreeDialog
    {
        Q_OBJECT

    public:
		/**  
		  * @note 构造和析构函数  
		*/
		CPropertyWidget(FeExtNode::CExternNode* pMark, QWidget* pParent, bool bCreate = false);
        ~CPropertyWidget();

		/**  
		  * @brief 将传进的标签页加到公共标签页中
		  * @param 参数 [in] pTab
		  * @return 0
		*/
		int AddTab(CBaseStyleWidget* pTab);

		/**  
		  * @brief 处理SDK事件
		  * @param eType [in] 事件类型，FeUtil::EUserEventType
		*/
		void HandleSDKEvent(unsigned int eType) { emit sig_SDKEventHappened(eType); }

		/**  
		  * @brief 标记类型(翻译使用)
		*/
		enum EMarkStyle
		{
			//标记
			E_PLACE_NODE = 0,		///点标记
			E_COMPOSE_NODE,         ///文件夹节点(节点组)
			E_MODEL_NODE,			///模型标记
			E_LINE_NODE,			///线标记
			E_POLYGON_NODE,			///多边形标记
			//E_STRING_ARROW_NODE,	///直箭头标绘
			//E_ASSAULT_NODE,			///突击方向标绘
			//E_ATTACK_NODE,			///进攻方向标绘
			E_BILL_BOARD_NODE,		///广告牌标记
			E_OVERLAY_NODE,			///遮盖标记
			E_ARC_NODE,				///弧形标记
			E_ELLIPSE_NODE,			///椭圆标记
			E_RECT_NODE,			///多边形标记
			E_SECTOR_NODE,			///扇形标记
			E_CIRCLE_NODE,			///圆形标记
			E_TEXT_NODE,			///文本标记
			E_LABEL_NODE,			///标牌标记
			E_TILT_MODEL_NODE,		///倾斜摄影
			E_PARTICLE_NODE,		///粒子标记

			//军标
			E_STRAIGHT_ARROW,				//直箭头
			E_STRAIGHT_MORE_ARROW,			//多点直箭头
			E_DOVETAIL_DIAGONAL_ARROW,		//燕尾斜箭头
			E_DOVETAIL_DIAGONAL_MORE_ARROW,	//多点燕尾斜箭头
			E_DOVETAIL_STRAIGHT_ARROW,		//燕尾直箭头
			E_DOVETAIL_STRAIGHT_MORE_ARROW,	//多点燕尾直箭头
			E_DIAGONAL_ARROW,	            //多点燕尾斜箭头
			E_DIAGONAL_MORE_ARROW,		   //燕尾直箭头
			E_DOUBLE_ARROW,	               //多点燕尾直箭头
			E_CLOSE_CURVE,                 //闭合曲线箭头
			E_GATHERING_PLACE,				//聚集区
			//E_RECT_FLAG,					//直角旗标
			//E_CURVE_FLAG,					//曲面旗标
			//E_TRIANGLE_FLAG,				//三角旗标
			E_ROUNDED_RECT,					//圆角矩形
			E_BEZIER_CURVE_ARROW,			//贝塞尔曲线箭头
			E_POLY_LINE_ARROW,				//折线箭头
			E_SECTOR_SEARCH,				//扇形搜索区
			E_PARALLEL_SEARCH,				//平行搜索区
			E_CARDINAL_CURVE_ARROW,			//Cardinal曲线箭头
			E_STRAIGHT_LINE_ARROW,			//直线箭头
			//E_FREE_LINE,					//自由线
			//E_FREE_POLYGON				//手绘面
		};

		/**  
		  * @brief 设置标题栏(翻译使用)
		*/
		void SetTitleFlag(EMarkStyle nFlag);

	protected:
		/**  
		  * @note 控件初始化
		*/
		void InitWidget();

		/**  
		  * @note 内容初始化
		*/
		void BuildContext();

		/**  
		  * @note 整形转字符串
		*/
		std::string UnintToString(unsigned int unInt);

	protected slots:
		/**  
		  * @note 确定取消处理函数
		*/
		virtual void reject();
		virtual void accept();

		/**  
		  * @note 标题变换
		*/
		void SlotTitleChanged(QString);

signals:
		/**  
		  * @note 确定取消处理信息号
		*/
		void sig_accept(FeExtNode::CExternNode* pNode, bool bCreate);
		void sig_reject(FeExtNode::CExternNode* pNode, bool bCreate);

		/**  
		  * @note SDK 事件处理信息号
		*/
		void sig_SDKEventHappened(unsigned int);

		/**  
		  * @note 视点改变信号
		*/
		void SignalViewPointChanged(CFreeViewPoint);

    private:
		bool	                                  m_bCreated;
		Ui::property_widget                       ui;

		/// 关联标记对象
		osg::observer_ptr<FeExtNode::CExternNode> m_opMark;

		/// 标签属性页
		typedef QVector<CBaseStyleWidget*>        TabVector;
		TabVector                                 m_vecTab;

		/// 初始值
	private:
		//bool        m_bVisible;
		std::string m_strName;
    };

}
#endif // PROPERTY_WIDGET_H
