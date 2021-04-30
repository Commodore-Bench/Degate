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

#include "PortPlacementWidget.h"

#include <utility>

#define TEXT_PADDING 2

namespace degate
{
    struct PortVertex2D
    {
        QVector2D pos;
        QVector3D color;
        float alpha;
    };

    PortPlacementWidget::PortPlacementWidget(QWidget* parent,
                                             Project_shptr project,
                                             const GateTemplate_shptr& gate,
                                             const GateTemplatePort_shptr& port)
            : ImageRenderer(parent, gate->get_image(Layer::LAYER_TYPE::LOGIC), false),
              gate(gate),
              project(std::move(project)),
              port(port),
              port_name_text(parent)
    {
        layer = Layer::LAYER_TYPE::LOGIC;

        pos = port->get_point();
    }

    PortPlacementWidget::~PortPlacementWidget()
    {
        makeCurrent();

        if (program != nullptr)
            delete program;

        Text::delete_context();

        if (QOpenGLContext::currentContext() == nullptr)
            return;

        if (glIsBuffer(vbo) == GL_TRUE)
            glDeleteBuffers(1, &vbo);
    }

    void PortPlacementWidget::update_screen()
    {
        makeCurrent();

        vao.bind();
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(PortVertex2D), nullptr, GL_STATIC_DRAW);

        create_port(port);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        vao.release();

        port_name_text.update(static_cast<unsigned int>(port->get_name().length()));
        port_name_text.add_sub_text(0, pos.get_x(), pos.get_y() + DEFAULT_PORT_SIZE / 2.0 + TEXT_PADDING, port->get_name(), 5, QVector3D(255, 255, 255), 1, true, false);

