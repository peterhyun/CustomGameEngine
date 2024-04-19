#include "Engine/Net/NetSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/EngineCommon.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#include "Game/EngineBuildPreferences.hpp"

#ifdef ENGINE_ENABLE_NET

NetSystem* g_theNetSystem = nullptr;

NetSystem::NetSystem(NetSystemConfig const& netConfig) : m_config(netConfig)
{
}

NetSystem::~NetSystem()
{
}

NetMode NetSystem::StringToNetMode(const std::string& netModeString)
{
	if (netModeString == "None") {
		return NetMode::NONE;
	}
	else if (netModeString == "Server") {
		return NetMode::SERVER;
	}
	else if (netModeString == "Client") {
		return NetMode::CLIENT;
	}
	ERROR_AND_DIE("Invalid String to get NetMode from!");
}

bool NetSystem::IsConnected() const
{
	if (m_config.m_netMode == NetMode::CLIENT) {
		if (m_clientState == ClientState::CONNECTED) {
			return true;
		}
		else {
			return false;
		}
	}
	else if (m_config.m_netMode == NetMode::SERVER) {
		if (m_serverState == ServerState::CONNECTED) {
			return true;
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
}

NetMode NetSystem::GetNetMode() const
{
	return m_config.m_netMode;
}

void NetSystem::Startup()
{
	if (m_config.m_netMode == NetMode::NONE) {
		return;
	}

	g_theEventSystem->SubscribeEventCallbackFunction("RemoteCommand", &NetSystem::Event_RemoteCommand);
	g_theEventSystem->SubscribeEventCallbackFunction("BurstTest", &NetSystem::Event_BurstTest);

	m_sendBuffer = new char[m_config.m_sendBufferSize];
	memset(m_sendBuffer, 0, m_config.m_sendBufferSize);
	m_recvBuffer = new char[m_config.m_recvBufferSize];
	memset(m_recvBuffer, 0, m_config.m_recvBufferSize);

	size_t semicolonPos = m_config.m_hostAddressString.find(':');
	if (semicolonPos == std::string::npos) {
		ERROR_AND_DIE("Net Host Address doesn't have the port info!");
	}
	std::string hostAddressStringWithoutPort = m_config.m_hostAddressString.substr(0, semicolonPos);
	std::string hostAddressStringJustPort = m_config.m_hostAddressString.substr(semicolonPos + 1, (size_t)-1);
	m_hostPort = (unsigned short)(atoi(hostAddressStringJustPort.c_str()));

	WSADATA data = {};
	int result = WSAStartup(MAKEWORD(2, 2), &data);
	if (result != 0) {
		ERROR_AND_DIE(Stringf("WSAStartup was not successful! Error code: %d", result));
	}
	if (m_config.m_netMode == NetMode::CLIENT) {
		m_clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		unsigned long blockingMode = 1;
		result = ioctlsocket(m_clientSocket, FIONBIO, &blockingMode);	//Non-blocking
		if (result != 0) {
			ERROR_AND_DIE(Stringf("ioctlsocket was not successful! Error code: %d", result));
		}

		IN_ADDR addr = {};
		result = inet_pton(AF_INET, hostAddressStringWithoutPort.c_str(), &addr);
		if (result != 1) {
			ERROR_AND_DIE(Stringf("inet_pton was not successful! Error code: %d", result));
		}
		m_hostAddress = ntohl(addr.S_un.S_addr);

		m_clientState = ClientState::READY_TO_CONNECT;
	}
	else if (m_config.m_netMode == NetMode::SERVER) {
		m_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		unsigned long blockingMode = 1;
		result = ioctlsocket(m_listenSocket, FIONBIO, &blockingMode);	//Non-blocking
		if (result != 0) {
			ERROR_AND_DIE(Stringf("ioctlsocket was not successful! Error code: %d", result));
		}
		m_hostAddress = INADDR_ANY;

		sockaddr_in addr = {};
		addr.sin_family = AF_INET;
		addr.sin_addr.S_un.S_addr = htonl(m_hostAddress);
		addr.sin_port = htons(m_hostPort);
		result = bind(m_listenSocket, (sockaddr*)&addr, (int)sizeof(addr));
		if (result != 0) {
			ERROR_AND_DIE(Stringf("Binding listen socket was unsuccessful! Error Code: %d", result));
		}
		result = listen(m_listenSocket, SOMAXCONN);
		if (result != 0) {
			ERROR_AND_DIE(Stringf("listen was not successful! Error code: %d", result));
		}

		m_serverState = ServerState::LISTENING;
	}
}

void NetSystem::Shutdown()
{
	if (m_config.m_netMode == NetMode::NONE) {
		return;
	}

	closesocket(m_clientSocket);
	if (m_config.m_netMode == NetMode::SERVER) {
		closesocket(m_listenSocket);
	}
	WSACleanup();

	delete m_sendBuffer;
	m_sendBuffer = nullptr;
	delete m_recvBuffer;
	m_recvBuffer = nullptr;
}

void NetSystem::BeginFrame()
{
	if (m_config.m_netMode == NetMode::CLIENT) {
		switch (m_clientState) {
			case ClientState::READY_TO_CONNECT: {
				sockaddr_in addr = {};
				addr.sin_family = AF_INET;
				addr.sin_addr.S_un.S_addr = htonl(m_hostAddress);
				addr.sin_port = htons(m_hostPort);
				int result = connect(m_clientSocket, (sockaddr*)(&addr), (int)sizeof(addr));	//blocking function
				if (result == SOCKET_ERROR) {
					int error = WSAGetLastError();
					if (error == WSAEISCONN) {
						m_clientState = ClientState::CONNECTING;
					}
					else if (error != WSAEALREADY && error != WSAEWOULDBLOCK) {
						ERROR_AND_DIE(Stringf("connect caused error: %d", error));
					}
				}
				else {
					m_clientState = ClientState::CONNECTING;
				}
				break;
			}
			case ClientState::CONNECTING: {
				fd_set sockets = {};
				FD_ZERO(&sockets);
				FD_SET(m_clientSocket, &sockets);
				timeval waitTime = {};
				int result = select(0, NULL, &sockets, NULL, &waitTime);
				if (result == SOCKET_ERROR) {
					int error = WSAGetLastError();
					if (error != WSAEWOULDBLOCK) {
						ERROR_AND_DIE(Stringf("select caused error: %d", error));
					}
				}
				else {
					if(result > 0 && FD_ISSET(m_clientSocket, &sockets))
						m_clientState = ClientState::CONNECTED;
				}
				break;
			}
			case ClientState::CONNECTED: {
				if (m_sendQueue.size() > 0) {
					int numBytesToSendIncludingNullChar = FillSendBufferFromSendQueue();
					int result = send(m_clientSocket, m_sendBuffer, numBytesToSendIncludingNullChar, 0);
					if (result == SOCKET_ERROR) {
						int error = WSAGetLastError();
						if (error == WSAECONNABORTED || error == WSAECONNRESET) {
							ClearBuffers();
							m_clientState = ClientState::READY_TO_CONNECT;
						}
						else if (error != WSAEWOULDBLOCK) {
							ERROR_AND_DIE(Stringf("send caused error code %d", error));
						}
					}
				}

				int result = recv(m_clientSocket, m_recvBuffer, sizeof(m_recvBuffer), 0);
				if (result == SOCKET_ERROR) {
					int error = WSAGetLastError();
					if (error == WSAECONNABORTED || error == WSAECONNRESET) {
						ClearBuffers();
						m_clientState = ClientState::READY_TO_CONNECT;
					}
					else if (error != WSAEWOULDBLOCK) {
						ERROR_AND_DIE(Stringf("recv caused error code %d", error));
					}
				}
				else if (result == 0) {
					ClearBuffers();
					m_clientState = ClientState::READY_TO_CONNECT;
					return;
				}
				else {
					std::string receivedString(m_recvBuffer, result);
					Strings commands = SplitStringOnDelimeter(receivedString, '\0');
					bool isLastCharacterNullChar = (receivedString.length() > 0) ? (receivedString.back() == '\0') : false;
					ProcessReceivedCommands(commands, isLastCharacterNullChar);
					memset(m_recvBuffer, 0, m_config.m_recvBufferSize);
				}

				//memset(m_sendBuffer, 0, m_config.m_sendBufferSize);
			}
		}
	}
	else if (m_config.m_netMode == NetMode::SERVER) {
		if (m_serverState == ServerState::LISTENING) {
			uintptr_t socket = accept(m_listenSocket, nullptr, nullptr);	//This is a blocking function
			if (socket == INVALID_SOCKET) {
				int error = WSAGetLastError();
				if (error != WSAEWOULDBLOCK) {
					ERROR_AND_DIE(Stringf("accept caused error code %d", error));
				}
			}
			else {
				m_clientSocket = socket;
				unsigned long blockingMode = 1;
				int result = ioctlsocket(m_clientSocket, FIONBIO, &blockingMode);
				if (result != 0) {
					ERROR_AND_DIE(Stringf("ioctlsocket caused error code %d", result));
				}
				m_serverState = ServerState::CONNECTED;
			}
		}
		if (m_serverState == ServerState::CONNECTED) {
			if (m_sendQueue.size() > 0) {
				int numBytesToSendIncludingNullChar = FillSendBufferFromSendQueue();
				int result = send(m_clientSocket, m_sendBuffer, numBytesToSendIncludingNullChar, 0);	//blocking!. +1 needed as strlen() doesn't count the null character at the end.
				if (result == SOCKET_ERROR) {
					int error = WSAGetLastError();
					if (error == WSAECONNABORTED || error == WSAECONNRESET) {
						ClearBuffers();
						m_serverState = ServerState::LISTENING;
					}
					else if (error != WSAEWOULDBLOCK) {
						ERROR_AND_DIE(Stringf("send caused error code %d", error));
					}
				}
			}

			int result = recv(m_clientSocket, m_recvBuffer, sizeof(m_recvBuffer), 0);	//blocking!
			if (result == SOCKET_ERROR) {
				int error = WSAGetLastError();
				if (error == WSAECONNABORTED || error == WSAECONNRESET) {
					ClearBuffers();
					m_serverState = ServerState::LISTENING;
				}
				else if (error != WSAEWOULDBLOCK) {
					ERROR_AND_DIE(Stringf("recv caused error code %d", error));
				}
			}
			else if (result == 0) {
				ClearBuffers();
				m_serverState = ServerState::LISTENING;
				return;
			}
			else {
				std::string receivedString(m_recvBuffer, result);
				Strings commands = SplitStringOnDelimeter(receivedString, '\0');
				bool isLastCharacterNullChar = (receivedString.length() > 0) ? (receivedString.back() == '\0') : false;
				ProcessReceivedCommands(commands, isLastCharacterNullChar);
				memset(m_recvBuffer, 0, m_config.m_recvBufferSize);
			}

			//memset(m_sendBuffer, 0, m_config.m_sendBufferSize);
		}
	}
}

void NetSystem::EndFrame()
{
}

bool NetSystem::Event_RemoteCommand(EventArgs& args)
{
	std::string commandString = args.GetValue("Command", std::string("INVALID"));
	TrimString(commandString, '"');
	g_theNetSystem->AddStringToSendQueue(commandString);
	return true;
}

bool NetSystem::Event_BurstTest(EventArgs& args)
{
	UNUSED(args);
	std::string burstCommands;
	for (int i = 1; i <= 20; i++) {
		burstCommands += Stringf("Echo Message=%d", i);
		burstCommands += '\0';
	}
	g_theNetSystem->AddStringToSendQueue(burstCommands);
	return true;
}

int NetSystem::FillSendBufferFromSendQueue()
{
	if (m_sendQueue.size() == 0) return 0;

	const std::string& stringToSend = m_sendQueue.front();

	//size_t stringToSendSizeDebug = stringToSend.size();
	//size_t stringToSendLengthDebug = stringToSend.length();

	if (stringToSend.length() > m_config.m_sendBufferSize) {
		ERROR_AND_DIE("String to send is longer than the send buffer size (Remainder should've been cut and pushed in the send queue)");
	}
	/*
	if (stringToSend.back() != '\0') {
		ERROR_AND_DIE("All strings in the send queue should terminate with '\0'!");
	}
	*/

	int lengthOfStringToSend = (int)stringToSend.length();

	//Send queued strings already have null characters as their terminators (can also be in the middle of the string)
	for (int i = 0; i < lengthOfStringToSend; i++) {
		m_sendBuffer[i] = stringToSend[i];
	}

	m_sendQueue.pop();

	return lengthOfStringToSend;
}

void NetSystem::AddStringToSendQueue(const std::string& stringToPush)
{
	if (stringToPush.empty())
		return;

	std::string actualStringToPush = stringToPush;
	if (actualStringToPush.back() != '\0') {
		actualStringToPush.push_back('\0');
	}

	/*
	if (actualStringToPush.length() < (size_t)(m_config.m_sendBufferSize)) {
		g_theNetSystem->m_sendQueue.push(actualStringToPush);
	}
	else {
		g_theNetSystem->m_sendQueue.push(actualStringToPush.substr(0, (size_t)m_config.m_sendBufferSize));
		g_theNetSystem->m_sendQueue.push(actualStringToPush.substr((size_t)m_config.m_sendBufferSize, (size_t)-1));
	}
	*/
	size_t bufferSize = (size_t)(m_config.m_sendBufferSize);
	size_t strLength = actualStringToPush.length();
	size_t startIndex = 0;
	while (startIndex < strLength) {
		size_t remainingLength = strLength - startIndex;
		size_t substringLength = min(bufferSize, remainingLength);

		std::string substring = actualStringToPush.substr(startIndex, substringLength);
		g_theNetSystem->m_sendQueue.push(substring);

		startIndex += substringLength;
	}
}

void NetSystem::ClearBuffers()
{
	delete[] m_sendBuffer;
	m_sendBuffer = new char[m_config.m_sendBufferSize];
	delete[] m_recvBuffer;
	m_recvBuffer = new char[m_config.m_recvBufferSize];
	std::queue<std::string> emptyQueue;
	std::swap(m_sendQueue, emptyQueue);
	m_recvRemaining.clear();
}

void NetSystem::ProcessReceivedCommands(const Strings& strings, bool isLastCharacterNullChar)
{
	size_t numberOfCommands = strings.size();
	if (numberOfCommands == 0) {
		return;
	}

	std::string firstCommandToProcess = m_recvRemaining + strings[0];
	m_recvRemaining = "";
	if (numberOfCommands == 1 && !isLastCharacterNullChar) {
		m_recvRemaining = firstCommandToProcess;
		return;
	}
	else if (firstCommandToProcess != "") {
		g_theDevConsole->Execute(firstCommandToProcess);
		if (m_config.m_netMode == NetMode::SERVER) {
			EventArgs args;
			args.SetValue("Command", Stringf("\"Echo Message=\"Executed remote console command: %s\"\"", firstCommandToProcess.c_str()));
			Event_RemoteCommand(args);
		}
	}

	if (numberOfCommands == 1)
		return;

	for (int commandIdx = 1; commandIdx < numberOfCommands - 1; commandIdx++) {
		g_theDevConsole->Execute(strings[commandIdx]);
		if (m_config.m_netMode == NetMode::SERVER) {
			EventArgs args;
			args.SetValue("Command", Stringf("\"Echo Message=\"Executed remote console command: %s\"\"", strings[commandIdx].c_str()));
			Event_RemoteCommand(args);
		}
	}

	std::string lastCommandToProcess = strings[numberOfCommands - 1];
	if (!isLastCharacterNullChar) {
		m_recvRemaining = lastCommandToProcess;
	}
	else if (lastCommandToProcess != "") {
		g_theDevConsole->Execute(lastCommandToProcess);
		if (m_config.m_netMode == NetMode::SERVER) {
			EventArgs args;
			args.SetValue("Command", Stringf("\"Echo Message=\"Executed remote console command: %s\"\"", lastCommandToProcess.c_str()));
			Event_RemoteCommand(args);
		}
	}
}

#endif