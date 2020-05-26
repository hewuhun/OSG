#include <sstream>
#include <TimeUtil.h>

namespace FeUtil
{

    void GetLocalTimeYMDHMS( int& nYear, int& nMonth, int& nDay, int& nHours, int& nMinute, int& nSecond )
    {
        time_t sysTime;
        time(&sysTime);
        tm* pLocalTime = localtime(&sysTime);
        if(pLocalTime)
        {
            nYear = pLocalTime->tm_year + 1900;
            nMonth = pLocalTime->tm_mon + 1;
            nDay = pLocalTime->tm_mday;
            nHours = pLocalTime->tm_hour;
            nMinute = pLocalTime->tm_min;
            nSecond = pLocalTime->tm_sec;
        }
    }

	void Convert2YMDHMS( time_t t, int& nYear, int& nMonth, int& nDay, int& nHours, int& nMinute, int& nSecond )
	{
		tm* pLocalTime = localtime(&t);
		if(pLocalTime)
		{
			nYear = pLocalTime->tm_year + 1900;
			nMonth = pLocalTime->tm_mon + 1;
			nDay = pLocalTime->tm_mday;
			nHours = pLocalTime->tm_hour;
			nMinute = pLocalTime->tm_min;
			nSecond = pLocalTime->tm_sec;
		}
	}

	void GetLocalTimeYMD( int& nYear, int& nMonth, int& nDay )
    {
        int nSecond;
        int nMinute;
        int nHours;
        GetLocalTimeYMDHMS(nYear, nMonth, nDay, nHours, nMinute, nSecond);
    }

    void GetLocalTimeHMS( int& nHours, int& nMinute, int& nSecond )
    {
        int nYear;
        int nMonth;
        int nDay;
        GetLocalTimeYMDHMS(nYear, nMonth, nDay, nHours, nMinute, nSecond);
    }

    void GetUTCTimeYMDHMS( int& nYear, int& nMonth, int& nDay, int& nHours, int& nMinute, int& nSecond )
    {
        time_t sysTime;
        time(&sysTime);
        tm* pLocalTime = gmtime(&sysTime);
        if(pLocalTime)
        {
            nYear = pLocalTime->tm_year;
            nMonth = pLocalTime->tm_mon;
            nDay = pLocalTime->tm_mday;
            nHours = pLocalTime->tm_hour;
            nMinute = pLocalTime->tm_min;
            nSecond = pLocalTime->tm_sec;
        }
    }

    void GetUTCTimeYMD( int& nYear, int& nMonth, int& nDay )
    {
        int nSecond;
        int nMinute;
        int nHours;
        GetUTCTimeYMDHMS(nYear, nMonth, nDay, nHours, nMinute, nSecond);
    }

    void GetUTCTimeHMS( int& nHours, int& nMinute, int& nSecond )
    {
        int nYear;
        int nMonth;
        int nDay;
        GetUTCTimeYMDHMS(nYear, nMonth, nDay, nHours, nMinute, nSecond);
    }

    tm GetLocalTime()
    {
        time_t sysTime;
        time(&sysTime);
        tm* pLocalTime = localtime(&sysTime);

        if(pLocalTime)
        {
            return *pLocalTime;
        }

        return tm();
    }

    tm GetUTCTime()
    {
        time_t sysTime;
        time( &sysTime );
        tm* pUTCTime = gmtime(&sysTime);
        if(pUTCTime)
        {
            return *pUTCTime;
        }

        return tm();
    }

    tm Local2UTC( tm localTM )
    {
        time_t sysTime = mktime(&localTM);
        tm* pUTCTime = ::gmtime( &sysTime );

        if ( pUTCTime )
        {
            return *pUTCTime;
        }

        return tm();
    }

    tm UTC2Local( tm UTCTM )
    {
        time_t sysTime = mktime(&UTCTM);
        tm* pLocalTime = ::localtime( &sysTime );

        if ( pLocalTime )
        {
            return *pLocalTime;
        }

        return tm();
    }

    tm DataTime2UTC( int nYear, int nMonth, int nDay, int nHours )
    {
        tm UTCTM = GetUTCTime();
        UTCTM.tm_year = nYear;
        UTCTM.tm_mon = nMonth;
        UTCTM.tm_mday = nDay;
        UTCTM.tm_hour = nHours;

        return UTCTM;
    }

    void UTC2DataTime( tm& UTCMT, int& nYear, int& nMonth, int& nDay, int& nHours )
    {
        nYear = UTCMT.tm_year;
        nMonth = UTCMT.tm_mon ;
        nDay = UTCMT.tm_mday;
        nHours = UTCMT.tm_hour;
    }

	bool IsAm( int nHours )
	{
		if(nHours >= 0 && nHours < 12)
		{
			return true;
		}

		return false;
	}

}


