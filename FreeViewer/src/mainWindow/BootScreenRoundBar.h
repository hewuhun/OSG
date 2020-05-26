/**************************************************************************************************
* @file BootScreenRoundBar.h
* @note 启动界面，圆形进度条
* @author w00040
* @data 2017-2-20
**************************************************************************************************/
#ifndef BOOT_SCREEN_ROUND_BAR_H
#define BOOT_SCREEN_ROUND_BAR_H

#include <QWidget>
#include <QPainter>
#include <QRectF>
#include <QLabel>

namespace FreeViewer
{
	/**
	  * @class CBootScreenRoundBar
	  * @brief 圆形进度条类
	  * @note 启动界面显示圆形进度条
	  * @author w00040
	*/
	class CBootScreenRoundBar : public QWidget
	{
		Q_OBJECT
	public:    
		/**  
		  * @brief 构造函数
		*/
		explicit CBootScreenRoundBar(QWidget *parent = 0);

		static const int PositionLeft = 180;
		static const int PositionTop = 90;
		static const int PositionRight = 0;
		static const int PositionBottom = -90;

		/**
		 * @brief 进度条样式
		 */
		enum BarStyle
		{
		    /// 圆环风格
		    StyleDonut,
		    /// 饼状风格
		    StylePie,
		    /// 线状风格
		    StyleLine
		};

		/**
		 * @brief 设置显示风格
		 */
		void setBarStyle(BarStyle style);

		/**
		 * @brief 设置圆环轮廓线的画笔宽度
		 */
		void setOutlinePenWidth(double penWidth);

		/**
		 * @brief 设置圆环数据圆笔的宽度
		 */
		void setDataPenWidth(double penWidth);

		/**
		 * @brief 设置可见数据的渐变色。
		 */
		void setDataColors(const QGradientStops& stopPoints);

		/**
		 * @brief 定义用于生成当前的文本字符串
		 */
		void setFormat(const QString& format);

		/**
		 * @brief 重置格式字符串为空字符串，将不会显示文本
		 */
		void resetFormat();

		/**
		 * @brief 设置显示小数点后的位数
		 */
		void setDecimals(int count);

		/**
		 * @brief 返回当前显示在控件上的值
		 */
		double value() const { return m_value; }

		/**
		 * @brief 返回允许值范围的最小值
		 */
		double minimum() const { return m_min; }

		/**
		 * @brief 返回允许值范围的最大值
		 */
		double maximum() const { return m_max; }

	public Q_SLOTS:
	    /**
	     * @brief 设置允许值范围
	     */
	    void setRange(double min, double max);

	    /**
	     * @brief 设置允许值范围的最小值
	     */
	    void setMinimum(double min);

	    /**
	     * @brief 设置允许值范围的最大值
	     */
	    void setMaximum(double max);

	    /**
	     * @brief 设置当前显示在控件上的值（浮点数），必须在最小值和最大值之间
	     */
	    void setValue(double val);

	    /**
	     * @brief 设置当前显示在控件上的值（整数），必须在最小值和最大值之间
	     */
	    void setValue(int val);
	
	protected:
	    virtual void paintEvent(QPaintEvent *event);

	    virtual void drawValue(QPainter& p, const QRectF& baseRect, double value, double arcLength);

	    virtual void calculateInnerRect(const QRectF& baseRect, double outerRadius, QRectF& innerRect, double& innerRadius);

	    virtual void drawInnerBackground(QPainter& p, const QRectF& innerRect);

	    virtual void drawText(QPainter& p, const QRectF& innerRect, double innerRadius, double value);

	    virtual QString valueToText(double value) const;

	    virtual void valueFormatChanged();
	
	    double m_min, m_max;
	    double m_value;
	
	    double m_nullPosition;
	    BarStyle m_barStyle;
	    double m_outlinePenWidth, m_dataPenWidth;
	
	    QGradientStops m_gradientData;
	    bool m_rebuildBrush;
	
	    QString m_format;
	    int m_decimals;
	
	    static const int UF_VALUE = 1;
	    static const int UF_PERCENT = 2;
	    static const int UF_MAX = 4;
	    int m_updateFlags;

		QLabel *m_pLabel;
	};
}

#endif