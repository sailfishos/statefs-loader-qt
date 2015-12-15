/*
 * Copyright (C) 2013-2015 Jolla Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _STATEFS_PRIVATE_EVENTS_HPP_
#define _STATEFS_PRIVATE_EVENTS_HPP_

#include <QEvent>
#include <functional>

namespace cor { namespace qt {

class Event : public QEvent
{
public:
    enum Type {
        Execute = QEvent::User
    };

    virtual ~Event() {}

protected:
    Event(Type t) : QEvent(static_cast<QEvent::Type>(t)) {}
private:
    Event();
};

class EventExecute : public Event
{
public:
    EventExecute(std::function<void()> const& fn)
        : Event(Event::Execute), fn_(fn) {}

    void execute() const { fn_(); }
private:
    std::function<void()> fn_;
};

}}

#endif // _STATEFS_PRIVATE_EVENTS_HPP_
