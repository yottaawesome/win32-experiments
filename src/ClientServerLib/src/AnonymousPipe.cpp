#include "pch.h"
#include <stdexcept>
#include <iostream>
#include "include/ClientServerLib.hpp"

namespace ClientServerLib
{
	AnonymousPipe::AnonymousPipe(const bool inheritable)
	{
		//CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0);

	}

	AnonymousPipe::~AnonymousPipe()
	{

	}
}