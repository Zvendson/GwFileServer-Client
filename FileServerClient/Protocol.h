#pragma once
#include <stdint.h>

namespace FStoC
{
#define HEADER_HANDSHAKE_FAILED 0x1F1
#define HEADER_MAIN_MANIFEST    0x2F1
#define HEADER_FILE_NOT_FOUND   0x4F2
#define HEADER_FILE_DETAILS     0x5F2
#define HEADER_FILE_DATA        0x6F2
#define HEADER_FILE_MORE_DATA   0x6F3
#define HEADER_COMPLETE         0x7F2
    
    struct Handshake
    {
	    uint16_t header;
	    uint16_t size;
	    uint32_t unk_3;
	    uint32_t asset_manifest_id;
	    uint32_t gw_exe_id;
	    uint32_t unk_6;
	    uint32_t unk_7;
	    uint32_t unk_80;
	    uint32_t ge_exe_2_id;
    };
    static_assert(sizeof(Handshake) == 0x20, "FStoC::MainManifest has invalid struct size.");

	struct FileDetails 
	{
        uint16_t header;
        uint16_t size;
        uint32_t file_id;
        uint32_t size_decompressed;
        uint32_t size_compressed;
        uint32_t crc;
    };
	static_assert(sizeof(FileDetails) == 0x14, "FStoC::FileManifest has invalid struct size.");

	struct Response
	{ 
        uint16_t header;
        uint16_t size;
    };
	static_assert(sizeof(Response) == 0x4, "FStoC::Response has invalid struct size.");

	struct ResponseMore
	{ 
        uint16_t header;
        uint16_t size;
    };
	static_assert(sizeof(ResponseMore) == 0x4, "FStoC::ResponseMore has invalid struct size.");

	struct Complete
	{ 
        uint16_t header;
        uint16_t size;
		uint32_t file_size;
    };
	static_assert(sizeof(Complete) == 0x8, "FStoC::Complete has invalid struct size.");
}

namespace CtoFS
{

#define HEADER_HANDSHAKE    0xF1
#define HEADER_REQUEST      0x3F2
#define HEADER_REQUEST_MORE 0x7F3

	enum class GameType
	{
		GuildWars1 = 1
	};

	#pragma pack(push, 1)
	struct Handshake
	{
		uint8_t  unk_1  = 1;
		uint32_t unk_2  = 0;
		uint16_t header = HEADER_HANDSHAKE;
		uint16_t size   = 0x10;
		GameType type   = GameType::GuildWars1;
		uint32_t unk_6  = 0;
		uint32_t unk_7  = 0;
	};
	#pragma pack(pop)
	static_assert(sizeof(Handshake) == 0x15, "CtoFS::Handshake has invalid struct size.");

	struct Request 
	{
        uint16_t header  = HEADER_REQUEST;
        uint16_t size    = 0xC;
        uint32_t file_id = 0;
        uint32_t version = 0;
    };
    static_assert(sizeof(Request) == 0xC, "CtoFS::Request has invalid struct size.");

	struct RequestMore 
	{
        uint16_t header      = HEADER_REQUEST_MORE;
        uint16_t size        = 0x8;
        uint32_t data_offset = 0;
    };
    static_assert(sizeof(RequestMore) == 0x8, "CtoFS::RequestMore has invalid struct size.");
}