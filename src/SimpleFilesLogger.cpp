#include <iostream>
#include <fstream>
#include <Windows.h>
#include <stdio.h>

#include "SimpleFilesLogger.h"


void SimpleFilesLogger::AddLogger(int index, std::string filename)
{
	char _fileName[512];

	char _documentDir[512];
#if defined(OS_W8) || defined (OS_WP8) || defined(OS_W10) || defined(OS_WP81)
	auto localFolder = Windows::Storage::ApplicationData::Current->LocalFolder->Path;
	const wchar_t* path = localFolder->Data();
	WideCharToMultiByte(CP_UTF8, 0, path, -1, _documentDir, 512, NULL, NULL);
	strcat(_documentDir, "\\");
#else
	//strcat(_documentDir, ".\\");
	strcat_s(_documentDir, sizeof(_documentDir), "\\");
#endif	
	
	//sprintf(_fileName, "%s_%s", _documentDir, filename.c_str());
	snprintf(_fileName, sizeof(_fileName), "%s_%s", _documentDir, filename.c_str());
	logFiles[index] = new std::ofstream;
	if (logFiles[index]->is_open())
	{
		logFiles[index]->close();
	}
	logFiles[index]->open(_fileName, std::ios::out);

	WriteStringLog(index, " :::::::: Starting Logging Session :::::::: \n");
}

void SimpleFilesLogger::WriteLog(int index, const char* msg, ...)
{
    char tmp[20240] = { 0 };
	va_list maker;
	va_start(maker, msg);
	vsnprintf(tmp, 20240, msg, maker);
	tmp[20240 - 1] = '\0';
	va_end(maker);
    WriteStringLog(index, tmp);	
}

void SimpleFilesLogger::WriteStringLog(int index, std::string msg)
{
	if (logFiles[index] == nullptr)
	{
		char buffer[20];
		snprintf(buffer, sizeof(buffer), "%d_log.txt", index);
		AddLogger(index, buffer);
	}
	
	if (logFiles[index]->is_open())
	{
		*logFiles[index] << msg.c_str();
		logFiles[index]->flush();
	}
}

void SimpleFilesLogger::PrintToConsole(int loglevel, const char* fmt, ...)
{
	char tmp[1024] = { 0 };
	va_list maker;
	va_start(maker, fmt);
	vsnprintf(tmp, 1024, fmt, maker);
	tmp[1024 - 1] = '\0';
	va_end(maker);
	OutputDebugStringA(tmp);
}