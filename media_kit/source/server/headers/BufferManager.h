struct _shared_buffer_list;

class BufferManager
{
public:
	BufferManager();
	~BufferManager();
	
	area_id		SharedBufferListID();
	
	status_t	RegisterBuffer(team_id teamid, media_buffer_id bufferid,
							   size_t *size, int32 *flags, size_t *offset, area_id *area);

	status_t	RegisterBuffer(team_id teamid, size_t size, int32 flags, size_t offset, area_id area,
							   media_buffer_id *bufferid);

	status_t	UnregisterBuffer(team_id teamid, media_buffer_id bufferid);


private:
	_shared_buffer_list *fBufferList;
	area_id	fAreaId;

};

