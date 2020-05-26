#include <FeEffects/SphereSegment.h>

//#include <gl/glut.h>
#include <osg/Notify>
#include <osg/CullFace>
#include <osg/LineWidth>
#include <osg/Transform>
#include <osg/Geometry>
#include <osg/TriangleIndexFunctor> 
#include <osg/ShapeDrawable>
#include <osg/io_utils>
#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Geode> 
#include <osg/Matrixd>
#include <osg/BlendFunc>
#include <algorithm>
#include <list>

using namespace osgSim;

namespace FeEffect
{
//CSphereSegment.cpp
CSphereSegment::CSphereSegment(void)
    :osgSim::SphereSegment(),
    m_centre(osg::Vec3(0.0f, 0.0f, 0.0f)), 
    m_radius(1.0f),
    m_azMin(0.0f),
    m_azMax(osg::PI_2),
    m_elevMin(0.0f),
    m_elevMax(osg::PI_2),
    m_density(10),
    m_surfacelineColor(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f)),
    m_bDrawSurfaceLine(true),
    m_dLineWidth(1.0),
    m_drawMask(DrawMask(ALL))
{
    m_surfaceline = new CSurfaceLine(this);
    addDrawable(m_surfaceline);
}

CSphereSegment::CSphereSegment (const osg::Vec3 &centre, float radius, float azMin, float azMax, float elevMin, float elevMax, int density) 
    :osgSim::SphereSegment(centre, radius, azMin, azMax, elevMin, elevMax, density),
    m_centre(centre), 
    m_radius(radius),
    m_azMin(azMin),
    m_azMax(azMax),
    m_elevMin(elevMin),
    m_elevMax(elevMax),
    m_density(density),
    m_surfacelineColor(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f)),
    m_bDrawSurfaceLine(true),
    m_dLineWidth(1.0),
    m_drawMask(DrawMask(ALL))
{
    m_surfaceline = new CSurfaceLine(this);
    addDrawable(m_surfaceline);
}
CSphereSegment::CSphereSegment (const osg::Vec3 &centre, float radius, const osg::Vec3 &vec, float azRange, float elevRange, int density) 
    :osgSim::SphereSegment(centre, radius, vec, azRange, elevRange, density),
    m_centre(centre), 
    m_radius(radius),
    m_density(density),
    m_surfacelineColor(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f)),
    m_bDrawSurfaceLine(true),
    m_dLineWidth(1.0),
    m_drawMask(DrawMask(ALL))
{
    m_azMin = -azRange / 2.0;
    m_azMax = azRange / 2.0;
    m_elevMin = -elevRange / 2.0;
    m_elevMax = elevRange / 2.0;
    m_surfaceline = new CSurfaceLine(this);
    addDrawable(m_surfaceline);
}
CSphereSegment::CSphereSegment (const SphereSegment &rhs, const osg::CopyOp &co) 
    :osgSim::SphereSegment(rhs, co),
    m_surfacelineColor(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f)),
    m_bDrawSurfaceLine(true),
    m_dLineWidth(1.0),
    m_drawMask(DrawMask(ALL))
{
    m_surfaceline = new CSurfaceLine(this);
    addDrawable(m_surfaceline);
}

CSphereSegment::~CSphereSegment(void)
{
}

bool CSphereSegment::SurfaceLine_computeBound( osg::BoundingBox& bbox) const
{
    bbox.init();    
    float azIncr = (m_azMax - m_azMin)/m_density;
    float elevIncr = (m_elevMax - m_elevMin)/m_density;
    for(int i=0; i<=m_density; i++)
    {        
        float az = m_azMin + (i*azIncr);
        for(int j=0; j<=m_density; j++)
        {            
            float elev = m_elevMin + (j*elevIncr); 
            bbox.expandBy(
                osg::Vec3(m_centre.x() + m_radius*cos(elev)*sin(az),
                m_centre.y() + m_radius*cos(elev)*cos(az),
                m_centre.z() + m_radius*sin(elev))
                );
        } 
    }
    return true;
}

