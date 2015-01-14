uniform sampler2D texture0;

in vec2 texCoord;

out vec4 frag_color;

void main(void)
{
    frag_color = texture2D( texture0, texCoord );
}