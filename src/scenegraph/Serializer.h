// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SERIALIZE_H
#define _SERIALIZE_H

#include "Aabb.h"
#include "ByteRange.h"
#include "Color.h"
#include "Quaternion.h"
#include "vector3.h"
#include <stdexcept>
#include <string>

#if (__GNUC__ && (__BYTE_ORDER_ == __ORDER_BIG_ENDIAN__)) || (__clang__ && __BIG_ENDIAN__)
#error Serializer.h is incompatible with big-endian architectures!
#endif

// Note: this serializer implementation is for use on little-endian runtimes
// only. It depends on floating point types having the same IEEE-754 semantics
// on both ends, and allows you to write unsafe code if care is not taken.
// It is intended only for the efficient (de)serialization of model binary data;
// where possible, prefer serializing state information via JSON instead.
namespace Serializer {
	static_assert((sizeof(Uint32) == 4 && alignof(Uint32) == 4), "Int32 is sized differently on this platform and will not serialize properly.");
	static_assert((sizeof(Uint64) == 8 && alignof(Uint64) == 8), "Int64 is sized differently on this platform and will not serialize properly.");
	static_assert((sizeof(Color) == 4 && alignof(Color) == 1), "Color is padded differently on this platform and will not serialize properly.");
	static_assert((sizeof(vector2f) == 8 && alignof(vector2f) == 4), "Vector2f is padded differently on this platform and will not serialize properly.");
	static_assert((sizeof(vector2d) == 16 && alignof(vector2d) == 8), "Vector2d is padded differently on this platform and will not serialize properly.");
	static_assert((sizeof(vector3f) == 12 && alignof(vector3f) == 4), "Vector3f is padded differently on this platform and will not serialize properly.");
	static_assert((sizeof(vector3d) == 24 && alignof(vector3d) == 8), "Vector3d is padded differently on this platform and will not serialize properly.");
	static_assert((sizeof(Quaternionf) == 16 && alignof(Quaternionf) == 4), "Quaternionf is padded differently on this platform and will not serialize properly.");
	static_assert((sizeof(Aabb) == 56 && alignof(Aabb) == 8), "Aabb is padded differently on this platform and will not serialize properly.");

	class Writer {
	public:
		Writer() {}
		const std::string &GetData() { return m_str; }

		template <typename T>
		void writeObject(const T &obj)
		{
			m_str.append(reinterpret_cast<const char *>(&obj), sizeof(T));
		}

		template <typename T>
		Writer &operator<<(const T &obj)
		{
			writeObject<T>(obj);
			return *this;
		}

		Writer &operator<<(const std::string &obj)
		{
			writeObject<Uint32>(obj.size());
			m_str.append(obj.c_str(), obj.size());
			return *this;
		}

		Writer &operator<<(const char *s)
		{
			if (!s) { // don't fail on invalid string, just write a zero-length blob.
				*this << 0U;
				return *this;
			}

			Uint32 len = strlen(s);
			*this << len;
			m_str.append(s, len);
			return *this;
		}

		void Blob(ByteRange range)
		{
			assert(range.Size() < SDL_MAX_UINT32);
			Int32(range.Size());
			if (range.Size()) {
				m_str.append(range.begin, range.Size());
			}
		}
		void Byte(Uint8 x) { *this << x; }
		void Bool(bool x) { *this << x; }
		void Int16(Uint16 x) { *this << x; }
		void Int32(Uint32 x) { writeObject<Uint32>(x); }
		void Int64(Uint64 x) { *this << x; }
		void Float(float f) { *this << f; }
		void Double(double f) { *this << f; }
		void String(const char *s) { *this << s; }
		void String(const std::string &s) { *this << s; }

		void Vector3f(vector3f vec) { *this << vec; }
		void Vector3d(vector3d vec) { *this << vec; }
		void WrQuaternionf(const Quaternionf &q) { *this << q; }
		void Color4UB(const Color &c) { *this << c; }
		void WrSection(const std::string &section_label, const std::string &section_data)
		{
			String(section_label);
			String(section_data);
		}

	private:
		std::string m_str;
	};

	class Reader {
	public:
		Reader() :
			m_at(nullptr),
			m_streamVersion(-1) {}

		explicit Reader(const ByteRange &data) :
			m_data(data),
			m_at(data.begin)
		{}

		bool AtEnd() { return m_at != m_data.end; }

		bool Check(std::size_t needed_size)
		{
			return m_data.end >= m_at + needed_size;
		}

		void Seek(int pos)
		{
			assert(pos >= 0 && std::size_t(pos) < m_data.Size());
			m_at = m_data.begin + pos;
		}

		std::size_t Pos() { return std::size_t(m_at - m_data.begin); }

		template <typename T>
		const T *readObject()
		{
#ifdef DEBUG
			if (!Check(sizeof(T)))
				throw std::out_of_range("Serializer::Reader encountered truncated stream.");
#endif

			const auto *out = reinterpret_cast<const T *>(m_at);
			m_at += sizeof(T);
			return out;
		};

		template <typename T>
		Reader &operator>>(T &out)
		{
			out = *readObject<T>();
			return *this;
		}

		Reader &operator>>(std::string &out)
		{
			ByteRange range = Blob();
			out = std::string(range.begin, range.Size());
			return *this;
		}

		ByteRange Blob()
		{
			auto len = *readObject<Uint32>();
			if (len == 0) return ByteRange();
			if (len > (m_data.end - m_at))
				throw std::out_of_range("Serializer::Reader encountered truncated stream.");

			auto range = ByteRange(m_at, m_at + len);
			m_at += len;
			return range;
		}

		bool Bool() { return *readObject<bool>(); }
		Uint8 Byte() { return *readObject<Uint8>(); }
		Uint16 Int16() { return *readObject<Uint16>(); }
		Uint32 Int32() { return *readObject<Uint32>(); }
		Uint64 Int64() { return *readObject<Uint64>(); }
		float Float() { return *readObject<float>(); }
		double Double() { return *readObject<double>(); }

		std::string String()
		{
			ByteRange range = Blob();
			if (range.Size() == 0)
				return "";
			return std::string(range.begin, range.Size());
		}

		Color Color4UB() { return *readObject<Color>(); }

		vector2f Vector2f() { return *readObject<vector2f>(); }
		vector2d Vector2d() { return *readObject<vector2d>(); }
		vector3f Vector3f() { return *readObject<vector3f>(); }
		vector3d Vector3d() { return *readObject<vector3d>(); }

		Quaternionf RdQuaternionf() { return *readObject<Quaternionf>(); }

		Reader RdSection(const std::string &section_label_expected);

		int StreamVersion() const { return m_streamVersion; }
		void SetStreamVersion(int x) { m_streamVersion = x; }

	private:
		ByteRange m_data;
		const char *m_at;
		int m_streamVersion;
	};

} // namespace Serializer

#endif /* _SERIALIZE_H */
