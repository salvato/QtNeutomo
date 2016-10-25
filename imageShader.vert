/*
 *
Copyright (C) 2016  Gabriele Salvato

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#version 450

/*
A vertex shader receives a single vertex from the vertex stream
and generates a single vertex to the output vertex stream.

Vertex Shaders have the following built-in input variables:
    in int gl_VertexID;
    in int gl_InstanceID;

Vertex Shaders have the following predefined outputs.

    out gl_PerVertex
    {
      vec4 gl_Position;
      float gl_PointSize;
      float gl_ClipDistance[];
    };
*/

in vec2 vertex_tex;
in vec3 vertex_pos;
uniform mat4 mvp_Matrix;

out vec2 v_texcoord;

void
main() {
    gl_Position = mvp_Matrix * vec4(vertex_pos, 1.0);
    v_texcoord = vertex_tex;
}
