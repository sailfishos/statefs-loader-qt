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

#include <statefs/loader.hpp>
#include <statefs/util.h>
#include <cor/so.hpp>
#include "wrapqt.hpp"

#include <iostream>
#include <mutex>
#include <QDebug>

static const char *sym_name = statefs_provider_accessor();

#if QT_VERSION < 0x050000
static const char *loader_name = "qt4";
#else
static const char *loader_name = "qt5";
#endif

class QtLoader : public statefs::Loader
{
public:
    QtLoader() : is_reloadable_(true) {
        if (QCoreApplication::instance())
            std::cerr << "NB! QCoreApplication is already created!"
                      << std::endl;
    }

    virtual ~QtLoader() {
    }

    std::shared_ptr<statefs_provider> load
    (std::string const& path, statefs_server *server)
    {
        // NB! No Qt calls in this function before QCoreApplication is
        // created by CoreAppContainer
        std::cerr << "Qt5 loader: loading " << path << std::endl;
        if (!app_) {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!app_) {
                is_reloadable_ = false;
                app_.reset(new cor::qt::CoreAppContainer());
            }
        }

        auto lib = std::make_shared<cor::SharedLib>(path, RTLD_LAZY);
        if (!lib->is_loaded()) {
            std::cerr << "qt5 loader: Can't load " << path.c_str()
                      << ": " << ::dlerror() << std::endl;
            return nullptr;
        }
        auto fn = lib->sym<statefs_provider_fn>(sym_name);
        if (!fn) {
            std::cerr << "qt5 loader: Can't resolve statefs_provider_fn in "
                      << path << std::endl;
            return nullptr;
        }

        statefs_provider *prov = nullptr;
        auto load_ = [fn, &prov, server]() {
            prov = fn(server);
        };
        app_->execute(load_);
        if (!prov) {
            std::cerr << "qt5 loader: provider "
                       << path << " is null\n";
            return nullptr;
        }

        auto app = app_;
        auto deleter = [lib, app](statefs_provider* p) mutable {
            app->execute([p]() {
                    if (p) statefs_provider_release(p);
                });
            lib.reset();
        };
        statefs::provider_ptr res(prov, deleter);
        return res;
    }

    virtual std::string name() const { return loader_name; }

    virtual bool is_reloadable() const { return is_reloadable_; }

private:

    std::mutex mutex_;
    std::shared_ptr<cor::qt::CoreAppContainer> app_;
    bool is_reloadable_;
};

EXTERN_C statefs::Loader * create_cpp_provider_loader()
{
    return new QtLoader();
}
