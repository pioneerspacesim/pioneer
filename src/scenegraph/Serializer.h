// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SERIALIZE_H
#define _SERIALIZE_H

#include "Aabb.h"
#include "ByteRange.h"
#include "Color.h"
#include "Quaternion.h"
#include "vector3.h"
#include <cstring>
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
	static_assert((sizeof(Uint32) == 4 && alignof(Uint32) <= 4), "Int32 is sized differently on this platform and will not serialize properly.");
	static_assert((sizeof(Uint64) == 8 && alignof(Uint64) <= 8), "Int64 is sized differently on this platform and will not serialize properly.");
	static_assert((sizeof(Color) == 4 && alignof(Color) <= 1), "Color is padded differently on this platform and will not serialize properly.");
	static_assert((sizeof(vector2f) == 8 && alignof(vector2f) <= 4), "Vector2f is padded differently on this platform and will not serialize properly.");
	static_assert((sizeof(vector2d) == 16 && alignof(vector2d) <= 8), "Vector2d is padded differently on this platform and will not serialize properly.");
	static_assert((sizeof(vector3f) == 12 && alignof(vector3f) <= 4), "Vector3f is padded differently on this platform and will not serialize properly.");
	static_assert((sizeof(vector3d) == 24 && alignof(vector3d) <= 8), "Vector3d is padded differently on this platform and will not serialize properly.");
	static_assert((sizeof(Quaternionf) == 16 && alignof(Quaternionf) <= 4), "Quaternionf is padded differently on this platform and will not serialize properly.");
	static_assert((sizeof(Aabb) == 56 && alignof(Aabb) <= 8), "Aabb is padded differently on this platform and will not serialize properly.");

	class Writer {
	public:
		Writer() {}
		const std::string &GetData() { return m_str; }

		template <typename T>
		void writeObject(const T &obj)
		{
			m_str.append(reinterpret_cast<const char *>(&obj), sizeof(T));
		}

		void writeObject(const std::string &obj)
		{
			writeObject<Uint32>(obj.size());
			m_str.append(obj.c_str(), obj.size());
		}

		void writeObject(const char *s)
		{
			if (!s) { // don't fail on invalid string, just write a zero-length blob.
				*this << 0U;
				return;
			}

			Uint32 len = strlen(s);
			*this << len;
			m_str.append(s, len);
		}

		void writeObject(const vector2f &vec) { *this << vec.x << vec.y; }
		void writeObject(const vector2d &vec) { *this << vec.x << vec.y; }
		void writeObject(const vector3f &vec) { *this << vec.x << vec.y << vec.z; }
		void writeObject(const vector3d &vec) { *this << vec.x << vec.y << vec.z; }
		void writeObject(const Color &col) { *this << col.r << col.g << col.b << col.a; }
		void writeObject(const Quaternionf &quat) { *this << quat.w << quat.x << quat.y << quat.z; }
		void writeObject(const Quaterniond &quat) { *this << quat.w << quat.x << quat.y << quat.z; }
		void writeObject(const Aabb &aabb) { *this << aabb.min << aabb.max << aabb.radius; }

		template <typename T>
		Writer &operator<<(const T &obj)
		{
			writeObject(obj);
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
		void Int32(Uint32 x) { *this << x; }
		void Int64(Uint64 x) { *this << x; }
		void Float(float f) { *this << f; }
		void Double(double f) { *this << f; }
		void String(const char *s) { *this << s; }
		void String(const std::string &s) { *this << s; }

		void Vector2f(vector2f vec) { *this << vec; }
		void Vector2d(vector2d vec) { *this << vec; }
		void Vector3f(vector3f vec) { *this << vec; }
		void Vector3d(vector3d vec) { *this << vec; }
		void WrQuaternionf(const Quaternionf &q) { *this << q; }
		void Color4UB(const Color &c) { *this << c; }
		void WrSection(const std::string &section_label, const std::string &section_data) { *this << section_label << section_data; }

	private:
		std::string m_str;
	};

	class Reader {
		template <typename T>
		T obj()
		{
			T _ob;
			readObject(_ob);
			return _ob;
		}

	public:
		Reader() :
			m_at(nullptr),
			m_streamVersion(-1)
		{
		}

		explicit Reader(const ByteRange &data) :
			m_data(data),
			m_at(data.begin)
		{
		}

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
		void readObject(T &out)
		{
#ifdef DEBUG
			if (!Check(sizeof(T)))
				throw std::out_of_range("Serializer::Reader encountered truncated stream.");
#endif

			std::memcpy(&out, m_at, sizeof(T)); // use memcpy to handle unaligned reads
			m_at += sizeof(T);
		}

		void readObject(std::string &out)
		{
			ByteRange range = Blob();
			out = std::string(range.begin, range.Size());
			if (!out.empty() && *(out.end() - 1) == '\0') // HACK, old SGM files have trailing '\0' de-serialised sometimes
				out.pop_back();
		}

		void readObject(vector2f &vec) { *this >> vec.x >> vec.y; }
		void readObject(vector2d &vec) { *this >> vec.x >> vec.y; }
		void readObject(vector3f &vec) { *this >> vec.x >> vec.y >> vec.z; }
		void readObject(vector3d &vec) { *this >> vec.x >> vec.y >> vec.z; }
		void readObject(Color &col) { *this >> col.r >> col.g >> col.b >> col.a; }
		void readObject(Quaternionf &quat) { *this >> quat.w >> quat.x >> quat.y >> quat.z; }
		void readObject(Quaterniond &quat) { *this >> quat.w >> quat.x >> quat.y >> quat.z; }
		void readObject(Aabb &aabb) { *this >> aabb.min >> aabb.max >> aabb.radius; }

		template <typename T>
		Reader &operator>>(T &out)
		{
			readObject(out);
			return *this;
		}

		ByteRange Blob()
		{
			auto len = Int32();
			if (len == 0) return ByteRange();
			if (len > (m_data.end - m_at))
				throw std::out_of_range("Serializer::Reader encountered truncated stream.");

			auto range = ByteRange(m_at, m_at + len);
			m_at += len;
			return range;
		}

		// Prefer using Reader::operator>> instead; these functions involve creating an unnessesary temporary variable.
		bool Bool() { return obj<bool>(); }
		Uint8 Byte() { return obj<Uint8>(); }
		Uint16 Int16() { return obj<Uint16>(); }
		Uint32 Int32() { return obj<Uint32>(); }
		Uint64 Int64() { return obj<Uint64>(); }
		float Float() { return obj<float>(); }
		double Double() { return obj<double>(); }

		std::string String() { return obj<std::string>(); }

		Color Color4UB() { return obj<Color>(); }
		vector2f Vector2f() { return obj<vector2f>(); }
		vector2d Vector2d() { return obj<vector2d>(); }
		vector3f Vector3f() { return obj<vector3f>(); }
		vector3d Vector3d() { return obj<vector3d>(); }

		Quaternionf RdQuaternionf() { return obj<Quaternionf>(); }

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
