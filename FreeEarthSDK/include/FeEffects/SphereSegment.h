#ifndef _SPHERE_SEGMENT_H__
#define _SPHERE_SEGMENT_H__

//雷达效果类
#include <osgSim/SphereSegment>

#include <FeEffects/Export.h>

namespace FeEffect
{

class CSphereSegment;
//雷达表面网格
class CSurfaceLine : public osg::Drawable
{
public:
    CSurfaceLine(CSphereSegment* ss) : osg::Drawable(), _ss(ss) {
        this->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
        this->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
        this->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
    };
    CSurfaceLine(const CSurfaceLine& rhs, const osg::CopyOp& co=osg::CopyOp::SHALLOW_COPY):osg::Drawable(rhs,co), _ss(0)
    {
        OSG_WARN<< "警告：表面线创建异常"<<std::endl;
    }
    CSurfaceLine() : _ss(0L) {};
    ~CSurfaceLine() {};
    META_Object(osgSim, CSurfaceLine)
public:
    void drawImplementation(osg::RenderInfo& renderInfo) const;  
    virtual osg::BoundingBox computeBoundingBox() const;

protected:
    CSphereSegment* _ss;
};
//整体雷达效果
class FEEFFECTS_EXPORT CSphereSegment : public osgSim::SphereSegment
{
public:
    CSphereSegment(void);
    //az 左右弧度角 elev上下弧度角, centre这里指的是局部坐标
    CSphereSegment (const osg::Vec3 &centre, float radius, float azMin, float azMax, float elevMin, float elevMax, int density);
    CSphereSegment (const osg::Vec3 &centre, float radius, const osg::Vec3 &vec, float azRange, float elevRange, int density);
    CSphereSegment (const SphereSegment &rhs, const osg::CopyOp &co);

    ~CSphereSegment(void);

public:
    //设置雷达表面网格颜色
    void  setSurfaceLineColor (const osg::Vec4 &c) ;
    const osg::Vec4 &  getSurfaceLineColor () const; 

    //设置是否绘制表面网格线
    void setDrawSurfaceLine(bool b);
    const bool getDrawSurfaceLine() const;

    //设置表面网格线的密度
    void setSurfaceLineDensity(const unsigned int d);
    const unsigned int getSurfaceLineDensity() const;

    void setAllColors(const osg::Vec4& c);

	///
    void setArea(float azMin, float azMax,float elevMin, float elevMax);

    ///设置表面线线宽，单位像素
    void setSurfacelineLineWidth(float w);
    const float getSurfacelineLineWidth() const;

    //设置绘制项（绘制外表面、边沿、侧边、侧面）
    void setDrawMask(int dm);
    int getDrawMask() const { return m_drawMask; };
    //设置半径
    void setRadius(float r);
    const float getRadius() const { return m_radius; }

    typedef enum DrawMask
    {
        SURFACE =   0x00000001, 
        SPOKES =    0x00000002, 
        SURFACELINE=0x00000004,
        EDGELINE =  0x00000008, 
        SIDES =     0x00000010,   
        ALL =       0x7fffffff  
    }DrawMask;

protected:
    //申请成CSurfaceLine友元
    friend class CSurfaceLine;

    //计算雷达表面网格包围球
    bool SurfaceLine_computeBound(osg::BoundingBox&) const;
    //绘制雷达表面网格
    void SurfaceLine_drawImplementation(osg::State&) const;

    //更新
    void dirtyAllDrawableDisplayLists();   
    void dirtyAllDrawableBounds();   

    //表面线
    osg::ref_ptr<CSurfaceLine> m_surfaceline;
    //雷达网格颜色
    osg::Vec4 m_surfacelineColor;

    //表面线线宽
    float m_dLineWidth;

    //雷达中心位置
    osg::Vec3 m_centre;
    //雷达半径
    float m_radius;
    //雷达扫描范围设置 左、右、下、上（弧度）
    float m_azMin, m_azMax, m_elevMin, m_elevMax;
    //雷达表面精细程度
    int m_density;

    //绘制表面线标示
    bool m_bDrawSurfaceLine;

    int m_drawMask;
};
}
#endif