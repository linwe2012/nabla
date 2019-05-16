#version 330 core
out vec4 FragColor;

// in vec2 TexCoords;

// uniform sampler2D texture_diffuse1;
uniform vec4 frag_color;
void main()
{    
   // FragColor = texture(texture_diffuse1, TexCoords);
  FragColor = frag_color;
  // FragColor = frag_color * clamp(gl_FragCoord.z, 0, 1); //= vec4(1.0f, 0.2f, 0.4f, 1.0f);
}