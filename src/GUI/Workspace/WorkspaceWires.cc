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

#include "WorkspaceWires.h"

namespace degate
{
    struct WiresVertex2D
    {
        QVector2D pos;
        QVector3D color;
        float alpha;
    };

    WorkspaceWires::WorkspaceWires(QWidget *parent) : WorkspaceElement(parent)
    {

    }

    WorkspaceWires::~WorkspaceWires()
    {

    }

    void WorkspaceWires::init()
    {
        WorkspaceElement::init();

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

    void WorkspaceWires::update()
    {
        if (project == nullptr)
            return;

        assert(context->glGetError() == GL_NO_ERROR);

        Layer_shptr layer = project->get_logic_model()->get_current_layer();

        if (layer == nullptr)
            return;

        // Keep only wires of the active layer.
        std::vector<Wire_shptr> wires;
        for (Layer::object_iterator iter = layer->objects_begin(); iter != layer->objects_end(); ++iter)
        {
            if (Wire_shptr a = std::dynamic_pointer_cast<Wire>(*iter))
            {
                wires.push_back(a);
            }
        }
        wires_count = static_cast<unsigned int>(wires.size());

        if (wires_count == 0)
            return;

        vao.bind();
        context->glBindBuffer(GL_ARRAY_BUFFER, vbo);

        context->glBufferData(GL_ARRAY_BUFFER, wires_count * 6 * sizeof(WiresVertex2D), nullptr, GL_STATIC_DRAW);

        context->glBindBuffer(GL_ARRAY_BUFFER, 0);
        vao.release();

        unsigned index = 0;
        for (auto& e : wires)
        {
            create_wire(e, index);
            e->set_index(index);
            index++;
        }

        assert(context->glGetError() == GL_NO_ERROR);
    }

    void WorkspaceWires::update(Wire_shptr &wire)
    {
        if (wire == nullptr)
            return;

        create_wire(wire, wire->get_index());
    }

    void WorkspaceWires::draw(const QMatrix4x4 &projection)
    {
        if (project == nullptr || wires_count == 0)
            return;

        program->bind();

        program->setUniformValue("mvp", projection);

        vao.bind();
        context->glBindBuffer(GL_ARRAY_BUFFER, vbo);

        program->enableAttributeArray("pos");
        program->setAttributeBuffer("pos", GL_FLOAT, 0, 2, sizeof(WiresVertex2D));

        program->enableAttributeArray("color");
        program->setAttributeBuffer("color", GL_FLOAT, 2 * sizeof(float), 3, sizeof(WiresVertex2D));

        program->enableAttributeArray("alpha");
        program->setAttributeBuffer("alpha", GL_FLOAT, 5 * sizeof(float), 1, sizeof(WiresVertex2D));

        context->glDrawArrays(GL_TRIANGLES, 0, wires_count * 6);

        context->glBindBuffer(GL_ARRAY_BUFFER, 0);
        vao.release();

        program->release();
    }

    void WorkspaceWires::create_wire(Wire_shptr &wire, unsigned int index)
    {
        if (wire == nullptr)
            return;

        vao.bind();
        context->glBindBuffer(GL_ARRAY_BUFFER, vbo);

        // Vertices and colors

        color_t color = wire->get_fill_color() == 0 ? project->get_default_color(DEFAULT_COLOR_EMARKER) : wire->get_fill_color();

        color = highlight_color_by_state(color, wire->get_highlighted());

        WiresVertex2D temp;
        temp.color = QVector3D(MASK_R(color) / 255.0, MASK_G(color) / 255.0, MASK_B(color) / 255.0);
        temp.alpha = MASK_A(color) / 255.0;

        float radius = static_cast<float>(wire->get_diameter()) / 2.0f;

        QVector2D difference_vector(wire->get_to_x() - wire->get_from_x(), wire->get_to_y() - wire->get_from_y());

        QVector2D parallel_vector = difference_vector;
        parallel_vector.normalize();
        float to_x = wire->get_to_x() + radius * parallel_vector.x(), from_x = wire->get_from_x() - radius * parallel_vector.x();
        float to_y = wire->get_to_y() + radius * parallel_vector.y(), from_y = wire->get_from_y() - radius * parallel_vector.y();
        QVector2D perpendicular_vector(difference_vector.y(), -difference_vector.x());
        perpendicular_vector.normalize();

        temp.pos = QVector2D(from_x + perpendicular_vector.x() * radius, from_y + perpendicular_vector.y() * radius);
        context->glBufferSubData(GL_ARRAY_BUFFER, index * 6 * sizeof(WiresVertex2D) + 0 * sizeof(WiresVertex2D), sizeof(WiresVertex2D), &temp);

        temp.pos = QVector2D(from_x - perpendicular_vector.x() * radius, from_y - perpendicular_vector.y() * radius);
        context->glBufferSubData(GL_ARRAY_BUFFER, index * 6 * sizeof(WiresVertex2D) + 1 * sizeof(WiresVertex2D), sizeof(WiresVertex2D), &temp);

        temp.pos = QVector2D(to_x + perpendicular_vector.x() * radius, to_y + perpendicular_vector.y() * radius);
        context->glBufferSubData(GL_ARRAY_BUFFER, index * 6 * sizeof(WiresVertex2D) + 2 * sizeof(WiresVertex2D), sizeof(WiresVertex2D), &temp);

        temp.pos = QVector2D(to_x + perpendicular_vector.x() * radius, to_y + perpendicular_vector.y() * radius);
        context->glBufferSubData(GL_ARRAY_BUFFER, index * 6 * sizeof(WiresVertex2D) + 4 * sizeof(WiresVertex2D), sizeof(WiresVertex2D), &temp);

        temp.pos = QVector2D(to_x - perpendicular_vector.x() * radius, to_y - perpendicular_vector.y() * radius);
        context->glBufferSubData(GL_ARRAY_BUFFER, index * 6 * sizeof(WiresVertex2D) + 3 * sizeof(WiresVertex2D), sizeof(WiresVertex2D), &temp);

        temp.pos = QVector2D(from_x - perpendicular_vector.x() * radius, from_y - perpendicular_vector.y() * radius);
        context->glBufferSubData(GL_ARRAY_BUFFER, index * 6 * sizeof(WiresVertex2D) + 5 * sizeof(WiresVertex2D), sizeof(WiresVertex2D), &temp);

        context->glBindBuffer(GL_ARRAY_BUFFER, 0);
        vao.release();
    }
}