        ImageRenderer::update_screen();
    }

    Point PortPlacementWidget::get_new_port_position()
    {
        return pos;
    }

    void PortPlacementWidget::next_layer()
    {
        if (gate->has_image(static_cast<Layer::LAYER_TYPE>(layer + 1)))
        {
            layer++;
            change_image(gate->get_image(static_cast<Layer::LAYER_TYPE>(layer)));
        }
    }

    void PortPlacementWidget::previous_layer()
    {
        if (gate->has_image(static_cast<Layer::LAYER_TYPE>(layer - 1)))
        {
            layer--;
            change_image(gate->get_image(static_cast<Layer::LAYER_TYPE>(layer)));
        }
    }

    void PortPlacementWidget::initializeGL()
    {
        makeCurrent();

        ImageRenderer::initializeGL();

        Text::init_context();

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

        delete vshader;
        delete fshader;

        program->link();

        glGenBuffers(1, &vbo);

        port_name_text.init();

        update_screen();
    }

    void PortPlacementWidget::paintGL()
    {
        makeCurrent();

        ImageRenderer::paintGL();

        program->bind();

        program->setUniformValue("mvp", projection);

        vao.bind();
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        program->enableAttributeArray("pos");
        program->setAttributeBuffer("pos", GL_FLOAT, 0, 2, sizeof(PortVertex2D));

        program->enableAttributeArray("color");
        program->setAttributeBuffer("color", GL_FLOAT, 2 * sizeof(float), 3, sizeof(PortVertex2D));

        program->enableAttributeArray("alpha");
        program->setAttributeBuffer("alpha", GL_FLOAT, 5 * sizeof(float), 1, sizeof(PortVertex2D));

        glDrawArrays(GL_TRIANGLES, 0, 9);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        vao.release();

        program->release();

        port_name_text.draw(projection);
    }

    void PortPlacementWidget::resizeGL(int w, int h)
    {
        makeCurrent();

        ImageRenderer::resizeGL(w, h);
    }

    void PortPlacementWidget::mousePressEvent(QMouseEvent* event)
    {
        makeCurrent();

        ImageRenderer::mousePressEvent(event);

        // Displacement
        if (event->button() == Qt::RightButton
            && get_opengl_mouse_position().x() >= 0
            && get_opengl_mouse_position().y() >= 0
            && get_opengl_mouse_position().x() <= gate->get_width()
            && get_opengl_mouse_position().y() <= gate->get_height())
        {
            pos = Point(get_opengl_mouse_position().x(), get_opengl_mouse_position().y());

            update_screen();
        }
    }

    void PortPlacementWidget::mouseReleaseEvent(QMouseEvent* event)
    {
        makeCurrent();

        ImageRenderer::mouseReleaseEvent(event);
    }

    void PortPlacementWidget::mouseMoveEvent(QMouseEvent* event)
    {
        makeCurrent();

        ImageRenderer::mouseMoveEvent(event);

        // Displacement
        if (event->buttons() & Qt::RightButton
            && get_opengl_mouse_position().x() >= 0
            && get_opengl_mouse_position().y() >= 0
            && get_opengl_mouse_position().x() <= gate->get_width()
            && get_opengl_mouse_position().y() <= gate->get_height())
        {
            pos = Point(get_opengl_mouse_position().x(), get_opengl_mouse_position().y());

            update_screen();
        }
    }

    void PortPlacementWidget::wheelEvent(QWheelEvent* event)
    {
        makeCurrent();

        ImageRenderer::wheelEvent(event);
    }

    void PortPlacementWidget::create_port_in_out(float x, float y, unsigned size, QVector3D color, float alpha)
    {
        PortVertex2D temp;

        temp.color = color;
        temp.alpha = alpha;

        int mid = size / 2.0;

        temp.pos = QVector2D(x - mid, y - mid);
        glBufferSubData(GL_ARRAY_BUFFER, 0 * sizeof(PortVertex2D), sizeof(PortVertex2D), &temp);

        temp.pos = QVector2D(x - mid, y + mid);
        glBufferSubData(GL_ARRAY_BUFFER, 1 * sizeof(PortVertex2D), sizeof(PortVertex2D), &temp);

        temp.pos = QVector2D(x + mid, y - mid);
        glBufferSubData(GL_ARRAY_BUFFER, 2 * sizeof(PortVertex2D), sizeof(PortVertex2D), &temp);

        temp.pos = QVector2D(x + mid, y - mid);
        glBufferSubData(GL_ARRAY_BUFFER, 3 * sizeof(PortVertex2D), sizeof(PortVertex2D), &temp);

        temp.pos = QVector2D(x, y);
        glBufferSubData(GL_ARRAY_BUFFER, 4 * sizeof(PortVertex2D), sizeof(PortVertex2D), &temp);

        temp.pos = QVector2D(x + mid, y + mid);
        glBufferSubData(GL_ARRAY_BUFFER, 5 * sizeof(PortVertex2D), sizeof(PortVertex2D), &temp);

        temp.pos = QVector2D(x, y);
        glBufferSubData(GL_ARRAY_BUFFER, 6 * sizeof(PortVertex2D), sizeof(PortVertex2D), &temp);

        temp.pos = QVector2D(x + mid, y + mid);
        glBufferSubData(GL_ARRAY_BUFFER, 7 * sizeof(PortVertex2D), sizeof(PortVertex2D), &temp);

        temp.pos = QVector2D(x - mid, y + mid);
        glBufferSubData(GL_ARRAY_BUFFER, 8 * sizeof(PortVertex2D), sizeof(PortVertex2D), &temp);
    }

    void PortPlacementWidget::create_port_in(float x, float y, unsigned size, QVector3D color, float alpha)
    {
        PortVertex2D temp;

        temp.color = color;
        temp.alpha = alpha;

        int mid = size / 2.0;

        temp.pos = QVector2D(x - mid, y - mid);
        glBufferSubData(GL_ARRAY_BUFFER, 0 * sizeof(PortVertex2D), sizeof(PortVertex2D), &temp);

        temp.pos = QVector2D(x + mid, y - mid);
        glBufferSubData(GL_ARRAY_BUFFER, 1 * sizeof(PortVertex2D), sizeof(PortVertex2D), &temp);

        temp.pos = QVector2D(x, y);
        glBufferSubData(GL_ARRAY_BUFFER, 2 * sizeof(PortVertex2D), sizeof(PortVertex2D), &temp);

        temp.pos = QVector2D(x + mid, y - mid);
        glBufferSubData(GL_ARRAY_BUFFER, 3 * sizeof(PortVertex2D), sizeof(PortVertex2D), &temp);

        temp.pos = QVector2D(x + mid, y + mid);
        glBufferSubData(GL_ARRAY_BUFFER, 4 * sizeof(PortVertex2D), sizeof(PortVertex2D), &temp);

        temp.pos = QVector2D(x, y);
        glBufferSubData(GL_ARRAY_BUFFER, 5 * sizeof(PortVertex2D), sizeof(PortVertex2D), &temp);

        temp.pos = QVector2D(x + mid, y + mid);
        glBufferSubData(GL_ARRAY_BUFFER, 6 * sizeof(PortVertex2D), sizeof(PortVertex2D), &temp);

        temp.pos = QVector2D(x - mid, y + mid);
        glBufferSubData(GL_ARRAY_BUFFER, 7 * sizeof(PortVertex2D), sizeof(PortVertex2D), &temp);

        temp.pos = QVector2D(x, y);
        glBufferSubData(GL_ARRAY_BUFFER, 8 * sizeof(PortVertex2D), sizeof(PortVertex2D), &temp);
    }

    void PortPlacementWidget::create_port_out(float x, float y, unsigned size, QVector3D color, float alpha)
    {
        PortVertex2D temp;

        temp.color = color;
        temp.alpha = alpha;

        int mid = size / 2.0;

        temp.pos = QVector2D(x - mid, y - mid);
        glBufferSubData(GL_ARRAY_BUFFER, 0 * sizeof(PortVertex2D), sizeof(PortVertex2D), &temp);

        temp.pos = QVector2D(x, y - mid);
        glBufferSubData(GL_ARRAY_BUFFER, 1 * sizeof(PortVertex2D), sizeof(PortVertex2D), &temp);

        temp.pos = QVector2D(x, y + mid);
        glBufferSubData(GL_ARRAY_BUFFER, 2 * sizeof(PortVertex2D), sizeof(PortVertex2D), &temp);

        temp.pos = QVector2D(x - mid, y - mid);
        glBufferSubData(GL_ARRAY_BUFFER, 3 * sizeof(PortVertex2D), sizeof(PortVertex2D), &temp);

        temp.pos = QVector2D(x - mid, y + mid);
        glBufferSubData(GL_ARRAY_BUFFER, 4 * sizeof(PortVertex2D), sizeof(PortVertex2D), &temp);

        temp.pos = QVector2D(x, y + mid);
        glBufferSubData(GL_ARRAY_BUFFER, 5 * sizeof(PortVertex2D), sizeof(PortVertex2D), &temp);

        temp.pos = QVector2D(x, y - mid);
        glBufferSubData(GL_ARRAY_BUFFER, 6 * sizeof(PortVertex2D), sizeof(PortVertex2D), &temp);

        temp.pos = QVector2D(x + mid, y);
        glBufferSubData(GL_ARRAY_BUFFER, 7 * sizeof(PortVertex2D), sizeof(PortVertex2D), &temp);

        temp.pos = QVector2D(x, y + mid);
        glBufferSubData(GL_ARRAY_BUFFER, 8 * sizeof(PortVertex2D), sizeof(PortVertex2D), &temp);
    }

    void PortPlacementWidget::create_port(GateTemplatePort_shptr port)
    {
        color_t color = port->get_fill_color() == 0 ? project->get_default_color(DEFAULT_COLOR_GATE_PORT) : port->get_fill_color();

        switch (port->get_port_type())
        {
            case GateTemplatePort::PORT_TYPE_UNDEFINED:
                create_port_in_out(pos.get_x(), pos.get_y(), DEFAULT_PORT_SIZE, QVector3D(MASK_R(color) / 255.0, MASK_G(color) / 255.0, MASK_B(color) / 255.0), MASK_A(color) / 255.0);
                break;
            case GateTemplatePort::PORT_TYPE_IN:
                create_port_in(pos.get_x(), pos.get_y(), DEFAULT_PORT_SIZE, QVector3D(MASK_R(color) / 255.0, MASK_G(color) / 255.0, MASK_B(color) / 255.0), MASK_A(color) / 255.0);
                break;
            case GateTemplatePort::PORT_TYPE_OUT:
                create_port_out(pos.get_x(), pos.get_y(), DEFAULT_PORT_SIZE, QVector3D(MASK_R(color) / 255.0, MASK_G(color) / 255.0, MASK_B(color) / 255.0), MASK_A(color) / 255.0);
                break;
            case GateTemplatePort::PORT_TYPE_INOUT:
                create_port_in_out(pos.get_x(), pos.get_y(), DEFAULT_PORT_SIZE, QVector3D(MASK_R(color) / 255.0, MASK_G(color) / 255.0, MASK_B(color) / 255.0), MASK_A(color) / 255.0);
                break;
            default:
                create_port_in_out(pos.get_x(), pos.get_y(), DEFAULT_PORT_SIZE, QVector3D(MASK_R(color) / 255.0, MASK_G(color) / 255.0, MASK_B(color) / 255.0), MASK_A(color) / 255.0);
                break;
        }
    }
}
