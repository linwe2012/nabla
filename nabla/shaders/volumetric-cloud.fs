// Fractional value for sample position in the cloud layer . 
float GetHeightFractionForPoint ( vec3 inPosition, vec2 inCloudMinMax) { 
    // Get global fractional position in cloud zone . 
    float height_fraction = (inPosition. z − inCloudMinMax.x) / 
                            (inCloudMinMax . y − inCloudMinMax . x );
    
    return saturate( height_fraction );
}

// Utility function that maps a value from one range to another . 
float Remap( float original_value , float original_min, 
             float original_max, float new_min , float new_max) 
{
     return new_min + (((original_value − original_min)/ 
        ( original_max − original_min)) ∗ ( new_max − new_min));
}