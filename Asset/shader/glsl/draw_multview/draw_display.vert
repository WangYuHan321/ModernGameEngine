#version 450

layout(location = 0) out vec2 outUV;

/*

vkCmdDraw(cmdBuffer, 3, 1, 0, 0);

输入值 0，1，2

     (-1,3) 
        +  
        |\ 
        | \  
        |  \   
        |   \    
        |    \     
        |     \      
(-1,-1) +------+ (3,-1)

实际渲染区域被视口裁剪到 [-1, 1]，覆盖整个屏幕

对于gl_VertexIndex=0: (0<<1)&2=0, 0&2=0 -> (0,0)
对于gl_VertexIndex=1: (1<<1)=2, 2&2=2, 1&2=0 -> (2,0)
对于gl_VertexIndex=2: (2<<1)=4, 4&2=0, 2&2=2 -> (0,2)

*/

void main()
{
	outUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(outUV * 2.0f - 1.0f, 0.0f, 1.0f);
}