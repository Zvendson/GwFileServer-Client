#include "FileClient.h"
#include <stdio.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <assert.h>

namespace
{
	void hexdump(char* data, uint32_t len)
	{
		for (uint16_t i = 0; i < len; i++)
		{
			if (i % 16 == 0)			
				printf("\n\t%04X: ", i * 16);
			
			printf("%02X ", data[i] & 0xff);
			
		}
		printf("\n");
	}

	void hexdump(uint8_t* data, uint32_t len)
	{
		for (uint16_t i = 0; i < len; i++)
		{
			if (i % 16 == 0)			
				printf("\n\t%04X: ", i * 16);
			
			printf("%02X ", data[i] & 0xff);
			
		}
		printf("\n");
	}
}


FileClient::FileClient()
{
	m_socket        = 0;
	m_handshakeData = { 0 };
	m_wsaData       = { 0 };
}

bool FileClient::Connect()
{
	if (IsConnected())
	{
		printf("Already connected.\n");
		return false;
	}
	 
	// Init
	int res;
	if (!m_wsaData.wVersion && (res = WSAStartup(MAKEWORD(2, 2), &m_wsaData)) != 0) 
	{
        printf("Failed to call WSAStartup: %d\n", res);
		return false;
    }


	// Connect to one of anets file server
	struct addrinfo hints, * servinfo;

	for (size_t i = 1; i < 12; i++)
	{
		snprintf(m_domain, sizeof(m_domain), "file%d.arenanetworks.com", i);
		
		memset(&hints, 0, sizeof hints);
		hints.ai_family   = AF_UNSPEC;    //IPv4 or IPv6 doesnt matter
		hints.ai_socktype = SOCK_STREAM;  //TCP stream socket

		if ((res = getaddrinfo(m_domain, "6112", &hints, &servinfo)) != 0)
			continue;

		if ((m_socket = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == INVALID_SOCKET)
			continue;

		if (connect(m_socket, servinfo->ai_addr, servinfo->ai_addrlen) == SOCKET_ERROR)
		{
			closesocket(m_socket);
			m_socket = 0;
			continue;
		}

		printf("Connected to '%s'\n", m_domain);
		break;
	}

	freeaddrinfo(servinfo);
	
	if (!IsConnected())
	{
		printf("Failed to connect to any fileserver.\n");
		return false;
	}

	if (!SendHandshake())
	{
		printf("Handshake failed.\n");
		Close();
		return false;
	}

	return true;
}
bool FileClient::Close()
{
	if (!IsConnected())
	{
		return false;
	}

	#ifdef WIN32
	if (shutdown(m_socket, 2) == SOCKET_ERROR)
	{
		printf("socket failed to shutdown with error: %d\n", WSAGetLastError());
	}
	#endif

	if (closesocket(m_socket) == SOCKET_ERROR)
	{
		printf("socket failed to close with error: %d\n", WSAGetLastError());
	}
	else
	{		
		printf("Disconnected from %s.\n", m_domain);
	}

	m_socket        = 0;
	m_wsaData       = { 0 };
	m_handshakeData = { 0 };
	ZeroMemory(m_domain, sizeof(m_domain));
	ZeroMemory(m_buffer, sizeof(m_buffer));

	WSACleanup();
	return true;	
}
bool FileClient::Download(uint32_t id, CompressedFile* out, uint32_t curr_version)
{
	if (!IsConnected())
		return false;

	// Get details
	FStoC::FileDetails details;
	if (!SendRequest(id, curr_version, &details))
		return false;	

	if (details.header == HEADER_FILE_NOT_FOUND)
	{
		printf("File not found: 0x%X (%d)\n", id, id);
		return false;
	}

	// Prepare memory to download file
	CompressedFile file;
	file.m_buffer =  new uint8_t[details.size_compressed + 1];

	if (file.m_buffer == nullptr)
	{
		printf("Could not allocate 0x%X (%d) bytes.\n", details.size_compressed, details.size_compressed);
		return false;
	}
	
	file.m_file_id           = id;
	file.m_size_compressed   = details.size_compressed;
	file.m_size_decompressed = details.size_decompressed;
	file.m_crc               = details.crc;
	file.m_size_downloaded   = 0;

	// Download file
	printf("Downloading file id: 0x%X (%d) CRC=0x%X\n", id, id, file.m_crc);
	while (file.m_size_compressed > file.m_size_downloaded)
	{
		// Get file chunk
		if (Recv(sizeof(FStoC::Response)) == 0)
		{
			printf("Failed to receive data for FileData.\n");
			delete[] file.m_buffer;
			return false;
		}
		FStoC::Response* data_packet = (FStoC::Response*)m_buffer;

		if (!( (data_packet->header == HEADER_FILE_DATA) || (data_packet->header == HEADER_FILE_MORE_DATA) ))
		{
			printf("FileData received invalid header: 0x%X\n", data_packet->header);
			delete[] file.m_buffer;
			return false;
		}

		// subtract packet size from filesize
		auto size = data_packet->size - 4;

		// download data chunk
		auto read = Recv(size);
		if (read != size)
		{
			printf("Failed to receive data chunk.\n");
			delete[] file.m_buffer;
			return false;
		}
		memcpy(file.m_buffer + file.m_size_downloaded, m_buffer, read);
		file.m_size_downloaded += read;
		
		if (file.m_size_downloaded < file.m_size_compressed)
		{
			if (!SendRequestMore(size))
			{
				delete[] file.m_buffer;
				return false;
			}
		}

		printf("     > Downloaded %6.2f%%\n", file.GetSizeInPercent() * 100);
	}

	file.Decompress();

	memcpy(out, &file, sizeof(CompressedFile));
	printf("Download completed!\n\n");

	return true;
}

bool FileClient::SendHandshake(CtoFS::GameType type)
{
	CtoFS::Handshake handshake;
	handshake.type = type;
	if (Send(&handshake, sizeof(handshake)) == SOCKET_ERROR)
	{
		printf("Handshake could not be sent.\n");
		return false;
	}

	
	if (Recv(sizeof(FStoC::Handshake)) == 0)
	{
		printf("Failed to receive data for Handshake.\n");
		return false;
	}
	FStoC::Handshake* packet = (FStoC::Handshake*)m_buffer;

	if (packet->header != HEADER_MAIN_MANIFEST)
	{
		printf("Handshake received invalid header: %d\n", packet->header);
		return false;
	}

	m_handshakeData = *packet;
	
	// printf("header      = 0x%X\n", m_handshakeData.header);
	// printf("size        = 0x%X\n", m_handshakeData.size);
	// printf("manifest_id = %d\n", m_handshakeData.asset_manifest_id);
	// printf("gw_exe_id   = %d\n", m_handshakeData.gw_exe_id);
	// printf("gw_exe_2_id = %d\n", m_handshakeData.ge_exe_2_id);
	

	return true;
}
bool FileClient::SendRequest(uint32_t id, uint32_t curr_version, FStoC::FileDetails* out)
{
	CtoFS::Request request;
	request.file_id = id;
	request.version = curr_version;

	if (Send(&request, sizeof(request)) == SOCKET_ERROR)
	{
		printf("Request could not be sent.\n");
		return false;
	}

	if (Recv(sizeof(FStoC::FileDetails)) == 0)
	{
		printf("Failed to receive data for FileDetails.\n");
		return false;
	}
	FStoC::FileDetails* packet = (FStoC::FileDetails*)m_buffer;

	if (packet->header != HEADER_FILE_DETAILS)
	{
		printf("FileDetails received invalid header: %d\n", packet->header);
		return false;
	}

	memcpy(out, packet, sizeof(FStoC::FileDetails));
	return true;
}
bool FileClient::SendRequestMore(uint32_t offset)
{
	CtoFS::RequestMore request_more;
	request_more.data_offset = offset;

	if (Send(&request_more, sizeof(request_more)) == SOCKET_ERROR)
	{
		printf("RequestMore could not be sent.\n");
		return false;
	}

	return true;
}

int      FileClient::Send(void* packet, int size)
{
    if (!m_socket) {
        printf("send_blocking without a socket.\n");
        return -1;
    }

    int sent_bytes = ::send(m_socket, (const char*)packet, size, NULL);
    if (sent_bytes == SOCKET_ERROR) {
        printf("send failed: %d\n", WSAGetLastError());
        return -1;
    }

    return sent_bytes;
}
uint16_t FileClient::Recv(const uint32_t len) 
{
	assert(len < MAX_BUFFER_LEN);
    ZeroMemory(m_buffer, MAX_BUFFER_LEN);
	/*
	printf("Recv: ");
	int recv_bytes = 0;
	do
	{
		recv_bytes = recv(m_socket, m_buffer, len, MSG_PEEK);
		if (recv_bytes == SOCKET_ERROR) {
			printf("recv PEEK failed with error %d\n", WSAGetLastError());
			return 0;
		}
		printf(".");
	}
	while (recv_bytes < len);
	printf("\n");
	*/

	int recv_bytes = recv(m_socket, m_buffer, len, MSG_WAITALL);
	if (recv_bytes == SOCKET_ERROR) {
		printf("recv failed with error %d\n", WSAGetLastError());
		return 0;
	}

    if (recv_bytes == 0) {
        printf("buffer did not receive any data.\n");
        return 0;
    }

    return recv_bytes;
}

bool CompressedFile::Decompress()
{
	Xentax xentax;
	m_decompress_buffer = xentax.DecompressFile((unsigned int*)m_buffer, m_size_compressed, (int&)(m_size_decompressed));

	if (!m_decompress_buffer || m_decompress_buffer[0] == '\0')
	{
		printf("Failed to decompress file 0x%X (%d)\n", m_file_id, m_file_id);
		return false;
	}

	return true;
}
