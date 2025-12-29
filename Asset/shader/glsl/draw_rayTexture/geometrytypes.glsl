
struct Vertex
{
	vec3 pos;
	vec2 uv;
};

struct Triangle
{
	Vertex vertices[3];
	vec2 uv;
};

//这个函数将会拆包 输出Vertex buffer 到 输出结构体 Triangle中
Triangle unpackTriangle(uint index, int vertexSize)
{
	Triangle tri;
	const uint triIndex = index * 3;
	
	Indices indices = Indices(bufferReferences.indices);
	Vertices vertices = Vertices(bufferReferences.vertices);
	
	for (uint i = 0; i < 3; i++) 
	{
		const uint offset = indices.i[triIndex + i] * (vertexSize / 16);
		vec4 d0 = vertices.v[offset + 0]; // pos.xyz, n.x
		vec4 d1 = vertices.v[offset + 1]; // n.yz, uv.xy
		tri.vertices[i].pos = d0.xyz;
		tri.vertices[i].uv = d1.zw;
	}
	
	//使用重心坐标进行属性插值
	vec3 barycentricCoords = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);
	tri.uv = tri.vertices[0].uv * barycentricCoords.x + tri.vertices[1].uv * barycentricCoords.y + tri.vertices[2].uv * barycentricCoords.z;
	return tri;
}




