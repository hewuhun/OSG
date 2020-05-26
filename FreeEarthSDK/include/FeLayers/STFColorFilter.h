/**************************************************************************************************
* @file STFColorFilter.h
* @note 红外效果实现二（老照片的模式即黑白效果）
* @author c00005
* @data 2015-7-8
**************************************************************************************************/
#ifndef FELAYERS_STF_COLOR_FILTER_H
#define FELAYERS_STF_COLOR_FILTER_H

#include <osg/Uniform>
#include <osgEarthUtil/Common>
#include <osgEarth/ColorFilter>

#include <FeLayers/Export.h>

namespace FeLayers
{
	/**
	  * @class CSTFColorFilter
	  * @note 红外效果实现类（老照片的模式即黑白效果）
	  * @author c00005
	*/
	class  FELAYERS_EXPORT CSTFColorFilter : public osgEarth::ColorFilter
	{
	public:
		CSTFColorFilter();

		CSTFColorFilter(const osgEarth::Config& conf);
		
		virtual ~CSTFColorFilter() { }

	public:
		/**  
		  * @brief 设置RGB颜色的偏移值
		  * @note 设置RGB颜色的偏移值，用于改变整体颜色趋向值
		  * @param rgb [in] 颜色的偏移值
		*/
		void setRGBOffset(const osg::Vec3f& rgb);

		/**  
		  * @note 返回颜色的偏移值
		  * @return osg::Vec3f 颜色的偏移值
		*/
		osg::Vec3f getRGBOffset(void) const;

	public:
		/**  
		  * @note 向某一个实体节点中注册红外效果
		  * @param stateSet [in] 需要实现红外效果的节点的状态集
		*/
		virtual void install(osg::StateSet* stateSet) const;

		virtual std::string getEntryPointFunctionName(void) const;

		virtual osgEarth::Config getConfig() const;

	protected:
		void init();

	protected:
		unsigned					m_instanceId;

		osg::ref_ptr<osg::Uniform>	m_rgb;

	};
}


#endif  //FELAYERS_STF_COLOR_FILTER_H
