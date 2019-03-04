#include "RequestBroker.h"

class GameSave;
class ThumbRenderRequest: public RequestBroker::Request
{
public:
	GameSave * Save;
	int Width, Height;
	bool Decorations;
	bool Fire;
	bool autoRescale;

	ThumbRenderRequest(GameSave * save, bool decorations, bool fire, int width, int height, bool autoRescale, ListenerHandle listener, int identifier = 0);
	virtual RequestBroker::ProcessResponse Process(RequestBroker & rb);
	virtual ~ThumbRenderRequest();
	virtual void Cleanup();
};

