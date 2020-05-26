
#include <FeSilverlining/SilverLiningOptions.h>
#include <FeSilverlining/SilverLiningNode.h>
#include <osgEarth/MapNode>
#include <osgEarthUtil/Sky>
#include <osgDB/FileNameUtils>
#include <FeSilverlining/Export.h>

#define LC "[FeSilverLiningDriver] "

namespace FeSilverLining
{
	class  FeSilverLiningDriver : public osgEarth::Util::SkyDriver
	{
	public:
		FeSilverLiningDriver()
		{
			supportsExtension(
				"osgearth_sky_hgsilverlining",
				"FePlugin" );
		}

		const char* className()
		{
			return "FePlugin";
		}

		ReadResult readNode(const std::string& file_name, const osgDB::Options* options) const
		{
			if ( !acceptsExtension(osgDB::getLowerCaseFileExtension( file_name )))
				return ReadResult::FILE_NOT_HANDLED;

			FeSilverLiningOptions slOptions = getSkyOptions(options);

			// if the Resource Path isn't set, attempt to set it from 
			// the SL environment variable.
			if ( !slOptions.resourcePath().isSet() )
			{
				const char* ev = ::getenv("SILVERLINING_PATH");
				if ( ev )
				{
					slOptions.resourcePath() = osgDB::concatPaths(
						std::string(ev),
						"Resources" );
				}
				else
				{
					OE_WARN << LC
						<< "No resource path! SilverLining might not initialize properly. "
						<< "Consider setting the SILVERLINING_PATH environment variable."
						<< std::endl;
				}
			}

			osgEarth::MapNode* mapnode = getMapNode(options);
			const osgEarth::Map* map = mapnode ? mapnode->getMap() : 0L;
			return new FeSilverLiningNode( map, slOptions );
		}

	protected:
		virtual ~FeSilverLiningDriver() { }
	};

	REGISTER_OSGPLUGIN(osgearth_sky_hgsilverlining, FeSilverLiningDriver)

}

    
