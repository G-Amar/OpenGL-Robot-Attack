#version 140

//in vec2 aTexCoord;

out vec3 N;
out vec3 v;
//out vec2 TexCoord;


//this program is a slightly modified version of one of the examples posted by the professor

void main(){
    v = vec3(gl_ModelViewMatrix * gl_Vertex);       
    N = normalize(gl_NormalMatrix * gl_Normal);
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    gl_TexCoord[0] = gl_MultiTexCoord0;
}
