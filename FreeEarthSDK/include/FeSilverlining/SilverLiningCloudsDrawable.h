#ifndef FE_SILVERLINING_DA
#define FE_SILVERLINING_DA 1

#include <osg/Drawable>
#include <osg/RenderInfo>
#include <osg/Version>
#include <osgEarth/NativeProgramAdapter>
#include <vector>
#include <map>

#include <FeSilverlining/Export.h>

namespace FeSilverLining
{
	class FeSilverLiningContext;

    /**
     * Custom drawable for rendering the SilverLining clouds
     */
    class  FeCloudsDrawable : public osg::Drawable
    {
    public:
        FeCloudsDrawable(FeSilverLiningContext* SL =0L);
        META_Object(SilverLining, FeCloudsDrawable);

		/* Sets whether to draw this item */
		void setDraw(bool draw);
     
    public: // osg::Drawable

        // custom draw (called with an active GC)
        void drawImplementation(osg::RenderInfo& ri) const;
        
        // custom bounds computation
#if OSG_VERSION_GREATER_THAN(3,3,1)
        osg::BoundingBox computeBoundingBox() const;
#else
        osg::BoundingBox computeBound() const;
#endif

    protected:
        virtual ~FeCloudsDrawable() { }

        osg::observer_ptr<FeSilverLiningContext> _SL;

        mutable osg::buffered_object<osgEarth::NativeProgramAdapterCollection> _adapters;
        
        FeCloudsDrawable(const FeCloudsDrawable& copy, const osg::CopyOp& op=osg::CopyOp::SHALLOW_COPY) { }
    };
}
    

#endif

