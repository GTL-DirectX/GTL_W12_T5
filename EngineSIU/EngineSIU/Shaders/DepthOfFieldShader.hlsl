float4 mainVS( float4 pos : POSITION ) : SV_POSITION
{
	return pos;
}

float4 mainPS(float4 pos : SV_POSITION) : SV_Target
{
    float4 color = float4(0.5, 0.5, 0.5, 1.0);
    return color;
}
