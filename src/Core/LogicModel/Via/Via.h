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

#ifndef __VIA_H__
#define __VIA_H__

#include "Globals.h"
#include "Core/LogicModel/LogicModelObjectBase.h"
#include "Core/LogicModel/Net.h"
#include "Core/LogicModel/Layer.h"
#include "Core/LogicModel/LogicModel.h"
#include "Core/LogicModel/ConnectedLogicModelObject.h"
#include "Core/Utils/DegateExceptions.h"
#include "Core/LogicModel/Wire/Wire.h"
#include "Core/Primitive/Circle.h"
#include "Core/Primitive/RemoteObject.h"

#include <memory>

namespace degate
{
	/**
	 * Representation of a via that interconnects layers of a chip.
	 */
	class Via : public Circle, public ConnectedLogicModelObject, public RemoteObject
	{
    Q_DECLARE_TR_FUNCTIONS(degate::Via)

	public:

		/**
		 * An enum to indicate, which layer is connected.
		 *
		 * Vias are placed on a layer. If the via connects an object
		 * from the current layer to a layer that is above, then the
		 * direction is up.
		 */
		enum DIRECTION
		{
			DIRECTION_UNDEFINED = 0,
			DIRECTION_UP = 1,
			DIRECTION_DOWN = 2
		};

	private:

		DIRECTION direction;

	public:

		explicit Via()
		{
		};

		/**
		 * Constructor for a via object.
		 */
		Via(float x, float y, diameter_t diameter, DIRECTION direction = DIRECTION_UNDEFINED);

		/**
		 * Destructor for a via object.
		 */
		virtual ~Via();

		//@{
		DeepCopyable_shptr clone_shallow() const;
		void clone_deep_into(DeepCopyable_shptr destination, oldnew_t* oldnew) const;
		//@}

		/**
		 * Get direction.
		 */
		virtual DIRECTION get_direction() const;

		/**
		 * Set direction.
		 */
		virtual void set_direction(DIRECTION dir);

		/**
		 * Get the direction as a human readable string.
		 */
		virtual const std::string get_direction_as_string() const;


		/**
		 * Parse a via direction string and return it as enum value.
		 */
		static Via::DIRECTION get_via_direction_from_string(std::string const& via_direction_str);

		/**
		 * Get a human readable string that describes the whole
		 * logic model object. The string should be unique in order
		 * to let the user identify the concrete object. But that
		 * is not a must.
		 */
		virtual const std::string get_descriptive_identifier() const;

		/**
		 * Get a human readable string that names the object type.
		 * Here it is "Via".
		 */
		virtual const std::string get_object_type_name() const;

		/**
		 * Print the object.
		 */
		void print(std::ostream& os, int n_tabs) const;


		void shift_x(float delta_x);
		void shift_y(float delta_y);
		void set_x(float x);
		void set_y(float y);
		void set_diameter(unsigned int diameter);

		virtual bool in_bounding_box(BoundingBox const& bbox) const
		{
			return Circle::in_bounding_box(bbox);
		}

		virtual BoundingBox const& get_bounding_box() const
		{
			return Circle::get_bounding_box();
		}

		virtual bool in_shape(float x, float y, float max_distance = 0) const
		{
			return Circle::in_shape(x, y, max_distance);
		}

	protected:

		virtual object_id_t push_object_to_server(std::string const& server_url);
	};
}

#endif
