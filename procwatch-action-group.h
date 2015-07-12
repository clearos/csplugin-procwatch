// ClearSync: file watch plugin.
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

#ifndef _PROCWATCH_ACTION_GROUP_H
#define _PROCWATCH_ACTION_GROUP_H

class csPluginProcessWatch;
class csActionGroup
{
public:
    csActionGroup(const string &name,
        time_t delay = _PROCWATCH_DEFAULT_DELAY);
    virtual ~csActionGroup();

    inline string GetName(void) { return name; };
    inline void AppendAction(const string &action) {
        this->action.push_back(action);
    }

    bool operator!=(cstimer_id_t id) {
        if (timer == NULL) return true;
        if (timer->GetId() != id) return true;
        return false;
    }

    void ResetDelayTimer(csPluginProcessWatch *plugin);
    void Execute(csEventClient *src, csEventClient *dst);

protected:
    string name;
    time_t delay;
    vector<string> action;
    csTimer *timer;
    static cstimer_id_t timer_index;
};

#endif // _PROCWATCH_ACTION_GROUP_H

// vi: expandtab shiftwidth=4 softtabstop=4 tabstop=4
