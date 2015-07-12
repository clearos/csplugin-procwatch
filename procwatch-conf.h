// ClearSync: Process Watch plugin.
// Copyright (C) 2011 ClearFoundation <http://www.clearfoundation.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef _PROCWATCH_CONF_H
#define _PROCWATCH_CONF_H

#define _PROCWATCH_REFRESH_RATE     5
#define _PROCWATCH_DEFAULT_DELAY    5
#define _PROCWATCH_DELAY_TIMER_ID   501
#define _PROCWATCH_EVENT_TIMEOUT    (500)
#define _PROCWATCH_DEFAULT_RETRY    5

class csPluginConf;
class csPluginXmlParser : public csXmlParser
{
public:
    virtual void ParseElementOpen(csXmlTag *tag);
    virtual void ParseElementClose(csXmlTag *tag);

protected:
    void ParseProcWatchOpen(csXmlTag *tag);
    void ParseProcWatchClose(csXmlTag *tag);
};

class csPluginProcessWatch;
class csPluginConf : public csConf
{
public:
    csPluginConf(csPluginProcessWatch *parent,
        const char *filename, csPluginXmlParser *parser);

    virtual void Reload(void);

    unsigned int GetRefreshRate(void) { return refresh_rate; }

protected:
    friend class csPluginXmlParser;

    csPluginProcessWatch *parent;

    unsigned int refresh_rate;
};

#endif // _PROCWATCH_CONF_H

// vi: expandtab shiftwidth=4 softtabstop=4 tabstop=4
