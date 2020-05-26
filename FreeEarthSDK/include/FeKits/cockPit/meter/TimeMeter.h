/**************************************************************************************************
* @file TimeMeter.h
* @note 飞机座舱显示时间的仪表
* @author ty00007
* @data 2014-3-22
**************************************************************************************************/
#ifndef _TIMEMETER_H
#define _TIMEMETER_H

#include <iostream>
#include <stdio.h>
#include <time.h>
#include <math.h>

#include <osg/MatrixTransform>
#include <osg/NodeCallback>

#include <FeUtils/tinyXml/tinystr.h>
#include <FeUtils/tinyXml/tinyxml.h>

#include <FeUtils/TimeUtil.h>


typedef std::vector<osg::MatrixTransform*> vecNeedles;

/**
  * @class CTimeChronoGraphCallback
  * @brief 指针运动回调类
  * @note 飞机座舱显示时间仪表根据系统时间运动
  * @author ty00007
*/
class CTimeChronoGraphCallback : public osg::NodeCallback
{
public:

	CTimeChronoGraphCallback(){ };

	CTimeChronoGraphCallback(std::vector<osg::MatrixTransform*> vectorNeedle)
	{
		int size = vectorNeedle.size();
		for (int i=0; i<size; i++)
		{
			m_vectorNeedle.push_back(vectorNeedle.at(i));
		}
	};

	~CTimeChronoGraphCallback(){ };
	
	/**  
	  * @brief 节点回调
	  * @note 节点回调通过传入的参数驱动仪表的指针根据系统时间运动 
	  * @param 参数 node[in] 指针节点 
	  * @param 参数 nv[in] 节点回调参数
	  * @return void
	*/
	virtual void operator() (osg::Node *node, osg::NodeVisitor *nv)
	{
        double dtime = nv->getFrameStamp()->getSimulationTime();
        double dtime_offset = dtime - (int)dtime;
        if (abs(dtime_offset) < 0.05)
		{
            int nHourse;
            int nMinute;
            int nSecond;

            FeUtil::GetLocalTimeHMS(nHourse, nMinute, nSecond);

			double dSecondangle = 6 * nSecond;
			double dMinuteangle = 6 * nMinute + 0.1*nSecond;
			double dHourangle = 30 * nHourse + 0.083333333 * (6 * nMinute + 0.1*nSecond);

			if (m_vectorNeedle.size() > 0)
			{
				m_vectorNeedle.at(2)->setMatrix(osg::Matrix::rotate(-osg::DegreesToRadians(dSecondangle), osg::X_AXIS));
				m_vectorNeedle.at(1)->setMatrix(osg::Matrix::rotate(-osg::DegreesToRadians(dMinuteangle), osg::X_AXIS));
				m_vectorNeedle.at(0)->setMatrix(osg::Matrix::rotate(-osg::DegreesToRadians(dHourangle), osg::X_AXIS));
			}
		}
	}
protected:
	//存放时、分、秒指针的容器
	vecNeedles m_vectorNeedle;
};

/**
  * @class CTimeMeter
  * @brief 时间仪表
  * @note 飞机中显示时间的仪表盘
  * @author ty00007
*/
class CTimeMeter :public osg::MatrixTransform
{
public:
	CTimeMeter(void);

	~CTimeMeter(void);

protected:
	/**  
	  * @brief 重组仪表
	  * @note 拆分时间仪表，提取时、分、秒指针并进行时间仪表重组加入必须的MT节点
	  * @return osg::group*
	*/
	osg::Group* RebuildTimeMeter();
protected:
	//存放时、分、秒指针的容器
	vecNeedles m_vectorNeedle;
};

#endif

