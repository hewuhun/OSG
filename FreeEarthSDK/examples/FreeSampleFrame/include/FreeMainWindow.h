/**************************************************************************************************
* @file FreeMainWindow.h
* @note FreeEarth中Demo使用的主框架，其中可以嵌入三维渲染窗口，以及控制面板
* @author c00005
* @data 2017-3-30
**************************************************************************************************/
#ifndef FREE_MAIN_WINDOW_H
#define FREE_MAIN_WINDOW_H 1

#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "FreeControlWidget.h"

#include "Export.h"

/**
* @class CFreeMainWindow
* @brief FreeEarth中Demo使用的主框架
* @note FreeEarth中Demo使用的主框架，其中可以嵌入三维渲染窗口，以及控制面板
* @author c00005
*/
class FREE_SAMPLE_FRAME_EXPORT CFreeMainWindow : public QFrame
{
	Q_OBJECT

public:
	/**  
	* @note 构造函数
	*/
	CFreeMainWindow(QWidget *parent = 0);

	/**  
	* @note 析构函数
	*/
	~CFreeMainWindow();

protected:
	/**  
	* @note 初始化窗口 
	*/
	void InitWidget();

	/**  
	* @note 初始化窗口上下文 
	*/
	void InitContext();

public:
	/**  
	* @note 设置控制面板的大小
	* @param size [in] 控制面板大小
	*/
	void SetControlWidgetSize(QSize size);

	/**  
	* @note 向控制面板中添加弹簧控件
	* @param pItem [in] 控制面板中的弹簧控件
	*/
	void AddControlItem(QSpacerItem* pItem);

	/**  
	* @note 增加控制面板中的窗口
	* @param pWidget [in] 控制面板内容窗口指针
	*/
	void AddControlWidget(QWidget* pWidget);

	/**  
	* @note 获取用于添加场景的layout，可以添加三维或者二维的渲染窗口
	*/
	QGridLayout* GetSceneLayout();

public:
	/**  
	* @note 设置窗口的名称  
	* @param strTitle [in] 窗口的名称
	*/
	void SetTitle(QString strTitle);

	/**  
	* @note 设置可点击拖拽移动的区域  
	* @param rectArea [in] 区域
	*/
	void SetMovableArea(QRect rectArea);

	/**  
	* @note 设置可点双击改变窗口大小的区域  
	* @param rectArea [in] 区域
	*/
	void SetDoubleClickedArea(QRect rectArea);

	/**  
	* @note 最大化显示 
	*/
	void ShowDialogMaxisize();

	/**  
	* @note 最小化显示 
	*/
	void ShowDialogMinisize();

	/**  
	* @note 正常显示 
	*/
	void ShowDialogNormal();

protected:
	/**  
	* @note 检查鼠标是否在窗体的边界处 
	*/
	void CheckEdge();

	/**  
	* @note 判断一个点是否在一个区域内
	* @param rectArea [in] 区域
	* @param point [in] 点
	* @return bool 点否是在此区域内
	*/
	bool PointInArea(const QRect& rectArea, const QPoint& point);

	/**  
	* @note 重置logo以及控制面板的位置以及大小接口
	*/
	void RepaintWidget();

public slots:
	/**  
	* @note 关闭窗体的槽函数 
	*/
	virtual void SlotCloseWidget();

	/**  
	* @note 最小化窗体的槽函数 
	*/
	virtual void SlotMinisizeWidget();

	/**  
	* @note 最大化窗体的槽函数 
	*/
	virtual void SlotMaxisizeWidget();

protected:
	/**  
	* @note 鼠标按下事件响应 
	*/
	virtual void mousePressEvent(QMouseEvent *event);

	/**  
	* @note 鼠标释放事件响应 
	*/
	virtual void mouseReleaseEvent(QMouseEvent *event);

	/**  
	* @note 鼠标移动事件响应 
	*/
	virtual void mouseMoveEvent(QMouseEvent *event);

	/**  
	* @note 鼠标双击事件响应 
	*/
	virtual void mouseDoubleClickEvent(QMouseEvent* event);

	/**  
	* @note 大小改变事件 
	*/
	virtual void resizeEvent(QResizeEvent* event);
	
	/**  
	* @note 窗口移动事件 
	*/
	virtual void moveEvent(QMoveEvent* event);

	/**  
	  * @brief 显示事件
	*/
    virtual void showEvent(QShowEvent *event);

	/**  
	  * @brief 隐藏事件
	*/
	virtual void hideEvent(QHideEvent *event);

protected:
	///标题栏
	QLabel*												m_pLabelTitle;

	///最小化按钮
	QPushButton*										m_pMiniPushBtn;

	///还原按钮
	QPushButton*										m_pNormalPushBtn;

	///关闭窗口按钮
	QPushButton*										m_pClosePushBtn;

	///可以添加二三维窗口的布局
	QGridLayout*										m_pSceneGridLayout;

	///公司logo显示窗口
	QWidget*											m_pLogoWidget;

	///控制面板
	CFreeControlWidget*									m_pControlWidget;

	///窗口是否还原显示或者最大化显示
	bool												m_bShowNormal;

	//窗口在鼠标按下一刻的位置
	QPoint												m_pointLastMouse;

	//鼠标是否按下
	bool												m_bMousePress;

	//鼠标检测的边缘距离
	int													m_nEdgeMargin;     

	///拖拽移动窗口的区域
	QRect												m_rectMovableArea;

	///双击放大缩小窗口的区域
	QRect												m_rectDoubleClickedArea;

	//更改窗口大小的方向
	typedef enum
	{
		NO_DIR						= 0x00,
		TOP							= 0x01,
		BOTTOM						= 0x02,
		LEFT						= 0x04,
		RIGHT						= 0x08,
		TOP_LEFT					= 0x01 | 0x04,
		TOP_RIGHT					= 0x01 | 0x08,
		BOTTOM_LEFT					= 0x02 | 0x04,
		BOTTOM_RIGHT				= 0x02 | 0x08
	} EMouseDir;
	EMouseDir											m_nMouseDir;
};

#endif 
