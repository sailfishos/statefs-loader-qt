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

#include "wrapqt.hpp"
#include <cor/mt.hpp>
#include <QTimer>
#include <QDebug>
#include <stdexcept>
#include <iostream>

namespace cor { namespace qt {

int CoreAppContainer::argc_ = 0;
char* CoreAppContainer::argv_[] = {nullptr};

CoreAppCondNotify::CoreAppCondNotify
(std::unique_lock<std::mutex> &lock, std::condition_variable &cond)
    : lock_(lock), cond_(cond)
{
    QTimer::singleShot(0, this, SLOT(notify()));
}

void CoreAppCondNotify::notify()
{
    cond_.notify_all();
    lock_.unlock();
}


CoreAppContainer::CoreAppContainer()
    : thread_([this]() {
            std::unique_lock<std::mutex> lock(mutex_);
            app_.reset(new CoreAppImpl(argc_, argv_));
            CoreAppCondNotify u(lock, started_);
            app_->exec();
            app_.reset(nullptr);
        })
{
    std::unique_lock<std::mutex> lock(mutex_);
    started_.wait_for(lock,  std::chrono::milliseconds(10000));
}

CoreAppContainer::~CoreAppContainer()
{
    app_->quit();
    if (thread_.joinable())
        thread_.join();
}

void CoreAppContainer::execute(std::function<void()> const& fn)
{
    cor::Future f;
    app_->postEvent(app_.get(), new EventExecute(f.wrap(fn)));
    f.wait(std::chrono::milliseconds(10000));
}

bool CoreAppImpl::event(QEvent *e)
{
    try {
        switch (static_cast<Event::Type>(e->type())) {
        case (Event::Execute): {
            auto s = static_cast<EventExecute*>(e);
            s->execute();
            return true;
        }
        default:
            return QObject::event(e);
        }
    } catch (std::exception const& e) {
        qDebug() << "event: caught std::exception: " << e.what();
    } catch (...) { // Qt does not allow exceptions from event handlers
        qDebug() << "event: caught some exception";
    }
    return false;
}

}}
