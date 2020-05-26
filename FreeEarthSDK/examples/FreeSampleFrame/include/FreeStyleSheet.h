/**************************************************************************************************
* @file FreeStyleSheet.h
* @note FreeEarth中Demo主框架使用的样式表
* @author c00005
* @data 2017-3-30
**************************************************************************************************/
#ifndef FREE_STYLE_SHEET_H
#define FREE_STYLE_SHEET_H 1

#include <QString>

/**  
* @note 最小化按钮的风格样式表
*/
extern QString GetMiniPushBtnStyle();

/**  
* @note 还原按钮的风格样式表
*/
extern QString GetNormalPushBtnStyle();

/**  
* @note 关闭按钮的风格样式表
*/
extern QString GetClosePushBtnStyle();

/**  
* @note 标题栏Label的风格样式表
*/
extern QString GetTitleLabelStyle();

/**  
* @note 标题栏的风格样式表
*/
extern QString GetTitleFrameStyle();

/**  
* @note 主窗口的风格样式表
*/
extern QString GetMainWindowStyle();

/**  
* @note 控制面板的风格样式表
*/
extern QString GetControlWidgetStyle();

/**  
* @note 控制面板中的文本标题的风格样式表
*/
extern QString GetLabelTitleStyle();

/**  
* @note 控制面板中的文本内容的风格样式表
*/
extern QString GetLabelContentStyle();

#endif 