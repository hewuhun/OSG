#ifndef GROUNDMANIPULATOR_H
#define GROUNDMANIPULATOR_H

#include <osgGA/FirstPersonManipulator>

#include <FeKits/Export.h>

namespace FeKit
{

    class FEKIT_EXPORT CGroundManipulator : public osgGA::FirstPersonManipulator
    {
    public:
        CGroundManipulator(void);
        ~CGroundManipulator(void);

        bool performMovementLeftMouseButton( const double /*eventTimeDelta*/, const double dx, const double dy );
        bool handleMouseWheel( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us );
        bool handleKeyDown (const osgGA::GUIEventAdapter &  ea,  osgGA::GUIActionAdapter &  us  );


        bool isHitByFrontBackLeftRight(int direction);

    private:
        float _speed;

        enum directions
        {
            Front = 1,
            Back = 2,
            Left = 3,
            Right = 4
        };

    };

}
#endif

