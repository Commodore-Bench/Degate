/**
 * This file is part of the IC reverse engineering tool Degate.
 *
 * Copyright 2008, 2009, 2010 by Martin Schobert
 * Copyright 2012 Robert Nitsch
 * Copyright 2019-2020 Dorian Bachelot
 *
 * Degate is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * Degate is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with degate. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "Via.h"
//#include <XmlRpc.h>
#include <boost/format.hpp>

using namespace degate;

Via::Via(float x, float y, diameter_t diameter, Via::DIRECTION direction) :
    Circle(x, y, diameter),
    direction(direction)
{
}

Via::~Via()
{
}

DeepCopyable_shptr Via::clone_shallow() const
{
    auto clone = std::make_shared<Via>();
    clone->direction = direction;
    return clone;
}

void Via::clone_deep_into(DeepCopyable_shptr dest, oldnew_t* oldnew) const
{
    Circle::clone_deep_into(dest, oldnew);
    ConnectedLogicModelObject::clone_deep_into(dest, oldnew);
    RemoteObject::clone_deep_into(dest, oldnew);
}

Via::DIRECTION Via::get_direction() const
{
    return direction;
}

void Via::set_direction(Via::DIRECTION dir)
{
    direction = dir;
}

const std::string Via::get_direction_as_string() const
{
    switch (direction)
    {
    case DIRECTION_UP: return std::string("up");
    case DIRECTION_DOWN: return std::string("down");
    case DIRECTION_UNDEFINED:
    default: return std::string("undefined");
    }
}

Via::DIRECTION Via::get_via_direction_from_string(std::string const& via_direction_str)
{
    if (via_direction_str == "up") return Via::DIRECTION_UP;
    else if (via_direction_str == "down") return Via::DIRECTION_DOWN;
    else if (via_direction_str == "undefined") return Via::DIRECTION_UNDEFINED;
    else throw DegateRuntimeException("Can't parse via direction type.");
}

const std::string Via::get_descriptive_identifier() const
{
    if (has_name())
    {
        return QString("%1 (%2)").arg(QString::fromStdString(get_name())).arg(get_object_id()).toStdString();
    }
    else
    {
        return QString("(%1)").arg(get_object_id()).toStdString();
    }
}

const std::string Via::get_object_type_name() const
{
    return tr("Via").toStdString();
}


void Via::print(std::ostream& os, int n_tabs) const
{
    os
        << gen_tabs(n_tabs) << "Via name          : " << get_name() << std::endl
        << gen_tabs(n_tabs) << "Object ID         : " << get_object_id() << std::endl
        << gen_tabs(n_tabs) << "Via position      : " << get_x() << " / " << get_y() << std::endl
        << gen_tabs(n_tabs) << "Bounding box      : " << Circle::get_bounding_box().to_string() << std::endl
        << std::endl;;
}

//bool Via::in_shape(int x, int y) const {
//  return Circle::in_shape(x,y);
//}

void Via::shift_x(float delta_x)
{
    auto bb = get_bounding_box();
    Circle::shift_x(delta_x);
    notify_shape_change(bb);
}

void Via::shift_y(float delta_y)
{
    auto bb = get_bounding_box();
    Circle::shift_y(delta_y);
    notify_shape_change(bb);
}

void Via::set_x(float x)
{
    auto bb = get_bounding_box();
    Circle::set_x(x);
    notify_shape_change(bb);
}

void Via::set_y(float y)
{
    auto bb = get_bounding_box();
    Circle::set_y(y);
    notify_shape_change(bb);
}

void Via::set_diameter(unsigned int diameter)
{
    auto bb = get_bounding_box();
    Circle::set_diameter(diameter);
    notify_shape_change(bb);
}


object_id_t Via::push_object_to_server(std::string const& server_url)
{
    /*
    try
    {
        xmlrpc_c::paramList params;
        params.add(xmlrpc_c::value_string("add"));
        params.add(xmlrpc_c::value_string("via"));

        Layer_shptr layer = get_layer();
        assert(layer != nullptr);
        params.add(xmlrpc_c::value_int(layer->get_layer_id()));

        params.add(xmlrpc_c::value_int(get_x()));
        params.add(xmlrpc_c::value_int(get_y()));
        params.add(xmlrpc_c::value_int(get_diameter()));
        params.add(xmlrpc_c::value_string(get_direction_as_string()));

        int const transaction_id =
            xmlrpc_c::value_int(remote_method_call(server_url, "degate.push", params));

        set_remote_object_id(transaction_id);

        std::cout << "Pushed via to server. remote id is: " << transaction_id << std::endl;
        return transaction_id;
    }
    catch (std::exception const& e)
    {
        std::cerr << "Client threw error: " << e.what() << std::endl;
        throw XMLRPCException(e.what());
    }
    catch (...)
    {
        std::cerr << "Client threw unexpected error." << std::endl;
        throw XMLRPCException("Client threw unexpected error.");
    }
    */

    return 0;
}
