#pragma once

#include <vector>
#include "MessageType.h"
#define TCP_MESSAGE_HEADER_SIZE			(13)


#define FLAG_ENCRIPTED				(0x1)
#define FLAG_COMPRESSED				(0x2)

typedef unsigned char byte;

class NetworkMessageW
{
public:
	NetworkMessageW(unsigned int type);
	virtual ~NetworkMessageW();

	void AddByte(byte);

	void AddInt(int);
	void AddUInt(unsigned int);

	void AddBytesAndLength(const byte*, unsigned int);
	void AddString(const char*);
	void AddString(const std::string&);
	
		
	void Pack(bool compress = false);

	inline	const unsigned int GetMsgSize() const { return m_msgLen; }
	inline	const unsigned char* GetMsgBody() const {return &m_msgBody[0]; }	

private:
	void SetHeader();
	void AddTempHeader();
	void AddMessageType(unsigned int);

	std::vector<byte> m_msgBody;
	unsigned int m_msgLen;
	unsigned int m_uncompressedLen;
	unsigned char m_flags;
	
};

class NetworkMessageR
{
public:
	NetworkMessageR(byte*, unsigned int);
	virtual ~NetworkMessageR();

	byte GetByte();

	int GetInt();
	unsigned int GetUInt();

	bool GetBytesAndLength(byte*, unsigned int&);

	bool GetString(byte*);
	bool GetString(std::string&);

	unsigned int GetMsgType() { return m_msgType; };
	unsigned int GetMsgSize() { return m_msgLen; };

private:
	void ReadHeader();
	byte* m_msgBody;
	unsigned int m_msgLen;
	unsigned int m_uncompressedLen;
	unsigned char m_flags;
	unsigned int m_curPos;
	unsigned int m_msgType;


};