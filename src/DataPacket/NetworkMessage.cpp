#include "NetworkMessage.h"




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NetworkMessageW::NetworkMessageW(unsigned int type)
{
	m_msgLen = 0;
	m_uncompressedLen = 0;
	m_flags = 0;
	AddMessageType(type);
}

NetworkMessageW::~NetworkMessageW()
{
	m_msgBody.clear();
}

NetworkMessageR::NetworkMessageR(byte* buffer, unsigned int len)
{
	m_msgBody = buffer;
	m_msgLen = len;
	m_curPos = 0;

	if (m_msgBody)
	{
		ReadHeader();
	}
	// unzip @#@$%
}

NetworkMessageR::~NetworkMessageR()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void NetworkMessageW::AddByte(byte i)
{
	m_msgBody.push_back(i);
}

byte NetworkMessageR::GetByte()
{
	if (m_curPos >= m_msgLen)
	{
		return 0;
	}
	return m_msgBody[m_curPos++];
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NetworkMessageW::AddInt(int i)
{
	m_msgBody.push_back((byte)((i & 0xFF000000) >> 24));
	m_msgBody.push_back((byte)((i & 0x00FF0000) >> 16));
	m_msgBody.push_back((byte)((i & 0x0000FF00) >> 8));
	m_msgBody.push_back((byte)((i & 0x000000FF)));
}

int NetworkMessageR::GetInt()
{
	if (m_curPos > m_msgLen - 4)
	{
		return 0;
	}

	int i = 0;
	i |= m_msgBody[m_curPos++] << 24;
	i |= m_msgBody[m_curPos++] << 16;
	i |= m_msgBody[m_curPos++] << 8;
	i |= m_msgBody[m_curPos++];

	return i;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NetworkMessageW::AddUInt(unsigned int i)
{
	AddInt((int)i);
}

unsigned int NetworkMessageR::GetUInt()
{
	return (unsigned int)(GetInt());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NetworkMessageW::AddBytesAndLength(const byte* buff, unsigned int len)
{
	AddUInt(len);
	m_msgBody.insert(m_msgBody.end(), buff, buff + len);
}

bool NetworkMessageR::GetBytesAndLength(byte* data, unsigned int& len)
{
	len = GetUInt();

	if (m_curPos > (m_msgLen - len))
	{		
		return false;
	}

	if (len > 0)
	{
		memcpy(data, (m_msgBody + m_curPos), len);
	}

	m_curPos += len;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NetworkMessageW::AddString(const char* data)
{
	AddBytesAndLength((const byte*)data, (unsigned int)strlen(data));
}

bool NetworkMessageR::GetString(byte* data)
{
	unsigned int len = 0;
	bool ret = GetBytesAndLength(data, len);

	if (ret == false)
	{
		data[0] = '\0';		
		return false;
	}

	data[len] = '\0';

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NetworkMessageW::AddString(const std::string& data)
{
	AddBytesAndLength((const byte*)data.c_str(), (unsigned int)data.size());
}

bool NetworkMessageR::GetString(std::string& data)
{
	byte* ptr;
	unsigned int len;
	
	len = GetUInt();

	if (m_curPos > m_msgLen - len)
	{		
		return false;
	}

	if (len > 0)
	{
		ptr = m_msgBody + m_curPos;				
		data.assign((char*)ptr, len);
		m_curPos += len;
		return true;
	}
	
	return false;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NetworkMessageW::AddTempHeader()
{
	// reserve for header : msg type -> len - flags - full data len
	AddUInt(0);
	AddByte(0);
	AddUInt(0);
}

void NetworkMessageW::SetHeader()
{
	// len
	m_msgBody[4] = (byte)((m_msgLen & 0xFF000000) >> 24);
	m_msgBody[5] = (byte)((m_msgLen & 0x00FF0000) >> 16);
	m_msgBody[6] = (byte)((m_msgLen & 0x0000FF00) >> 8);
	m_msgBody[7] = (byte)((m_msgLen & 0x000000FF));
	//flags
	m_msgBody[8] = m_flags;
	// full data len
	m_msgBody[9] = (byte)((m_uncompressedLen & 0xFF000000) >> 24);
	m_msgBody[10] = (byte)((m_uncompressedLen & 0x00FF0000) >> 16);
	m_msgBody[11] = (byte)((m_uncompressedLen & 0x0000FF00) >> 8);
	m_msgBody[12] = (byte)((m_uncompressedLen & 0x000000FF));
}

void NetworkMessageR::ReadHeader()
{
	m_msgType = GetUInt();
	m_msgLen = GetUInt();
	m_flags = GetByte();
	m_uncompressedLen = GetUInt();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NetworkMessageW::AddMessageType(unsigned int type)
{
	AddUInt(type);
	AddTempHeader();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NetworkMessageW::Pack(bool compress)
{
	m_msgLen = (unsigned int)m_msgBody.size();
	m_flags = 0;
	m_uncompressedLen = m_msgLen;
	
	if (compress)
	{
		// TODO
		m_flags |= FLAG_COMPRESSED;
	}

	SetHeader();
}