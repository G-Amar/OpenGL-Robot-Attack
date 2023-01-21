#version 140
 
in vec3 N;
in vec3 v;
//in vec2 TexCoord;

uniform sampler2D ourTexture;

//this program is a slightly modified version of one of the examples posted by the professor

void main() {
    vec3 L0 = normalize(gl_LightSource[0].position.xyz - v); 
    vec3 L1 = normalize(gl_LightSource[1].position.xyz - v);   
    vec3 V = normalize(-v); // we are in Eye Coordinates, so EyePos is (0,0,0) (0 - v)
    vec3 R0 = normalize(-reflect(L0,N));  
    vec3 R1 = normalize(-reflect(L1,N));
    vec3 newN = N;

    if(dot(N, v) >0 ){
        newN = -N;
    }
    
    //calculate Ambient Term:  
    vec4 Iamb = gl_FrontLightProduct[0].ambient;    

    //calculate Diffuse Term:  
    vec4 Idiff = gl_FrontLightProduct[0].diffuse * (max(dot(newN,L0), 0.0) + max(dot(newN,L1), 0.0));  
    
    // calculate Specular Term:
    vec4 Ispec = gl_FrontLightProduct[0].specular * (pow(max(dot(R0,V),0.0),0.6*gl_FrontMaterial.shininess) + pow(max(dot(R1,V),0.0),0.6*gl_FrontMaterial.shininess));

    vec4 texColor = texture2D(ourTexture, gl_TexCoord[0].st);

    // write Total Color:  
    //gl_FragColor = texColor;
    
    //gl_FragColor = mix(texture2D(ourTexture, gl_TexCoord[0].st),(gl_FrontLightModelProduct.sceneColor + Iamb + Idiff + Ispec));
    //gl_FragColor = 0.5*texture2D(ourTexture, gl_TexCoord[0].st) + 0.5*(gl_FrontLightModelProduct.sceneColor + Iamb + Idiff + Ispec);
    gl_FragColor = texture2D(ourTexture, gl_TexCoord[0].st)*(gl_FrontLightModelProduct.sceneColor + Iamb + Idiff + Ispec);
    //gl_FragColor = gl_FrontLightModelProduct.sceneColor + Iamb + Idiff + Ispec;   
     
     //gl_FragColor = gl_FrontLightModelProduct.sceneColor + Iamb + (Idiff * vec4(texColor.rgb,1.0)) +(Ispec * texColor.a); 

}
