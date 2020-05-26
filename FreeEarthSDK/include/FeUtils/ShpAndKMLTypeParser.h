#ifndef ShpAndKMLTypeParser_h__
#define ShpAndKMLTypeParser_h__

/**************************************************************************************************
* @file ShpAndKMLTypeParser.h
* @note Shp和KML文件包含的图元类型解析
*      
* @author c00010
* @data 2014-03-13
**************************************************************************************************/

#include <vector>
#include <string>
#include "FeUtils/Export.h"
namespace FeUtil
{
	enum GeometryType
	{
		UNKNOWN,
		POINT,
		POLYLINE,
		POLYGON,
		MULTI_POLYGON
	};
	class FEUTIL_EXPORT ShpAndKMLTypeParser
	{
	public:
		ShpAndKMLTypeParser();
		~ShpAndKMLTypeParser();

		//返回图元列表
		static std::vector<GeometryType> Parse(const std::string & fileName);

	};
}

#endif //ShpAndKMLTypeParser_h__