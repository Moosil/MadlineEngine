#include "game_engine.h"

int Madline::startGame(Madline::Engine& gameEngine) {
	Madline::Window gameWindow{};

	Madline::GraphicsEngine graphicsEngine{};
	
	if (!gameEngine.initGameplay()) {
		gameWindow.stopRunning();
		return -1;
	}
	
	graphicsEngine.init(gameWindow);

	while (gameWindow.isRunning()) {
		gameWindow.gameLoop();
		float dt = gameWindow.getDeltaTime();
		float adt = (dt > 1.f / (float) gameWindow.getMinFps()) ? 1.f / (float) gameWindow.getMinFps() : dt;// augmented delta time

		if (!gameEngine.process(adt, gameWindow)) {
			gameWindow.stopRunning();
		}
		
		graphicsEngine.draw();

		if (!gameWindow.isFocused()) {
			resetInput(gameWindow.getInput());
		}

		processInputAfter(gameWindow.getInput());
	}

	gameEngine.onCloseGame();
	graphicsEngine.cleanup();

	return 0;
}

std::string getLastErrorString() {
	unsigned long err = GetLastError();

	if (err == ERROR_SUCCESS) {
		return "";
	}

	char *message = nullptr;

	size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
	                            nullptr, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
	                            (char *) &message, 0, nullptr);

	std::string rez(message, size);
	return rez;
}

void errorPrint(const std::string &message) {
	std::cout << "line " << __LINE__ << ": " << message.c_str()
	          << ". Error code " << GetLastError() << ": " << getLastErrorString().c_str();
}

size_t getFileSize(const std::string &name) {
	HANDLE file = CreateFile(name.c_str(),
	                         GENERIC_READ, FILE_SHARE_READ, nullptr,
	                         OPEN_EXISTING,
	                         FILE_ATTRIBUTE_NORMAL, nullptr);

	if (file == INVALID_HANDLE_VALUE) {
		errorPrint("Failed to get file " + name + "\'s size");
		return 0;
	}

	LARGE_INTEGER fileSize;
	if (!GetFileSizeEx(file, &fileSize)) {
		CloseHandle(file);
		return 0;
	}

	CloseHandle(file);
	return fileSize.QuadPart;
}

bool readEntireFile(const std::string &name, std::vector<char> &data) {
	data.clear();

	HANDLE file = CreateFile(name.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
	                         OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

	if (file == INVALID_HANDLE_VALUE) {
		errorPrint("Failed to read file " + name);
		return false;
	}

	LARGE_INTEGER fileSize;
	if (!GetFileSizeEx(file, &fileSize)) {
		CloseHandle(file);
		return false;
	}

	data.resize(fileSize.QuadPart);

	unsigned long bytesRead = 0;

	bool rez = ReadFile(file, data.data(), fileSize.QuadPart, &bytesRead, nullptr);

	CloseHandle(file);

	if (!rez || bytesRead != fileSize.QuadPart) {
		return false;
	} else {
		return true;
	}
}

bool writeEntireFile(const std::string &name, void *buffer, size_t s) {
	HANDLE file = CreateFile(name.c_str(), GENERIC_WRITE, 0, nullptr,
	                         CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

	if (file == INVALID_HANDLE_VALUE) {
		errorPrint("Failed to write to file " + name);
		return false;
	}

	if (!buffer || s == 0) {
		CloseHandle(file);
		return true;
	}

	unsigned long bytesWritten = 0;
	bool success = WriteFile(file, buffer, s, &bytesWritten, nullptr);

	if (bytesWritten != s) {
		success = false;
	}

	CloseHandle(file);
	return success;
}