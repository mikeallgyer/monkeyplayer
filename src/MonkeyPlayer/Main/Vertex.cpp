// Vertex.cpp
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// classes containing all vertex declarations

#include "d3dUtil.h"
#include "Vertex.h"

IDirect3DVertexDeclaration9* Vertex::VertexPosition::Decl = 0;
IDirect3DVertexDeclaration9* Vertex::VertexPosTex::Decl = 0;

void Vertex::initVertexDeclarations()
{
	D3DVERTEXELEMENT9 vertexPosElems[] =
	{
		{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		D3DDECL_END()
	};
	HR(gDevice->CreateVertexDeclaration(vertexPosElems, &VertexPosition::Decl));
	D3DVERTEXELEMENT9 vertexPosTexElems[] =
	{
		{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		D3DDECL_END()
	};
	HR(gDevice->CreateVertexDeclaration(vertexPosTexElems, &VertexPosTex::Decl));
}

void Vertex::destroyVertexDeclarations()
{
	ReleaseCOM(VertexPosition::Decl);
	ReleaseCOM(VertexPosTex::Decl);
}