#include "pch.h"
#include <stdexcept>
#include <iostream>
#include "include/ClientServerLib.hpp"

namespace ClientServerLib
{
	Message::Message(const std::wstring msg)
	: m_msg(msg)
	{ }

	std::wstring& Message::GetMsg()
	{
		return m_msg;
	}
}