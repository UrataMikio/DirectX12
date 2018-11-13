struct Output {
	float4 svpos : SV_POSITION;
	float4 pos : POSITION;
};

Output VS(float4 pos : POSITION)
{
	Output o;
	o.svpos = pos;
	o.pos   = pos;

	return o;
}

float4 PS(Output o) : SV_Target
{
	return float4(1 - o.pos.xy, 1, 1);
}