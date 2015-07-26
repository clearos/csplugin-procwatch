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

#ifndef _CSPLUGIN_PROCWATCH_H
#define _CSPLUGIN_PROCWATCH_H

#define _CSPLUGIN_PROCWATCH_REFRESH_TIMER   500

class csPluginProcessWatch : public csPlugin
{
public:
    enum csProcessEvent {
        csPEVENT_ON_START,
        csPEVENT_ON_TERMINATE,
    };

    enum csProcessStates {
        csPSTATE_NOT_RUNNING,
        csPSTATE_RUNNING,
        csPSTATE_INIT,
    };

    enum csProcessMatchType {
        csPMATCH_TEXT,
        csPMATCH_PATTERN,
    };

    typedef struct {
        enum csProcessMatchType type;
        string text;
        csRegEx *pattern;
        enum csProcessStates state;
        enum csProcessEvent event;
        time_t retry_delay;
        time_t last_action;
        string action_group;
        bool one_shot;
    } csProcessState;

    csPluginProcessWatch(const string &name,
        csEventClient *parent, size_t stack_size);
    virtual ~csPluginProcessWatch();

    virtual void SetConfigurationFile(const string &conf_filename);

    virtual void *Entry(void);

protected:
    friend class csPluginXmlParser;

    void ProcessTableRefresh(void);
    void ProcessStateUpdate(void);
    void ProcessStateChange(csProcessState *state);

    typedef map<string, vector<pid_t> > csProcessTableMap;
    typedef vector<csProcessState *> csProcessStateVector;
    typedef map<string, csActionGroup *> csActionGroupMap;

    csProcessTableMap proc_map;
    csProcessStateVector proc_state;
    csActionGroupMap action_group;

    csPluginConf *conf;
};

#endif // _CSPLUGIN_PROCWATCH_H

// vi: expandtab shiftwidth=4 softtabstop=4 tabstop=4
