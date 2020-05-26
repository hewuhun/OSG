/**************************************************************************************************
* @file FreeControlWidget.h
* @note 控制面板类以及控制面板中的文本内容框
* @author c00005
* @data 2017-3-30
**************************************************************************************************/
#ifndef FREE_CONTROL_WIDGET_H
#define FREE_CONTROL_WIDGET_H 1

#include <QWidget>
#include <QLabel>
#include <QGridLayout>

#include "Export.h"

/**
  * @class CFreeLableWidget
  * @brief 控制面板中的文本显示框，其中包括文本标头和文本内容
  * @author c00005
*/
class FREE_SAMPLE_FRAME_EXPORT CFreeLableWidget: public QWidget
{
public:
	/**  
	  * @brief 构造函数
	*/
	CFreeLableWidget(QWidget* parent=NULL);

	/**  
	  * @brief 西沟函数
	*/
	~CFreeLableWidget();

public:
	/**  
	  * @brief 初始化窗口
	*/
	void InitWidget();
	
	/**  
	  * @brief 设置文本的标题
	*/
	void SetTitleText(QString strTitle);
	
	/**  
	  * @brief 设置文本的内容
	*/
	void SetContentText(QString strContent);

protected:
	///标头
	QLabel *m_pTitleLabel;

	///文本内容
	QLabel *m_pContentLabel;
};

/**
* @class CFreeControlWidget
* @brief 控制面板窗口
* @note 控制面板窗口，可以添加文本对话框以及用户定义的控制按钮
* @author c00005
*/
class FREE_SAMPLE_FRAME_EXPORT CFreeControlWidget : public QWidget
{
	Q_OBJECT

public:
	/**  
	* @note 构造函数
	*/
	CFreeControlWidget(QWidget *parent = 0);

	/**  
	* @note 析构函数
	*/
	~CFreeControlWidget();

public:
	/**  
	* @note 获取控制面板的布局
	*/
	QVBoxLayout* GetLayout();

protected:
	/**  
	* @note 初始化窗口 
	*/
	void InitWidget();

	/**  
	* @note 初始化窗口上下文 
	*/
	void InitContext();

protected:
	///垂直布局
	QVBoxLayout*			m_pVLayout;
};

#endif 
