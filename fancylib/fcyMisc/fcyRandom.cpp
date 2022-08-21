#include "fcyMisc/fcyRandom.h"

#include <Windows.h>

////////////////////////////////////////////////////////////////////////////////

fcyRandomWELL512::fcyRandomWELL512()
{
	SetSeed(GetTickCount());
}

fcyRandomWELL512::fcyRandomWELL512(uint32_t Seed)
{
	SetSeed(Seed);
}

fcyRandomWELL512::~fcyRandomWELL512()
{}

uint32_t fcyRandomWELL512::GetRandSeed() const
{
	return m_RSeed;
}

void fcyRandomWELL512::SetSeed(uint32_t Seed)
{
	m_RSeed = Seed;

	m_Index = 0;

	// 线性同余产生种子
	const unsigned int mask = ~0u;
	m_State[0] = Seed & mask;
	for (unsigned int i = 1; i < 16; ++i)
	{
		m_State[i] = (1812433253UL * (m_State[i - 1] ^ (m_State[i - 1] >> 30)) + i) & mask;
	}
}

uint32_t fcyRandomWELL512::GetRandUInt()
{
	unsigned int a, b, c, d;
	a = m_State[m_Index];
	c = m_State[(m_Index + 13) & 15];
	b = a ^ c ^ (a << 16) ^ (c << 15);
	c = m_State[(m_Index + 9) & 15];
	c ^= (c >> 11);
	a = m_State[m_Index] = b ^ c;
	d = a ^ ((a << 5) & 0xDA442D24UL);
	m_Index = (m_Index + 15) & 15;
	a = m_State[m_Index];
	m_State[m_Index] = a ^ b ^ d ^ (a << 2) ^ (b << 18) ^ (c << 28);
	return m_State[m_Index];
}

uint32_t fcyRandomWELL512::GetRandUInt(uint32_t Max)
{
	return GetRandUInt() % (Max + 1);
}

float fcyRandomWELL512::GetRandFloat()
{
	return GetRandUInt(1000000)/1000000.f;
}

float fcyRandomWELL512::GetRandFloat(float MinBound, float MaxBound)
{
	return GetRandFloat()*(MaxBound-MinBound) + MinBound;
}
