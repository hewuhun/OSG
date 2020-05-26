
#include <FeUtils/Version>
#include <string>
#include <stdio.h>

extern "C" {

const char* FREEEARTHGetVersion()
{
    static char FREEEARTH_version[256];
    static int FREEEARTH_version_init = 1;
    if (FREEEARTH_version_init)
    {
        if (FREEEARTH_RC_VERSION == 0 )
        {
            sprintf(FREEEARTH_version,"%d.%d.%d (%s)",
                FREEEARTH_MAJOR_VERSION,
                FREEEARTH_MINOR_VERSION,
                FREEEARTH_PATCH_VERSION,
                FREEEARTHGitSHA1() );
        }
        else
        {
            sprintf(FREEEARTH_version,"%d.%d.%d RC%d (%s)",
                FREEEARTH_MAJOR_VERSION,
                FREEEARTH_MINOR_VERSION,
                FREEEARTH_PATCH_VERSION,
                FREEEARTH_RC_VERSION,
                FREEEARTHGitSHA1() );
        }

        FREEEARTH_version_init = 0;
    }
    
    return FREEEARTH_version;
}

const char* FREEEARTHGetSOVersion()
{
    static char FREEEARTH_soversion[32];
    static int FREEEARTH_soversion_init = 1;
    if (FREEEARTH_soversion_init)
    {
        sprintf(FREEEARTH_soversion,"%d",FREEEARTH_SOVERSION);
        FREEEARTH_soversion_init = 0;
    }
    
    return FREEEARTH_soversion;
}

const char* FREEEARTHGetLibraryName()
{
    return "FREEEARTH Library";
}

}
