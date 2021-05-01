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

#include "WorkspaceVias.h"

#define TEXT_PADDING 2

namespace degate
{
    struct ViasVertex2D
    {
        QVector2D pos;
        QVector3D color;
        float alpha;
    };

    WorkspaceVias::WorkspaceVias(QWidget *parent) : WorkspaceElement(parent), text(parent)
    {

    }

    WorkspaceVias::~WorkspaceVias()
    {

    }

    void WorkspaceVias::init()
    {
        WorkspaceElement::init();

        text.init();

        QOpenGLShader* vshader = new QOpenGLShader(QOpenGLShader::Vertex);
        const char* vsrc =
                "#version 330 core\n"
                "in vec2 pos;\n"
                "in vec3 color;\n"
                "in float alpha;\n"
                "uniform mat4 mvp;\n"
                "out vec4 out_color;\n"
                "void main(void)\n"
                "{\n"
                "    gl_Position = mvp * vec4(pos, 0.0, 1.0);\n"
                "    out_color = vec4(color, alpha);\n"
                "}\n";
        vshader->compileSourceCode(vsrc);

        QOpenGLShader* fshader = new QOpenGLShader(QOpenGLShader::Fragment);
        const char* fsrc =
                "#version 330 core\n"
                "in vec4 out_color;\n"
                "out vec4 color;\n"
                "void main(void)\n"
                "{\n"
                "    color = out_color;\n"
                "}\n";
        fshader->compileSourceCode(fsrc);

        program = new QOpenGLShaderProgram;
        program->addShader(vshader);
        program->addShader(fshader);

        program->link();

        delete vshader;
        delete fshader;
    }

    void WorkspaceVias::update()
    {
        if (project == nullptr)
            return;

        assert(context->glGetError() == GL_NO_ERROR);

        Layer_shptr layer = project->get_logic_model()->get_current_layer();

        if (layer == nullptr)
            return;

        // Keep only emarkers of the active layer.
        std::vector<Via_shptr> vias;
        for (Layer::object_iterator iter = layer->objects_begin(); iter != layer->objects_end(); ++iter)
        {
            if (Via_shptr a = std::dynamic_pointer_cast<Via>(*iter))
            {
                vias.push_back(a);
            }
        }
        vias_count = static_cast<unsigned int>(vias.size());

        if (vias_count == 0)
            return;

        vao.bind();
        context->glBindBuffer(GL_ARRAY_BUFFER, vbo);

        context->glBufferData(GL_ARRAY_BUFFER, vias_count * 24 * sizeof(ViasVertex2D), nullptr, GL_STATIC_DRAW);

        context->glBindBuffer(GL_ARRAY_BUFFER, 0);
        vao.release();

        unsigned text_size = 0;

        unsigned index = 0;
        for (auto& e : vias)
        {
            create_via(e, index);
            e->set_index(index);

            text_size += static_cast<unsigned int>(e->get_name().length());
            index++;
        }

        text.update(text_size);

        unsigned text_offset = 0;
        for (auto& e : vias)
        {
            unsigned x = e->get_x();
            unsigned y = e->get_y() + e->get_diameter() / 2.0 + TEXT_PADDING;
            text.add_sub_text(text_offset, x, y, e->get_name(), 5, QVector3D(255, 255, 255), 1, true, false);

            text_offset += static_cast<unsigned int>(e->get_name().length());
            index++;
        }

        assert(context->glGetError() == GL_NO_ERROR);
    }

    void WorkspaceVias::update(Via_shptr &via)
    {
        if (via == nullptr)
            return;

        create_via(via, via->get_index());
    }

    void WorkspaceVias::draw(const QMatrix4x4& projection)
    {
        if (project == nullptr || vias_count == 0)
            return;

        program->bind();

        program->setUniformValue("mvp", projection);

        vao.bind();
        context->glBindBuffer(GL_ARRAY_BUFFER, vbo);

        program->enableAttributeArray("pos");
        program->setAttributeBuffer("pos", GL_FLOAT, 0, 2, sizeof(ViasVertex2D));

        program->enableAttributeArray("color");
        program->setAttributeBuffer("color", GL_FLOAT, 2 * sizeof(float), 3, sizeof(ViasVertex2D));

        program->enableAttributeArray("alpha");
        program->setAttributeBuffer("alpha", GL_FLOAT, 5 * sizeof(float), 1, sizeof(ViasVertex2D));

        context->glDrawArrays(GL_TRIANGLES, 0, vias_count * 24);

        context->glBindBuffer(GL_ARRAY_BUFFER, 0);
        vao.release();

        program->release();
    }

    void WorkspaceVias::draw_name(const QMatrix4x4 &projection)
    {
        if (project == nullptr || vias_count == 0)
            return;

        text.draw(projection);
    }

