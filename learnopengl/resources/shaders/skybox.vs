#version 300 es
precision mediump float;

in vec3 vertexPosition;

uniform mat4 matProjection;
uniform mat4 matView;

out vec3 fragTexCoord;

void main()
{
    fragTexCoord = vertexPosition;
    vec4 pos = matProjection * matView * vec4(vertexPosition, 1.0);
    gl_Position = pos.xyww;
}
