#ifndef GOOGLEDEM_DATA_TILE_DATA_SOURCE___
#define GOOGLEDEM_DATA_TILE_DATA_SOURCE___

#include <Windows.h>
#include <vector>
#include <string>

using namespace std;

struct tile
{
	char tile_type;

	unsigned x;
	unsigned y;
	unsigned l;

	inline bool operator == (const tile& t) const { return x==t.x && y==t.y && l==t.l && tile_type==t.tile_type; }
};

struct tile_d
{
	tile t;
	int  state;//0 没被下载过，1正在下载，2下载成功。
};

struct tile_info_for_saved_to_disk
{
	tile t;
	int size;
	__int64 offset;
	char * data;
	int idx;
};

// 这个结构体兼顾硬盘上的数据和内存中的缓存
struct tile_data
{
	tile t;
	char * data;
	int size;
	__int64 offset;
	bool saved;
};

class CHgVectorDataSource
{
public:
	CHgVectorDataSource();

	~CHgVectorDataSource();

public:
	virtual tile_data GetTDTImageTile(tile tx);

protected:
	virtual tile_data DownLoadImageTile(tile t);
};


#endif // BGC_FAN_TILE_VECTOR_H