    void WorkspaceVias::create_via(Via_shptr &via, unsigned int index)
    {
        if (via == nullptr)
            return;

        vao.bind();
        context->glBindBuffer(GL_ARRAY_BUFFER, vbo);

        const float hole_radius = via->get_diameter() / 4.0;

        // Vertices and colors

        color_t color;

        if (via->get_direction() == Via::DIRECTION_UP)
            color = via->get_fill_color() == 0 ? project->get_default_color(DEFAULT_COLOR_VIA_UP) : via->get_fill_color();
        else if (via->get_direction() == Via::DIRECTION_DOWN)
            color = via->get_fill_color() == 0 ? project->get_default_color(DEFAULT_COLOR_VIA_DOWN) : via->get_fill_color();
        else
            color = via->get_fill_color() == 0 ? project->get_default_color(DEFAULT_COLOR_EMARKER) : via->get_fill_color();

        color = highlight_color_by_state(color, via->get_highlighted());

        ViasVertex2D temp;
        temp.color = QVector3D(MASK_R(color) / 255.0, MASK_G(color) / 255.0, MASK_B(color) / 255.0);
        temp.alpha = MASK_A(color) / 255.0;


        // Rect 1

        temp.pos = QVector2D(via->get_x() - via->get_diameter() / 2.0, via->get_y() - via->get_diameter() / 2.0);
        context->glBufferSubData(GL_ARRAY_BUFFER, index * 24 * sizeof(ViasVertex2D) + 0 * sizeof(ViasVertex2D), sizeof(ViasVertex2D), &temp);

        temp.pos = QVector2D(via->get_x() - hole_radius, via->get_y() - via->get_diameter() / 2.0);
        context->glBufferSubData(GL_ARRAY_BUFFER, index * 24 * sizeof(ViasVertex2D) + 1 * sizeof(ViasVertex2D), sizeof(ViasVertex2D), &temp);

        temp.pos = QVector2D(via->get_x() - via->get_diameter() / 2.0, via->get_y() + via->get_diameter() / 2.0);
        context->glBufferSubData(GL_ARRAY_BUFFER, index * 24 * sizeof(ViasVertex2D) + 2 * sizeof(ViasVertex2D), sizeof(ViasVertex2D), &temp);

        temp.pos = QVector2D(via->get_x() - hole_radius, via->get_y() + via->get_diameter() / 2.0);
        context->glBufferSubData(GL_ARRAY_BUFFER, index * 24 * sizeof(ViasVertex2D) + 3 * sizeof(ViasVertex2D), sizeof(ViasVertex2D), &temp);

        temp.pos = QVector2D(via->get_x() - via->get_diameter() / 2.0, via->get_y() + via->get_diameter() / 2.0);
        context->glBufferSubData(GL_ARRAY_BUFFER, index * 24 * sizeof(ViasVertex2D) + 4 * sizeof(ViasVertex2D), sizeof(ViasVertex2D), &temp);

        temp.pos = QVector2D(via->get_x() - hole_radius, via->get_y() - via->get_diameter() / 2.0);
        context->glBufferSubData(GL_ARRAY_BUFFER, index * 24 * sizeof(ViasVertex2D) + 5 * sizeof(ViasVertex2D), sizeof(ViasVertex2D), &temp);


        // Rect 2

        temp.pos = QVector2D(via->get_x() - hole_radius, via->get_y() - via->get_diameter() / 2.0);
        context->glBufferSubData(GL_ARRAY_BUFFER, index * 24 * sizeof(ViasVertex2D) + 6 * sizeof(ViasVertex2D), sizeof(ViasVertex2D), &temp);

        temp.pos = QVector2D(via->get_x() + hole_radius, via->get_y() - via->get_diameter() / 2.0);
        context->glBufferSubData(GL_ARRAY_BUFFER, index * 24 * sizeof(ViasVertex2D) + 7 * sizeof(ViasVertex2D), sizeof(ViasVertex2D), &temp);

        temp.pos = QVector2D(via->get_x() - hole_radius, via->get_y() - hole_radius);
        context->glBufferSubData(GL_ARRAY_BUFFER, index * 24 * sizeof(ViasVertex2D) + 8 * sizeof(ViasVertex2D), sizeof(ViasVertex2D), &temp);

        temp.pos = QVector2D(via->get_x() - hole_radius, via->get_y() - hole_radius);
        context->glBufferSubData(GL_ARRAY_BUFFER, index * 24 * sizeof(ViasVertex2D) + 9 * sizeof(ViasVertex2D), sizeof(ViasVertex2D), &temp);

        temp.pos = QVector2D(via->get_x() + hole_radius, via->get_y() - via->get_diameter() / 2.0);
        context->glBufferSubData(GL_ARRAY_BUFFER, index * 24 * sizeof(ViasVertex2D) + 10 * sizeof(ViasVertex2D), sizeof(ViasVertex2D), &temp);

        temp.pos = QVector2D(via->get_x() + hole_radius, via->get_y() - hole_radius);
        context->glBufferSubData(GL_ARRAY_BUFFER, index * 24 * sizeof(ViasVertex2D) + 11 * sizeof(ViasVertex2D), sizeof(ViasVertex2D), &temp);



        // Rect 3

        temp.pos = QVector2D(via->get_x() + hole_radius, via->get_y() - via->get_diameter() / 2.0);
        context->glBufferSubData(GL_ARRAY_BUFFER, index * 24 * sizeof(ViasVertex2D) + 12 * sizeof(ViasVertex2D), sizeof(ViasVertex2D), &temp);

        temp.pos = QVector2D(via->get_x() + via->get_diameter() / 2.0, via->get_y() - via->get_diameter() / 2.0);
        context->glBufferSubData(GL_ARRAY_BUFFER, index * 24 * sizeof(ViasVertex2D) + 13 * sizeof(ViasVertex2D), sizeof(ViasVertex2D), &temp);

        temp.pos = QVector2D(via->get_x() + hole_radius, via->get_y() + via->get_diameter() / 2.0);
        context->glBufferSubData(GL_ARRAY_BUFFER, index * 24 * sizeof(ViasVertex2D) + 14 * sizeof(ViasVertex2D), sizeof(ViasVertex2D), &temp);

        temp.pos = QVector2D(via->get_x() + via->get_diameter() / 2.0, via->get_y() - via->get_diameter() / 2.0);
        context->glBufferSubData(GL_ARRAY_BUFFER, index * 24 * sizeof(ViasVertex2D) + 15 * sizeof(ViasVertex2D), sizeof(ViasVertex2D), &temp);

        temp.pos = QVector2D(via->get_x() + via->get_diameter() / 2.0, via->get_y() + via->get_diameter() / 2.0);
        context->glBufferSubData(GL_ARRAY_BUFFER, index * 24 * sizeof(ViasVertex2D) + 16 * sizeof(ViasVertex2D), sizeof(ViasVertex2D), &temp);

        temp.pos = QVector2D(via->get_x() + hole_radius, via->get_y() + via->get_diameter() / 2.0);
        context->glBufferSubData(GL_ARRAY_BUFFER, index * 24 * sizeof(ViasVertex2D) + 17 * sizeof(ViasVertex2D), sizeof(ViasVertex2D), &temp);



        // Rect 4

        temp.pos = QVector2D(via->get_x() - hole_radius, via->get_y() + hole_radius);
        context->glBufferSubData(GL_ARRAY_BUFFER, index * 24 * sizeof(ViasVertex2D) + 18 * sizeof(ViasVertex2D), sizeof(ViasVertex2D), &temp);

        temp.pos = QVector2D(via->get_x() - hole_radius, via->get_y() + via->get_diameter() / 2.0);
        context->glBufferSubData(GL_ARRAY_BUFFER, index * 24 * sizeof(ViasVertex2D) + 19 * sizeof(ViasVertex2D), sizeof(ViasVertex2D), &temp);

        temp.pos = QVector2D(via->get_x() + hole_radius, via->get_y() + hole_radius);
        context->glBufferSubData(GL_ARRAY_BUFFER, index * 24 * sizeof(ViasVertex2D) + 20 * sizeof(ViasVertex2D), sizeof(ViasVertex2D), &temp);

        temp.pos = QVector2D(via->get_x() + hole_radius, via->get_y() + hole_radius);
        context->glBufferSubData(GL_ARRAY_BUFFER, index * 24 * sizeof(ViasVertex2D) + 21 * sizeof(ViasVertex2D), sizeof(ViasVertex2D), &temp);

        temp.pos = QVector2D(via->get_x() + hole_radius, via->get_y() + via->get_diameter() / 2.0);
        context->glBufferSubData(GL_ARRAY_BUFFER, index * 24 * sizeof(ViasVertex2D) + 22 * sizeof(ViasVertex2D), sizeof(ViasVertex2D), &temp);

        temp.pos = QVector2D(via->get_x() - hole_radius, via->get_y() + via->get_diameter() / 2.0);
        context->glBufferSubData(GL_ARRAY_BUFFER, index * 24 * sizeof(ViasVertex2D) + 23 * sizeof(ViasVertex2D), sizeof(ViasVertex2D), &temp);

        context->glBindBuffer(GL_ARRAY_BUFFER, 0);
        vao.release();
    }
}