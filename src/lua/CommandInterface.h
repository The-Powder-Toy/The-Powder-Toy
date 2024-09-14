#pragma once
#include "CommandInterfacePtr.h"
#include "common/ExplicitSingleton.h"
#include "common/String.h"
#include "gui/game/GameControllerEvents.h"
#include "TPTSTypes.h"
#include <deque>
#include <optional>

class GameModel;
class GameController;
class Tool;

class CommandInterface : public ExplicitSingleton<CommandInterface>
{
protected:
	String lastError;
	GameModel * m;
	GameController * c;


	int PlainCommand(String command);
	String PlainFormatCommand(String command);

public:
	CommandInterface(GameController *newGameController, GameModel *newGameModel);

	enum LogType { LogError, LogWarning, LogNotice };
	enum FormatType { FormatInt, FormatString, FormatChar, FormatFloat, FormatElement };
	int GetPropertyOffset(ByteString key, FormatType & format);
	void Log(LogType type, String message);
	//void AttachGameModel(GameModel * m);

	void OnTick();
	void Init();

	bool HandleEvent(const GameControllerEvent &event);
	bool HaveSimGraphicsEventHandlers();

	int Command(String command);
	String FormatCommand(String command);
	void SetLastError(String err)
	{
		lastError = err;
	}
	String GetLastError();

	AnyType eval(std::deque<String> * words);
	int parseNumber(String str);
	AnyType tptS_set(std::deque<String> * words);
	AnyType tptS_create(std::deque<String> * words);
	AnyType tptS_delete(std::deque<String> * words);
	AnyType tptS_load(std::deque<String> * words);
	AnyType tptS_reset(std::deque<String> * words);
	AnyType tptS_bubble(std::deque<String> * words);
	AnyType tptS_quit(std::deque<String> * words);
	ValueType testType(String word);

	void SetToolIndex(ByteString identifier, std::optional<int> index);

	static CommandInterfacePtr Create(GameController *newGameController, GameModel *newGameModel);
};