void CSphereSegment::SurfaceLine_drawImplementation( osg::State& state) const
{

    if (!m_bDrawSurfaceLine)
    {
        return;
    }
    const float azIncr = (m_azMax - m_azMin) / m_density;
    const float elevIncr = (m_elevMax - m_elevMin) / m_density;
    if (m_drawMask & SURFACELINE)
    {
        osg::GLBeginEndAdapter& gl = state.getGLBeginEndAdapter();

        gl.Color4fv(m_surfacelineColor.ptr());
        bool drawBackSide = false; 
        bool drawFrontSide = true; 

        glLineWidth(m_dLineWidth);
        glEnable(GL_LINE_SMOOTH);

        if (drawFrontSide)
        {
            //竖线
            for(int i=0; i+1<m_density; i++)
            {               
                float az = m_azMin + ((i+1)*azIncr);
                gl.Begin(GL_LINE_STRIP);

                for (int j=0; j<=m_density; j++) 
                {                  
                    float elev = m_elevMin + (j*elevIncr);     
                    float x = cos(elev)*sin(az);
                    float y = cos(elev)*cos(az);
                    float z = sin(elev); 
                    gl.Normal3f(x, y, z);
                    gl.Vertex3f(m_centre.x() + m_radius*x, 
                        m_centre.y() + m_radius*y,
                        m_centre.z() + m_radius*z);
                }
                gl.End();
            }
            //画横线
            for (int i=0; i+1<m_density; i++)
            {
                float elev = m_elevMin + ((i+1)*elevIncr);

                gl.Begin(GL_LINE_STRIP);
                for (int j=0; j<=m_density; j++)
                {
                    float az = m_azMin + (j*azIncr);
                    float x = cos(elev)* sin(az);
                    float y = cos(elev)* cos(az);
                    float z = sin(elev);
                    gl.Normal3f(x, y, z);    
                    gl.Vertex3f(m_centre.x() + m_radius*x, 
                        m_centre.y() + m_radius*y,       
                        m_centre.z() + m_radius*z);             
                }
                gl.End();
            }
        }
#if 1
        if (drawBackSide)
        {
            //竖线
            for(int i=0; i+1<m_density; i++)
            {               
                float az = m_azMin + ((i+1)*azIncr);
                gl.Begin(GL_LINE_STRIP); 
                for (int j=0; j<=m_density; j++) 
                {                  
                    float elev = m_elevMin + (j*elevIncr);     
                    float x = cos(elev)*sin(az);
                    float y = cos(elev)*cos(az);
                    float z = sin(elev); 
                    gl.Normal3f(-x, -y, -z);
                    gl.Vertex3f(m_centre.x() + m_radius*x, 
                        m_centre.y() + m_radius*y,
                        m_centre.z() + m_radius*z);
                }
                gl.End();
            }
            //画横线
            for (int i=0; i+1<m_density; i++)
            {
                float elev = m_elevMin + ((i+1)*elevIncr);

                gl.Begin(GL_LINE_STRIP);
                for (int j=0; j<=m_density; j++)
                {
                    float az = m_azMin + (j*azIncr);
                    float x = cos(elev)* sin(az);
                    float y = cos(elev)* cos(az);
                    float z = sin(elev);
                    gl.Normal3f(-x, -y, -z);    
                    gl.Vertex3f(m_centre.x() + m_radius*x, 
                        m_centre.y() + m_radius*y,       
                        m_centre.z() + m_radius*z);             
                }
                gl.End();
            }
        }
#endif
    }
}

struct ActivateTransparencyOnType
{
    ActivateTransparencyOnType(const std::type_info& t): _t(t) {}

    void operator()(osg::ref_ptr<osg::Node>& nptr) const
    {
        const osg::Node* ptr = nptr.get();
        if(typeid(*ptr)==_t)
        {
            osg::Drawable* drawable = nptr->asDrawable();
            osg::StateSet* ss = drawable->getOrCreateStateSet();
            ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

            ss->setAttributeAndModes(new osg::CullFace(osg::CullFace::BACK),osg::StateAttribute::ON);
            ss->setMode(GL_BLEND,osg::StateAttribute::ON);

            drawable->dirtyDisplayList();
        }
    }

    const std::type_info&  _t;

protected:

    ActivateTransparencyOnType& operator = (const ActivateTransparencyOnType&) { return *this; }
};

struct DeactivateTransparencyOnType
{
    DeactivateTransparencyOnType(const std::type_info& t): _t(t) {}

