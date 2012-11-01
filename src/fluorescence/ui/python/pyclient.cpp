/*
 * fluorescence is a free, customizable Ultima Online client.
 * Copyright (C) 2011-2012, http://fluorescence-client.org

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "pyclient.hpp"

#include <client.hpp>
#include <misc/log.hpp>
#include <ui/gumpmenus.hpp>

namespace fluo {
namespace ui {
namespace python {

bool PyClient::connect(const UnicodeString& host, unsigned int port, const UnicodeString& account, const UnicodeString& password) {
    return Client::getSingleton()->connect(host, port, account, password);
}

UnicodeString PyClient::getConfig(const UnicodeString& key) {
    if (Client::getSingleton()->getConfig().exists(key)) {
        return Client::getSingleton()->getConfig()[key].asString();
    } else {
        LOG_WARN << "Requested unknown config value: " << key << std::endl;
        return "";
    }
}

void PyClient::shutdown() {
    Client::getSingleton()->shutdown();
}

void PyClient::messagebox(const UnicodeString& msg) {
    ui::GumpMenus::openMessageBox(msg);
}

}
}
}
