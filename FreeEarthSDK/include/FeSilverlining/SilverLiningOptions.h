
#ifndef FE_SILVERLINING_OPTIONS
#define FE_SILVERLINING_OPTIONS 1

#include <osgEarthUtil/Sky>
#include <FeSilverlining/Export.h>

namespace FeSilverLining
{
	class   FeSilverLiningOptions : public osgEarth::Util::SkyOptions
	{
	public:
		FeSilverLiningOptions(const osgEarth::Util::SkyOptions& options =osgEarth::Util::SkyOptions()) :
		  osgEarth::Util::SkyOptions(options),
			  _drawClouds(false),
			  _cloudsMaxAltitude(20000)
		  {
			  //setDriver( "hgsilverlining" );
			  fromConfig( _conf );
		  }
		  virtual ~FeSilverLiningOptions() { }

	public: // properties

		/* User name for license activation */
		osgEarth::optional<std::string>& user() { return _user; }
		const osgEarth::optional<std::string>& user() const { return _user; }

		/* License code string */
		osgEarth::optional<std::string>& licenseCode() { return _licenseCode; }
		const osgEarth::optional<std::string>& licenseCode() const { return _licenseCode; }

		/* SilverLining resource path */
		osgEarth::optional<std::string>& resourcePath() { return _resourcePath; }
		const osgEarth::optional<std::string>& resourcePath() const { return _resourcePath; }

		/* Whether to draw clouds */
		osgEarth::optional<bool>& drawClouds() { return _drawClouds; }
		const osgEarth::optional<bool>& drawClouds() const { return _drawClouds; }

		/* Max altitude at which to draw/update clouds */
		osgEarth::optional<double>& cloudsMaxAltitude() { return _cloudsMaxAltitude; }
		const osgEarth::optional<double>& cloudsMaxAltitude() const { return _cloudsMaxAltitude; }

	public:
		osgEarth::Config getConfig() const {
			osgEarth::Config conf = osgEarth::Util::SkyOptions::getConfig();
			conf.addIfSet("user", _user);
			conf.addIfSet("license_code", _licenseCode);
			conf.addIfSet("resource_path", _resourcePath);
			conf.addIfSet("clouds", _drawClouds);
			conf.addIfSet("clouds_max_altitude", _cloudsMaxAltitude);
			return conf;
		}

	protected:
		void mergeConfig( const osgEarth::Config& conf ) {
			osgEarth::Util::SkyOptions::mergeConfig( conf );
			fromConfig(conf);
		}

	private:
		void fromConfig( const osgEarth::Config& conf ) {
			conf.getIfSet("user", _user);
			conf.getIfSet("license_code", _licenseCode);
			conf.getIfSet("resource_path", _resourcePath);
			conf.getIfSet("clouds", _drawClouds);
			conf.getIfSet("clouds_max_altitude", _cloudsMaxAltitude);
		}

		osgEarth::optional<std::string> _user;
		osgEarth::optional<std::string> _licenseCode;
		osgEarth::optional<std::string> _resourcePath;
		osgEarth::optional<bool>        _drawClouds;
		osgEarth::optional<double>      _cloudsMaxAltitude;
		int                             _lastCullFrameNumber;
	};

}
#endif // FE_DRIVER_SILVERLINING_OPTIONS

