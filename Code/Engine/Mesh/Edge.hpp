#pragma once

struct HalfEdge;

struct Edge {
public:
	Edge(HalfEdge* he1, HalfEdge* he2) : m_he1(he1), m_he2(he2) {};
public:
	HalfEdge* m_he1 = nullptr;
	HalfEdge* m_he2 = nullptr;
};