    void operator()(osg::ref_ptr<osg::Node>& nptr) const
    {
        const osg::Node* ptr = nptr.get();
        if(typeid(*ptr)==_t)
        {
            osg::Drawable* drawable = nptr->asDrawable();
            osg::StateSet* ss = drawable->getOrCreateStateSet();
            if(ss) ss->setRenderingHint(osg::StateSet::OPAQUE_BIN);

            drawable->dirtyDisplayList();
        }
    }

    const std::type_info&  _t;

protected:

    DeactivateTransparencyOnType& operator = (const DeactivateTransparencyOnType&) { return *this; }
};
void CSphereSegment::setSurfaceLineColor( const osg::Vec4 &c )
{
    m_surfacelineColor=c;    
    if(c.w() != 1.0) std::for_each(_children.begin(), _children.end(), ActivateTransparencyOnType(typeid(CSurfaceLine)));
    else std::for_each(_children.begin(), _children.end(), DeactivateTransparencyOnType(typeid(CSurfaceLine)));
}

const osg::Vec4 & CSphereSegment::getSurfaceLineColor() const
{
    return m_surfacelineColor;
}

void CSphereSegment::setDrawSurfaceLine( bool b )
{
    m_bDrawSurfaceLine = b; 
    if (m_surfaceline.valid())
    {
        m_surfaceline->dirtyDisplayList();
        m_surfaceline->dirtyBound();
    }
}

const bool CSphereSegment::getDrawSurfaceLine() const
{
    return m_bDrawSurfaceLine;
}

void CSphereSegment::setAllColors( const osg::Vec4& c )
{
    osgSim::SphereSegment::setAllColors(c);
    setSurfaceLineColor(c);
}

void CSphereSegment::setArea( float azMin, float azMax,float elevMin, float elevMax )
{
    m_azMin=azMin;
    m_azMax=azMax;
    m_elevMin=elevMin;
    m_elevMax=elevMax;

    osgSim::SphereSegment::setArea(azMin, azMax, elevMin, elevMax);


}

void CSphereSegment::setSurfaceLineDensity( const unsigned int d )
{
    if (m_density == d)
    {
        return;
    }
    m_density = d;
    if (m_surfaceline.valid())
    {
        m_surfaceline->dirtyDisplayList();
        m_surfaceline->dirtyBound();
    }
    setDensity(m_density);
}
void CSphereSegment::dirtyAllDrawableDisplayLists()
{
    for(unsigned int i=0; i<getNumDrawables(); ++i)
    {
        osg::Drawable* drawable = getDrawable(i);
        if (drawable) drawable->dirtyDisplayList();
    }
}

void CSphereSegment::dirtyAllDrawableBounds()
{
    for(unsigned int i=0; i<getNumDrawables(); ++i)
    {
        osg::Drawable* drawable = getDrawable(i);
        if (drawable) drawable->dirtyBound();
    }
}

const unsigned int CSphereSegment::getSurfaceLineDensity() const
{
    return m_density;
}

void CSphereSegment::setSurfacelineLineWidth( float w )
{
    if (m_dLineWidth == w)
    {
        return;
    }
    m_dLineWidth = w;
    if (m_surfaceline.valid())
    {
        m_surfaceline->dirtyDisplayList();
        m_surfaceline->dirtyBound();
    }
}

const float CSphereSegment::getSurfacelineLineWidth() const
{
    return m_dLineWidth;
}

void CSphereSegment::setRadius( float r )
{
    if (m_radius == r)
    {
        return;
    }
    m_radius = r;
    osgSim::SphereSegment::setRadius(m_radius);
    if (m_surfaceline.valid())
    {
        m_surfaceline->dirtyDisplayList();
        m_surfaceline->dirtyBound();
    }
}

void CSphereSegment::setDrawMask( int dm )
{
    m_drawMask = dm; 
    dirtyAllDrawableDisplayLists();   
    dirtyAllDrawableBounds();  
    dirtyBound();
}

//CSurfaceLine.cpp
void CSurfaceLine::drawImplementation( osg::RenderInfo& renderInfo ) const
{
    _ss->SurfaceLine_drawImplementation(*renderInfo.getState());
}

osg::BoundingBox CSurfaceLine::computeBoundingBox() const
{
    osg::BoundingBox bbox;
    _ss->SurfaceLine_computeBound(bbox);
    return bbox;
}
}
//////////////////////////////////////////////////////////////////////////
