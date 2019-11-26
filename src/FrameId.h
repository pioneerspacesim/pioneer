#ifndef FRAMEID_H_INCLUDED
#define FRAMEID_H_INCLUDED

#include <cstddef>

struct FrameId {
	static constexpr int Invalid = -1;
	constexpr FrameId() :
		m_id(Invalid) {}
	constexpr FrameId(int new_id) :
		m_id(new_id) {}

	constexpr operator bool() const { return m_id > Invalid; }
	constexpr operator int() const { return m_id; }
	constexpr operator size_t() const { return m_id; }

	constexpr bool operator==(FrameId rhs) const { return m_id == rhs.m_id; }
	constexpr bool operator!=(FrameId rhs) const { return m_id != rhs.m_id; }

	constexpr bool valid() const { return m_id > Invalid; }
	constexpr int id() const { return m_id; }

private:
	int m_id;
};

static_assert(sizeof(FrameId) == sizeof(int) && alignof(FrameId) == alignof(int),
	"Error: FrameId sized differently than the underlying type on this platform!");

#endif // FRAMEID_H_INCLUDED
