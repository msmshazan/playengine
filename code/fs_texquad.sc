$input v_texcoord0

#include "common.sh"

SAMPLER2D(s_texture,  0);

uniform vec4 tile_size;
uniform vec4 tex_sizeinv;

void main()
{
        
        v_texcoord0 = v_texcoord0 * tex_sizeinv.xy;
        gl_FragColor =  texture2D( s_texture, v_texcoord0);
}
