#ifndef FE_SILVERLINING_DRAW
#define FE_SILVERLINING_DRAW 1

#include <osg/Drawable>
#include <osg/RenderInfo>
#include <osg/Version>
#include <FeSilverlining/Export.h>

namespace FeSilverLining
{
	class FeSilverLiningContext;

    /**
     * Custom drawable for rendering the SilverLining effects
     */
    class  FeSkyDrawable : public osg::Drawable
    {
    public:
        FeSkyDrawable(FeSilverLiningContext* SL =0L);
        META_Object(SilverLining, FeSkyDrawable);
     
		void accept(osg::NodeVisitor& nv);
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
        virtual ~FeSkyDrawable() { }

        osg::observer_ptr<FeSilverLiningContext> _SL;
        
        FeSkyDrawable(const FeSkyDrawable& copy, const osg::CopyOp& op=osg::CopyOp::SHALLOW_COPY) { }
    };

}

#endif
