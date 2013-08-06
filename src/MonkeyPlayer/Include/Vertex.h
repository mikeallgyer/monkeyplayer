// Vertex.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// classes containing all vertex declarations

#include <d3dx9.h>

#ifndef I_VERTEX_H
#define I_VERTEX_H

namespace Vertex
{
	void initVertexDeclarations();
	void destroyVertexDeclarations();

	struct VertexPosition
	{
		D3DXVECTOR3 Pos;
		static IDirect3DVertexDeclaration9* Decl;

		VertexPosition() : Pos(0, 0, 0) {}
		VertexPosition(float x, float y, float z) : Pos(x, y, z) {}
		VertexPosition(D3DXVECTOR3 v) : Pos(v) {}
	};
	struct VertexPosTex
	{
		D3DXVECTOR3 Pos;
		D3DXVECTOR2 UVs;
		static IDirect3DVertexDeclaration9* Decl;

		VertexPosTex() : Pos(0, 0, 0), UVs(0, 0) {}
		VertexPosTex(float x, float y, float z, float u, float v) : Pos(x, y, z), UVs(u, v) {}
		VertexPosTex(D3DXVECTOR3 v, D3DXVECTOR2 uv) : Pos(v), UVs(uv) {}
	};
}

#endif
