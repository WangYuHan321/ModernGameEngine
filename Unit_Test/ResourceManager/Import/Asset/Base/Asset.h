#pragma once 

#include <string>
#include <cstdint>
#include <sstream>
#include <iomanip>
#include <vector>

std::string MD5(const std::string& input) {
    uint32_t h0 = 0x67452301;
    uint32_t h1 = 0xEFCDAB89;
    uint32_t h2 = 0x98BADCFE;
    uint32_t h3 = 0x10325476;

    // MD5常量
    const uint32_t k[64] = {
        0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
        0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
        0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
        0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
        0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
        0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
        0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
        0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
        0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
        0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
        0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
        0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
        0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
        0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
        0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
        0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
    };

    const int s[64] = {
        7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
        5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
        4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
        6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
    };

    // 预处理
    std::vector<uint8_t> msg(input.begin(), input.end());
    uint64_t bit_len = input.length() * 8;

    // 添加填充位
    msg.push_back(0x80);
    while ((msg.size() * 8) % 512 != 448) {
        msg.push_back(0x00);
    }

    // 添加长度
    for (int i = 0; i < 8; i++) {
        msg.push_back((bit_len >> (i * 8)) & 0xFF);
    }

    // 处理消息块
    for (size_t i = 0; i < msg.size(); i += 64) {
        uint32_t w[16];

        for (int j = 0; j < 16; j++) {
            w[j] = (msg[i + j * 4]) |
                (msg[i + j * 4 + 1] << 8) |
                (msg[i + j * 4 + 2] << 16) |
                (msg[i + j * 4 + 3] << 24);
        }

        uint32_t a = h0, b = h1, c = h2, d = h3;

        for (int j = 0; j < 64; j++) {
            uint32_t f, g;

            if (j < 16) {
                f = (b & c) | ((~b) & d);
                g = j;
            }
            else if (j < 32) {
                f = (d & b) | ((~d) & c);
                g = (5 * j + 1) % 16;
            }
            else if (j < 48) {
                f = b ^ c ^ d;
                g = (3 * j + 5) % 16;
            }
            else {
                f = c ^ (b | (~d));
                g = (7 * j) % 16;
            }

            f = f + a + k[j] + w[g];
            a = d;
            d = c;
            c = b;
            b = b + ((f << s[j]) | (f >> (32 - s[j])));
        }

        h0 += a;
        h1 += b;
        h2 += c;
        h3 += d;
    }

    // 生成十六进制输出
    std::stringstream result;
    result << std::hex << std::setfill('0');

    auto output = [&result](uint32_t n) {
        result << std::setw(2) << ((n >> 0) & 0xFF)
            << std::setw(2) << ((n >> 8) & 0xFF)
            << std::setw(2) << ((n >> 16) & 0xFF)
            << std::setw(2) << ((n >> 24) & 0xFF);
        };

    output(h0);
    output(h1);
    output(h2);
    output(h3);

    return result.str();
}
namespace Core
{
	class IAsset
	{
	private:
		std::string m_name;
		void* m_pData;
		int m_size;

		std::string GUID = "xawdwadwad12321312312454543e5d13q534154123";

	public:
		IAsset(std::string& path) :
            GUID(MD5(path)),
            m_pData(nullptr)
		{
		}

		virtual ~IAsset() { if (m_pData) { delete m_pData; } }

		virtual std::string GetName() const { return "nil"; }
	};

	class TextureAsset : public IAsset
	{
	private:
		int m_width;
		int m_height;

	public:
		TextureAsset(std::string& path) :
            m_width(0),
            m_height(0),
			IAsset(path)
		{
		}

	};

	class MaterialAsset : public IAsset
	{
	private:

	};

	class MeshAsset : public IAsset
	{
	private:

	};
}