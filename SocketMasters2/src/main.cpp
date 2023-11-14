#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <Windows.h>
#include <iostream>

#define EXIT_PROCESS(msg) MessageBoxA(nullptr, msg, "Error!", MB_OKCANCEL); ExitProcess(EXIT_FAILURE)
#define WSA_VERIFY(expr, msg) if ((expr) != 0) { EXIT_PROCESS(msg); }

enum class AddressFamily
{
	None = 0, InternetIPv4 = AF_INET, InternetIPv6 = AF_INET6, Blutooth = AF_BTH,
};

enum class ConnectionType
{
	None = 0, Tcp = SOCK_STREAM, Udp = SOCK_RDM,
};

enum class InternetProtocol
{
	None = 0, Tcp = IPPROTO_TCP, Udp = IPPROTO_UDP,
};

class SocketMaster
{
public:
	void Init() { WSA_VERIFY(WSAStartup(MAKEWORD(2, 2), &wsa), "Failed to initialize WinSock"); }
	void Shutdown() { WSACleanup(); }

private:
	WSAData wsa;
};

class Server
{
public:
	void Serve(const char* hostName, uint16_t port) {
		host = gethostbyname(hostName);
		if (host == nullptr) {
			EXIT_PROCESS("Failed to locate host");
		}

		ZeroMemory(&sin, sizeof(sin));
		sin.sin_port = htons(port);
		sin.sin_family = AF_INET;
		memcpy(&sin.sin_addr.S_un.S_addr, host->h_addr_list[0], sizeof(sin.sin_addr.S_un.S_addr));
	}

	const SOCKADDR_IN& GetInfo() const { return sin; }

private:
	HOSTENT* host = nullptr;
	SOCKADDR_IN sin;
};

class Client
{
public:
	int OpenConnection(AddressFamily family, ConnectionType type, InternetProtocol protocol) {
		sock = socket((int)family, (int)type, (int)protocol);
		int result = sock < 0;
		return result;
	}

	void CloseConnection() {
		WSA_VERIFY(closesocket(sock), "Failed to close socket");
	}

	void Connect(Server& server) {
		const auto& info = server.GetInfo();
		WSA_VERIFY(connect(sock, (const sockaddr*)&info, sizeof(info)), "Failed to connect to host");
	}

	void Send(const char* message) {
		if (!send(sock, message, strlen(message), 0)) {
			EXIT_PROCESS("Failed to send packet");
		}
	}

	int Recieve(char* data, int datalen) {
		return recv(sock, data, datalen, 0);
	}

private:
	SOCKET sock = INVALID_SOCKET;
};

int main()
{
	SocketMaster ctx;
	Client client;
	Server server;

	const char szMsg[] = "HEAD / HTTP/1.0\r\n\r\n";
	constexpr auto bufSize = 4096;
	char szBuffer[bufSize];
	char szTemp[bufSize];

	ctx.Init();
	client.OpenConnection(AddressFamily::InternetIPv4, ConnectionType::Tcp, InternetProtocol::Tcp);
	server.Serve("www.google.com", 80);
	client.Connect(server);
	client.Send(szMsg);

	while (client.Recieve(szTemp, bufSize)) {
		strcat(szBuffer, szTemp);
	}
	
	client.CloseConnection();
	ctx.Shutdown();

	//std::cout << szBuffer << '\n';
	std::cin.get();

	ExitProcess(EXIT_SUCCESS);
}