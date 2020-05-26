/**************************************************************************************************
* @file NodeUtils.h
* @note 定义了场景中的节点的公共操作函数
* @author l00008
* @data 2014-11-03
**************************************************************************************************/

#ifndef FE_TNODE_UTILS_H
#define FE_TNODE_UTILS_H

#include <osgEarth/NodeUtils>

#include <FeUtils/Export.h>

namespace FeUtil
{

    /**
    * @brief 显示和隐藏场景中的节点
    * @note 显示和隐藏场景中的节点
    * @param pNode [in]（对象节点）
    * @return 节点操作成功显示true，失败返回false。
    */
    extern FEUTIL_EXPORT bool SetNodeVisible(osg::Node* pNode, bool bVisible);

	/**  
	  * @brief 更改节点的颜色
	  * @note 此函数将节点中包含的所有几何图元的颜色全部设置为指定颜色
	  * @param pNode [in] 节点指针
	  * @param vecClr [in] 颜色
	  * @return 无
	*/
	FEUTIL_EXPORT void ChangeNodeColor(osg::Node* pNode, const osg::Vec4d& vecClr);
}

#endif //HG_HUD_CAMERA_H
