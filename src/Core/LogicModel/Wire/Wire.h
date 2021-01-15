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

#ifndef __WIRE_H__
#define __WIRE_H__

#include <memory>

#include "Globals.h"
#include "Core/LogicModel/LogicModelObjectBase.h"
#include "Core/LogicModel/ConnectedLogicModelObject.h"
#include "Core/Primitive/Line.h"
#include "Core/Primitive/RemoteObject.h"

namespace degate
{
    /**
     * Represents a wire.
     */
    class Wire : public Line, public ConnectedLogicModelObject, public RemoteObject
    {
    Q_DECLARE_TR_FUNCTIONS(degate::Wire)

    public:

        /**
         * The constructor for a wire object.
         */
        Wire(float from_x, float from_y, float to_x, float to_y, diameter_t diameter);
        Wire(Line line);

        /**
         * Destructor for a wire object.
         */
        virtual ~Wire()
        {
        }

        //@{
        DeepCopyable_shptr clone_shallow() const;
        void clone_deep_into(DeepCopyable_shptr destination, oldnew_t* oldnew) const;
        //@}

        /**
         * Get a human readable string that describes the whole
         * logic model object. The string should be unique in order
         * to let the user identify the concrete object. But that
         * is not a must.
         */
        virtual const std::string get_descriptive_identifier() const;

        /**
         * Get a human readable string that names the object type.
         * Here it is "Wire".
         */
        virtual const std::string get_object_type_name() const;

        /**
         * Print wire.
         */
        void print(std::ostream& os = std::cout, int n_tabs = 0) const;


        void shift_x(float delta_x)
        {
            auto bb = get_bounding_box();
            Line::shift_x(delta_x);
            notify_shape_change(bb);
        }

        void shift_y(float delta_y)
        {
            auto bb = get_bounding_box();
            Line::shift_y(delta_y);
            notify_shape_change(bb);
        }

        virtual bool in_bounding_box(BoundingBox const& bbox) const
        {
            return in_bounding_box(bbox);
        }

        virtual BoundingBox const& get_bounding_box() const
        {
            return Line::get_bounding_box();
        }

        virtual bool in_shape(float x, float y, float max_distance = 0) const
        {
            return Line::in_shape(x, y, max_distance);
        }

    protected:

        virtual object_id_t push_object_to_server(std::string const& server_url);
    };
}

#endif
