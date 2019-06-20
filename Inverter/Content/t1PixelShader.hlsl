

Texture2D           gridTexture         :       register(t0);
Texture2D           galloTexture        :       register(t1);

SamplerState        LinearSampler       :       register(s0);



struct PixelShaderInput
{
    float4      s_pos       : SV_POSITION;
    float3      s_color     : COLOR0;
    float2      s_texco     : TEXCOORD0;
    float2      s_model     : TEXCOORD1;
};


#define tau 6.2831853071795864769252867665590   //  a lot more digits of 2pi;



float gv_squared_norm(float2 a)
{
    return a.x * a.x + a.y * a.y;
}




float2 gv_div_complex(float2 a, float2 b)
{
    //   return the quotient  a over b

    //  TODO: handle divide by zero...

    float d = gv_squared_norm(b);

    return float2(
        (a.x * b.x + a.y * b.y) / d,
        (-a.x * b.y + a.y * b.x) / d
        );
}





float2 gv_mul_complex(float2 a, float2 b)
{
    return float2(
        a.x * b.x - a.y * b.y,
        a.x * b.y + a.y * b.x
        );
}



float gv_arg_complex(float2 a)
{
    return atan2(a.y, a.x);
}








float4 main(PixelShaderInput input) : SV_TARGET
{

    float4 finalColor = float4(0.0, 0.0, 1.0, 1.0); 

    //  apply complex rotation to the image depending on its quadrant


    if (gv_squared_norm(input.s_texco) > 0.01)
    {

        float2 zmult = gv_mul_complex(input.s_texco, float2(0.0, 1.0)); 

        float2 tuv = input.s_texco; 


        if ( (gv_arg_complex(tuv) > 0) && (gv_arg_complex(tuv) < tau / 4.0) )
        {
            zmult = gv_mul_complex(input.s_texco, float2(-1.0, 0.0)); 
        }
        else if ( (gv_arg_complex(tuv) > tau / 4.0) && (gv_arg_complex(tuv) < tau / 2.0) )
        {
            zmult = gv_mul_complex(input.s_texco, float2(-1.0, 0.0)); 
        }
        else if ((gv_arg_complex(tuv) < 0.0) && (gv_arg_complex(tuv) > -tau / 4.0))
        {
            zmult = gv_mul_complex(input.s_texco, float2(+1.0, 0.0)); 
        }
        else if ((gv_arg_complex(tuv) < -tau / 4.0) && (gv_arg_complex(tuv) >  -tau / 2.0))
        {
            zmult = gv_mul_complex(input.s_texco, float2(+1.0, 0.0));
        }

        float2 zinv = gv_div_complex(float2(1.0, 0.0), zmult);

        float4 gridColor = gridTexture.Sample(LinearSampler, zinv);

        float4 galloColor = galloTexture.Sample(LinearSampler, zinv);

        finalColor = lerp(gridColor, galloColor, 0.55);  //  originally 0.85 lerp; 
    }

    return finalColor;
}
