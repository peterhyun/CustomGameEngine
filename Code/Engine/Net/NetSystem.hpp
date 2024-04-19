#pragma once
#include <string>
#include <queue>
class NamedProperties;
typedef NamedProperties EventArgs;

typedef enum class NetMode {
	NONE = 0,
	CLIENT,
	SERVER
} NetMode;

struct NetSystemConfig {
	NetMode m_netMode = NetMode::NONE;
	std::string m_hostAddressString;
	int m_sendBufferSize = 2048;
	int m_recvBufferSize = 2048;
};

typedef enum class ClientState
{
	INVALID,
	READY_TO_CONNECT,
	CONNECTING,
	CONNECTED,
} ClientState;

typedef enum class ServerState
{
	INVALID,
	LISTENING,
	CONNECTED,
} ServerState;

typedef std::vector<std::string> Strings;

class NetSystem {
public:
	NetSystem(NetSystemConfig const& netConfig);
	virtual ~NetSystem();
	static NetMode StringToNetMode(const std::string& netModeString);
	bool IsConnected() const;
	NetMode GetNetMode() const;

public:
	void			Startup();
	void			Shutdown();
	virtual void	BeginFrame();
	virtual void	EndFrame();

	static bool Event_RemoteCommand(EventArgs& args);
	static bool Event_BurstTest(EventArgs& args);

private:
	int FillSendBufferFromSendQueue();
	void AddStringToSendQueue(const std::string& string);
	void ClearBuffers();
	void ProcessReceivedCommands(const Strings& strings, bool isLastCharacterNullChar);

private:
	NetSystemConfig m_config;
	ClientState m_clientState = ClientState::INVALID;
	ServerState m_serverState = ServerState::INVALID;
	uintptr_t m_clientSocket = ~0ull;
	uintptr_t m_listenSocket = ~0ull;

	unsigned long m_hostAddress = 0;
	unsigned short m_hostPort = 0;

	char* m_sendBuffer = nullptr;
	char* m_recvBuffer = nullptr;

	std::queue<std::string> m_sendQueue;
	std::string m_recvRemaining;
};