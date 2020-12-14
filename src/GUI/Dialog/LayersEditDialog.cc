/**
 * This file is part of the IC reverse engineering tool Degate.
 *
 * Copyright 2008, 2009, 2010 by Martin Schobert
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

#include "LayersEditDialog.h"

namespace degate
{
    LayersEditDialog::LayersEditDialog(QWidget* parent, const Project_shptr& project)
            : QDialog(parent), project(project), layers(parent, project)
    {
        setWindowTitle(tr("Edit layers"));
        resize(500, 400);

        validate_button.setText(tr("Ok"));
        cancel_button.setText(tr("Cancel"));

        buttons_layout.addStretch(1);
        buttons_layout.addWidget(&validate_button);
        buttons_layout.addWidget(&cancel_button);

        layout.addWidget(&layers);
        layout.addLayout(&buttons_layout);

        setLayout(&layout);

        QObject::connect(&validate_button, SIGNAL(clicked()), this, SLOT(validate()));
        QObject::connect(&cancel_button, SIGNAL(clicked()), this, SLOT(reject()));
    }

    bool LayersEditDialog::project_need_reopen() const
    {
        return need_reopen;
    }

    void LayersEditDialog::validate()
    {
        layers.validate();

        // Size changed.
        auto size = layers.get_max_size();
        if (project->update_size(size.width(), size.height()))
        {
            need_reopen = true;
        }

        accept();
    }
}
