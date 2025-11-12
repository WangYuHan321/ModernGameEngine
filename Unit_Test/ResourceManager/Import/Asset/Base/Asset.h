#pragma once 

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
namespace Core
{
	class IAsset
	{
	private:
		std::string m_name;
		void* m_pData;
		int m_size;
	public:
		IAsset(std::string& path) : m_pData(nullptr)

		{
			std::ifstream file(path, std::ios::binary | std::ios::ate);

			m_size = file.tellg();
			file.seekg(0, std::ios::beg);

			std::vector<char> buffer(m_size);
			file.read((char*)m_pData, m_size);

			file.close();
		}

		virtual ~IAsset() { if (m_pData) { delete m_pData; } }

		virtual std::string GetName() const { return "nil"; }
	};

	class TextureAsset : public IAsset
	{
	};

	class MaterialAsset : public IAsset
	{
	};

	class MeshAsset : public IAsset
	{
	};
}