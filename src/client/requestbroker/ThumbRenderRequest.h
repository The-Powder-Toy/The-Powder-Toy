#include "RequestBroker.h"

class GameSave;
class ThumbRenderRequest: public RequestBroker::Request
{
public:
	int Width, Height;
	bool Decorations;
	bool Fire;
	GameSave * Save;
	ThumbRenderRequest(GameSave * save, bool decorations, bool fire, int width, int height, ListenerHandle listener, int identifier = 0);
	virtual RequestBroker::ProcessResponse Process(RequestBroker & rb);
	virtual ~ThumbRenderRequest();
	virtual void Cleanup();
};

