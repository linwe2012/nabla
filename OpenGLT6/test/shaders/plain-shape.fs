#version 330 core
in vec3 Color;

out vec4 FragColor;

float near = 0.1;
float far = 100.0;

float LinearizeDepth(float depth){
  float z = depth * 2.0 - 1.0;
  return (2.0 * near * far) / (far + near - z * (far - near));
}

void main()
{
  //float depth = LinearizeDepth(gl_FragCoord.z) / far;
  //FragColor = vec4(vec3(gl_FragCoord.z), 1.0f);
  FragColor = vec4(Color, 1.0f);

}