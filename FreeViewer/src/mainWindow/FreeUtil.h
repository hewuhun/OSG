#ifndef FREE_WIDGET_H
#define FREE_WIDGET_H 1

#include <QWidget>
#include <QValidator>

namespace FreeViewer
{
	///全局变量
	///默认的窗口大小，系统使用图标均根据这个尺寸设计
	const QSizeF	g_sizeDefaultScreen = QSizeF(1920, 1080);
	///默认的菜单栏的大小
	const QSizeF	g_sizeMenu = QSizeF(1920, 92.0);
	///默认的系统Logo使用的图标大小
	const QSizeF	g_sizeMenuLogo = QSizeF(309.0, 92.0);
	const QSizeF	g_sizeMenuLogoImg = QSizeF(48.0, 48.0);
	const QSizeF	g_sizeMenuLogoText = QSizeF(200.0, 43.0);

	///默认的系统Logo使用的图标的起始X位置
	const int		g_nMenuLogoImgX = 15;
	///默认的系统Logo图标与文字之间的间距
	const int		g_nMenuLogoSpace = 8;

	///最小化按钮和关闭按钮的大小
	const QSizeF	g_sizeMinCloseButton = QSizeF(32.0, 23.0);
	///还原按钮的大小
	const QSizeF	g_sizeRestoreButton = QSizeF(28.0, 23.0);

	///工具栏中按钮的大小
	const QSize	g_sizeToolButton = QSize(36.0, 36.0);
	///工具栏中间隔符的大小
	const QSize		g_sizeToolSperator = QSize(1, 30);
	///工具栏的高度
	const int		g_nToolBarHeight = 48;
	///工具栏中相邻按钮之间的间距
	const int		g_nToolBtnSpacing = 4;
	///工具栏中两条自绘线的间距
	const int		g_nToolBarLineDelta = 4.0;
	///工具栏中最左侧按钮距离左边的距离
	const int		g_nToolBarLeftDis = 60.0;
	///工具栏中侧边的斜线的角度，弧度值
	const double	g_dToolBarBorderAngle = 60.0/180.0*3.1415926;

	///主窗口中所有自绘线的宽度值
	const int		g_nLineWidth = 0;
	///主窗口中绘制线时右侧边和下侧边相对于原坐标的偏移值
	const int		g_nDeltaWidth = 1;
	///主窗口自绘制边框线的颜色值
	const QColor	g_colorFrameLine = QColor(44.0, 80.0, 114.0, 255.0);
	//const QColor	g_colorFrameLine = QColor(28, 51, 73, 255);

	///全局函数
	/**  
	  * @note 获取主屏幕的序号  
	  * @return int 主屏幕序号
	*/
	extern int GetPrimaryScreenIndex();

	/**  
	  * @note 获取当前程序的可用桌面大小  
	  * @return QRect 可用桌面大小
	*/
	extern QRect GetAvailableGeometry(int nIndex = -1);
	
	/**  
	  * @note 获取当前屏幕的大小 
	  * @return QRect 屏幕大小
	*/
	extern QRect GetScreenGeometry(int nIndex = -1);

	/**  
	  * @note 为某窗口设置样式集  
	  * @param pWidget [in] 窗口
	  * @param strQSS [in] 样式集路径
	*/
	extern void SetStyleSheet(QWidget* pWidget, QString strQSS);

	/**  
	  * @note 将GB2312编码字符串转换为UTF8编码字符串  
	  * @param strGB2312 [in] GB2312编码字符串 
	  * @return QString UTF8编码字符串
	*/
	extern QString GB2312ToUTF8(std::string strGB2312);

	/**  
	  * @note 将UTF8编码字符串转换为GB2312编码字符串  
	  * @param strUTF8 [in] UTF8编码字符串 
	  * @return QString GB2312编码字符串
	*/
	extern QString UTF8ToGB2312(std::string strUTF8);

    /**
      * @note 将字符串转换为当前系统的编码格式
      * @param str [in] 需要转换的字符串
      * @return QString 装换完成的字符串
    */
    extern QString ConvertToCurrentEncoding(std::string str);

	 /**
      * @note 将字符串转换为FreeSDK中使用的字体编码格式
      * @param str [in] 需要转换的字符串
      * @return std::string 装换完成的字符串
    */
    extern std::string ConvertToSDKEncoding(QString str);

	 /**
      * @note 格式化路径字符串，将字符串中的 “//”，“\\” 都替换为“/”
      * @param str [in] 需要转换的字符串
      * @return std::string 装换完成的字符串
    */
    extern void FormatPathString(QString& str);

	 /**
      * @note 度量单位之间转换，并返回对应字符串
		规则为：
			如果 dVal * multiNum 的值大于 baseValid，则
				返回字符串 dVal unit2
			否则
				返回字符串 dVal unit1

		例如要将 200m 转换 0.2km ，并返回“0.2 km” 字符串，则对应的输入参数应为：
			dVal = 200, multiNum = 1000, unit1 = "m", unit2="km"

      * @param dVal [in] 需要转换的原始值
	  * @param multiNum [in] 两单位之间的转换进制，如 1000
	  * @param unit1 [in] 原始单位对应的字符串，如 "m"
	  * @param unit2 [in] 目标单位对应的字符串，如 "km"
	  * @param baseValid [in] 转换后的有效基准值，默认为0.1
      * @return std::string 格式化完成的字符串。
    */
	extern QString GetMeasureFormatString(double dVal, double multiNum, const std::string& unit1, const std::string& unit2, double baseValid = 0.1); 

	/**  
	  * @brief 只能输入数字、字母和中文
	*/
	extern QValidator* PutInNumberAndLetterAndCN();

	/**  
	  * @brief 不能输入任意空白符
	*/
	extern QValidator* PutInNoEmpty();

	/**  
	  * @note 返回不存在任意空白符的字符串
	*/
	extern QString GetStringNoEmpty(QString);

	/**  
	  * @brief 只能输入数字、字母、下划线
	*/
	extern QValidator*	PutInNumberAndLetter();

	/**  
	  * @brief 只能输入0-100的整数
	*/
	extern QValidator* PutInZeroToOneHundred();

	/**  
	  * @brief 只能输入比例
	*/
	extern QValidator* PutInScale();
}
#endif 







