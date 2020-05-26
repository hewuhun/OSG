#include <HgVectorImageDataSource.h>

#include <osgEarth/HTTPClient>

static std::string FormatURL(int x,int y,int z)
{
	char str[512];
	int server_number = rand()%7;
	char *UrlFormat = "http://t%d.tianditu.cn/cia_c/wmts?SERVICE=WMTS&REQUEST=GetTile&VERSION=1.0.0&LAYER=cia&STYLE=default&TILEMATRIXSET=c&TILEMATRIX=%d&TILEROW=%d&TILECOL=%d&FORMAT=tiles";
	sprintf(str,UrlFormat,server_number,z,y,x);
	return str;
}

CHgVectorDataSource::CHgVectorDataSource()
{

}

CHgVectorDataSource::~CHgVectorDataSource()
{

}

tile_data CHgVectorDataSource::DownLoadImageTile(tile t)
{
	tile_data t_d;
	t_d.data = 0;
	t_d.saved = false;
	t_d.size = 0;
	t_d.t = t;

	char *png_data;
	int size;

	//if(!osgEarth::HTTPClient::get_data_buf_from_url(FormatURL(t.x, t.y,t.l+1),size,png_data))
		//return t_d;

	osgEarth::HTTPClient http;

	std::string url = FormatURL(t.x, t.y,t.l+1);

	osgEarth::HTTPResponse response = http.get( url );

	if ( response.isOK() )
	{
		std::string content = response.getPartAsString(0);
		t_d.data = new char[content.size()];
		t_d.size = content.size();
		memcpy(t_d.data,content.c_str(),t_d.size);
	}
	else
	{
		return t_d;
	}

	return t_d;
}

tile_data CHgVectorDataSource::GetTDTImageTile( tile tx )
{
	tile_data t;
	t.size = 0;
	t.saved = false;


	//t = disk_cache.find_tile(tx);

	//if(t.size>0)
	//{
	//	return t;
	//}


	t = DownLoadImageTile(tx);
	if(t.size>0)
	{
		tile_data t_d;
		t_d.saved = false;
		t_d.t = tx;
		t_d.size = t.size;
		t_d.data = new char[t_d.size];
		memcpy(t_d.data,t.data,t.size);

		//disk_cache.save_to_disk_cache(t_d);
		return t;
	}

	return t;
}

