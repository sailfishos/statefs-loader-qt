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

#ifndef _QCOREAPWRAPPER_HPP_
#define _QCOREAPWRAPPER_HPP_

#include "events.hpp"

#include <QObject>
#include <QCoreApplication>

#include <memory>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace cor { namespace qt {

class CoreAppImpl : public QCoreApplication
{
    Q_OBJECT;
public:
    CoreAppImpl(int &argc, char *argv[])
        : QCoreApplication(argc, argv)
    {}

    virtual ~CoreAppImpl() { }
    virtual bool event(QEvent*);
};

class CoreAppCondNotify : public QObject
{
    Q_OBJECT;
public:
    CoreAppCondNotify(std::unique_lock<std::mutex> &lock
                      , std::condition_variable &cond);
private slots:
    void notify();
private:
    std::unique_lock<std::mutex> &lock_;
    std::condition_variable &cond_;
};


class CoreAppContainer
{
public:
    CoreAppContainer();
    virtual ~CoreAppContainer();

    void execute(std::function<void()> const&);
private:
    static int argc_;
    static char* argv_[];

    std::mutex mutex_;
    std::condition_variable started_;
    std::unique_ptr<CoreAppImpl> app_;
    std::thread thread_;
};

}}
 
#endif // _QCOREAPWRAPPER_HPP_
