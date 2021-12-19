#pragma once
#include "Logging.h"

namespace StormDB
{

	template<typename T>
	class STORMDB_API Optional
	{
	private:
		T m_Value;
		std::string m_Error = "Error";

	public:
		Optional() = default;
		Optional(const T& value)
			: m_Value(value), m_Error("")
		{}

		Optional<T> operator=(const T& value)
		{
			m_Value = value;
			m_Error = "";
			return *this;
		}

		inline T* operator->() { return &value(); }

		inline operator bool() const { return has_value(); }
		inline bool has_value() const { return m_Error.empty(); }

		inline const T& value() const { return m_Value; }
		inline T& value() { return m_Value; }

		inline const std::string& get_error() const { return m_Error; }
		inline void set_error(const std::string& error) { m_Error = error; }

	public:
		static Optional<T> from_error(const std::string& error)
		{
			Optional<T> result;
			result.set_error(error);
			return result;
		}
	};

}
