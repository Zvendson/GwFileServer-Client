#include <stdio.h>
#include "FileClient.h"
#include <fstream>
#include <string>
#include <iostream>
#include <windows.h>


void DownloadFile(uint32_t file_id, std::string fileName = "")
{
	auto client = FileClient();
	if (client.Connect())
	{
		CreateDirectoryA("Downloads", NULL);
		char buffer[MAX_PATH];
		if (fileName.empty())
		{
			snprintf(buffer, sizeof(buffer), "Downloads\\file_%X.raw", file_id);
		}
		else
		{
			snprintf(buffer, sizeof(buffer), "Downloads\\%s", fileName.c_str());
		}
		fileName = buffer;

		CompressedFile file;
		if (client.Download(file_id, &file))
		{
			if (file.Decompress())
			{
				std::ofstream file_out;
				file_out.open(fileName, std::ios::binary | std::ios::out);
				file_out.write((char*)file.m_decompress_buffer, file.m_size_decompressed);
				file_out.close();
				printf("Download saved in '%s'\n", fileName.c_str());
				delete[] file.m_decompress_buffer;
			}
			else
			{
				printf("Could not decompress file: 0x%X (%d)\n", file_id, file_id);
			}
			delete[] file.m_buffer;
		}
		client.Close();
	}
}


int main(int argc, char const* argv[])
{ 
	std::cout << "##########################\n";
	std::cout << "# Guild Wars File Server #\n";
	std::cout << "##########################\n\n";

	auto gw1_id = 0;
	auto gw2_id = 0;

	auto client = FileClient();
	if (client.Connect())
	{
		gw1_id = client.GetGwExeID1();
		gw2_id = client.GetGwExeID2();
	}
	client.Close();

	DownloadFile(gw1_id, "Gw_US.exe");
	DownloadFile(gw2_id, "Gw_EU.exe");

	return 0;	
}