#pragma once
#include <iostream>
#include <vector>

#define LOG_LEVEL SimpleFilesLogger::LVL_DEBUG
#define LOGGER(level, fmt, ...)		SimpleFilesLogger::PrintToConsole(level, fmt, ##__VA_ARGS__)
#define LOGGER_CRIT(fmt, ...)		SimpleFilesLogger::PrintToConsole(SimpleFilesLogger::LVL_CRIT, fmt, ##__VA_ARGS__)
#define LOGGER_WARN(fmt, ...)		SimpleFilesLogger::PrintToConsole(SimpleFilesLogger::LVL_WARN, fmt, ##__VA_ARGS__)
#define LOGGER_NOTICE(fmt, ...)		SimpleFilesLogger::PrintToConsole(SimpleFilesLogger::LVL_NOTICE, fmt, ##__VA_ARGS__)
#define LOGGER_INFO(fmt, ...)		SimpleFilesLogger::PrintToConsole(SimpleFilesLogger::LVL_INFO, fmt, ##__VA_ARGS__)
#define LOGGER_DEBUG(fmt, ...)		SimpleFilesLogger::PrintToConsole(SimpleFilesLogger::LVL_DEBUG, fmt, ##__VA_ARGS__)

class SimpleFilesLogger
{
public:

	enum LogLevels
	{
		LVL_NONE = 0,
		LVL_DEBUG,
		LVL_INFO,
		LVL_NOTICE,
		LVL_WARN,
		LVL_CRIT
	};

	static SimpleFilesLogger& getInstance()
	{
		static SimpleFilesLogger ins;
		return ins;
	}
	SimpleFilesLogger(SimpleFilesLogger const&) = delete; // C++11
	void operator=(SimpleFilesLogger const&) = delete; // C++11
	
	
	void AddLogger(int index, std::string filename);
	void WriteLog(int index, const char* msg, ...);
	void WriteStringLog(int index, std::string msg);
	static void PrintToConsole(int loglevel, const char* msg, ...);
private:
	SimpleFilesLogger() {}
	//SimpleFilesLogger(SimpleFilesLogger const&);
	//void operator=(SimpleFilesLogger const&);
	
	std::ofstream* logFiles[10];
	
};