#include <stdlib.h>
#include "Buffer.h"

Render::Buffer::Buffer()
{
}

Render::Buffer::~Buffer()
{
	if (pData)
	{
		free(pData);
		pData = nullptr;
	}
}