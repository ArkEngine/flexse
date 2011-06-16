#include "MyException.h"

namespace flexse
{
	MyException::MyException(const char * message, const char * filename, u_int32_t line, const char * method)
	{
		snprintf(m_message,  sizeof(m_message),  "%s", message);
		snprintf(m_filename, sizeof(m_filename), "%s", filename);
		snprintf(m_method,   sizeof(m_method),   "%s", method);
		m_line = line;
	}
	inline const char * MyException::Message(void) const
	{ return this->m_message; }

	inline const char * MyException::FileName(void) const
	{ return this->m_filename; }

	inline u_int32_t MyException::Line(void) const
	{ return this->m_line; }

	inline const char * MyException::Method(void) const
	{ return this->m_method; }
}
