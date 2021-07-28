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


#include "Globals.h"
#include "LogicModelObjectBase.h"
#include "Layer.h"
#include "PlacedLogicModelObject.h"

using namespace degate;

LogicModelObjectBase::LogicModelObjectBase(object_id_t oid) :
    object_id(oid)
{
}

LogicModelObjectBase::LogicModelObjectBase(std::string const& object_name,
                                           std::string const& object_description) :
    object_id(0),
    name(object_name),
    description(object_description)
{
}

LogicModelObjectBase::LogicModelObjectBase(object_id_t oid,
                                           std::string const& object_name,
                                           std::string const& object_description) :
    object_id(oid),
    name(object_name),
    description(object_description)
{
}

LogicModelObjectBase::~LogicModelObjectBase()
{
}

void LogicModelObjectBase::clone_deep_into(DeepCopyable_shptr dest, oldnew_t* oldnew) const
{
    auto clone = std::dynamic_pointer_cast<LogicModelObjectBase>(dest);
    clone->object_id = object_id;
    clone->name = name;
    clone->description = description;
}

void LogicModelObjectBase::set_name(std::string const& name)
{
    this->name = name;
}

void LogicModelObjectBase::set_description(std::string const& description)
{
    this->description = description;
}

std::string const& LogicModelObjectBase::get_name() const
{
    return name;
}

std::string const& LogicModelObjectBase::get_description() const
{
    return description;
}

bool LogicModelObjectBase::has_name() const
{
    return !name.empty();
}

bool LogicModelObjectBase::has_description() const
{
    return !description.empty();
}


void LogicModelObjectBase::set_object_id(object_id_t oid)
{
    object_id = oid;
}

object_id_t LogicModelObjectBase::get_object_id() const
{
    return object_id;
}

bool LogicModelObjectBase::has_valid_object_id() const
{
    return object_id != 0;
}


const std::string LogicModelObjectBase::get_descriptive_identifier() const
{
    // should be dispatched. In debug mode we break here.
    assert(1 == 0);

    return QString("%1 %2").arg(tr("Generic object")).arg(get_object_id()).toStdString();
}

const std::string LogicModelObjectBase::get_object_type_name() const
{
    return tr("Generic object").toStdString();
}
