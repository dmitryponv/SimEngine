////////////////////////////////////////////////////////////////////////////////
// Filename: light.vs
////////////////////////////////////////////////////////////////////////////////


/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};


//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};


////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType LightVertexShader(VertexInputType input)
{
    PixelInputType output;
    

	// Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

	//input.position.x = input.position.x/(sqrt(input.position.x*input.position.x + input.position.y*input.position.y+input.position.z*input.position.z));
	//input.position.y = input.position.y/(sqrt(input.position.x*input.position.x + input.position.y*input.position.y+input.position.z*input.position.z));
	//input.position.z = input.position.z/(sqrt(input.position.x*input.position.x + input.position.y*input.position.y+input.position.z*input.position.z));

	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
	// Store the texture coordinates for the pixel shader.
	output.tex = input.tex;
    
	//Calculate NORMAL against world matrix only
	output.normal = mul(input.normal, (float3x3)worldMatrix);
	
	//normalize the normal vector
	output.normal = normalize(output.normal);	
	
    return output;
}