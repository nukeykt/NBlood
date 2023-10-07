//-------------------------------------------------------------------------
/*
Copyright (C) 2010 EDuke32 developers and contributors

This file is part of EDuke32.

EDuke32 is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License version 2
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
//-------------------------------------------------------------------------

#ifndef EDUKE32_STANDALONE
#define CPPHTTPLIB_NO_EXCEPTIONS
#include "httplib.h"
#include "loguru.hpp"

int32_t windowsCheckForUpdates(char *buffer)
{
    char const *host = "http://www.eduke32.com";

    LOG_F(INFO, "Connecting to %s", host);

    httplib::Client cli(host);

    if (auto res = cli.Get("/VERSION"))
    {
        if (res->status == 200)
        {
            strcpy(buffer, res->body.c_str());
            return 1;
        }
    }
    else
    {
        auto err = res.error();
        LOG_F(WARNING, "HTTP error: %s", httplib::to_string(err).c_str());
    }

    return 0;
}
#endif